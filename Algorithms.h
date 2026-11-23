#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Course.h"
#include <cstddef>
#include <vector>

void mergeSort(std::vector<Course>& courses);

std::vector<Course> selectCourses(
    const std::vector<Course>& sortedCourses);

std::vector<Course> selectCoursesDP(
    const std::vector<Course>& sortedCourses);

// Optional extension: O(n log n) DP using binary search for predecessor p(j)
std::vector<Course> selectCoursesDPBinary(
    const std::vector<Course>& sortedCourses);

std::size_t optimalCountExhaustive(
    const std::vector<Course>& courses);

std::vector<std::vector<Course>> allocateClassrooms(
    const std::vector<Course>& courses);

std::size_t minClassroomsExhaustive(
    const std::vector<Course>& courses);

long long optimalEnrollmentsExhaustive(
    const std::vector<Course>& courses);

#endif // ALGORITHMS_H
