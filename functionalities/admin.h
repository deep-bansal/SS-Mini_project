#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include "./basic.h"

#include <crypt.h>

// activate Student
// block student
// modify student details
// modify faculty details

bool admin_operation_handler(int connFD);


// bool add_account(int connFD);
int add_student(int connFD);
int add_faculty(int connFD);
// bool delete_account(int connFD);
bool modify_student_info(int connFD);
bool modify_faculty_info(int connFD);

bool admin_operation_handler(int connFD)
{

    if (login_handler(true,false, connFD, NULL,NULL))
    {
        ssize_t writeBytes, readBytes;
        char readBuffer[1000], writeBuffer[1000];
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
            // case 5:
            //     activate_account(connFD);
            //     break;
            // case 6:
            //     block_student(connFD);
            //     break;
            case 7:
                modify_student_info(connFD);
                break;
            case 8:
                modify_faculty_info(connFD);
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

// bool add_account(int connFD)
// {
//     ssize_t readBytes, writeBytes;
//     char readBuffer[1000], writeBuffer[1000];

//     struct Account newAccount, prevAccount;

//     int accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
//     if (accountFileDescriptor == -1 && errno == ENOENT)
//     {
//         // Account file was never created
//         newAccount.accountNumber = 0;
//     }
//     else if (accountFileDescriptor == -1)
//     {
//         perror("Error while opening account file");
//         return false;
//     }
//     else
//     {
//         int offset = lseek(accountFileDescriptor, -sizeof(struct Account), SEEK_END);
//         if (offset == -1)
//         {
//             perror("Error seeking to last Account record!");
//             return false;
//         }

//         struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
//         int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
//         if (lockingStatus == -1)
//         {
//             perror("Error obtaining read lock on Account record!");
//             return false;
//         }

//         readBytes = read(accountFileDescriptor, &prevAccount, sizeof(struct Account));
//         if (readBytes == -1)
//         {
//             perror("Error while reading Account record from file!");
//             return false;
//         }

//         lock.l_type = F_UNLCK;
//         fcntl(accountFileDescriptor, F_SETLK, &lock);

//         close(accountFileDescriptor);

//         newAccount.accountNumber = prevAccount.accountNumber + 1;
//     }
//     writeBytes = write(connFD, ADMIN_ADD_ACCOUNT_TYPE, strlen(ADMIN_ADD_ACCOUNT_TYPE));
//     if (writeBytes == -1)
//     {
//         perror("Error writing ADMIN_ADD_ACCOUNT_TYPE message to client!");
//         return false;
//     }

//     bzero(readBuffer, sizeof(readBuffer));
//     readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
//     if (readBytes == -1)
//     {
//         perror("Error reading account type response from client!");
//         return false;
//     }

//     newAccount.isRegularAccount = atoi(readBuffer) == 1 ? true : false;

//     newAccount.owners[0] = add_customer(connFD, true, newAccount.accountNumber);

//     if (newAccount.isRegularAccount)
//         newAccount.owners[1] = -1;
//     else
//         newAccount.owners[1] = add_customer(connFD, false, newAccount.accountNumber);

//     newAccount.active = true;
//     newAccount.balance = 0;

//     memset(newAccount.transactions, -1, MAX_TRANSACTIONS * sizeof(int));

//     accountFileDescriptor = open(ACCOUNT_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
//     if (accountFileDescriptor == -1)
//     {
//         perror("Error while creating / opening account file!");
//         return false;
//     }

//     writeBytes = write(accountFileDescriptor, &newAccount, sizeof(struct Account));
//     if (writeBytes == -1)
//     {
//         perror("Error while writing Account record to file!");
//         return false;
//     }

//     close(accountFileDescriptor);

//     bzero(writeBuffer, sizeof(writeBuffer));
//     sprintf(writeBuffer, "%s%d", ADMIN_ADD_ACCOUNT_NUMBER, newAccount.accountNumber);
//     strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
//     writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
//     readBytes = read(connFD, readBuffer, sizeof(read)); // Dummy read
//     return true;
// }

int add_student(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Student newStudent, previousStudent;

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
        ;
        return false;
    }

    strcpy(newStudent.name, readBuffer);

    //student age
    
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, ADMIN_ADD_STUDENT_AGE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_AGE message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
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

    char hashedPassword[1000];
    strcpy(hashedPassword, crypt(AUTOGEN_PASSWORD, KEY_TO_CRYPT));
    strcpy(newStudent.password, hashedPassword);

    //student email
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
        ;
        return false;
    }

    strcpy(newStudent.email, readBuffer);

     //student address
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
        ;
        return false;
    }

    strcpy(newStudent.address, readBuffer);



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
    sprintf(writeBuffer, "%s%d\n%s%s", ADMIN_ADD_STUDENT_AUTOGEN_LOGIN, newStudent.login_id, ADMIN_ADD_STUDENT_AUTOGEN_PASSWORD, AUTOGEN_PASSWORD);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending student loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));

    return newStudent.login_id;
}

