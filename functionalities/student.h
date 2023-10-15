// enroll to a course
// drop course
// view enrolled course details
//change pwd

#ifndef STUDENT_FUNCTIONS
#define STUDENT_FUNCTIONS

#include "./basic.h"

bool student_operation_handler(int connFD);

bool view_all_courses(int connFD);
bool enroll_to_new_course(int connFD);
bool drop_course(int connFD);
bool view_enrolled_course_details(int connFD);
// bool change_pwd(int connFD);s

struct Student loggedIn_student;
                

bool student_operation_handler(int connFD)
{

    if (login_handler(false,true, connFD, &loggedIn_student,NULL))
    {
        ssize_t writeBytes, readBytes;
        char readBuffer[500], writeBuffer[1000];
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Welcome");
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, STUDENT_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing STUDENT_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for STUDENT_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                view_all_courses(connFD);
                break;
            case 2:
                enroll_to_new_course(connFD);
                break;
            case 3:
                // drop_course(connFD);
                break;
            case 4:
                view_enrolled_course_details(connFD);
                break;
            case 5:
                // change_pwd(connFD);
                break;
            case 6:
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

bool view_all_courses (int connFD)
{
    ssize_t writeBytes, readBytes;            
    char writeBuffer[1000];

    if(loggedIn_student.isActive == 0){
        bzero(writeBuffer,sizeof(writeBuffer));
        strcpy(writeBuffer,"You are blocked by admin!\n Contact him to unblock\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing blocked message to client!");
            return false;
        }
        return false;
    }
    
    int courseFileDescriptor;

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        if(errno = ENOENT){
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, "No course exist!\n");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing no course exist message to client!");
                return false;
            }
            return false;
        }
        else{
            perror("Error opening student file");
            return false;
        }
        
    }
    
    int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    while(1){

        struct Course course;
        readBytes = read(courseFileDescriptor, &course, sizeof(struct Course)); 
        if(readBytes <= 0) break;
        if(course.isActive == false) continue;
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Course Details- \n\tCourse ID- %d\n\tName : %s\n\tDepartment : %s\n\tTotal Seats: %d\n\tCredits : %d\n\tAvailable Seats : %d\n\n", course.course_id, course.name, course.dept, course.total_seats, course.credits, course.available_seats);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing course info to client!");
            return false;
        }
    }

    if (readBytes == -1)
    {
        perror("Error reading course record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);
    close(courseFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing redirect msg to client!");
        return false;
    }

    return true;
}

bool enroll_to_new_course(int connFD){

    ssize_t readBytes, writeBytes;            
    char readBuffer[10], writeBuffer[500];

    int courseID;

    if(loggedIn_student.isActive == 0){
        strcpy(writeBuffer,"You are blocked by admin!\n Contact him to unblock\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing blocked message to client!");
            return false;
        }
        return false;
    }

    bool slot_empty = false;
    
    for(int i = 0; i < 4; i++){
        if(loggedIn_student.courses_enrolled[i] == -1 ){
            slot_empty = true;
            break;
        }
    }

    if(!slot_empty){
        bzero(write,sizeof(writeBuffer));
        strcpy(writeBuffer,"You have already taken all the courses, drop some to enroll\n");
        strcat(writeBuffer,"\nRedirecting to main menu");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing already full message to client!");
            return false;
        }
        return false;
    }
    
    int courseFileDescriptor;

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    courseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (courseFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "No course exist!");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing no course exist message to client!");
            return false;
        }
        return false;
    }
    
    int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    while(1){

        struct Course course;
        readBytes = read(courseFileDescriptor, &course, sizeof(struct Course)); 
        
        if(readBytes <= 0) break;
        if(course.isActive == false) continue;
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Course ID- %d\tName : %s\n", course.course_id, course.name);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing course info to client!");
            return false;
        }

    }

    if (readBytes == -1)
    {
        perror("Error reading course record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);
    bzero(writeBuffer,sizeof(writeBuffer));
    strcpy(writeBuffer, "\nEnter the course ID you want to enroll for..\n");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing enter course ID msg to client!");
        return false;
    }

    bzero(readBuffer,sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading client's choice for course to enroll in");
        return false;
    }

    courseID = atoi(readBuffer);

    lock.l_type = F_RDLCK;

    lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    struct Course course;
    int offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
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

    readBytes = read(courseFileDescriptor, &course, sizeof(struct Course)); 
    if(readBytes == -1){
        perror("Error in fetching the reqd course record");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);

    if(course.course_id != courseID){
        bzero(writeBuffer,strlen(writeBuffer));
        strcpy(writeBuffer,"The course doesn't exist\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing course doesn't exist message to client!");
            return false;
        }
        return false;
    }

    if(course.available_seats <= 0 ){
        bzero(writeBuffer,strlen(writeBuffer));
        strcpy(writeBuffer,"The course is already full\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing course full message to client!");
            return false;
        }
        return false;
    }

    bool already_enrolled = false;
    for (int i = 0; i < 4; i++)
    {
        if(loggedIn_student.courses_enrolled[i] == course.course_id){
            already_enrolled = true;
            break;
        }
    }

    if(already_enrolled ){
        bzero(writeBuffer,strlen(writeBuffer));
        strcpy(writeBuffer,"You are already enrolled\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while already enrolled message to client!");
            return false;
        }
        return false;
    }

    lock.l_type = F_WRLCK;

    struct flock lock2;
    lock2.l_type = F_WRLCK;
    lock2.l_start = 0;
    lock2.l_len = 0;
    lock2.l_whence = SEEK_SET;
    lock2.l_pid = getpid();

    int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1)
    {
        perror("Error in opening student file");
        return false;
    }

    lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    int lockingStatus2 = fcntl(studentFileDescriptor,F_SETLKW, &lock2);

    if(lockingStatus2 == -1 || lockingStatus == -1){
        perror("error in acquiring locks");
        return false;
    }

    course.available_seats = course.available_seats - 1;

    offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return false;
    }

    writeBytes = write(courseFileDescriptor, &course, sizeof(struct Course));
    if (writeBytes == -1)
    {
        perror("Error while reducing number of course seats");
        return false;
    }

    struct Student student;
    offset = lseek(studentFileDescriptor, (loggedIn_student.login_id) * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student)); 
    if(readBytes == -1){
        perror("Error in fetching the reqd student record");
        return false;
    }

    for(int i = 0; i < 4; i++){
        if(student.courses_enrolled[i] == -1){
            student.courses_enrolled[i] = courseID;
            break;
        }
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while pushing course in student record");
        return false;
    }


    lock.l_type = F_UNLCK;
    lock2.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLKW, &lock);
    fcntl(studentFileDescriptor, F_SETLKW, &lock2);

    close(studentFileDescriptor);
    close(courseFileDescriptor);

    bzero(writeBuffer,sizeof(writeBuffer));
    strcpy(writeBuffer,"Enrolled Successfully");

    strcat(writeBuffer, "\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing redirect msg to client!");
        return false;
    }
}

