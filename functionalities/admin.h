#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include "./basic.h"
bool admin_operation_handler(int connFD);

int add_student(int connFD);
int add_faculty(int connFD);
bool modify_student_info(int connFD);
bool modify_faculty_info(int connFD);
bool activate_student(int connFD);
bool block_student(int connFD);

bool admin_operation_handler(int connFD)
{

    if (login_handler(true,false, connFD, NULL,NULL))
    {
        ssize_t writeBytes, readBytes;
        char readBuffer[500], writeBuffer[1000];
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Welcome");
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, ADMIN_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                add_student(connFD);
                break;
            case 2:
                get_student_details(connFD, -1);
                break;
            case 3: 
                add_faculty(connFD);
                break;
            case 4:
                get_faculty_details(connFD,-1);
                break;
            case 5:
                activate_student(connFD);
                break;
            case 6:
                block_student(connFD);
                break;
            case 7:
                modify_student_info(connFD);
                break;
            case 8:
                modify_faculty_info(connFD);
                break;
            case 9:
                writeBytes = write(connFD, ADMIN_LOGOUT, strlen(ADMIN_LOGOUT));
                return false;
            default:
                writeBytes = write(connFD, ADMIN_LOGOUT, strlen(ADMIN_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

int add_student(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[500], writeBuffer[500];

    struct Student newStudent, previousStudent;
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));

    //student login id

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1 && errno == ENOENT)
    {
        newStudent.login_id = 0;
    }
    else if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return -1;
    }
    else
    {
        int offset = lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Student record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};
        int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Student record!");
            return false;
        }

        readBytes = read(studentFileDescriptor, &previousStudent, sizeof(struct Student));
        if (readBytes == -1)
        {
            perror("Error while reading Student record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(studentFileDescriptor, F_SETLK, &lock);

        close(studentFileDescriptor);

        newStudent.login_id = previousStudent.login_id + 1;
    }

    //student name

    sprintf(writeBuffer, "%s", ADMIN_ADD_STUDENT_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_NAME message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student name response from client!");
        return false;
    }

    strcpy(newStudent.name, readBuffer);

    //student age
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    strcpy(writeBuffer, ADMIN_ADD_STUDENT_AGE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_AGE message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student age response from client!");
        return false;
    }

    int studentAge = atoi(readBuffer);
    if (studentAge == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    newStudent.age = studentAge;


    sprintf(writeBuffer, "%d", newStudent.login_id);

    //student passsword
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));

    char password[500];
    strcpy(password,AUTOGEN_PASSWORD);
    strcpy(newStudent.password, password);

    //student email
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    sprintf(writeBuffer, "%s", ADMIN_ADD_STUDENT_EMAIL);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_EMAIL message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student email response from client!");
        return false;
    }

    strcpy(newStudent.email, readBuffer);

     //student address
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    sprintf(writeBuffer, "%s",ADMIN_ADD_STUDENT_ADDRESS);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_ADDRESS message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student address response from client!");
        return false;
    }

    strcpy(newStudent.address, readBuffer);

    //active status
    newStudent.isActive = 1;

    //enrolled courses
    for (int i = 0; i < 4; i++)
    {
        newStudent.courses_enrolled[i] = -1;
    }
    
    studentFileDescriptor = open(STUDENT_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (studentFileDescriptor == -1)
    {
        perror("Error while creating / opening student file!");
        return false;
    }
    writeBytes = write(studentFileDescriptor, &newStudent, sizeof(newStudent));
    if (writeBytes == -1)
    {
        perror("Error while writing Student record to file!");
        return false;
    }

    close(studentFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d\n%s%s\n", ADMIN_ADD_STUDENT_AUTOGEN_LOGIN, newStudent.login_id, ADMIN_ADD_STUDENT_AUTOGEN_PASSWORD, AUTOGEN_PASSWORD);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending student loginID and password to the client!");
        return false;
    }

    return newStudent.login_id;
}

