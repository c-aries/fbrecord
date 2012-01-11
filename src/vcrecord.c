#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <asm-generic/ioctls.h>
#include <asm-generic/termios.h>
#include <linux/tty.h>

static void
usage ()
{
  printf ("Usage: vcrecord ttynum\n");
}

int main (int argc, char *argv[])
{
  int vc_fd, tty_fd, size;
  int currcons = 0;		/* current tty */
  int rows, cols, i, j;
  char vcname[10], ttyname[10];
  unsigned char *buf;
  struct winsize wins;

  if (argc != 2) {
    usage ();
    exit (EXIT_FAILURE);
  }

  currcons = strtoul (argv[1], NULL, 10);
  if (currcons < 0 || currcons > 7) {
    fprintf (stderr, "tty number must be < 0 or > 7\n");
    exit (EXIT_FAILURE);
  }
  snprintf (vcname, 10, "/dev/vcs%d", currcons);
  snprintf (ttyname, 10, "/dev/tty%d", currcons);

  vc_fd = open (vcname, O_RDONLY);
  if (vc_fd < 0) {
    fprintf (stderr, "open %s fail\n", vcname);
    exit (EXIT_FAILURE);
  }
  tty_fd = open (ttyname, O_RDONLY);
  if (tty_fd < 0) {
    fprintf (stderr, "open %s fail\n", ttyname);
    close (vc_fd);
    exit (EXIT_FAILURE);
  }

  size = lseek (vc_fd, 0, SEEK_END);
  lseek (vc_fd, 0, SEEK_SET);
  buf = malloc (sizeof (*buf) * size);
  read (vc_fd, buf, size);
  if (ioctl (tty_fd, TIOCGWINSZ, &wins)) {
    fprintf (stderr, "ioctl %s TIOCGWINSZ fail\n", ttyname);
    free (buf);
    close (vc_fd);
    close (tty_fd);
    exit (EXIT_FAILURE);
  }
  rows = wins.ws_row;
  cols = wins.ws_col;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      printf ("%c", buf[i * cols + j]);
    }
    puts("");
  }
  free (buf);
  close (vc_fd);
  close (tty_fd);
  exit (EXIT_SUCCESS);
}