bool drop_course(int connFD){

    ssize_t readBytes, writeBytes;            
    char readBuffer[10], writeBuffer[500];

    int courseID,lockingStatus;

    if(loggedIn_student.isActive == 0){
        strcpy(writeBuffer,"You are blocked by admin!\n Contact him to unblock\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing blocked message to client!");
            return false;
        }
        return false;
    }

    
    int CourseFileDescriptor;

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    CourseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (CourseFileDescriptor == -1)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "No course exist!");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing no course exist message to client!");
            return false;
        }
        return false;
    }
    
    
    strcpy(writeBuffer, "\nEnter the course ID you want to drop ..\n");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing enter course ID msg to client!");
        return false;
    }

    bzero(readBuffer,sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading client's choice for course to drop");
        return false;
    }

    courseID = atoi(readBuffer);

    bool enrolled = false;
    for (int i = 0; i < 4; i++)
    {
        if(loggedIn_student.courses_enrolled[i] == courseID){
            enrolled = true;
            break;
        }
    }

    if(!enrolled){
        bzero(writeBuffer,strlen(writeBuffer));
        strcpy(writeBuffer,"You are not enrolled in this course");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing not enrolled message to client!");
            return false;
        }
        return false;
    }
    
    lock.l_type = F_WRLCK;

    lockingStatus = fcntl(CourseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    struct Course course;
    int offset = lseek(CourseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
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

    lockingStatus = fcntl(CourseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on course record!");
        return false;
    }


    readBytes = read(CourseFileDescriptor, &course, sizeof(struct Course)); 
    if(readBytes == -1){
        perror("Error in fetching the reqd course record");
        return false;
    }

    if(course.isActive == false){
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "course ID doesn't exist");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing course ID doesn't exist message to client!");
            return false;
        }
        lock.l_type = F_UNLCK;
        fcntl(CourseFileDescriptor, F_SETLK, &lock);
        close(CourseFileDescriptor);
        return false;

    }

    struct flock lock2;
    lock2.l_type = F_WRLCK;
    lock2.l_start = 0;
    lock2.l_len = 0;
    lock2.l_whence = SEEK_SET;
    lock2.l_pid = getpid();

    int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1)
    {
        perror("Error in opening student file");
        return false;
    }

    lockingStatus = fcntl(CourseFileDescriptor, F_SETLKW, &lock);
    int lockingStatus2 = fcntl(studentFileDescriptor,F_SETLKW, &lock2);

    if(lockingStatus2 == -1 || lockingStatus == -1){
        perror("error in acquiring locks");
        return false;
    }

    course.available_seats = course.available_seats + 1;

    offset = lseek(CourseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required course record!");
        return false;
    }

    writeBytes = write(CourseFileDescriptor, &course, sizeof(struct Course));
    if (writeBytes == -1)
    {
        perror("Error while increasing number of course seats");
        return false;
    }


    struct Student student;
    offset = lseek(studentFileDescriptor, (loggedIn_student.login_id) * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student)); 
    if(readBytes == -1){
        perror("Error in fetching the reqd student record");
        return false;
    }

    for(int i = 0; i < 4; i++){
        if(student.courses_enrolled[i] == courseID){
            student.courses_enrolled[i] = -1;
            break;
        }
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while removing course in student record");
        return false;
    }


    lock.l_type = F_UNLCK;
    lock2.l_type = F_UNLCK;
    fcntl(CourseFileDescriptor, F_SETLKW, &lock);
    fcntl(studentFileDescriptor, F_SETLKW, &lock2);

    close(studentFileDescriptor);
    close(CourseFileDescriptor);

    bzero(writeBuffer,sizeof(writeBuffer));
    strcpy(writeBuffer,"Dropped Successfully");

    strcat(writeBuffer, "\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing redirect msg to client!");
        return false;
    }

}

