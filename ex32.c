//Guy Shoham 302288444

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

bool isDir(int type);
bool isCFile(const char* str);
void removeCompiledFile();
void removeOutputFile();
void moveToDir(char* path);
void checkWriteSysCall();

int main(int argc, char** argv) {
  char msg[100];
  char* path = argv[1], * currentPwd, * initPwd;
  char* line1, * line2, * line3;
  char buf[1024], cwdCurr[PATH_MAX], cwdInit[PATH_MAX];
  int fd_conf, fd_input, fd_output, fd_csv, status;
  bool fileFound;
  DIR* pOuterDir, * pInnerDir;
  struct dirent* pOuterDirent, * pInnerDirent;
  time_t start, end;
  double dif;

  initPwd = getcwd(cwdInit, sizeof(cwdInit));

  //open conf.txt
  if ((fd_conf = open(path, O_RDONLY)) < 0) {
    strcpy(msg, "error opening file conf.txt\0");
    status = write(2, msg, strlen(msg));
    checkWriteSysCall(status);
    exit(-1);
  }

  //split conf.txt
  if ((status = read(fd_conf, buf, 1024)) < 0) {
    strcpy(msg, "error reading conf.txt\0");
    status = write(2, msg, strlen(msg));
    checkWriteSysCall(status);
    exit(-1);
  }
  line1 = strtok(buf, "\n");
  line2 = strtok(NULL, "\n");
  line3 = strtok(NULL, "\n");

  if ((fd_csv = open("result.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
    strcpy(msg, "error opening result.csv\0");
    status = write(2, msg, strlen(msg));
    checkWriteSysCall(status);
    exit(-1);
  }

  // cd into line1
  moveToDir(line1);

  if ((pOuterDir = opendir(line1)) == NULL) {
    strcpy(msg, "opendir error\0");
    status = write(2, msg, strlen(msg));
    checkWriteSysCall(status);
    exit(-1);
  }

  // looping through the directory, printing the directory entry name
  while ((pOuterDirent = readdir(pOuterDir)) != NULL) {
    if (isDir(pOuterDirent->d_type)) { //DIR type
      moveToDir(pOuterDirent->d_name);

      /// look for C file inside dir
      fileFound = false;
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
      if ((pInnerDir = opendir(currentPwd)) == NULL) {
        exit(-1);
      }

      while ((pInnerDirent = readdir(pInnerDir)) != NULL) {
        if (isCFile(pInnerDirent->d_name)) {
          fileFound = true;
          if (fork() == 0) { //child process
            if (execlp("gcc", "gcc", pInnerDirent->d_name, "-o", "example.out", NULL) == -1) {
              strcpy(msg, "gcc: error compiling\0");
              status = write(2, msg, strlen(msg));
              checkWriteSysCall(status);
              return -1;
            }
          }

          wait(&status);
          ///done compiling
          if (WEXITSTATUS(status) != 0) { //gcc failed
            char result[1024];
            strcpy(result, pOuterDirent->d_name);
            strcat(result, ",COMPILATION_ERROR,10\n\0");
            status = write(fd_csv, result, strlen(result));
            checkWriteSysCall(status);
            continue;
          }

          time(&start); // begin timeout check
          if (fork() == 0) { //child process
            if ((fd_input = open(line2, O_RDONLY)) < 0) {
              strcpy(msg, "error opening file input.txt\0");
              status = write(2, msg, strlen(msg));
              checkWriteSysCall(status);
              exit(-1);
            }
            if ((fd_output = open("output.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
              strcpy(msg, "error opening file output.txt\0");
              status = write(2, msg, strlen(msg));
              checkWriteSysCall(status);
              exit(-1);
            }

            dup2(fd_input, STDIN_FILENO);
            dup2(fd_output, STDOUT_FILENO);

            if (execlp("./example.out", "example.out", NULL) == -1) {
              strcpy(msg, "error executing example.out\0");
              status = write(2, msg, strlen(msg));
              checkWriteSysCall(status);
              exit(-1);
            }
          }
          wait(NULL);
          ///done executing
          time(&end); // stop timeout check
          dif = difftime(end, start);
          if (dif > 3) {
            char result[1024];
            strcpy(result, pOuterDirent->d_name);
            strcat(result, ",TIMEOUT,20\n\0");
            status = write(fd_csv, result, strlen(result));
            checkWriteSysCall(status);
            continue;
          }

          //build the output path
          char outputPath[PATH_MAX];
          strcpy(outputPath, line1);
          strcat(outputPath, "/");
          strcat(outputPath, pOuterDirent->d_name);
          strcat(outputPath, "/output.txt");

          if (fork() == 0) { //child process
            moveToDir(initPwd);
            if (execlp("./comp.out", "comp.out", outputPath, line3, NULL) == -1) {
              strcpy(msg, "error executing comp.out\0");
              status = write(2, msg, strlen(msg));
              checkWriteSysCall(status);
              exit(-1);
            }
          }
          wait(&status);
          ///done comparing

          int retVal = WEXITSTATUS(status);
          char result[1024];
          strcpy(result, pOuterDirent->d_name);
          switch (retVal) {
            case 1: {
              strcat(result, ",EXCELLENT,100\n\0");
              break;
            }
            case 2: {
              strcat(result, ",WRONG,50\n\0");
              break;
            }
            case 3: {
              strcat(result, ",SIMILAR,75\n\0");
              break;
            }
            default:break;
          }//end of switch case
          write(fd_csv, result, strlen(result));

        } // end of "if C file"
      } //end of student dir while loop

      if (!fileFound && strcmp(pOuterDirent->d_name, ".") && strcmp(pOuterDirent->d_name, "..")) {
        char result[1024];
        strcpy(result, pOuterDirent->d_name);
        strcat(result, ",NO_C_FILE,0\n\0");
        write(fd_csv, result, strlen(result));
      }

      /// end of innerDir searching, removing files i created

      moveToDir(line1);
      moveToDir(pOuterDirent->d_name);
      removeCompiledFile();
      removeOutputFile();

      ///go back to line1
      moveToDir(line1);
    }

  } //end of while loop
  closedir(pOuterDir);
  closedir(pInnerDir);
  close(fd_conf);
  close(fd_input);
  close(fd_output);
  close(fd_csv);
}

bool isCFile(const char* str) { return (str && *str && str[strlen(str) - 1] == 'c') ? true : false; }
bool isDir(int type) { return type == 4 ? true : false; }
void removeCompiledFile() {
  if (access("example.out", F_OK) != -1) { // file exist
    if (fork() == 0) { //child process
      if (execlp("rm", "rm", "example.out", NULL) == -1) {
        int status;
        char msg[100];
        strcpy(msg, "error removing example.out\0");
        status = write(2, msg, strlen(msg));
        checkWriteSysCall(status);
        exit(-1);      }
    }
    wait(NULL);
  }
}
void removeOutputFile() {
  if (access("output.txt", F_OK) != -1) { // file exist
    if (fork() == 0) { //child process
      if (execlp("rm", "rm", "output.txt", NULL) == -1) {
        int status;
        char msg[100];
        strcpy(msg, "error removing output.txt\0");
        status = write(2, msg, strlen(msg));
        checkWriteSysCall(status);
        exit(-1);
      }
    }
    wait(NULL);
  }
}
void moveToDir(char* path) {
  int status;

  status = chdir(path);
  if (status == -1) {
    char msg[100];
    strcpy(msg, "chdir error\0");
    status = write(2, msg, strlen(msg));
    checkWriteSysCall(status);
    exit(-1);
  }
}
void checkWriteSysCall(int status) { if (WEXITSTATUS(status) == -1) { exit(-1); }}
