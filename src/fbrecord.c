#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
#include <jpeglib.h>

static void genjpeg (unsigned char *buf, int width, int height)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];
  FILE *out;
  int row_stride;

  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_compress (&cinfo);

  if (!(out = fopen ("fb.jpeg", "wb"))) {
    fprintf (stderr, "can't open fb.jpeg\n");
    exit (EXIT_FAILURE);
  }
  jpeg_stdio_dest (&cinfo, out);
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults (&cinfo);
  jpeg_start_compress (&cinfo, TRUE);

  row_stride = width * 2;

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &buf[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines (&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress (&cinfo);
  jpeg_destroy_compress (&cinfo);
  fclose (out);
}

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

  log = fopen ("log", "wb");
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
  genjpeg(buf, var.xres, var.yres);

  munmap (buf, size);
  close (fd);
  exit (EXIT_SUCCESS);

 logfail:
  fclose (log);
 fail:
  close (fd);
  exit (EXIT_FAILURE);
}
