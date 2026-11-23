#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Algorithms.h"

#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

bool courseEquals(const Course& a, const Course& b) {
    return a.id == b.id && a.start == b.start &&
           a.finish == b.finish && a.enrollments == b.enrollments;
}

// Validates single-classroom output: non-overlapping, exact original records, no duplicate IDs
void verifySingleRoomSchedule(const std::vector<Course>& input,
                              const std::vector<Course>& selected) {
    std::set<std::string> seenIds;
    for (std::size_t i = 0; i < selected.size(); ++i) {
        CHECK(seenIds.insert(selected[i].id).second);

        bool foundInInput = false;
        for (const Course& orig : input) {
            if (courseEquals(orig, selected[i])) {
                foundInInput = true;
                break;
            }
        }
        CHECK(foundInInput);

        if (i > 0) {
            CHECK(selected[i - 1].finish <= selected[i].start);
        }
    }
}

// Validates multi-classroom partition: every input course appears once, no empty rooms, no overlaps
void verifyClassroomAllocation(const std::vector<Course>& input,
                               const std::vector<std::vector<Course>>& rooms) {
    if (input.empty()) {
        CHECK(rooms.empty());
        return;
    }

    std::set<std::string> assignedIds;
    std::size_t totalAssigned = 0;

    for (const auto& room : rooms) {
        REQUIRE(!room.empty());
        for (std::size_t i = 0; i < room.size(); ++i) {
            CHECK(assignedIds.insert(room[i].id).second);
            ++totalAssigned;

            bool foundInInput = false;
            for (const Course& orig : input) {
                if (courseEquals(orig, room[i])) {
                    foundInInput = true;
                    break;
                }
            }
            CHECK(foundInInput);

            if (i > 0) {
                CHECK(room[i - 1].finish <= room[i].start);
            }
        }
    }

    CHECK(totalAssigned == input.size());
}

long long totalEnrollments(const std::vector<Course>& courses) {
    long long sum = 0;
    for (const Course& c : courses) {
        sum += c.enrollments;
    }
    return sum;
}

} // namespace

// Exposes bugs when n = 0 (out-of-bounds access or non-empty default outputs)
TEST_CASE("Empty input edge case") {
    std::vector<Course> courses;
    mergeSort(courses);
    CHECK(courses.empty());

    auto countRes = selectCourses(courses);
    auto dpRes = selectCoursesDP(courses);
    auto dpBinRes = selectCoursesDPBinary(courses);
    auto roomsRes = allocateClassrooms(courses);

    CHECK(countRes.empty());
    CHECK(dpRes.empty());
    CHECK(dpBinRes.empty());
    CHECK(roomsRes.empty());
    CHECK(optimalCountExhaustive(courses) == 0);
    CHECK(minClassroomsExhaustive(courses) == 0);
    CHECK(optimalEnrollmentsExhaustive(courses) == 0);
}

// Exposes off-by-one bugs on single-element arrays
TEST_CASE("Single course input") {
    std::vector<Course> courses = {{"Algorithms", 540, 600, 45}};
    std::vector<Course> sorted = courses;
    mergeSort(sorted);

    auto countRes = selectCourses(sorted);
    auto dpRes = selectCoursesDP(sorted);
    auto dpBinRes = selectCoursesDPBinary(sorted);
    auto roomsRes = allocateClassrooms(courses);

    REQUIRE(countRes.size() == 1);
    CHECK(courseEquals(countRes[0], courses[0]));
    REQUIRE(dpRes.size() == 1);
    CHECK(courseEquals(dpRes[0], courses[0]));
    REQUIRE(dpBinRes.size() == 1);
    CHECK(courseEquals(dpBinRes[0], courses[0]));
    REQUIRE(roomsRes.size() == 1);
    REQUIRE(roomsRes[0].size() == 1);
    CHECK(courseEquals(roomsRes[0][0], courses[0]));
}

