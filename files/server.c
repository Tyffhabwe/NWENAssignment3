/**
 * Skeleton file for server.c
 * 
 * You are free to modify this file to implement the server specifications
 * as detailed in Assignment 3 handout.
 * 
 * As a matter of good programming habit, you should break up your imple-
 * mentation into functions. All these functions should contained in this
 * file as you are only allowed to submit this file.
 */ 

// Include necessary header files

#include <stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

void error(const char * msg){
    printf("Error : %s.\n", msg);
    exit(-1);
} 

void handle_get_response(char buffer[]) {
    FILE *in;
    char command[3];
    char filename[20];

    sscanf(buffer, "%s %s", command, filename);

    in = fopen(filename, "r");

    if(in == NULL) {
        printf("Could not open: %s\n", filename);
        printf("Make sure that you got the name right.\n");
        return;
    }

    printf("Successfully opened: %s\n", filename);
}

/**
 * The main function should be able to accept a command-line argument
 * argv[0]: program name
 * argv[1]: port number
 * 
 * Read the assignment handout for more details about the server program
 * design specifications.
 */

int main(int argc, char *argv[])
{   
    //Making sure that we get at least one argument in terminal
    if(argc < 2) {
        printf("Error: not enough arguments.\n");
        return -1;
    }
    
    //Read the port number into an int
    int portNumberUserWants;
    sscanf(argv[1], "%d", &portNumberUserWants);

    //Make sure user port number is above 1024
    if(portNumberUserWants < 1024) {
        printf("Error: port number must be above 1024.\n");
        return -1;
    }

    //Create the socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        error("Error creating socket");
    }

    //Time to bind the socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNumberUserWants);
    addr.sin_addr.s_addr = INADDR_ANY;

    int bindResult = bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    if(bindResult < 0) {
        error("Error binding sock");
    }

    //Time to listen for connections
    if(listen(fd, SOMAXCONN) < 0) {
        error("Listening error");
    }

    //Accept client connection
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    int client_fd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);

    if(client_fd < 0) {
        error("Error accepting client. ");
    }

    //Message work
    char* start_message = "Hello from the server!.\n";
    char buffer[100];
    memset(buffer, 0, 100);

    strncpy(buffer, start_message, strlen(start_message));

    int s = write(client_fd, buffer, 100);
    if(s<0) {
        error("Writing to socket");
    }
    
    memset(buffer, 0, 100);
    int r = read(client_fd, buffer, 100);

    //Keep running until I get 'bye' or BYE in the first 3 letters from user
    while(strncmp(buffer, "bye", 3) != 0 && strncmp(buffer, "BYE", 3) != 0) {
        if(r < 0) {
            error("Error reading from client socket");
        }

        //Do something if it is a 'get' request
        if(strncmp(buffer, "get", 3) == 0 || strncmp(buffer, "GET", 3) == 0) {
            char *pointerToBuffer = &buffer;
            handle_get_response(pointerToBuffer);
        }
        //Print out content if it is a 'put' request
        else if(strncmp(buffer, "put", 3) == 0 || strncmp(buffer, "PUT", 3) == 0) {

        }
        else {
            printf("Did not recognise command: %s\n", buffer);
        }

        memset(buffer, 0, 100);
        r = read(client_fd, buffer, 100);
    }
    
    //END THE CONNECTION
    close(client_fd);
    printf("Closed connection!\n");
    return 0;
}


