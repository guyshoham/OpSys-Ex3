//Guy Shoham 302288444

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

bool isDir();
bool isCFile();

int main(int argc, char** argv) {
  char* path = argv[1];
  char* line1, * line2, * line3;

  //TODO: strtok to extract lines from conf.txt
}
