# Algorithms Assignment 1 — Course Classroom Scheduling

**Team Members:** <saeed azem>, <hussin othman> ,< mohmd abrahim >

## Project Structure
- `Course.h` — Defines the `Course` struct (`id`, `start`, `finish`, `enrollments`).
- `Algorithms.h` / `Algorithms.cpp` — Implements custom Merge Sort (two sorting orders), Greedy `selectCourses` (COUNT), Greedy `allocateClassrooms` (ROOMS), Dynamic Programming `selectCoursesDP` and optional `selectCoursesDPBinary` (ENROLLMENTS), and three exhaustive-search functions for testing.
- `main.cpp` — Reads input from `stdin` and prints `COUNT`, `ROOMS`, and `ENROLLMENTS` outputs.
- `tests.cpp` / `doctest.h` — Automated unit and property-based tests using `doctest` (single-header library, v2.4.11).
- `testdata/` — Baseline input/output files (`sample`, `empty`, `ties`).

## Build & Run Instructions (Tested with `g++ 13.2.0`, C++17)
g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
### Main Program
```bash
g++ -std=c++17 -Wall -Wextra main.cpp Algorithms.cpp -o courses
./courses < testdata/sample.in
