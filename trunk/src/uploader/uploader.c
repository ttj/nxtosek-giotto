/*

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

main(int argc, char *argv[]) {

  unsigned fd_file, fd_fifo, j;
  void *heap;

  unsigned fd_fileA, fd_fileB, i, k;
  char st[32];

  fd_file=open(argv[1], O_RDONLY);
  if(fd_file == -1) {
    if(errno == ENOENT)
      printf(" Error: File doesn't exist\n");
    else
      printf(" Error\n");
    exit(1);
  }
  fd_fifo=open("loaderpipe", O_WRONLY);

  read(fd_file, &j, sizeof(int));
  write(fd_fifo, &j, sizeof(int));
  j = j*(32+sizeof(int));
  heap = malloc(j);
  read(fd_file, heap, j);
  write(fd_fifo, heap, j);
  free(heap);

  read(fd_file, &j, sizeof(int));
  write(fd_fifo, &j, sizeof(int));
  j = j*4*sizeof(int);
  heap = malloc(j);
  read(fd_file, heap, j);
  write(fd_fifo, heap, j);
  free(heap);

  close(fd_fifo);
  close(fd_file);
}
*/

