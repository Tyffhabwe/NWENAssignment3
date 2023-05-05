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

/** A store of messages that are useful */
char* test = "YOU ARE OUT OF THE WRITING PHASE BROTHER";
char* start_message = "HELLO\n";
char* server_ok_message = "SERVER 200 OK\n\n";
char* server_error_message = "SERVER 404 Not Found\n";
char* server_505_message = "SERVER 501 Put Error\n";
char* server_500_message = "SERVER 500 Get Error\n";
char* server_501_message = "SERVER 501 Put Error\n";
char* server_502_message = "SERVER 502 Command Error\n";
char* newline_at_end = "\n\n";
char* singular_newline_char = "\n";

/** Helper method that throws an error and exits when stuff go wrong*/
void error(const char * msg){
    printf("Error : %s.\n", msg);
    exit(-1);
}

/** Helped method that takes content from the client and copies it to buffer */
void read_user_content_to_buffer(char buffer[], int client_fd, int buflen) {
    memset(buffer, 0, buflen);
    int r = read(client_fd, buffer, buflen);
    if(r < 0) {error("Error reading from client socket");}
}

/** Helper method to write whatever is in the buffer to the user */
void write_to_socket(int client_fd, char buffer[], int buflen) {
    int s = write(client_fd, buffer, buflen);
    if(s<0) {
        error("Writing to socket");
    }
    printf("Successful write\n");
}

/** Helper method to write a specific message to the user */
void write_to_user(char buffer[], char* message, int message_length, int client_fd, int buflen) {
    memset(buffer, 0, buflen);
    strncpy(buffer, message, message_length);
    write_to_socket(client_fd, buffer, buflen);
}

/** Method that handles getting the files and printing stuff back for a get response */
void handle_get_response(char buffer[], int client_fd, int buflen) {
    int server_ok_message_len = strlen(server_ok_message); 
    int server_error_message_len = strlen(server_error_message);
    int server_505_len = strlen(server_505_message);
    int server_500_len = strlen(server_500_message);
    int newline_len = strlen(newline_at_end);
    int singular_newline_len = strlen(singular_newline_char);

    FILE *in;
    char command[3];
    char filename[20];
    memset(filename, '\0', 20);
    sscanf(buffer, "%s %s", command, filename);
    
    if(filename[0] == '\0') {
        write_to_user(buffer, server_500_message, server_500_len, client_fd, buflen);
        return;
    }

    in = fopen(filename, "r");

    if(in == NULL) {
        write_to_user(buffer, server_error_message, server_error_message_len, client_fd, buflen);
        return;
    }
    
    write_to_user(buffer, server_ok_message, server_ok_message_len, client_fd, buflen);

    //Print file contents
    memset(buffer, 0, buflen);

    while (EOF != fscanf(in, "%200[^\n]\n", buffer)){
        /* Print one line to user with /n afterwards */
        write_to_socket(client_fd, buffer, buflen);
        memset(buffer, 0, buflen);
        write_to_user(buffer, singular_newline_char, singular_newline_len, client_fd, buflen);
    }
    write_to_user(buffer, newline_at_end, newline_len, client_fd, buflen);
    fclose(in);
}

/** Method that handles file editing work for a put response */
void handle_put_request(char buffer[], int client_fd, int buflen) {
    int taking_user_input = 1; //Boolean on whether to keep taking input
    int on_alert = 0; //Boolean that can either be 0 or 1.
    int server_error_message_len = strlen(server_error_message);

    FILE *in;
    char command[3];
    char filename[20];
    memset(filename, '\0', 20);
    sscanf(buffer, "%s %s", command, filename);

    in = fopen(filename, "w");

    if(in == NULL) {
        write_to_user(buffer, server_501_message, strlen(server_501_message), client_fd, buflen);
        return;
    }

    read_user_content_to_buffer(buffer, client_fd, buflen);
    while(taking_user_input == 1) {
        //Being on alert means the last buffer was a '\n'
        if(on_alert == 1) {
            //Means we have found a second straight '\n' and so stop taking user input
            if(strncmp(buffer, "\n", 1) == 0) {
                taking_user_input = 0;
            }
            //Means we found an '\n' in previous buffer but have found something else so no longer on alert
            else{
                on_alert = 0;
            }
        }
        //We are not currently on alert
        else {
            //We have found a first '\n' and so need to be 'on alert' for the second '\n'
            if(strncmp(buffer, "\n", 1) == 0) {
                on_alert = 1;
            }
        }

        fputs(buffer, in);

        printf("What is in the buffer: %s\n", buffer);
        read_user_content_to_buffer(buffer, client_fd, buflen);
    }

    write_to_user(buffer, test, strlen(test), client_fd, buflen);
    fclose(in);
}

/** Method to ecapsulate doing all the server request work exactly once
 * What this means is that this method stops after we get a 'bye' from client
 * and closes client_fd
*/
void handle_server_requests_once(int client_fd) {
    int buflen = 200;
    char buffer[buflen];

    write_to_user(buffer, start_message, strlen(start_message), client_fd, buflen);
    read_user_content_to_buffer(buffer, client_fd, buflen);

    //Keep running until I get 'bye' or BYE in the first 3 letters from user
    while(strncmp(buffer, "bye", 3) != 0 && strncmp(buffer, "BYE", 3) != 0) {
        
        if(strncmp(buffer, "get", 3) == 0 || strncmp(buffer, "GET", 3) == 0) {
            handle_get_response(buffer, client_fd, buflen);
        }
        else if(strncmp(buffer, "put", 3) == 0 || strncmp(buffer, "PUT", 3) == 0) {
            handle_put_request(buffer, client_fd, buflen);
        }
        else {
            write_to_user(buffer, server_502_message, strlen(server_502_message), client_fd, buflen);
        }
        read_user_content_to_buffer(buffer, client_fd, buflen);
    }

    close(client_fd);
    printf("Closed connection!\n");
    exit(0);
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

    //WHILE LOOP WAS HERE

    while(1) {

        //Accept client connection
        struct sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        int client_fd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
        if(client_fd < 0) {error("Error accepting client. ");}

        int pid;
        pid = fork();

        if(pid == -1) {
            /** Unsuccessful fork*/
            int buflen = 200;
            char buffer[buflen];
            write_to_user(buffer, start_message, strlen(start_message), client_fd, buflen);
            read_user_content_to_buffer(buffer, client_fd, buflen);
        }
        else if(pid == 0) {
            /** CHILD PROCESS */
            handle_server_requests_once(client_fd);
            exit(0);
        }
        else{
            /** PARENT PROCESS */
            close(client_fd);
        }

    }
    return 0;
}


