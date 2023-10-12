#ifndef STUDENT_RECORD
#define STUDENT_RECORD

struct Student{
    char name[20];
    int login_id;
    char password[30];
    int age;
    char email[30];
    char address[30];
    bool isActive;
};
 #endif