bool view_enrolled_course_details (int connFD)
{
    int readBytes, writeBytes;            
    char readBuffer[10], writeBuffer[1000];

    if(loggedIn_student.isActive == 0){
        strcpy(writeBuffer,"You are blocked by admin!\n Contact him to unblock\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing blocked message to client!");
            return false;
        }
        return false;
    }

    int arr[4];
    int cnt = 0;
    for (int i = 0; i < 4; i++)
    {
        if(loggedIn_student.courses_enrolled[i] != -1){
            arr[cnt++] = loggedIn_student.courses_enrolled[i];
        }
    }
    
    int CourseFileDescriptor;

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    CourseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (CourseFileDescriptor == -1)
    {
        perror("Error in opening the file");
        return false;
    }
    
    int lockingStatus = fcntl(CourseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the course file!");
        return false;
    }

    while(1){

        struct Course course;
        readBytes = read(CourseFileDescriptor, &course, sizeof(struct Course)); 
        
        if(readBytes <= 0) break;

        if(course.isActive == false) continue;

        bool isEnrolled = false;
        for(int i = 0; i < cnt; i++){
            if(course.course_id == arr[i]){
                isEnrolled = true;
                break;
            }
        }

        if(isEnrolled){
            bzero(writeBuffer, sizeof(writeBuffer));
            sprintf(writeBuffer, "Course Details- \n\tCourse ID- %d\n\tName : %s\n\tDepartment : %s\n\tTotal Seats: %d\n\tCredits : %d\n\tAvailable Seats : %d\n\n", course.course_id, course.name, course.dept, course.total_seats, course.credits, course.available_seats);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error writing course info to client!");
                return false;
            }

        }

    }

    if (readBytes == -1)
    {
        perror("Error reading course record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(CourseFileDescriptor, F_SETLK, &lock);
    close(CourseFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing redirect msg to client!");
        return false;
    }

    return true;
}

#endif