int add_faculty(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[500], writeBuffer[500];

    struct Faculty newFaculty, previousFaculty;

    //faculty login id

    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1 && errno == ENOENT)
    {
        newFaculty.login_id = 0;
    }
    else if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return -1;
    }
    else
    {
        int offset = lseek(facultyFileDescriptor, -sizeof(struct Faculty), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Faculty record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};
        int lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Faculty record!");
            return false;
        }

        readBytes = read(facultyFileDescriptor, &previousFaculty, sizeof(struct Faculty));
        if (readBytes == -1)
        {
            perror("Error while reading Faculty record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(facultyFileDescriptor, F_SETLK, &lock);

        close(facultyFileDescriptor);

        newFaculty.login_id = previousFaculty.login_id + 1;
    }

    //faculty name

    sprintf(writeBuffer, "%s", ADMIN_ADD_FACULTY_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_NAME message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty name response from client!");
        return false;
    }

    strcpy(newFaculty.name, readBuffer);
    
    //faculty dept

    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    sprintf(writeBuffer, "%s", ADMIN_ADD_FACULTY_DEPT);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_DEPT message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty dept response from client!");
        return false;
    }

    strcpy(newFaculty.dept, readBuffer);

    //faculty desig
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    sprintf(writeBuffer, "%s", ADMIN_ADD_FACULTY_DESIG);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_DESIG message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty desig response from client!");
        return false;
    }

    strcpy(newFaculty.desig, readBuffer);

    sprintf(writeBuffer, "%d", newFaculty.login_id);

    //faculty email

    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    sprintf(writeBuffer, "%s", ADMIN_ADD_FACULTY_EMAIL);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_EMAIL message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty email response from client!");
        return false;
    }

    strcpy(newFaculty.email, readBuffer);


    //faculty passsword

    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    char hashedPassword[500];
    strcpy(hashedPassword,AUTOGEN_PASSWORD);
    strcpy(newFaculty.password, hashedPassword);

    facultyFileDescriptor = open(FACULTY_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while creating / opening Faculty file!");
        return false;
    }
    writeBytes = write(facultyFileDescriptor, &newFaculty, sizeof(newFaculty));
    if (writeBytes == -1)
    {
        perror("Error while writing Faculty record to file!");
        return false;
    }

    close(facultyFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d\n%s%s", ADMIN_ADD_FACULTY_AUTOGEN_LOGIN, newFaculty.login_id, ADMIN_ADD_FACULTY_AUTOGEN_PASSWORD, AUTOGEN_PASSWORD);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending faculty loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));

    return newFaculty.login_id;
}

bool modify_student_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[500], writeBuffer[500];

    struct Student student;

    int studentID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_ID, strlen(ADMIN_UPDATE_STUDENT_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_STUDENT_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading student ID from client!");
        return false;
    }

    studentID = atoi(readBuffer);

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Student ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Student ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    
    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (errno == EINVAL)
    {
        // Student record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Student ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Student ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    close(studentFileDescriptor);

    writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_MENU, strlen(ADMIN_UPDATE_STUDENT_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_STUDENT_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting student modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_NAME, strlen(ADMIN_UPDATE_STUDENT_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new name from client!");
            return false;
        }
        strcpy(student.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_AGE, strlen(ADMIN_UPDATE_STUDENT_AGE));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_AGE message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new age from client!");
            return false;
        }
        int updatedAge = atoi(readBuffer);
        if (updatedAge == 0)
        {
            // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            return false;
        }
        student.age = updatedAge;
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_EMAIL, strlen(ADMIN_UPDATE_STUDENT_EMAIL));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_EMAIL message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new email from client!");
            return false;
        }
        strcpy(student.email, readBuffer);
        break;
    case 4:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_ADDRESS, strlen(ADMIN_UPDATE_STUDENT_ADDRESS));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_ADDRESS message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new address from client!");
            return false;
        }
        strcpy(student.address, readBuffer);
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Invalid Menu Choice");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Invalid Menu Choice message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }

    studentFileDescriptor = open(STUDENT_FILE, O_WRONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }
    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on student record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while writing update student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);

    writeBytes = write(connFD, "Updated Successfully", strlen("Updated Successfully"));
    if (writeBytes == -1)
    {
        perror("Error while writing Updated Successfully message to client!");
        return false;
    }

    return true;
}

