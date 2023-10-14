#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main() {
    // Open a text file for writing
    int fileDescriptor = open("data.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    
    if (fileDescriptor == -1) {
        perror("Error opening the file");
        return 1;
    }

    // Data to be written
    char data[] = "Name: John Doe\nAge: 30\nEmail: john@example.com\n";
    
    // Write data to the file
    int bytesWritten = write(fileDescriptor, data, strlen(data));
    
    if (bytesWritten == -1) {
        perror("Error writing to the file");
        close(fileDescriptor);
        return 1;
    }

    // Close the file
    close(fileDescriptor);

    return 0;
}
