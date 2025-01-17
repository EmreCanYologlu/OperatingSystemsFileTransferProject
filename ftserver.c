#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <signal.h>
#include "shareddefs.h"


#define TARGET_DIR ""




//globals
int child_pids[10];
int client_pids[10];
char mqname[MAX_FILE_NAME_LENGTH];
int child_count;
int client_count;


int extractChildID(const char *str) {
    const char *prefix = "cs"; // Define the prefix
    int prefixLength = strlen(prefix); // Get the length of the prefix

    // Check if the string starts with the prefix
    if (strncmp(str, prefix, prefixLength) == 0) {
        // Convert the part after the prefix to an integer using atoi
        return atoi(str + prefixLength);
    } else {
        printf("String does not start with the required prefix.\n");
        return -1; // Return an error value if prefix is not found
    }
}

int append(int arr[], int size, int newElement) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == 0) {  // Check for the first empty position
            arr[i] = newElement; // Insert the new element
            return 1; // Return 1 indicating successful insertion
        }
    }
    return 0; // Return 0 indicating the array is full and no insertion was made
}


void removeElement(int arr[], int size, int element) {
    int i, found = 0;

    // Find the element and shift the subsequent elements to the left
    for (i = 0; i < size; i++) {
        if (arr[i] == element) {
            found = 1;
        }
        if (found && i < size - 1) {
            arr[i] = arr[i + 1]; // Shift the elements to the left
        }
    }

    // If the element was found, replace the last element with a placeholder (e.g., 0)
    if (found) {
        arr[size - 1] = 0; // Placeholder
    } else {
        printf("Element %d not found in the array.\n", element);
    }
}

struct response* createResponse(struct request* req, char target_dir[], int sendsize){


    char temp_dir[MAX_FILE_NAME_LENGTH];
    strcpy(temp_dir, target_dir);
    struct response* res = malloc(sizeof(struct response));
    res->type = req->type;
    if (res->type == LIST){
        struct dirent *entry;
        DIR *dp = opendir(temp_dir);
        
        if (dp == NULL) {
            perror("opendir");
            exit(-1);
        }
        res->data[0] = '\0';


        while ((entry = readdir(dp)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }


            // Ensure we don't exceed the buffer size
            size_t name_length = strlen(entry->d_name);
            size_t current_length = strlen(res->data);
            
            // Check if adding the file name and space would exceed MAX_SEND_SIZE
            if (current_length + name_length + 1 >= sendsize) {
                break;
            }

            strcat(res->data, entry->d_name);
            strcat(res->data, " ");
        }
    }
    else if(res->type == DOWNLOAD){
        printf("Download started \n");
        strcpy(res->data, temp_dir);
    }
    else if(res->type == QUIT){
        strcpy(res->data, "You are now quitting...");
    }
    else{
        printf("Unacceptable request type!!!");
    }
    return res;
}

void handle_sigchld(int sig) {
    int status;
    pid_t pid;

    // Loop to handle all terminated children (in case multiple children terminate)
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child with PID %d has terminated\n", pid);

        // Remove the child PID from the child_pids array
        

        // We now need to remove the corresponding client PID (child_client_pid)
        // Find the client PID associated with this child PID
        int child_index = -1;
        for (int i = 0; i < sizeof(child_pids) / sizeof(child_pids[0]); i++) {
            if (child_pids[i] == pid) {
                child_index = i;
                break;
            }
        }

        removeElement(child_pids, sizeof(child_pids) / sizeof(child_pids[0]), pid);
        child_count--;  // Decrement child count

        // If a corresponding client PID is found, remove it
        if (child_index != -1) {
            int client_pid_to_remove = client_pids[child_index];
            printf("Client with pid %d has terminated\n", client_pid_to_remove);
            removeElement(client_pids, sizeof(client_pids) / sizeof(client_pids[0]), client_pid_to_remove);
            client_count--;  // Decrement client count
        }
    }
}

void handle_sigint(int sig) {
    printf("\nSIGINT received! Terminating all children and clients...\n");

    // Terminate all client processes
    for (int i = 0; i < child_count; i++) {
        if (client_pids[i] > 0) {
            printf("Sending SIGTERM to client with PID: %d\n", client_pids[i]);
            kill(client_pids[i], SIGTERM);  // Send SIGTERM to terminate the client process
        }
    }

    // Terminate all child server processes
    for (int i = 0; i < child_count; i++) {
        if (child_pids[i] > 0) {
            printf("Sending SIGTERM to child server with PID: %d\n", child_pids[i]);
            int ret = kill(child_pids[i], SIGTERM);  // Send SIGTERM to terminate the child process

        }
    }

    

    mq_unlink(mqname);
    // After sending signals, exit the main process
    exit(0);
}