// Exposes strict inequality bugs (< instead of <=) on touching endpoints
TEST_CASE("Sample input and touching endpoints") {
    std::vector<Course> courses = {
        {"Algorithms", 540, 600, 4},
        {"Databases", 600, 660, 4},
        {"Operating_Systems", 540, 660, 11}
    };

    std::vector<Course> sorted = courses;
    mergeSort(sorted);

    auto countRes = selectCourses(sorted);
    REQUIRE(countRes.size() == 2);
    CHECK(countRes[0].id == "Algorithms");
    CHECK(countRes[1].id == "Databases");
    CHECK(totalEnrollments(countRes) == 8);

    auto roomsRes = allocateClassrooms(courses);
    REQUIRE(roomsRes.size() == 2);
    REQUIRE(roomsRes[0].size() == 2);
    CHECK(roomsRes[0][0].id == "Algorithms");
    CHECK(roomsRes[0][1].id == "Databases");
    REQUIRE(roomsRes[1].size() == 1);
    CHECK(roomsRes[1][0].id == "Operating_Systems");

    auto dpRes = selectCoursesDP(sorted);
    REQUIRE(dpRes.size() == 1);
    CHECK(dpRes[0].id == "Operating_Systems");
    CHECK(totalEnrollments(dpRes) == 11);
    CHECK(selectCoursesDPBinary(sorted).size() == 1);
}

// Exposes tie-breaking bugs in mergeSort, ROOMS room index tie-breaker, and DP reconstruction tie rule
TEST_CASE("Identical times and tie-breaking rules") {
    std::vector<Course> courses = {
        {"Databases", 540, 600, 5},
        {"Algorithms", 540, 600, 5},
        {"Operating_Systems", 600, 660, 2}
    };

    std::vector<Course> sorted = courses;
    mergeSort(sorted);
    REQUIRE(sorted.size() == 3);
    CHECK(sorted[0].id == "Algorithms");
    CHECK(sorted[1].id == "Databases");
    CHECK(sorted[2].id == "Operating_Systems");

    auto countRes = selectCourses(sorted);
    REQUIRE(countRes.size() == 2);
    CHECK(countRes[0].id == "Algorithms");
    CHECK(countRes[1].id == "Operating_Systems");

    auto roomsRes = allocateClassrooms(courses);
    REQUIRE(roomsRes.size() == 2);
    REQUIRE(roomsRes[0].size() == 2);
    CHECK(roomsRes[0][0].id == "Algorithms");
    CHECK(roomsRes[0][1].id == "Operating_Systems");
    REQUIRE(roomsRes[1].size() == 1);
    CHECK(roomsRes[1][0].id == "Databases");

    // In DP, Algorithms and Databases tie with 5 enrollments; tie rule excludes Databases
    auto dpRes = selectCoursesDP(sorted);
    REQUIRE(dpRes.size() == 2);
    CHECK(dpRes[0].id == "Algorithms");
    CHECK(dpRes[1].id == "Operating_Systems");
    CHECK(totalEnrollments(dpRes) == 7);
}

// Exposes bugs when all courses overlap pairwise
TEST_CASE("All courses mutually overlapping") {
    std::vector<Course> courses = {
        {"C1", 500, 600, 20},
        {"C2", 520, 580, 35},
        {"C3", 550, 650, 15},
        {"C4", 510, 590, 30}
    };
    std::vector<Course> sorted = courses;
    mergeSort(sorted);

    auto countRes = selectCourses(sorted);
    auto roomsRes = allocateClassrooms(courses);
    auto dpRes = selectCoursesDP(sorted);

    CHECK(countRes.size() == 1);
    CHECK(roomsRes.size() == 4);
    REQUIRE(dpRes.size() == 1);
    CHECK(dpRes[0].id == "C2");
    CHECK(totalEnrollments(dpRes) == 35);
}

// Exposes the buggy p[j] implementation that only checks index j - 1
TEST_CASE("Algorithmic critique test: p[j] requires looking past j - 1") {
    // Sorted by finish:
    // [0] Intro_CS:       [540, 600], enrollments = 30
    // [1] Data_Structures:[560, 620], enrollments = 20  (overlaps with [2])
    // [2] Linear_Algebra: [600, 660], enrollments = 25  (compatible with [0], NOT [1])
    std::vector<Course> courses = {
        {"Linear_Algebra", 600, 660, 25},
        {"Data_Structures", 560, 620, 20},
        {"Intro_CS", 540, 600, 30}
    };
    std::vector<Course> sorted = courses;
    mergeSort(sorted);

    // Buggy p[2] = -1 would yield max enrollment 30; true optimal is Intro_CS + Linear_Algebra = 55
    auto dpRes = selectCoursesDP(sorted);
    REQUIRE(dpRes.size() == 2);
    CHECK(dpRes[0].id == "Intro_CS");
    CHECK(dpRes[1].id == "Linear_Algebra");
    CHECK(totalEnrollments(dpRes) == 55);
    CHECK(selectCoursesDPBinary(sorted).size() == 2);
    CHECK(optimalEnrollmentsExhaustive(courses) == 55);
}

