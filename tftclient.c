#define _XOPEN_SOURCE 700

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>


#include "shareddefs.h"


char cs_name[8];
char sc_name[8];
int fd_cs;
int fd_sc;


enum req_type getMessageType(const char *typeStr) {
    if (strcmp(typeStr, "list") == 0) {
        return LIST;
    } else if (strcmp(typeStr, "get") == 0) {
        return DOWNLOAD;
    } else if (strcmp(typeStr, "quit") == 0) {
        return QUIT;
    } else {
        return -1;  // Invalid enum value for error handling
    }
}


struct request* createRequest(char request[]) {
    struct request* msg = malloc(sizeof(struct request));
    char* tok;


    // Initialize filename and localfilename to empty strings
    msg->filename[0] = '\0';
    msg->localfilename[0] = '\0';


    tok = strtok(request, " ");
    msg->type = getMessageType(tok);


    tok = strtok(NULL, " ");
    if (tok != NULL) {
        strcpy(msg->filename, tok);
    }


    tok = strtok(NULL, " ");
    if (tok != NULL) {
        strcpy(msg->localfilename, tok);
    }


    return msg;
}


int handleResponse(struct response* res){
    int status;
    if (res->type == LIST){
        printf("Directory content:\n");
        char* tok;
        tok = strtok(res->data, " ");
        while (tok != NULL){
            printf("%s\n", tok);
            tok = strtok(NULL, " ");
        }
        status = 1;
    }
    else if(res->type == DOWNLOAD){
        printf("Downloading \n");
        status = 2;
    }
    else if(res->type == QUIT){
        printf("Quit request granted\n");
        printf("Response data: %s\n", res->data);
        status = -1;
    }
    else{
        printf("invalid response!!\n");
    }
    return status;
}


void handle_sigterm(int sig) {
    printf("Received SIGTERM, cleaning up and exiting...\n");

    close(fd_cs);
    close(fd_sc);
    unlink(cs_name);
    unlink(sc_name);
    exit(0);
}



int main(int argc, char *argv[]) {

    struct sigaction sa;
    sa.sa_handler = &handle_sigterm;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error registering SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    if (argc < 2){
        printf("You have entered incorrect number of args\n");
        exit(-1);
    }


    char mqname[MAX_FILE_NAME_LENGTH];
    strcpy(mqname, argv[1]);
    printf("%s\n", mqname);

    char pid_str[6];
    pid_t pid = getpid();
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    strcpy(cs_name, "cs");
    strcpy(sc_name, "sc");
    strcat(cs_name, pid_str);
    strcat(sc_name, pid_str);




    // Create pipes
    mkfifo(cs_name, 0666);
    mkfifo(sc_name, 0666);







    mqd_t mq = mq_open(mqname, O_RDWR);
    if (mq == -1){
        perror("mq_open failed\n");
        exit(1);
    }




    struct mq_connection connection;
    strcpy(connection.cs_name, cs_name);
    strcpy(connection.sc_name, sc_name);
    int n = mq_send(mq, (char *)&connection, sizeof(struct mq_connection), 0);
    if (n == -1){
        printf("You f'ed up\n");
    }
    else{
        printf("Sent connection\n");
    }


    struct mq_attr mq_attr;
    int buflen;
    char * bufptr;


    fd_cs = open(cs_name, O_WRONLY);
    if (fd_cs == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    printf("Opened sc\n");


    fd_sc = open(sc_name, O_RDONLY);
    if (fd_sc == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    printf("Opened cs\n");


    //receive acknowledgement
    //set sendsize
    int sendsize;
    char ack_buffer[100];
    int bytes_read = read(fd_sc, ack_buffer, sizeof(ack_buffer));
    if (bytes_read == -1){
        perror("Failed to receive acknowledgement.\n");
        exit(EXIT_FAILURE);
    }

    printf("Connection established.\n");
    sendsize = atoi(ack_buffer);
    printf("Max sendsize: %d\n", sendsize);

    while (1) {
        char request[100];
        printf("Enter request:\n");
        fgets(request, sizeof(request), stdin);
        request[strcspn(request, "\n")] = '\0'; // Remove newline character from input


        struct request* req = createRequest(request);


        // Send request via cs
        write(fd_cs, req, sizeof(struct request));
        printf("Request sent\n");


        // Receive response via sc
        struct response res;
        int bytes_read = read(fd_sc, &res, sizeof(struct response));
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }


        int status = handleResponse(&res);


        if (status == 2) {
            char* localfilename = req->localfilename;
            int output_fd = open(localfilename, O_WRONLY | O_CREAT, 0666);
            if (output_fd == -1) {
                perror("Failed to open output file");
                return 1;
            }
            char buffer[sendsize];
            ssize_t bytes_read;

            struct timespec start, end;
            timespec_get(&start, TIME_UTC);
            while ((bytes_read = read(fd_sc, buffer, sendsize)) > 0) {
                if(strcmp(buffer, "Finished") == 0){
                    break;
                }
                // Write the data to the output file
                if (write(output_fd, buffer, bytes_read) == -1) {
                    perror("Failed to write to output file");
                    break;
                }


                // Send acknowledgment to the server (through cs)
                char ack[1] = {'A'};
                if (write(fd_cs, ack, 1) == -1) {
                    perror("Failed to write acknowledgment to cs pipe");
                    break;
                }
                printf("Writing\n");
            }
            timespec_get(&end, TIME_UTC);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            printf("Download finished. Elapsed time: %.8f\n", time_taken); 
            close(output_fd);
        }
        free(req);
        if (status == -1){
            break;
        }
    }


    close(fd_cs);
    close(fd_sc);
    unlink(cs_name);
    unlink(sc_name);
    return 0;

}


