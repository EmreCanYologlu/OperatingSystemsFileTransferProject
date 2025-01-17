#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv){
    if (argc <= 1){
        printf("You have entered wrong number of arguments.\n");
        return -1;
    }

    pid_t pid = atoi(argv[1]);
    if (kill(pid, SIGINT) == 0){
        printf("Signal sent successfully!\n");
    }
    else{
        perror("Failed to send signal");
        exit(EXIT_FAILURE);
    }
    return 0;
}