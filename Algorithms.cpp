#include "Algorithms.h"

namespace {

// Comparator 1: finish ascending, then start ascending, then id ascending
bool compareByFinish(const Course& a, const Course& b) {
    if (a.finish != b.finish) {
        return a.finish < b.finish;
    }
    if (a.start != b.start) {
        return a.start < b.start;
    }
    return a.id < b.id;
}

// Comparator 2: start ascending, then finish ascending, then id ascending
bool compareByStart(const Course& a, const Course& b) {
    if (a.start != b.start) {
        return a.start < b.start;
    }
    if (a.finish != b.finish) {
        return a.finish < b.finish;
    }
    return a.id < b.id;
}

using CourseComparator = bool (*)(const Course&, const Course&);

void mergeRanges(std::vector<Course>& courses,
                 std::vector<Course>& temp,
                 std::size_t left,
                 std::size_t mid,
                 std::size_t right,
                 CourseComparator comp) {
    std::size_t i = left;
    std::size_t j = mid + 1;
    std::size_t k = left;

    while (i <= mid && j <= right) {
        // Use !comp(courses[j], courses[i]) to preserve stability on ties
        if (!comp(courses[j], courses[i])) {
            temp[k++] = courses[i++];
        } else {
            temp[k++] = courses[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = courses[i++];
    }

    while (j <= right) {
        temp[k++] = courses[j++];
    }

    for (std::size_t idx = left; idx <= right; ++idx) {
        courses[idx] = temp[idx];
    }
}

void mergeSortRecursive(std::vector<Course>& courses,
                        std::vector<Course>& temp,
                        std::size_t left,
                        std::size_t right,
                        CourseComparator comp) {
    if (left >= right) {
        return;
    }
    std::size_t mid = left + (right - left) / 2;
    mergeSortRecursive(courses, temp, left, mid, comp);
    mergeSortRecursive(courses, temp, mid + 1, right, comp);
    mergeRanges(courses, temp, left, mid, right, comp);
}

void mergeSortWithComparator(std::vector<Course>& courses, CourseComparator comp) {
    if (courses.size() <= 1) {
        return;
    }
    std::vector<Course> temp(courses.size());
    mergeSortRecursive(courses, temp, 0, courses.size() - 1, comp);
}

// Shared DP table calculation and solution reconstruction helper
std::vector<Course> solveWeightedIntervalDP(const std::vector<Course>& sortedCourses,
                                            const std::vector<int>& p) {
    if (sortedCourses.empty()) {
        return {};
    }

    const std::size_t n = sortedCourses.size();
    std::vector<long long> dp(n, 0);

    for (std::size_t j = 0; j < n; ++j) {
        long long prevBest = (p[j] != -1) ? dp[static_cast<std::size_t>(p[j])] : 0LL;
        long long includeVal = sortedCourses[j].enrollments + prevBest;
        long long excludeVal = (j > 0) ? dp[j - 1] : 0LL;
        dp[j] = (includeVal > excludeVal) ? includeVal : excludeVal;
    }

    // Reconstruct chosen courses: on ties (includeVal == excludeVal), exclude current course
    std::vector<Course> selected;
    int idx = static_cast<int>(n) - 1;
    while (idx >= 0) {
        std::size_t j = static_cast<std::size_t>(idx);
        long long prevBest = (p[j] != -1) ? dp[static_cast<std::size_t>(p[j])] : 0LL;
        long long includeVal = sortedCourses[j].enrollments + prevBest;
        long long excludeVal = (j > 0) ? dp[j - 1] : 0LL;

        if (includeVal > excludeVal) {
            selected.push_back(sortedCourses[j]);
            idx = p[j];
        } else {
            idx = idx - 1;
        }
    }

    // Reverse to chronological order
    std::size_t left = 0;
    std::size_t right = selected.size();
    while (left + 1 < right) {
        Course swapTmp = selected[left];
        selected[left] = selected[right - 1];
        selected[right - 1] = swapTmp;
        ++left;
        --right;
    }

    return selected;
}

bool areDisjoint(const Course& a, const Course& b) {
    return a.finish <= b.start || b.finish <= a.start;
}

void searchPartitions(const std::vector<Course>& courses,
                      std::size_t idx,
                      std::vector<std::vector<Course>>& rooms,
                      std::size_t& bestRooms) {
    if (rooms.size() >= bestRooms) {
        return; // Branch-and-bound pruning
    }
    if (idx == courses.size()) {
        bestRooms = rooms.size();
        return;
    }

    const Course& current = courses[idx];

    // Try placing current course into each already-opened room
    for (std::size_t r = 0; r < rooms.size(); ++r) {
        bool compatible = true;
        for (const Course& scheduled : rooms[r]) {
            if (!areDisjoint(scheduled, current)) {
                compatible = false;
                break;
            }
        }
        if (compatible) {
            rooms[r].push_back(current);
            searchPartitions(courses, idx + 1, rooms, bestRooms);
            rooms[r].pop_back();
        }
    }

    // Try opening a new canonical room (avoids symmetric room permutations)
    if (rooms.size() + 1 < bestRooms) {
        rooms.push_back({current});
        searchPartitions(courses, idx + 1, rooms, bestRooms);
        rooms.pop_back();
    }
}

} // namespace

void mergeSort(std::vector<Course>& courses) {
    mergeSortWithComparator(courses, compareByFinish);
}

std::vector<Course> selectCourses(const std::vector<Course>& sortedCourses) {
    if (sortedCourses.empty()) {
        return {};
    }

    std::vector<Course> selected;
    selected.push_back(sortedCourses[0]);
    int lastFinish = sortedCourses[0].finish;

    for (std::size_t i = 1; i < sortedCourses.size(); ++i) {
        if (sortedCourses[i].start >= lastFinish) {
            selected.push_back(sortedCourses[i]);
            lastFinish = sortedCourses[i].finish;
        }
    }

    return selected;
}

std::vector<Course> selectCoursesDP(const std::vector<Course>& sortedCourses) {
    if (sortedCourses.empty()) {
        return {};
    }

    const std::size_t n = sortedCourses.size();
    std::vector<int> p(n, -1);

    // Backward linear scan to find the latest compatible course before j
    for (std::size_t j = 1; j < n; ++j) {
        for (int i = static_cast<int>(j) - 1; i >= 0; --i) {
            if (sortedCourses[static_cast<std::size_t>(i)].finish <= sortedCourses[j].start) {
                p[j] = i;
                break;
            }
        }
    }

    return solveWeightedIntervalDP(sortedCourses, p);
}

std::vector<Course> selectCoursesDPBinary(const std::vector<Course>& sortedCourses) {
    if (sortedCourses.empty()) {
        return {};
    }

    const std::size_t n = sortedCourses.size();
    std::vector<int> p(n, -1);

    // Binary search over [0, j - 1] since finish times are non-decreasing
    for (std::size_t j = 1; j < n; ++j) {
        int low = 0;
        int high = static_cast<int>(j) - 1;
        int ans = -1;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (sortedCourses[static_cast<std::size_t>(mid)].finish <= sortedCourses[j].start) {
                ans = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        p[j] = ans;
    }

    return solveWeightedIntervalDP(sortedCourses, p);
}

std::vector<std::vector<Course>> allocateClassrooms(const std::vector<Course>& courses) {
    if (courses.empty()) {
        return {};
    }

    std::vector<Course> sorted = courses;
    mergeSortWithComparator(sorted, compareByStart);

    std::vector<std::vector<Course>> classrooms;
    std::vector<int> lastFinishTimes;

    for (const Course& course : sorted) {
        bool foundRoom = false;
        std::size_t bestRoom = 0;
        int earliestFinish = 0;

        // Linear scan for available room with earliest last finish time;
        // strict '<' preserves lowest room index on ties
        for (std::size_t r = 0; r < classrooms.size(); ++r) {
            if (lastFinishTimes[r] <= course.start) {
                if (!foundRoom || lastFinishTimes[r] < earliestFinish) {
                    foundRoom = true;
                    earliestFinish = lastFinishTimes[r];
                    bestRoom = r;
                }
            }
        }

        if (foundRoom) {
            classrooms[bestRoom].push_back(course);
            lastFinishTimes[bestRoom] = course.finish;
        } else {
            classrooms.push_back({course});
            lastFinishTimes.push_back(course.finish);
        }
    }

    return classrooms;
}

std::size_t optimalCountExhaustive(const std::vector<Course>& courses) {
    const std::size_t n = courses.size();
    if (n == 0) {
        return 0;
    }

    std::size_t bestCount = 0;
    const std::size_t totalSubsets = (1ULL << n);

    for (std::size_t mask = 1; mask < totalSubsets; ++mask) {
        bool valid = true;
        std::size_t count = 0;

        for (std::size_t i = 0; i < n && valid; ++i) {
            if ((mask & (1ULL << i)) == 0) {
                continue;
            }
            ++count;
            for (std::size_t j = i + 1; j < n; ++j) {
                if ((mask & (1ULL << j)) != 0 && !areDisjoint(courses[i], courses[j])) {
                    valid = false;
                    break;
                }
            }
        }

        if (valid && count > bestCount) {
            bestCount = count;
        }
    }

    return bestCount;
}

long long optimalEnrollmentsExhaustive(const std::vector<Course>& courses) {
    const std::size_t n = courses.size();
    if (n == 0) {
        return 0;
    }

    long long bestEnrollments = 0;
    const std::size_t totalSubsets = (1ULL << n);

    for (std::size_t mask = 1; mask < totalSubsets; ++mask) {
        bool valid = true;
        long long total = 0;

        for (std::size_t i = 0; i < n && valid; ++i) {
            if ((mask & (1ULL << i)) == 0) {
                continue;
            }
            total += courses[i].enrollments;
            for (std::size_t j = i + 1; j < n; ++j) {
                if ((mask & (1ULL << j)) != 0 && !areDisjoint(courses[i], courses[j])) {
                    valid = false;
                    break;
                }
            }
        }

        if (valid && total > bestEnrollments) {
            bestEnrollments = total;
        }
    }

    return bestEnrollments;
}

std::size_t minClassroomsExhaustive(const std::vector<Course>& courses) {
    if (courses.empty()) {
        return 0;
    }

    std::size_t bestRooms = courses.size();
    std::vector<std::vector<Course>> rooms;
    searchPartitions(courses, 0, rooms, bestRooms);
    return bestRooms;
}
