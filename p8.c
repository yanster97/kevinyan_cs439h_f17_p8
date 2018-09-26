#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    uint32_t i;
    uint32_t timeout = 0;
    char* stderrName = 0;
    char* stdoutName = 0;
    //
    int noCom = 0;
  
    int EndsWith(const char *str, const char *suffix)
    {
        if(!str || !suffix) {
            return 0;
        }
        size_t lenstr = strlen(str);
        size_t lensuffix = strlen(suffix);
        if(lensuffix > lenstr) {
            return 0;
        }
        return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
    }

    
    //check all flags, else break. If there is no command then noArg will not update
    if(EndsWith(argv[0], "p8") == 1) { 
        for(i = 1; i < argc; i++) {
            if(strcmp(argv[i], "-t") == 0) {
                timeout = atoi (argv[++i]);
            } else if(strcmp(argv[i], "-e") == 0) {
                stderrName = argv[++i];
            } else if(strcmp(argv[i], "-o") == 0) {
                stdoutName = argv[++i];
            } else if(strcmp(argv[i], "-f") == 0) {
                return atoi (argv[++i]);
            } else if(strcmp(argv[i], "-s") == 0) {
                uint32_t* kill = 0x0000;
                *(kill) = 1337;
            } else {
                noCom = i;
                break;
            }
        }
    } else if(EndsWith(argv[0], "to") == 1) {
        //set timeout
        timeout = atoi (argv[1]);
        for(i = 2; i < argc; i++) {
            if(strcmp(argv[i], "-t") == 0) {
                timeout = atoi (argv[++i]);
            } else if(strcmp(argv[i], "-e") == 0) {
                stderrName = argv[++i];
            } else if(strcmp(argv[i], "-o") == 0) {
                stdoutName = argv[++i];
            } else if(strcmp(argv[i], "-f") == 0) {
                return atoi (argv[++i]);
            } else if(strcmp(argv[i], "-s") == 0) {
                uint32_t* kill = 0x0000;
                *(kill) = 1337;
            } else {
                noCom = i;
                break;
            }
        }
    } else { // should be capture
        //malloc enough space for 7 characters plus terminated character of a string
        stderrName = malloc(sizeof(argv[1]) + 8);
        stdoutName = malloc(sizeof(argv[1]) + 8);
        //copy <name>
        strcpy(stdoutName, argv[1]);
        strcpy(stderrName, argv[1]);
        //concat appropriate suffixes
        strcat(stderrName, ".stderr");
        strcat(stdoutName, ".stdout");
        for(i = 2; i < argc; i++) {
            if(strcmp(argv[i], "-t") == 0) {
                timeout = atoi (argv[++i]);
            } else if(strcmp(argv[i], "-e") == 0) {
                stderrName = argv[++i];
            } else if(strcmp(argv[i], "-o") == 0) {
                stdoutName = argv[++i];
            } else if(strcmp(argv[i], "-f") == 0) {
                return atoi (argv[++i]);
            } else if(strcmp(argv[i], "-s") == 0) {
                uint32_t* kill = 0x0000;
                *(kill) = 1337;
            } else {
                noCom = i;
                break;
            }
        }
    }

    //if noArg is unchanged, which means we traversed through all command line arguments but did not find a command, return 0
    if(noCom == 0) {
        return 0;
    }

    //create child process
    int pid = fork();

    int childStat;
    //child process
    if(pid == 0) {
        if(stdoutName != 0) {
            //printf("out\n");
            int newfd = open(stdoutName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            dup2(newfd, 1);
            close(newfd);
        }
        if(stderrName != 0) {
            //printf("err\n");
            int newfd = open(stderrName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            dup2(newfd, 2);
            close(newfd);
        }
        execvp(argv[i], &argv[i]);
    } else if (pid > 0) { //parent process
        //timeout specified
        if(timeout != 0) {
            //create 2nd child process to accomodate for timeout
            int pidTO = fork();
            //parent process (same)
            if(pidTO == 0) {
                sleep(timeout);
                //in the case that the timeout finishes first, kill the original child process
                kill(pid, SIGTERM);
                return SIGTERM;
            }
        
            waitpid(pid, &childStat, 0);
            //in the case that the original child process finishes first, kill the 2nd child process (sleep)
            if(WIFSIGNALED(childStat) == 0) {
                kill(pidTO, SIGTERM);
            }
        } else {
            waitpid(pid, &childStat, 0);
        }
        
        //if the original child was passed a signal, then return that signal id
        if(WIFSIGNALED(childStat)) {
            return WTERMSIG(childStat);
        }
    }

    //no signals, return exit status
    return WEXITSTATUS(childStat);
}
