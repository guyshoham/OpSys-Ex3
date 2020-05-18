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

int main(int argc, char** argv) {
  char* path = argv[1], * currentPwd, * initPwd;
  char* line1, * line2, * line3;
  char buf[1024], cwdCurr[PATH_MAX], cwdInit[PATH_MAX];
  int readStatus, cdStatus;
  //pid_t forkVal;
  int fd_conf, fd_input, fd_output, fd_csv;
  bool fileFound;
  DIR* pOuterDir, * pInnerDir;
  struct dirent* pOuterDirent, * pInnerDirent;
  time_t start, end;
  double dif;

  initPwd = getcwd(cwdInit, sizeof(cwdInit));

  //open conf.txt
  fd_conf = open(path, O_RDONLY);
  if (fd_conf < 0) /* means file open did not take place */
  {
    perror("after open ");   /* text explaining why */
    return (-1);
  }

  //split conf.txt
  readStatus = read(fd_conf, buf, 1024);
  line1 = strtok(buf, "\n");
  line2 = strtok(NULL, "\n");
  line3 = strtok(NULL, "\n");

  if ((fd_csv = open("result.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
    perror("result.csv: "); /* open failed */
    exit(-1);
  }

  // cd into line1
  cdStatus = chdir(line1);
  if (cdStatus == -1) {
    perror("Error: \n");
    exit(-1);
  }

  if ((pOuterDir = opendir(line1)) == NULL)
    exit(-1);

  // looping through the directory, printing the directory entry name
  while ((pOuterDirent = readdir(pOuterDir)) != NULL) {
    if (isDir(pOuterDirent->d_type)) { //DIR type

      cdStatus = chdir(pOuterDirent->d_name);
      if (cdStatus == -1) {
        perror("Error: ");
        exit(-1);
      }

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
              perror("error compiling: ");
              return -1;
            }
          }

          wait(NULL);
          ///done compiling
          if (access("example.out", F_OK) == -1) { // file doesn't exist (means compilation error)
            char result[1024];
            strcpy(result, pOuterDirent->d_name);
            strcat(result, ",COMPILATION_ERROR,10\n\0");
            write(fd_csv, result, strlen(result));
            continue;
          }

          time(&start); // begin timeout check
          if (fork() == 0) { //child process
            if ((fd_input = open(line2, O_RDONLY)) < 0) {
              perror("open input file: "); /* open failed */
              exit(-1);
            }
            if ((fd_output = open("output.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
              perror("output.txt: "); /* open failed */
              exit(-1);
            }

            dup2(fd_input, STDIN_FILENO);
            dup2(fd_output, STDOUT_FILENO);

            if (execlp("./example.out", "example.out", NULL) == -1) {
              perror("error executing: ");
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
            write(fd_csv, result, strlen(result));
            continue;
          }

          char outputPath[PATH_MAX];
          strcpy(outputPath, line1);
          strcat(outputPath, "/");
          strcat(outputPath, pOuterDirent->d_name);
          strcat(outputPath, "/output.txt");

          int ret;
          if (fork() == 0) { //child process
            chdir(initPwd);
            if (execlp("./comp.out", "comp.out", outputPath, line3, NULL) == -1) {
              perror("error: ");
            }
          }
          wait(&ret);
          ///done comparing

          int retVal = WEXITSTATUS(ret);
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

      /// end of innerDir searching, go back to line1

      //todo: remove output.txt and example.out

      chdir(line1);
      chdir(pOuterDirent->d_name);
      removeCompiledFile();
      removeOutputFile();


      chdir(line1);
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
        perror("rm: ");
      }
    }
    wait(NULL);
  }
}
void removeOutputFile() {
  if (access("output.txt", F_OK) != -1) { // file exist
    if (fork() == 0) { //child process
      if (execlp("rm", "rm", "output.txt", NULL) == -1) {
        perror("rm: ");
      }
    }
    wait(NULL);
  }
}
