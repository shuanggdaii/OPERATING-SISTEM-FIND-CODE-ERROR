#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdarg.h>
#ifdef _WIN32
#include <direct.h>
#define stat _stat
#else
#include <unistd.h>
#endif

#define SIZE 10240 




// Lexical token linked list node structure
typedef struct node {
    int key;        // Code (used to distinguish different types)
    int type;       // Type (corresponding to enum values)
    int line;       // Source code line number
    char value[1024]; // Lexical token string value
    struct node* next;  // Pointer to next node
} node;


// Variable name checking
typedef enum {
    NORMAL,
    SINGLE_LINE_COMMENT,  // Single-line comment
    MULTI_LINE_COMMENT,   // Multi-line comment
    STRING_LITERAL,
    CHAR_LITERAL,
    INCLUDE
} State;

typedef struct {
    char* filename;
    int line;
    char* message;
} ErrorInfo;

typedef struct Stats {
    int variables_checked;
    int errors;
    ErrorInfo* error_list;
    int error_capacity;
    int comments_removed;
    int included_files;
    int total_lines;
    size_t total_size;
    int output_lines;
    size_t output_size;
} Stats;

typedef struct FileNode {
    int startLine;
    int endLine;
    char* filename;
    int includeFile;  // Whether it includes a file
} FileNode;

// In processor.h
typedef struct {
    FILE* main_output;
    FILE* verbose_output;
    int duplicate_to_terminal;
} OutputController;


void add_Error(Stats* stats, struct node* pointer, int count);

// Entry point for syntax analysis, analyzing the entire C language program.
void process_file(char* pre, char* curPath, struct node* p, char* src, FILE* fd2, int verbose);


void cleanup_stats(Stats* stats);