int main(int argc, char *argv[]) {
    
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sa.sa_flags = SA_RESTART;  // To automatically restart interrupted system calls
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_int;
    sa_int.sa_handler = &handle_sigint;
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }

    // Register the signal handler for SIGCHLD
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pid_t server_pid;


    child_count = 0;
    memset(child_pids, 0, sizeof(child_pids));
    memset(client_pids, 0, sizeof(client_pids)); 



    if (argc < 4){
        printf("You did not input correct amount of arguments\n");
        exit(-1);
    }
    char target_dir[MAX_FILE_NAME_LENGTH];
    strcpy(target_dir, argv[1]);
    strcpy(mqname, argv[2]);
    int sendsize = atoi(argv[3]);


    server_pid = getpid();
    printf("Server pid: %d\n", server_pid);


    mqd_t mq;
    struct mq_attr mq_attr;
    struct item* itemptr;
    int n, buflen;
    char * bufptr;


    //create the queue
    mq = mq_open(mqname, O_RDWR | O_CREAT, 0666, NULL);
    if (mq == -1){
        perror("can not create msq queue\n");
        exit(1);
    }
    mq_getattr(mq, &mq_attr);
    printf("mq maximum msgsize = %d\n", (int)mq_attr.mq_msgsize);
    buflen = mq_attr.mq_msgsize;
    bufptr = (char*)malloc(buflen);


    printf("Server started:\n");
    printf("Message Queue Name: %s\n", mqname);


    pid_t pi;
    while(1){
        printf("Waiting for connections...\n");
        printf("Number of clients: %d\n", child_count);
        n = mq_receive(mq, (char *) bufptr, buflen, NULL);
        if (n == -1){
            mq_unlink(mqname);
            perror("Cannot receive message\n");
            exit(-1);
        }
        else {
            printf("Received msg\n");
        }

        
        struct mq_connection* connect_req = (struct mq_connection *)bufptr;
        printf("CS: %s\n", connect_req->cs_name);
        printf("SC: %s\n", connect_req->sc_name);

        int client_pid = extractChildID(connect_req->cs_name);
        append(client_pids, sizeof(client_pids), client_pid);
        
        pi = fork();
        if (pi == 0){
            // Open pipes
            
            
            int fd_cs = open(connect_req->cs_name, O_RDONLY);
            if (fd_cs == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }


            int fd_sc = open(connect_req->sc_name, O_WRONLY);
            if (fd_sc == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }


            int child_client_pid = extractChildID(connect_req->cs_name);
            //send acknowledgement message

            char ack_buffer[100];
            snprintf(ack_buffer, sizeof(ack_buffer), "%d", sendsize);

            int bytes_read = write(fd_sc, ack_buffer, sizeof(ack_buffer));
            if (bytes_read == -1){
                perror("Failed to send acknowledgement message\n");
            }

            printf("Connection established, you can now interact.\n");
            while (1) {
                printf("Waiting for requests...\n");


                // Allocate memory for request
                struct request req;


                // Read from cs
                int bytes_read = read(fd_cs, &req, sizeof(struct request));  // Correct size
                if (bytes_read == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }


                char response[128];
                if (req.type == LIST) {
                    strcpy(response, "List request received");
                } else if (req.type == DOWNLOAD) {
                    strcpy(response, "Download request received");
                } else if (req.type == QUIT) {
                    strcpy(response, "Quit request received");
                } else {
                    strcpy(response, "Invalid request");
                }

                struct response* res = createResponse(&req, target_dir, sendsize);
                printf("Response ready: %s\n", response);
                
                // Send response to sc
                write(fd_sc, res, sizeof(struct response));


                enum req_type status = res->type;
                free(res);
                if (status == DOWNLOAD){
                    char filename[1024];  // Buffer large enough for full path
                    snprintf(filename, sizeof(filename), "%s/%s", target_dir, req.filename);  // Safely concatenate
                    printf("Getting file: %s\n", filename);


                    char buffer[sendsize];
                    ssize_t bytes_read;


                    int input_fd = open(filename, O_RDONLY);
                    if (input_fd == -1) {
                        perror("Failed to open input file");
                        return 1;
                    }
                    printf("Opened %s\n", filename);
                    while ((bytes_read = read(input_fd, buffer, sendsize)) > 0){
                        if (write(fd_sc, buffer, bytes_read) == -1) {
                            perror("Failed to write to sc pipe");
                            break;
                        }


                        char temp_req[1];
                        if (read(fd_cs, temp_req, 1) == -1){
                            perror("Failed to read acknowledgment from cs pipe");
                            break;
                        }
                        printf("Reading...");
                    }
                    char small_msg[10] = "Finished";
                    write(fd_sc, small_msg, sizeof(small_msg));
                    printf("Finished download \n");
                    close(input_fd);
                }


                else if(status == QUIT){
                    printf("O father, I am now terminating !\n");
                    break;
                }

                
            }
            close(fd_cs);
            close(fd_sc);

            return 0;
        }
        else{
            child_count += 1;
            client_count += 1;
            append(child_pids, sizeof(child_pids), pi);           
        }
    }
    mq_unlink(mqname);
}