// Exposes subtle room-assignment bugs (must pick earliest-finishing free room, not first-fit)
TEST_CASE("Classroom reuse vs opening new room") {
    std::vector<Course> courses = {
        {"A", 480, 540, 10}, // Room 1: [480, 540]
        {"B", 500, 600, 10}, // Room 2: [500, 600]
        {"C", 550, 620, 10}, // Reuses Room 1 (540 <= 550) -> Room 1: [550, 620]
        {"D", 600, 660, 10}  // Reuses Room 2 (600 <= 600) -> Room 2: [600, 660]
    };

    auto roomsRes = allocateClassrooms(courses);
    verifyClassroomAllocation(courses, roomsRes);
    CHECK(roomsRes.size() == 2);
    CHECK(minClassroomsExhaustive(courses) == 2);
}

// Randomized property-based testing against exhaustive search (n <= 5, fixed seed 42)
TEST_CASE("Randomized exhaustive comparison for n <= 5 (seed = 42)") {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> startDist(480, 900);
    std::uniform_int_distribution<int> durDist(30, 180);
    std::uniform_int_distribution<long long> enrollDist(1, 100);

    for (int trial = 0; trial < 200; ++trial) {
        std::size_t n = static_cast<std::size_t>((trial % 5) + 1);
        std::vector<Course> courses;
        for (std::size_t i = 0; i < n; ++i) {
            int s = startDist(rng);
            int f = s + durDist(rng);
            courses.push_back({"Course_" + std::to_string(i), s, f, enrollDist(rng)});
        }

        std::vector<Course> sorted = courses;
        mergeSort(sorted);

        auto countRes = selectCourses(sorted);
        auto roomsRes = allocateClassrooms(courses);
        auto dpRes = selectCoursesDP(sorted);
        auto dpBinRes = selectCoursesDPBinary(sorted);

        verifySingleRoomSchedule(courses, countRes);
        verifySingleRoomSchedule(courses, dpRes);
        verifySingleRoomSchedule(courses, dpBinRes);
        verifyClassroomAllocation(courses, roomsRes);

        CHECK(countRes.size() == optimalCountExhaustive(courses));
        CHECK(roomsRes.size() == minClassroomsExhaustive(courses));
        CHECK(totalEnrollments(dpRes) == optimalEnrollmentsExhaustive(courses));
        CHECK(totalEnrollments(dpBinRes) == optimalEnrollmentsExhaustive(courses));

        // Same sorting order + same tie-breaker must produce identical lists
        REQUIRE(dpRes.size() == dpBinRes.size());
        for (std::size_t i = 0; i < dpRes.size(); ++i) {
            CHECK(courseEquals(dpRes[i], dpBinRes[i]));
        }
    }
}

// Larger random inputs (n = 25..60) checking invariants and DP vs DPBinary equivalence
TEST_CASE("Larger inputs invariant and binary DP consistency (seed = 1337)") {
    std::mt19937 rng(1337);
    std::uniform_int_distribution<int> startDist(400, 1200);
    std::uniform_int_distribution<int> durDist(15, 240);
    std::uniform_int_distribution<long long> enrollDist(5, 500);

    for (std::size_t n : {25, 40, 60}) {
        std::vector<Course> courses;
        for (std::size_t i = 0; i < n; ++i) {
            int s = startDist(rng);
            int f = s + durDist(rng);
            courses.push_back({"C_" + std::to_string(i), s, f, enrollDist(rng)});
        }

        std::vector<Course> sorted = courses;
        mergeSort(sorted);

        auto countRes = selectCourses(sorted);
        auto roomsRes = allocateClassrooms(courses);
        auto dpRes = selectCoursesDP(sorted);
        auto dpBinRes = selectCoursesDPBinary(sorted);

        verifySingleRoomSchedule(courses, countRes);
        verifySingleRoomSchedule(courses, dpRes);
        verifySingleRoomSchedule(courses, dpBinRes);
        verifyClassroomAllocation(courses, roomsRes);

        // COUNT must select at least as many courses as ENROLLMENTS
        CHECK(countRes.size() >= dpRes.size());
        // ENROLLMENTS must achieve at least as many enrollments as COUNT
        CHECK(totalEnrollments(dpRes) >= totalEnrollments(countRes));

        REQUIRE(dpRes.size() == dpBinRes.size());
        for (std::size_t i = 0; i < dpRes.size(); ++i) {
            CHECK(courseEquals(dpRes[i], dpBinRes[i]));
        }
    }
}