int add_faculty(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

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

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty name response from client!");
        ;
        return false;
    }

    strcpy(newFaculty.name, readBuffer);


    //faculty dept

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
        ;
        return false;
    }

    strcpy(newFaculty.dept, readBuffer);

      //faculty desig

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
        ;
        return false;
    }

    strcpy(newFaculty.desig, readBuffer);

    sprintf(writeBuffer, "%d", newFaculty.login_id);

    //faculty passsword

    char hashedPassword[1000];
    strcpy(hashedPassword, crypt(AUTOGEN_PASSWORD, KEY_TO_CRYPT));
    strcpy(newFaculty.password, hashedPassword);

    //faculty email
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
        ;
        return false;
    }

    strcpy(newFaculty.email, readBuffer);

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
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error sending faculty loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));

    return newFaculty.login_id;
}


// bool delete_account(int connFD)
// {
//     ssize_t readBytes, writeBytes;
//     char readBuffer[1000], writeBuffer[1000];

//     struct Account account;

//     writeBytes = write(connFD, ADMIN_DEL_ACCOUNT_NO, strlen(ADMIN_DEL_ACCOUNT_NO));
//     if (writeBytes == -1)
//     {
//         perror("Error writing ADMIN_DEL_ACCOUNT_NO to client!");
//         return false;
//     }

//     bzero(readBuffer, sizeof(readBuffer));
//     readBytes = read(connFD, readBuffer, sizeof(readBuffer));
//     if (readBytes == -1)
//     {
//         perror("Error reading account number response from the client!");
//         return false;
//     }

//     int accountNumber = atoi(readBuffer);

//     int accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
//     if (accountFileDescriptor == -1)
//     {
//         // Account record doesn't exist
//         bzero(writeBuffer, sizeof(writeBuffer));
//         strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
//         strcat(writeBuffer, "^");
//         writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
//         if (writeBytes == -1)
//         {
//             perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!");
//             return false;
//         }
//         readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
//         return false;
//     }


//     int offset = lseek(accountFileDescriptor, accountNumber * sizeof(struct Account), SEEK_SET);
//     if (errno == EINVAL)
//     {
//         // Customer record doesn't exist
//         bzero(writeBuffer, sizeof(writeBuffer));
//         strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
//         strcat(writeBuffer, "^");
//         writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
//         if (writeBytes == -1)
//         {
//             perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!");
//             return false;
//         }
//         readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
//         return false;
//     }
//     else if (offset == -1)
//     {
//         perror("Error while seeking to required account record!");
//         return false;
//     }

//     struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
//     int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
//     if (lockingStatus == -1)
//     {
//         perror("Error obtaining read lock on Account record!");
//         return false;
//     }

//     readBytes = read(accountFileDescriptor, &account, sizeof(struct Account));
//     if (readBytes == -1)
//     {
//         perror("Error while reading Account record from file!");
//         return false;
//     }

//     lock.l_type = F_UNLCK;
//     fcntl(accountFileDescriptor, F_SETLK, &lock);

//     close(accountFileDescriptor);

//     bzero(writeBuffer, sizeof(writeBuffer));
//     if (account.balance == 0)
//     {
//         // No money, hence can close account
//         account.active = false;
//         accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
//         if (accountFileDescriptor == -1)
//         {
//             perror("Error opening Account file in write mode!");
//             return false;
//         }

//         offset = lseek(accountFileDescriptor, accountNumber * sizeof(struct Account), SEEK_SET);
//         if (offset == -1)
//         {
//             perror("Error seeking to the Account!");
//             return false;
//         }

//         lock.l_type = F_WRLCK;
//         lock.l_start = offset;

//         int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
//         if (lockingStatus == -1)
//         {
//             perror("Error obtaining write lock on the Account file!");
//             return false;
//         }

//         writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
//         if (writeBytes == -1)
//         {
//             perror("Error deleting account record!");
//             return false;
//         }

//         lock.l_type = F_UNLCK;
//         fcntl(accountFileDescriptor, F_SETLK, &lock);

//         strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_SUCCESS);
//     }
//     else
//         // Account has some money ask customer to withdraw it
//         strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_FAILURE);
//     writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
//     if (writeBytes == -1)
//     {
//         perror("Error while writing final DEL message to client!");
//         return false;
//     }
//     readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

//     return true;
// }

bool modify_student_info(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

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
        strcat(writeBuffer, "^");
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
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on customer record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &customer, sizeof(struct Customer));
    if (readBytes == -1)
    {
        perror("Error while reading customer record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    close(studentFileDescriptor);

    writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_MENU, strlen(ADMIN_MOD_CUSTOMER_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_CUSTOMER_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting customer modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    { // A non-numeric string was passed to atoi
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_NEW_NAME, strlen(ADMIN_MOD_CUSTOMER_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new name from client!");
            return false;
        }
        strcpy(customer.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_NEW_AGE, strlen(ADMIN_MOD_CUSTOMER_NEW_AGE));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_AGE message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new age from client!");
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
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        customer.age = updatedAge;
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_NEW_GENDER, strlen(ADMIN_MOD_CUSTOMER_NEW_GENDER));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_GENDER message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new gender from client!");
            return false;
        }
        customer.gender = readBuffer[0];
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    studentFileDescriptor = open(CUSTOMER_FILE, O_WRONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening customer file");
        return false;
    }
    offset = lseek(studentFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on customer record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &customer, sizeof(struct Customer));
    if (writeBytes == -1)
    {
        perror("Error while writing update customer info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);

    writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_SUCCESS, strlen(ADMIN_MOD_CUSTOMER_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_CUSTOMER_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}

#endif