bool modify_faculty_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[500], writeBuffer[500];

    struct Faculty faculty;

    int facultyID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_ID, strlen(ADMIN_UPDATE_FACULTY_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_FACULTY_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading faculty ID from client!");
        return false;
    }

    facultyID = atoi(readBuffer);

    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "faculty ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing faculty ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    
    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
    if (errno == EINVAL)
    {
        // Faculty record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "faculty ID doesn't exist");
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

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on faculty record!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error while reading faculty record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    close(facultyFileDescriptor);

    writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_MENU, strlen(ADMIN_UPDATE_FACULTY_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_FACULTY_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting faculty modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_NAME, strlen(ADMIN_UPDATE_FACULTY_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_FACULTY_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new name from client!");
            return false;
        }
        strcpy(faculty.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_DEPT , strlen(ADMIN_UPDATE_FACULTY_DEPT));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_FACULTY_DEPT message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new department from client!");
            return false;
        }
        
        strcpy(faculty.dept,readBuffer);
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_EMAIL, strlen(ADMIN_UPDATE_FACULTY_EMAIL));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_FACULTY_EMAIL message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new email from client!");
            return false;
        }
        strcpy(faculty.email, readBuffer);
        break;
    case 4:
        writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_DESIG, strlen(ADMIN_UPDATE_FACULTY_DESIG));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_FACULTY_DESIG message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new designation from client!");
            return false;
        }
        strcpy(faculty.desig, readBuffer);
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Invalid Menu Choice");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Invalid Menu Choice message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }

    facultyFileDescriptor = open(FACULTY_FILE, O_WRONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }
    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on faculty record!");
        return false;
    }

    writeBytes = write(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (writeBytes == -1)
    {
        perror("Error while writing update faculty info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLKW, &lock);

    close(facultyFileDescriptor);

    writeBytes = write(connFD, "Updated Successfully", strlen("Updated Successfully"));
    if (writeBytes == -1)
    {
        perror("Error while writing Updated Successfully message to client!");
        return false;
    }

    return true;
}

bool activate_student(int connFD){
    ssize_t readBytes, writeBytes;            
    char readBuffer[10], writeBuffer[100];

    struct Student student;
    int studentFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};

    writeBytes = write(connFD, "Enter the student ID of the student you want to activate", strlen("Enter the student ID of the student you want to activate"));
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

    int studentID = atoi(readBuffer);

    studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Student ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Student ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
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
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
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

    if(student.isActive == 1){
    writeBytes = write(connFD, "Student already active", strlen("Student already active"));
    if (writeBytes == -1)
    {
        perror("Error in sending already active message to client");
        return false;
    }

    close(studentFileDescriptor);
    return false;
    }
    
    student.isActive = 1;

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on student record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while writing update student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);

    writeBytes = write(connFD, "Activated Successfully", strlen("Activated Successfully"));
    if (writeBytes == -1)
    {
        perror("Error while writing Activated Successfully message to client!");
        return false;
    }

    return true;
}


bool block_student (int connFD){
    ssize_t readBytes, writeBytes;            
    char readBuffer[10], writeBuffer[100];

    struct Student student;
    int studentFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};

    writeBytes = write(connFD, "Enter the student ID of the student you want to block", strlen("Enter the student ID of the student you want to block"));
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

    int studentID = atoi(readBuffer);

    studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1)
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
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
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

    if(student.isActive == 0){
    writeBytes = write(connFD, "Student already blocked", strlen("Student already blocked"));
    if (writeBytes == -1)
    {
        perror("Error in sending already updated message to client");
        return false;
    }

    close(studentFileDescriptor);
    return false;
    }
    
    student.isActive = 0;

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on student record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while writing block student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);

    writeBytes = write(connFD, "Blocked Successfully", strlen("Blocked Successfully"));
    if (writeBytes == -1)
    {
        perror("Error while writing Blocked Successfully message to client!");
        return false;
    }

    return true;
}


#endif