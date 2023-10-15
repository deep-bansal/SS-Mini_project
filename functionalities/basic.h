#ifndef BASIC_FUNCTIONS
#define BASIC_FUNCTIONS
#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>   
#include <errno.h>     
#include "../structures/student.h"
#include "../structures/faculty.h"
#include "../structures/course.h"
#include "./admin_cred.h"
#include "./constants.h"

bool login_handler(bool isAdmin,bool isStudent, int connFD,struct Student *ptrToStudentID,struct Faculty *ptrToFacultyID)
{
    ssize_t readBytes, writeBytes;            
    char readBuffer[1000], writeBuffer[1000];
    struct Student student;
    struct Faculty faculty;

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type
    if (isAdmin)
    {
        const char* admin_welcome = "WELCOME ADMIN\nEnter Your Credentials to proceed\n";
        strcpy(writeBuffer, admin_welcome);
    }
    else if(isStudent){
        const char* student_welcome = "WELCOME STUDENT\nEnter Your Credentials to proceed\n";
        strcpy(writeBuffer, student_welcome);
    }
    else{
        const char* faculty_welcome = "WELCOME FACULTY\nEnter Your Credentials to proceed\n";
        strcpy(writeBuffer, faculty_welcome);
    }
        
    strcat(writeBuffer,"Enter Your Login ID");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing WELCOME & LOGIN_ID message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;

    if (isAdmin)
    {
        if (strcmp(readBuffer, ADMIN_LOGIN_ID) == 0)
            userFound = true;
    }
    else if(isStudent)
    {
        ID = atoi(readBuffer);

        int studentFD = open(STUDENT_FILE, O_RDONLY);
        if (studentFD == -1)
        {
            if(errno == ENOENT){
                bzero(writeBuffer,sizeof(writeBuffer));
                strcpy(writeBuffer,"No student is enrolled till now!");
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                if (writeBytes == -1)
                {
                    perror("Error while writing no student exist message to client!");
                    return false;
                }
                return false;
            }
            else{
                perror("Error opening student file in read mode!");
                return false;
            }
        }

        off_t offset = lseek(studentFD, ID * sizeof(struct Student), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Student), sizeof(struct Student), getpid()};

            int lockingStatus = fcntl(studentFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on student record!");
                return false;
            }

            readBytes = read(studentFD, &student, sizeof(struct Student));
            if (readBytes == -1)
            {
                perror("Error reading student record from file!");
                return 0;
            }

            lock.l_type = F_UNLCK;
            fcntl(studentFD, F_SETLK, &lock);



            if (student.login_id == atoi(readBuffer))
                userFound = true;

            close(studentFD);

        }
        else
        {
            writeBytes = write(connFD, "Student login ID doesn't exist", strlen("Student login ID doesn't exist"));
        }
    }
    else{
        ID = atoi(readBuffer);

        int facultyFD = open(FACULTY_FILE, O_RDONLY);
        if (facultyFD == -1)
        {
            if(errno == ENOENT){
                bzero(writeBuffer,sizeof(writeBuffer));
                strcpy(writeBuffer,"No faculty is present till now!");
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                if (writeBytes == -1)
                {
                    perror("Error while no faculty exist message to client!");
                    return false;
                }
                return false;
            }
            else{
                perror("Error opening faculty file in read mode!");
                return false;
            }
        }

        off_t offset = lseek(facultyFD, ID * sizeof(struct Faculty), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Faculty), sizeof(struct Faculty), getpid()};

            int lockingStatus = fcntl(facultyFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on faculty record!");
                return false;
            }

            readBytes = read(facultyFD, &faculty, sizeof(struct Faculty));
            if (readBytes == -1)
            {
                perror("Error reading faculty record from file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(facultyFD, F_SETLK, &lock);

            if (faculty.login_id == atoi(readBuffer))
                userFound = true;

            close(facultyFD);
        }
        else
        {
            writeBytes = write(connFD, "Faculty login ID doesn't exist", strlen("Faculty login ID doesn't exist"));
        }
    }

    if (userFound)
    {
        
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD,"Enter Your Password", strlen("Enter Your Password"));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == 1)
        {
            perror("Error reading password from the client!");
            return false;
        }

        char hashedPassword[1000];
        strcpy(hashedPassword,readBuffer);

        if (isAdmin)
        {
            if (strcmp(hashedPassword, ADMIN_PASSWORD) == 0)
                return true;
        }
        else if(isStudent)
        {
            if (strcmp(hashedPassword, student.password) == 0)
            {
                *ptrToStudentID = student;
                return true;
            }
        }
        else{
        
            if (strcmp(hashedPassword, faculty.password) == 0)
            {
                *ptrToFacultyID = faculty;
                return true;
            }

        }

        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, "Invalid Password", strlen("Invalid Password"));
    }
    else
    {
        writeBytes = write(connFD,"Inavlid login", strlen("Inavlid login"));
    }

    return false;
}

