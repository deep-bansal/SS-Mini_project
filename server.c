/*
============================================================================
Author : Deep Bansal
Description : This file contain the server code 
============================================================================
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "./functionalities/admin.h"
#include "./functionalities/faculty.h"


int socket_descriptor;

void handle_client(int client_socket) {

    char readbuffer[1000];
    const char* str = "Welcome to Couse Registration Portal! \nIdentify Yourself?\n1. Administrator\t2. Faculty\t3. Student\nPress any other number to exit";
    int writeCount = write(client_socket, str, strlen(str));
    if (writeCount == -1) {
        perror("Error while writing to network via socket!");
        close(client_socket);
        return;
    }
    bzero(readbuffer, sizeof(readbuffer));
    int readCount = read(client_socket, readbuffer, sizeof(readbuffer));
    if (readCount == -1) {
        perror("Error while reading from network via socket!");
        close(client_socket);
        return;
    }
    else if(readCount == 0){
        printf("No data received from client");
    }
    else{
        int userChoice = atoi(readbuffer);
            switch (userChoice)
            {
            case 1:
                // Admin
                admin_operation_handler(client_socket);
                break;
            case 2:
                // Faculty
                faculty_operation_handler(client_socket);
                break;
            case 3:
                //Student
                // student_operation_handler(client_socket);
                write(client_socket, "Student", strlen("Student"));
            default:
                // Exit
                break;
            }
        }
}

int main() {
    struct sockaddr_in address, client;
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socket_descriptor == -1) {
        perror("Error while creating socket!");
        return 1;
    }
    printf("Server side socket successfully created!\n");

    // socket creation done successfully

    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(8088);

    int bind_status = bind(socket_descriptor, (struct sockaddr *)&address, sizeof(address));
    if (bind_status == -1) {
        perror("Error while binding name to socket!");
        return 1;
    }
    printf("Binding to socket was successful!\n");

    // binding performed successfully

    int listen_status = listen(socket_descriptor, 2);
    if (listen_status == -1) {
        perror("Error while trying to listen for connections!");
        return 1;
    }
    printf("Now listening for connections on a socket!\n");

    while (1) {
        int client_size = sizeof(client);
        int connection_descriptor = accept(socket_descriptor, (struct sockaddr *)&client, &client_size);

        if (connection_descriptor == -1) {
            perror("Error while accepting a connection!");
            continue;
        }

        // Create a new process to handle the client connection
        pid_t pid = fork();

        if (pid == -1) {
            perror("Error while forking a process for the client connection");
            close(connection_descriptor);
            continue;
        }

        if (pid == 0) {
            // This is the child process
            close(socket_descriptor); // Close the listening socket in the child

            handle_client(connection_descriptor);

            return 0; 
        } else {
            // This is the parent process (listening)
            close(connection_descriptor); // Close the connection descriptor in the parent
        }
    }

    close(socket_descriptor);

    return 0;
}
