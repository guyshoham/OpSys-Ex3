#include <stdbool.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

bool checkSimilarity(int fdSmall, int fdBig, int halfSize);
bool checkIdentical(int fd1, int fd2, int size);

int main(int argc, char** argv) {
  char* fileOne = argv[1];
  char* fileTwo = argv[2];
  char msg[64];
  bool sameSize = false, isOneSmaller = false, similar = false, equal = false;
  int smallHalfSize, fullSize;

  struct stat stat_p1, stat_p2;

  if (stat(fileOne, &stat_p1) == -1) /* declare the 'stat' structure */
  {
    sprintf(msg, " Error occurred attempting to stat %s\n", fileOne); /*printf*/
    perror(msg);
    return -1;
  }
  if (stat(fileTwo, &stat_p2) == -1) /* declare the 'stat' structure */
  {
    printf(" Error occurred attempting to stat %s\n", fileTwo); /*printf*/
    perror(msg);
    return -1;
  }

  fullSize = stat_p1.st_size; // doesnt matter if assign stat_p1 or stat_p2

  //check which file is smaller and calculate half size
  if (stat_p1.st_size == stat_p2.st_size) {
    sameSize = true;
    if (stat_p1.st_size % 2 == 0) { smallHalfSize = stat_p1.st_size / 2; }
    else { smallHalfSize = (stat_p1.st_size / 2) + 1; }
  } else if (stat_p1.st_size < stat_p2.st_size) {
    isOneSmaller = true;
    if (stat_p1.st_size % 2 == 0) { smallHalfSize = stat_p1.st_size / 2; }
    else { smallHalfSize = (stat_p1.st_size / 2) + 1; }
  } else {
    if (stat_p2.st_size % 2 == 0) { smallHalfSize = stat_p2.st_size / 2; }
    else { smallHalfSize = (stat_p2.st_size / 2) + 1; }
  }

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

  if (sameSize) {
    equal = checkIdentical(fd1, fd2, fullSize);
  } //end of same size statement

  //if not equal, check similarity
  if (!equal) {
    if (isOneSmaller || sameSize) {
      similar = checkSimilarity(fd1, fd2, smallHalfSize);
    } else { //fileTwo is smaller
      similar = checkSimilarity(fd2, fd1, smallHalfSize);
    }
  }

  //close both files
  close(fd1);
  close(fd2);

  if (equal) { return 1; }
  else if (similar) { return 3; }
  else { return 2; }
}

bool checkSimilarity(int fdSmall, int fdBig, int halfSize) {
  char read1_buf_half[halfSize], read2_buf_half[halfSize];
  int smallOffset = 0, bigOffset = 0;
  int readSmall, readBig;
  lseek(fdSmall, smallOffset, SEEK_SET);
  lseek(fdBig, bigOffset, SEEK_SET);
  readSmall = read(fdSmall, read1_buf_half, halfSize);

  while (true) {
    readBig = read(fdBig, read2_buf_half, halfSize);

    //fix the buffer size
    read1_buf_half[halfSize] = '\0';
    read2_buf_half[halfSize] = '\0';

    if (readSmall == halfSize && readBig == halfSize) {
      if (strcmp(read1_buf_half, read2_buf_half) == 0) { // similarity found
        return true;
      } else {
        bigOffset++;
        lseek(fdBig, bigOffset, SEEK_SET);
      }
    } else {
      if (readBig < halfSize) {
        smallOffset++;
        bigOffset = 0;
        lseek(fdSmall, smallOffset, SEEK_SET);
        lseek(fdBig, bigOffset, SEEK_SET);
        readSmall = read(fdSmall, read1_buf_half, halfSize);
      } else {
        break;
      }
    }
  } // end of while loop
  return false;
}
bool checkIdentical(int fd1, int fd2, int size) {
  char read1_buf_full[size], read2_buf_full[size];
  int read1, read2;
  //read both files
  read1 = read(fd1, read1_buf_full, size);
  read2 = read(fd2, read2_buf_full, size);

  //fix the buffer size
  read1_buf_full[size] = '\0';
  read2_buf_full[size] = '\0';

  //compare two buffers
  if (read1 > 0 && read2 > 0) {
    if (strcmp(read1_buf_full, read2_buf_full) == 0) {
      return true;
    }
  }
  return false;
}

