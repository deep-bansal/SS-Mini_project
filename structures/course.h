#ifndef COURSE_RECORD
#define COURSE_RECORD

struct Course{
    char name[20];
    char dept[20];
    int total_seats;
    int credits;
    int available_seats;
    int faculty_id;
    int course_id;

};

#endif