bool get_student_details(int connFD, int studentID)
{
    ssize_t readBytes, writeBytes;            
    char readBuffer[1024], writeBuffer[1024];

    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));

    struct Student student = {0};
    int studentFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};

    if (studentID == -1)
    {
        writeBytes = write(connFD, "Enter the student ID of the student you're searching for", strlen("Enter the student ID of the student you're searching for"));
        if (writeBytes == -1)
        {
            perror("Error while writing GET STUDENT ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error getting student ID from client!");
            return false;
        }

        studentID = atoi(readBuffer);
    }

    studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        if(errno == ENOENT){
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, "Student ID doesn't exist");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing Student ID doesn't exist message to client!");
                return false;
            }
            return false;
        }
        else{
            perror("error in opening the file");
            return false;
        }
    }
    int offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Student ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Student ID doesn't exist message to client!");
            return false;
        }
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the student file!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error reading student record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    if(student.login_id != studentID){
        bzero(writeBuffer,sizeof(writeBuffer));
        strcpy(writeBuffer,"Student doesn't exist!");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing student info to client!");
            return false;
        }
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Student Details - \n\tID : %d\n\tName : %s\n\tRoll No. : %d\n\tAge: %d\n\tEmail : %s\n\tAddress : %s\n\t", student.login_id, student.name, student.login_id, student.age, student.email, student.address);
    if(student.isActive){
        char* appendStatus = "Active Status: Active";
        strcat(writeBuffer,appendStatus);
    }
    else{
        char* appendStatus = "Active Status: Blocked";
        strcat(writeBuffer,appendStatus);
    }

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing student info to client!");
        return false;
    }

    return true;
}

bool get_faculty_details(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;             
    char readBuffer[1024], writeBuffer[1024]; 

    struct Faculty faculty = {0};
    int facultyFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};

    if (facultyID == -1)
    {
        writeBytes = write(connFD, "Enter the faculty ID of the faculty you're searching for", strlen("Enter the faculty ID of the faculty you're searching for"));
        if (writeBytes == -1)
        {
            perror("Error while writing GET faculty ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error getting faculty ID from client!");
            return false;
        }

        facultyID = atoi(readBuffer);
    }

    facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Faculty ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Faculty ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    int offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Faculty ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Faculty ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the faculty file!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error reading faculty record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    if(faculty.login_id != facultyID){
        bzero(writeBuffer,sizeof(writeBuffer));
        strcpy(writeBuffer,"Student doesn't exist!");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing student info to client!");
            return false;
        }
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Faculty Details - \n\tID : %d\n\tName : %s\n\tDepatment : %s\n\tDesignation : %s\n\tEmail : %s\n", faculty.login_id, faculty.name, faculty.dept, faculty.desig, faculty.email);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing faculty info to client!");
        return false;
    }

    return true;
}

#endif