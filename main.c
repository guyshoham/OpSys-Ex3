#include <stdbool.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
  char* fileOne = argv[1];
  char* fileTwo = argv[2];
  char msg[64];
  bool sameSize = false, isOneSmaller = false;
  int smallHalfSize, fullSize;
  int read1, read2;

  struct stat stat_p1, stat_p2;

  if (stat(fileOne, &stat_p1) == -1) /* declare the 'stat' structure */
  {
    sprintf(msg, " Error occurred attempting to stat %s\n", fileOne); /*printf*/
    perror(msg);
    return 1;
  }
  if (stat(fileTwo, &stat_p2) == -1) /* declare the 'stat' structure */
  {
    printf(" Error occurred attempting to stat %s\n", fileTwo); /*printf*/
    perror(msg);
    return 1;
  }

  fullSize = stat_p1.st_size; // doesnt matter if assign stat_p1 or stat_p2

  //check which file is smaller and calculate half size
  if (stat_p1.st_size == stat_p2.st_size) {
    printf("The files are same size\n");
    sameSize = true;
    smallHalfSize = stat_p1.st_size / 2;
  } else if (stat_p1.st_size < stat_p2.st_size) {
    printf("fileOne is smaller\n");
    isOneSmaller = true;
    smallHalfSize = stat_p1.st_size / 2;
  } else {
    printf("fileTwo is smaller\n");
    smallHalfSize = stat_p2.st_size / 2;
  }

  //printf("full = %ld\n", stat_p1.st_size);
  //printf("half = %d\n", smallHalfSize);

  int fd1 = open(fileOne, O_RDONLY);
  if (fd1 < 0) /* means file open did not take place */
  {
    perror("after open ");   /* text explaining why */
    return (-1);
  }
  int fd2 = open(fileTwo, O_RDONLY);
  if (fd2 < 0) /* means file open did not take place */
  {
    perror("after open ");   /* text explaining why */
    return (-1);
  }

  //char read1_buf[smallHalfSize], read2_buf[smallHalfSize];
  char read1_buf_full[fullSize], read2_buf_full[fullSize];

  /*read1 = read(fd1, read1_buf, sizeof(read1_buf));
  read2 = read(fd2, read2_buf, sizeof(read2_buf));

  if (read1 > 0) {
    printf("one: %s\n", read1_buf);
    printf("two: %s\n", read2_buf);
  }*/

  if (sameSize) {
    //TODO: check if files are identical

    //read both files
    read1 = read(fd1, read1_buf_full, fullSize);
    read2 = read(fd2, read2_buf_full, fullSize);

    //fix the buffer size
    read1_buf_full[fullSize] = '\0';
    read2_buf_full[fullSize] = '\0';

    //compare two buffers
    if (read1 > 0 && read2 > 0) {
      printf("one: %s\n", read1_buf_full);
      printf("two: %s\n", read2_buf_full);

      //printf("%d\n", strcmp(read1_buf_full, read2_buf_full));
      if (strcmp(read1_buf_full, read2_buf_full) == 0) {
        printf("EQUAL!\n");
      } else {
        printf("NOT EQUAL!\n");
      }

      /*printf("\n");
      printf("file size: %ld\n", stat_p1.st_size);
      printf("buffer size: %lu\n", strlen(read1_buf_full));
      printf("buffer size: %lu\n", sizeof(read1_buf_full));*/
    }
  } //end of same size statement
  else {
    if (isOneSmaller) {
      int pos = 0;
      lseek(fd1, 0, SEEK_SET);
      lseek(fd2, 0, SEEK_SET);
    }
  }

  close(fd1);
  close(fd2);
  return 0;
}
