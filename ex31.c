#include <stdbool.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

bool checkSimilarity(int fdSmall, int fdBig, int countToReach, int offsets);
bool checkIdentical(int fd1, int fd2, int size);

int main(int argc, char** argv) {
  char* fileOne = argv[1], * fileTwo = argv[2];
  bool sameSize = false, isOneSmaller = false, similar = false, equal = false;
  int smallHalfSize, fullSize, offsets;
  long fileOneSize, fileTwoSize;
  struct stat stat_p1, stat_p2;

  if (stat(fileOne, &stat_p1) == -1) /* declare the 'stat' structure */
  {
    return -1;
  }
  if (stat(fileTwo, &stat_p2) == -1) /* declare the 'stat' structure */
  {
    return -1;
  }

  fileOneSize = stat_p1.st_size;
  fileTwoSize = stat_p2.st_size;

  //check which file is smaller and calculate half size
  if (fileOneSize == fileTwoSize) {
    sameSize = true;
    fullSize = fileOneSize; // doesnt matter if assign stat_p1 or stat_p2
    offsets = 1;
    if (fileOneSize % 2 == 0) {
      smallHalfSize = fileOneSize / 2;
    } else {
      smallHalfSize = (fileOneSize / 2) + 1;
    }
  } else if (fileOneSize < fileTwoSize) {
    isOneSmaller = true;
    offsets = fileTwoSize - fileOneSize + 1;
    if (fileOneSize % 2 == 0) {
      smallHalfSize = fileOneSize / 2;
    } else {
      smallHalfSize = (fileOneSize / 2) + 1;
    }
  } else { //fileTwo is smaller
    offsets = fileOneSize - fileTwoSize + 1;
    if (fileTwoSize % 2 == 0) {
      smallHalfSize = fileTwoSize / 2;
    } else {
      smallHalfSize = (fileTwoSize / 2) + 1;
    }
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
      similar = checkSimilarity(fd1, fd2, smallHalfSize, offsets);
    } else { //fileTwo is smaller
      similar = checkSimilarity(fd2, fd1, smallHalfSize, offsets);
    }
  }

  //close both files
  close(fd1);
  close(fd2);

  if (equal) {
    printf("equal\n");
    return 1;
  } else if (similar) {
    printf("similar\n");
    return 3;
  } else {
    printf("different\n");
    return 2;
  }
}

bool checkSimilarity(int fdSmall, int fdBig, int countToReach, int offsets) {
  int charMatched = 0;
  char buf1[1], buf2[1];
  int offsetCount = 0;
  int readSmall, readBig;

  for (int i = 0; i < offsets; i++) {
    charMatched = 0;
    lseek(fdSmall, offsetCount, SEEK_SET);
    lseek(fdBig, offsetCount, SEEK_SET);
    readSmall = read(fdSmall, buf1, 1);

    while (readSmall != 0) {
      readBig = read(fdBig, buf2, 1);
      if (buf1[0] == buf2[0]) {
        charMatched++;
      }
      readSmall = read(fdSmall, buf1, 1);
    } // end of while loop

    if (charMatched >= countToReach) {
      return true;
    } else {
      offsetCount++;
    }

  } // end of for loop

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
