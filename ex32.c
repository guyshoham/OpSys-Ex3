//Guy Shoham 302288444

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

bool isDir();
bool isCFile(const char* str);

int main(int argc, char** argv) {
  char* path = argv[1], * currentPwd;
  char* line1, * line2, * line3;
  char buf[1024], cwdCurr[PATH_MAX];
  int readStatus, status;
  int cdStatus;
  pid_t val;

  //open conf.txt
  int fd = open(path, O_RDONLY);
  if (fd < 0) /* means file open did not take place */
  {
    perror("after open ");   /* text explaining why */
    return (-1);
  }

  //split conf.txt
  readStatus = read(fd, buf, 1024);
  line1 = strtok(buf, "\n");
  line2 = strtok(NULL, "\n");
  line3 = strtok(NULL, "\n");

  // cd into line1
  cdStatus = chdir(line1);
  if (cdStatus == -1) {
    fprintf(stderr, "Error: No such file or directory\n");
    _exit(-1);
  }

  DIR* pDir;
  struct dirent* pDirent;
  if ((pDir = opendir(line1)) == NULL)
    _exit(-1);

  // looping through the directory, printing the directory entry name
  while ((pDirent = readdir(pDir)) != NULL) {
    printf("%s\n", pDirent->d_name);
    if (pDirent->d_type == 4) { //DIR type

      cdStatus = chdir(pDirent->d_name);
      if (cdStatus == -1) {
        fprintf(stderr, "Error: No such file or directory\n");
        _exit(-1);
      }

      /// look for C file inside dir

      DIR* studentDir;
      struct dirent* studentDirent;
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
      if ((studentDir = opendir(currentPwd)) == NULL) { _exit(-1); }
      printf("C files:\n");
      while ((studentDirent = readdir(studentDir)) != NULL) {
        if (isCFile(studentDirent->d_name)) {
          printf("%s\n", studentDirent->d_name);
          val = fork();
          if (val == 0) { //child process
            status = execlp("gcc", "gcc", studentDirent->d_name, "-o", "example.out", NULL);
            if (status == -1) {
              perror("gcc ");
            }
          } else {
            wait(&status);
          }

        }
      } //end of student dir while loop

      /// end of searching, go back to line1
      chdir(line1);

    }
    printf("\n");

  } //end of while loop
  closedir(pDir);
}

bool isCFile(const char* str) {
  return (str && *str && str[strlen(str) - 1] == 'c') ? 1 : 0;
}
