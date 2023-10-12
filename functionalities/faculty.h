#ifndef FACULTY_FUNCTIONS
#define FACULTY_FUNCTIONS

#include "./basic.h"

bool faculty_operation_handler(int connFD);

bool view_offering_courses(int connFD);
int add_new_course(int connFD);
// bool remove_course(int connFD);
// bool update_course_details(int connFD);
// bool change_pwd(int connFD);

struct Faculty loggedIn_faculty;
                

bool faculty_operation_handler(int connFD)
{

    if (login_handler(false,false, connFD, NULL,&loggedIn_faculty))
    {
        ssize_t writeBytes, readBytes;
        char readBuffer[1000], writeBuffer[1000];
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Welcome");
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, FACULTY_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing FACULTY_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for FACULTY_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                view_offering_courses(connFD);
                break;
            case 2:
                add_new_course(connFD);
                break;
            case 3:
                // remove_course(connFD);
                break;
            case 4:
                // update_course_details(connFD);
                break;
            case 5:
                // change_pwd(connFD);
                break;
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

int add_new_course(int connFD){
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Course newCourse, previousCourse;

    //course_id

    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1 && errno == ENOENT)
    {
        newCourse.course_id = 0;
    }
    else if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
        return -1;
    }
    else
    {
        int offset = lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last course record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
        int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on course record!");
            return false;
        }

        readBytes = read(courseFileDescriptor, &previousCourse, sizeof(struct Course));
        if (readBytes == -1)
        {
            perror("Error while reading course record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLK, &lock);

        close(courseFileDescriptor);

        newCourse.course_id = previousCourse.course_id + 1;
    }

    //course name

    sprintf(writeBuffer, "%s", FACULTY_ADD_COURSE_NAME);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_NAME message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }

    strcpy(newCourse.name, readBuffer);

    //course dept

    sprintf(writeBuffer, "%s", FACULTY_ADD_COURSE_DEPT);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_DEPT message to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }

    strcpy(newCourse.dept, readBuffer);

    //course total seats
    
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, FACULTY_ADD_COURSE_TOTAL_SEATS);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_TOTAL_SEATS message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading total seats response from client!");
        return false;
    }

    int totalseats = atoi(readBuffer);
    if (totalseats == 0)
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
    newCourse.total_seats = totalseats;

    //course credits
    
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, FACULTY_ADD_COURSE_CREDITS);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_CREDITS message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading credits response from client!");
        return false;
    }

    int credits = atoi(readBuffer);
    if (credits == 0)
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
    newCourse.credits = credits;

    //course availabel seats
    
    newCourse.available_seats = totalseats;

    //course faculty id
    newCourse.faculty_id = loggedIn_faculty.login_id;


    // sprintf(writeBuffer, "%d", newCourse.course_id);


    courseFileDescriptor = open(COURSE_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (courseFileDescriptor == -1)
    {
        perror("Error while creating / opening course file!");
        return false;
    }
    writeBytes = write(courseFileDescriptor, &newCourse, sizeof(newCourse));
    if (writeBytes == -1)
    {
        perror("Error while writing Course record to file!");
        return false;
    }

    close(courseFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d\n", "Course ID-", newCourse.course_id);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending course ID  to the client!");
        return false;
    }    

    return newCourse.course_id;

}


bool view_offering_courses (int connFD)
{
    ssize_t readBytes, writeBytes;            
    char readBuffer[10], writeBuffer[10000];
    char tempBuffer[1000];

    struct Course course;
    int CourseFileDescriptor;
    int courseID;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Course), getpid()};

    writeBytes = write(connFD, "Enter the course ID you're searching for", strlen("Enter the course ID you're searching for"));
    if (writeBytes == -1)
    {
        perror("Error while writing GET COURSE ID message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error getting course ID from client!");
        return false;
    }

    courseID = atoi(readBuffer);
    printf("%d",courseID);
    

    CourseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (CourseFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist");
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Course ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    int offset = lseek(CourseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist");
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Course ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(CourseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    readBytes = read(CourseFileDescriptor, &course, sizeof(struct Course));
    if (readBytes == -1)
    {
        perror("Error reading course record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(CourseFileDescriptor, F_SETLK, &lock);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Course Details - \n\tID : %d\n\tName : %s\n\tDepartment : %s\n\tTotal Seats: %d\n\tCredits : %d\n\tAvailable Seats : %d\n\t", course.course_id, course.name, course.dept, course.total_seats, course.credits, course.available_seats);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing course info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    return true;
}


#endif