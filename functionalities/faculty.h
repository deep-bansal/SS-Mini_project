#ifndef FACULTY_FUNCTIONS
#define FACULTY_FUNCTIONS

#include "./basic.h"

bool faculty_operation_handler(int connFD);

bool view_offering_courses(int connFD);
int add_new_course(int connFD);
bool remove_course(int connFD);
bool update_course_details(int connFD);
// bool change_pwd(int connFD);

struct Faculty loggedIn_faculty;
                

bool faculty_operation_handler(int connFD)
{
    if (login_handler(false,false, connFD, NULL,&loggedIn_faculty))
    {
        ssize_t writeBytes, readBytes;
        char readBuffer[500], writeBuffer[900];
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
            bzero(readBuffer,sizeof(readBuffer));
        
            switch (choice)
            {
            case 1:
                view_offering_courses(connFD);
                break;
            case 2:
                add_new_course(connFD);
                break;
            case 3:
                remove_course(connFD);
                break;
            case 4:
                update_course_details(connFD);
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
    char readBuffer[500], writeBuffer[500];

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
    bzero(readBuffer,sizeof(readBuffer));
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
    bzero(readBuffer,sizeof(readBuffer));
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
    
    newCourse.credits = credits;

    //course availabel seats
    
    newCourse.available_seats = totalseats;

    //course faculty id
    newCourse.faculty_id = loggedIn_faculty.login_id;


    //course Active status
    newCourse.isActive = true;

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
    char readBuffer[10], writeBuffer[500];

    struct Course course = {0};
    int courseFileDescriptor;
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

    courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist ");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Course ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    int offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist 2");
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

    int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
    if (readBytes == -1)
    {
        perror("Error reading course record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);
    close(courseFileDescriptor);

    if(course.faculty_id != loggedIn_faculty.login_id){
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "This course doesn't belong to you");
        strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing this course doesn't belong to you message to client!");
            return false;
        }
        return false;
    }

    if(course.isActive == false){
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "This course doesn't exist ");
        strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing this course doesn't exist message to client!");
            return false;
        }
        return false;

    }

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Course Details - \n\tID : %d\n\tName : %s\n\tDepartment : %s\n\tTotal Seats: %d\n\tCredits : %d\n\tAvailable Seats : %d\n\t", course.course_id, course.name, course.dept, course.total_seats, course.credits, course.available_seats);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing course info to client!");
        return false;
    }
    return true;
}

bool remove_course (int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[500], writeBuffer[500];

    struct Course course;

    writeBytes = write(connFD, FACULTY_COURSE_TO_DELETE, strlen(FACULTY_COURSE_TO_DELETE));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_COURSE_TO_DELETE to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course ID response from the client!");
        return false;
    }

    int courseID = atoi(readBuffer);

    int courseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (courseFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Course ID doesn't exist message to client!");
            return false;
        }
        close(courseFileDescriptor);
        return false;
    }


    int offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing Course ID doesn't exist message to client!");
            return false;
        }
        close(courseFileDescriptor);
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        close(courseFileDescriptor);
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
    int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error obtaining read lock on Course record!");
        return false;
    }

    readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
    if (readBytes == -1)
    {
        perror("Error while reading Course record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);


    if(course.isActive == false){
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "This course doesn't exist");
        strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing this course doesn't exist message to client!");
            return false;
        }
        return false;

    }

    bzero(writeBuffer, sizeof(writeBuffer));

    if (course.faculty_id == loggedIn_faculty.login_id)
    {
        course.isActive = false;

        offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
        if (offset == -1)
        {
            perror("Error while seeking to required course record!");
            return false;
        }

        lock.l_type = F_WRLCK;
        lock.l_start = offset;
        lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error while obtaining write lock on course record!");
            return false;
        }

        writeBytes = write(courseFileDescriptor, &course, sizeof(struct Course));
        if (writeBytes == -1)
        {
            perror("Error while writing update course info into file");
        }

        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLKW, &lock);
        bzero(writeBuffer,sizeof(writeBuffer));
        strcpy(writeBuffer,"Removed Successfully");
    }

    else{
        strcpy(writeBuffer, "Sorry! You can't remove this course");
    }

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error while writing final DEL message to client!");
        return false;
    }

    return true;
}


bool update_course_details(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[500], writeBuffer[500];

    struct Course course = {0};

    int courseID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, FACULTY_UPDATE_COURSE_ID, strlen(FACULTY_UPDATE_COURSE_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing FACULTY_UPDATE_COURSE_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading course ID from client!");
        return false;
    }

    courseID = atoi(readBuffer);

    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Course ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing course ID doesn't exist message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        return false;
    }
    
    offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (errno == EINVAL)
    {
        // Course record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "course ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing course ID doesn't exist message to client!");
            return false;
        }
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on course record!");
        return false;
    }

    readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
    if (readBytes == -1)
    {
        perror("Error while reading course record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);

    close(courseFileDescriptor);


    if(course.faculty_id == loggedIn_faculty.login_id){

    writeBytes = write(connFD, FACULTY_UPDATE_COURSE_MENU, strlen(FACULTY_UPDATE_COURSE_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing FACULTY_UPDATE_COURSE_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting course modification menu choice from client!");
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
        writeBytes = write(connFD, FACULTY_UPDATE_COURSE_NAME, strlen(FACULTY_UPDATE_COURSE_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_UPDATE_COURSE_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for course new name from client!");
            return false;
        }
        strcpy(course.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, FACULTY_UPDATE_COURSE_DEPT, strlen(FACULTY_UPDATE_COURSE_DEPT));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_UPDATE_COURSE_DEPT message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for course new dept from client!");
            return false;
        }
        strcpy(course.dept, readBuffer);
        break;
    case 3:
       writeBytes = write(connFD, FACULTY_UPDATE_COURSE_CREDITS, strlen(FACULTY_UPDATE_COURSE_CREDITS));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_UPDATE_COURSE_CREDITS message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for course new credits from client!");
            return false;
        }
        int updatedCredits = atoi(readBuffer);
        if (updatedCredits == 0)
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
        course.credits = updatedCredits;
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

    courseFileDescriptor = open(COURSE_FILE, O_WRONLY);
    if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
        return false;
    }
    offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on course record!");
        return false;
    }

    writeBytes = write(courseFileDescriptor, &course, sizeof(struct Course));
    if (writeBytes == -1)
    {
        perror("Error while writing update course info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLKW, &lock);
    bzero(writeBuffer,sizeof(writeBuffer));
    strcpy(writeBuffer,"Updated Successfully");
    }
    else{
        bzero(writeBuffer,sizeof(writeBuffer));
        strcpy(writeBuffer,"Sorry! You can't modify this course");

    }

    close(courseFileDescriptor);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error while writing Updated Successfully message to client!");
        return false;
    }

    return true;
}

#endif