/*
============================================================================
Author : Deep Bansal
Description : This file contain the client code 
============================================================================
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h> 
#include <stdio.h>      
#include <unistd.h>     
#include <fcntl.h>
#include <string.h>

void connection_handler(int sockFD)
{
    char readBuffer[800], writeBuffer[800];
    int readBytes, writeBytes;            

    char tempBuffer[800];

    do
    {
        bzero(readBuffer, sizeof(readBuffer)); // Empty the read buffer
        bzero(tempBuffer, sizeof(tempBuffer));
        readBytes = read(sockFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
            perror("Error while reading from client socket!");
        else if (readBytes == 0)
            printf("Closing the connection to the server now!\n");
        else if (strchr(readBuffer, '^') != NULL)
        {
            strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 1);
            printf("%s\n", tempBuffer);
            writeBytes = write(sockFD, "^", strlen("^"));
            if (writeBytes == -1)
            {
                perror("Error while writing to client socket!");
                break;
            }
        }
        else if (strchr(readBuffer, '$') != NULL)
        {
            strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 2);
            printf("%s\n", tempBuffer);
            printf("Closing the connection to the server now!\n");
            break;
        }
        else
        {
            bzero(writeBuffer, sizeof(writeBuffer));

            if (strchr(readBuffer, '#') != NULL)
                strcpy(writeBuffer, getpass(readBuffer));
            else
            {
                printf("%s\n", readBuffer);
                scanf("%[^\n]%*c", writeBuffer);
            }

            writeBytes = write(sockFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing to client socket!");
                printf("Closing the connection to the server now!\n");
                break;
            }
        }
    } while (readBytes > 0);

    close(sockFD);
}

int main()
{
    struct sockaddr_in address; 
    char buffer[100];

    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1)
    {
        perror("Error while creating socket!");
        return 0;
    }
    printf("Client side socket successfully created!\n");

    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(8088);

    int connect_status = connect(socket_descriptor, (struct sockaddr *)&address, sizeof(address));
    if (connect_status == -1)
    {
        perror("Error while connecting to server!");
        return 0;
    }
    printf("Client to server connection successfully established!\n");

    connection_handler(socket_descriptor);
    close(socket_descriptor);
}