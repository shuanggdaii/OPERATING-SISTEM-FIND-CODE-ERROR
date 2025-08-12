/*************************************************************
 * Utility functions for custom test
 * Contains function implementations
 *************************************************************/


// Global variables
int util_counter = 0;
double $invalid_price = 99.99;  // Invalid: starts with dollar sign

// Function to add two numbers
int add_numbers(int a, int b) {
    // Single line comment
    return a + b;  // Inline comment
}

/*
 * Function to print a message
 * with increment of counter
 */
void print_message(const char* msg) {
    // Increment counter
    util_counter++;
    
    // Local variable with invalid name
    int 2nd_counter = 0;  // Invalid: starts with a number
    
    // Print message with counter
    printf("[%d] %s\n", util_counter, msg);
}

/* Nested /* comment */ test */

// Another utility function with an invalid parameter name
void process_data(int valid_param, float *invalid^ptr) {  // Invalid: contains caret
    // Function body
    printf("Processing data...\n");
}
