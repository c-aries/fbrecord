#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/fb.h>

int main (int argc, char *argv[])
{
  int fd, size;
  struct fb_var_screeninfo var;
  struct fb_fix_screeninfo fix;
  unsigned char *buf;
  FILE *log;

  fd = open ("/dev/fb0", O_RDWR);
  if (fd < 0) {
    fprintf (stderr, "fb0 open error\n");
    exit (EXIT_FAILURE);
  }

  log = fopen ("log", "w");
  if (!log) {
    fprintf (stderr, "log open error\n");
    goto fail;
  }

  if (ioctl (fd, FBIOGET_VSCREENINFO, &var)) {
    fprintf (stderr, "ioctl FBIOGET_VSCREENINFO error\n");
    goto logfail;
  }
  if (var.bits_per_pixel != 16) {
    fprintf (stderr, "Error: bpp != 16\n");
    goto logfail;
  }

  if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)) {
    fprintf (stderr, "ioctl FBIOGET_FSCREENINFO error\n");
    goto logfail;
  }
  size = fix.line_length * var.yres;
  buf = mmap (NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (!buf) {
    fprintf (stderr, "map fail\n");
    goto logfail;
  }

  fwrite (buf, size, 1, log);

  munmap (buf, size);
  close (fd);
  exit (EXIT_SUCCESS);

 logfail:
  fclose (log);
 fail:
  close (fd);
  exit (EXIT_FAILURE);
}
