#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static void
usage ()
{
  printf ("Usage: vcrecord ttynum\n");
}

int main (int argc, char *argv[])
{
  int fd, size;
  int currcons = 0;		/* current tty */
  char ttyname[10];

  if (argc != 2) {
    usage ();
    exit (EXIT_FAILURE);
  }

  currcons = strtoul (argv[1], NULL, 10);
  if (currcons < 0 || currcons > 7) {
    fprintf (stderr, "tty number must be < 0 or > 7\n");
    exit (EXIT_FAILURE);
  }
  snprintf (ttyname, 10, "/dev/vcs%d", currcons);

  fd = open (ttyname, O_RDONLY);
  if (fd < 0) {
    fprintf (stderr, "open %s fail\n", ttyname);
    exit (EXIT_FAILURE);
  }

  size = lseek (fd, 0, SEEK_END);
  printf ("size %d\n", size);

  close (fd);
  exit (EXIT_SUCCESS);
}
