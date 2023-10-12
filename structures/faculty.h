#ifndef FACULTY_RECORD
#define FACULTY_RECORD

struct Faculty{
    char name[20];
    char dept[20];
    char desig[20];
    int login_id;
    char password[10];
    char email[30];
};

#endif