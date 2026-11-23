#include "Algorithms.h"
#include "Course.h"
#include <cstddef>
#include <iostream>
#include <vector>

namespace {

long long sumEnrollments(const std::vector<Course>& courses) {
    long long total = 0;
    for (const Course& c : courses) {
        total += c.enrollments;
    }
    return total;
}

void printCourseLine(const Course& c) {
    std::cout << c.id << ' ' << c.start << ' ' << c.finish << ' ' << c.enrollments << '\n';
}

} // namespace

int main() {
    std::size_t n = 0;
    if (!(std::cin >> n)) {
        return 0;
    }

    std::vector<Course> originalCourses;
    originalCourses.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        Course c;
        std::cin >> c.id >> c.start >> c.finish >> c.enrollments;
        originalCourses.push_back(c);
    }

    std::vector<Course> sortedByFinish = originalCourses;
    mergeSort(sortedByFinish);

    std::vector<Course> countResult = selectCourses(sortedByFinish);
    std::vector<std::vector<Course>> roomsResult = allocateClassrooms(originalCourses);
    std::vector<Course> enrollmentsResult = selectCoursesDP(sortedByFinish);

    // 1. COUNT
    std::cout << "COUNT\n";
    std::cout << countResult.size() << ' ' << sumEnrollments(countResult) << '\n';
    for (const Course& c : countResult) {
        printCourseLine(c);
    }

    // 2. ROOMS
    std::cout << "ROOMS\n";
    std::cout << roomsResult.size() << '\n';
    for (std::size_t r = 0; r < roomsResult.size(); ++r) {
        for (const Course& c : roomsResult[r]) {
            std::cout << (r + 1) << ' ';
            printCourseLine(c);
        }
    }

    // 3. ENROLLMENTS
    std::cout << "ENROLLMENTS\n";
    std::cout << enrollmentsResult.size() << ' ' << sumEnrollments(enrollmentsResult) << '\n';
    for (const Course& c : enrollmentsResult) {
        printCourseLine(c);
    }

    return 0;
}
