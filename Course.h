#ifndef COURSE_H
#define COURSE_H

#include <string>

struct Course {
    std::string id;
    int start;
    int finish;
    long long enrollments;
};

#endif // COURSE_H
