/*************************************************************
 * Main test file for the preprocessor
 * This file contains several invalid identifiers and various comments
 *************************************************************/

#include "test_header.h"

// Global variable declarations with some errors
int valid_var = 10;
float 3invalid_var = 3.14;  // Invalid: starts with a number
char *ptr-var;              // Invalid: contains hyphen

/* Multi-line comment
 * that spans several
 * lines of code
 */

int main(int argc, char *argv[]) {
    // Local variables with errors
    int x = 5;
    int y = 10;
    int if = 20;           // Invalid: uses keyword
    double result@calc;    // Invalid: contains @ symbol
    
    // Function calls
    printf("Testing preprocessor functionality\n");
    
    /* Nested comment /* testing */ handling */
    
    int result = add_numbers(x, y);
    printf("Result: %d\n", result);
    
    // Testing utility functions
    print_message("Hello from test_main.c");
    
    return 0;
}

// Helper function
void helper_function() {
    // This is a comment
    int local_var = 42;
    printf("Helper function called\n");
}