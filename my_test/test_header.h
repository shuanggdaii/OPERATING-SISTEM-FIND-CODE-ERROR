/*************************************************************
 * Header file for custom test
 * Contains function declarations and macros
 *************************************************************/

#ifndef TEST_HEADER_H
#define TEST_HEADER_H

// Include our utility functions
#include "test_utils.c"

// Define some macros
#define MAX_VALUE 100
#define MIN_VALUE 0
#define AVERAGE(a, b) ((a + b) / 2)

// Some invalid typedefs
typedef int valid_type;
typedef float 5invalid_type;   // Invalid: starts with a number
typedef char *string-ptr;      // Invalid: contains hyphen

// Function declarations
int add_numbers(int a, int b);
void print_message(const char* msg);

/* 
 * Multi-line comment with function documentation
 * This is just for testing comment removal
 */
 
// Structure definition with invalid member
struct test_struct {
    int valid_member;
    float invalid&member;   // Invalid: contains ampersand
    char name[50];
};

#endif /* TEST_HEADER_H */