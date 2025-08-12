#include "processor.h"

// Global variable declarations
char token;             // Currently processed character
int line = 1;           // Current source code line number (used for error reporting)
char* src;
FileNode fileNodes[10]; // Stores file information
int fileNodeMax = 10;
int fileNodeCounter = 9;
int fileCounter = 0;
char* fileNames[10];    // All file names

/* Token type enumeration
 * Notes:
 * [0-127] reserved for ASCII characters
 * 128+ for custom types
 */
enum {
    Num = 128,  // Numeric literal
    Fun, Sys, Glo, Loc, Id,          // Function, system call, global variable, local variable, identifier
    Char, Else, Enum, If, Int, Return, Sizeof, While,  // Keyword types
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak,  // Operators
    Var, Separator, Keyw, Other,  // Variable, separator, keyword, other
    Str // String
};

int keyWordsLength = 24;

// List of C language keywords
char* keywords[24] = {
    "if",       "then",   "while",  "do",     "static", "default",     "int", "double", "long",  "float", "char",
    "short",    "struct", "break",  "else",
    "switch",   "case",   "typedef", "return", "const", "continue",   "for", "void",   "sizeof"
};

// Read the next character from source and optionally write to output file
void nextChar(FILE* fd2, int putFile, struct Stats* stats)
{
    stats->total_size++;
    if (putFile && *src != '\0')
    {
        fputc(*src, fd2);
    }
    src++;
}

// Count the number of lines in a file
int count_file_lines(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;
    int lines = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++;
    }
    fclose(file);
    return lines + 1;
}

// Get the size of a file in bytes
size_t get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// File reading function
char* read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(size + 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }

    fread(buf, 1, size, fp);
    buf[size] = '\0';
    fclose(fp);
    return buf;
}



/* Core function of lexical analysis
 * Purpose: Extract the next token from the source code
 * Parameters: p - current node pointer
 * Returns: new node pointer (containing analysis result)
 */
struct node* next(char* pre,struct node* p, FILE* fd2,struct Stats* stats) {
    int key = 0, type = 0, i = 0, j = 0;
    char value[1024];         // Stores the current token string
    int outWhile = 0;       // Loop exit flag

    memset(value, 0, 20);  // Initialize value array


    // Main processing loop (skip whitespace/comments, extract valid tokens)
    while (1 && !outWhile) {
        token = *src;
        nextChar(fd2,0,stats);
        switch (token)
        {
        case '\n':
            // Handle newline character, increment current line number
            ++line;
            fputc(token, fd2);
            break;
        case '\t':
            fputc(token, fd2);
            break;
        case ' ':
            fputc(token, fd2);
            if (*src == ' ') {
                nextChar(fd2,1,stats);
            }
            break;
        case '#':
            {
                char* p = src;
                // Skip spaces and tabs
                while (*p == ' ' || *p == '\t') p++;

                // Check if it's an include directive
                if (strncmp(p, "include", 7) == 0) {
                    p += 7;
                    // Check if followed by space/tab
                    if (!(*p == ' ' || *p == '\t')) goto process_other_macro;
                
                    // Skip whitespace
                    while (*p == ' ' || *p == '\t') p++;
                
                    // Get file type
                    char quote = *p;
                    if (quote != '"' && quote != '<') goto process_other_macro;
                
                    // Find filename boundaries
                    char* start = ++p;
                    char* end = (quote == '"') ? strchr(p, '"') : strchr(p, '>');
                    if (!end) goto process_other_macro;
                
                    // Extract filename
                    char filename[100];
                    int len = end - start;
                    strncpy(filename,start,len);
                    // Construct full path
                    filename[len] = '\0';
                    char path[100];
                    int i = 0;
                    while (1)
                    {
                        path[i] = pre[i];
                        if (pre[i] == '\0')
                            break;
                        i++;
                    }
                    int begin = i;
                    while (1)
                    {
                        path[i] = filename[i-begin];
                        if (filename[i-begin] == '\0')
                            break;
                        i++;
                    }
                    p = end + 1;
                    // Record file
                    fileNames[fileCounter++] = strdup(path);
                    // Save current file line info
                    fileNodes[fileNodeCounter-1].filename = fileNodes[fileNodeCounter+1].filename;
                    fileNodes[fileNodeCounter-1].startLine = fileNodes[fileNodeCounter+1].startLine;
                    fileNodes[fileNodeCounter-1].endLine = line;
                    fileNodes[fileNodeCounter + 1].includeFile = 1;

                    // Rebuild fileNode for included file
                    fileNodes[fileNodeCounter].filename = strdup(path);
                    fileNodes[fileNodeCounter].startLine = line;
                    fileNodes[fileNodeCounter].endLine = line + count_file_lines(path);


                    fileNodes[fileNodeCounter+1].startLine = fileNodes[fileNodeCounter].endLine;
                    fileNodes[fileNodeCounter + 1].endLine = fileNodes[fileNodeCounter + 1].endLine -
                        fileNodes[fileNodeCounter - 1].endLine+
                        fileNodes[fileNodeCounter].endLine - 1;

                    for (int i = fileNodeCounter+2; i < fileNodeMax; i++)
                    {
                        fileNodes[i].endLine = fileNodes[i].endLine - 
                            fileNodes[i].startLine + fileNodes[i - 1].endLine;
                        fileNodes[i].startLine = fileNodes[i-1].endLine;
                    }
                    for (int i = fileNodeCounter - 1; i < fileNodeMax; i++)
                    {
                        if (fileNodes[i].startLine == fileNodes[i].endLine)
                        {
                            fileNodeCounter++;
                            break;
                        }
                    }

                    fileNodeCounter-= 2;

                    // Read included file content
                    char* include_content = read_file(path);
                    if (!include_content) include_content = strdup("");
                
                    // Move to end of #include line
                    while (*p && *p != '\n') p++;
                    if (*p == '\n') p++;
                
                    // Create new src buffer
                    size_t content_len = strlen(include_content);
                    size_t remain_len = strlen(p);
                    char* new_src = malloc(content_len + remain_len + 2);
                    if (!new_src) {
                        free(include_content);
                        return NULL;  // Or handle error appropriately
                    }
                    memset(new_src, 0, content_len + remain_len + 2);
                    memcpy(new_src, include_content, content_len);
                    memcpy(new_src + content_len, "\n", 1);
                    memcpy(new_src + content_len + 1, p, remain_len + 1);
                    
                    // Update global state
                    free(include_content);
                    src = new_src;
                    stats->included_files++;
                }
                else {
                process_other_macro:
                    // Default macro handling
                    fputc(token, fd2);
                    while (*src && *src != '\n') nextChar(fd2, 1, stats);
                }
            }
            break;
        case '"':
            fputc(token, fd2);
            //Default macro handling
            while (*src != 0 && *src != '"') {
                nextChar(fd2,1,stats);
            }
            nextChar(fd2, 1, stats);
            
            break;
        case '/':
            if (*src == '/') {
                // skip comments
                while (*src != 0 && *src != '\n') {
                    nextChar(fd2,0,stats);
                }
                stats->comments_removed++;
            }
            else if (*src == '*') // multiple lines
            {
                char last = '0';

                while (*src != 0) {
                    if (*src == '/' && last == '*')
                    {
                        nextChar(fd2,0,stats);
                        break;
                    }
                    last = *src;
                    nextChar(fd2,0,stats);
                    if (last == '\n')
                    {
                        line++;
                        fputc(last, fd2);
                        stats->comments_removed++;
                    }
                }

                stats->comments_removed++;
                last = 1;
            }
            else {
                // divide operator
                key = 44;
                type = Div;
                value[0] = token;
                outWhile = 1;
            }
            break;
        case '+':
            fputc(token, fd2);
            // '+' and '++'
            if (*src == '+') {

                nextChar(fd2,1,stats);
                key = 45;
                type = Inc;
                value[0] = value[1] = '+';
                outWhile = 1;
                
            }
            else {
                key = 41;
                type = Add;
                value[0] = '+';
                outWhile = 1;
                
            }
            break;
        case '-':
            fputc(token, fd2);
            // '-' and '--'
            if (*src == '-') {

                nextChar(fd2,1,stats);
                key = 46;
                type = Dec;
                value[0] = value[1] = '-';
                outWhile = 1;
                
            }
            else {
                key = 42;
                type = Sub;
                value[0] = '-';
                outWhile = 1;
                
            }
            break;
        case '*':
            fputc(token, fd2);
            key = 43;
            type = Mul;
            value[0] = '*';
            outWhile = 1;
            
            break;
        case '=':
            fputc(token, fd2);
            // '==' and '='
            if (*src == '=') {

                nextChar(fd2,1,stats);
                type = Eq;
                value[0] = value[1] = '=';
                outWhile = 1;
            }
            else {
                key = 47;
                type = Assign;
                value[0] = '=';
                outWhile = 1;
            }
            
            break;
        case '!':
            //  '!='
            if (*src == '=') {

                nextChar(fd2,1,stats);
                key = 48;
                type = Ne;
                value[0] = '!';
                value[1] = '=';
                outWhile = 1;
            }
            
            break;
        case '<':
            //  '<=', '<<' or '<'
            if (*src == '=') {

                nextChar(fd2,1,stats);
                key = 49;
                type = Le;
                value[0] = '<';
                value[1] = '=';
                outWhile = 1;
            }
            else if (*src == '<') {

                nextChar(fd2,1,stats);
                key = 50;
                type = Shl;
                value[0] = value[1] = '<';
                outWhile = 1;
            }
            else {
                key = 51;
                type = Lt;
                value[0] = '<';
                outWhile = 1;
            }
            
            break;
        case '>':
            if (*src == '=') {

                nextChar(fd2,1,stats);
                key = 52;
                type = Ge;
                value[0] = '>';
                value[1] = '=';
                outWhile = 1;
            }
            else if (*src == '>') {

                nextChar(fd2,1,stats);
                key = 53;
                type = Shr;
                value[0] = value[1] = '>';
                outWhile = 1;
            }
            else {
                key = 54;
                type = Gt;
                value[0] = '>';
                outWhile = 1;
            }
            
            break;
        case '|':
            if (*src == '|') {

                nextChar(fd2,1,stats);
                key = 55;
                type = Lor;
                value[0] = value[1] = '|';
                outWhile = 1;
            }
            else {
                key = 56;
                type = Or;
                value[0] = '|';
                outWhile = 1;
            }
            
            break;
        case '&':
            fputc(token, fd2);
            // '&' and '&&'
            if (*src == '&') {

                nextChar(fd2,1,stats);
                key = 58;
                type = Lan;
                value[0] = value[1] = '&';
                outWhile = 1;
            }
            else {
                key = 59;
                type = And;
                value[0] = '&';
                outWhile = 1;
            }
            
            break;
        case '^':
            fputc(token, fd2);
            key = 60;
            type = Xor;
            value[0] = '^';
            outWhile = 1;
            
            break;
        case '%':
            fputc(token, fd2);
            key = 61;
            type = Mod;
            value[0] = '%';
            outWhile = 1;
            
            break;
        case '[':
            fputc(token, fd2);
            key = 62;
            type = Brak;
            value[0] = '[';
            outWhile = 1;
            
            break;
        case '?':
            fputc(token, fd2);
            key = 63;
            type = Cond;
            value[0] = '?';
            outWhile = 1;
            
            break;
        default:
            if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
                fputc(token, fd2);
                //  Parse identifier
                i = 0;
                value[i] = token;
                while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                    
                    value[i + 1] = *src;
                    nextChar(fd2,1,stats);
                    i++;
                }
                value[i + 1] = '\0';
                //Check if the identifier is a keyword
                for (j = 0; j < keyWordsLength; j++) {
                    if (strcmp(value, keywords[j]) == 0) {
                        key = j + 1;
                        type = Keyw;
                        break;
                    }
                }
                if (j >= keyWordsLength) {
                    key = 39;
                    type = Var;
                }
                outWhile = 1;
            }
            // Number
            // In ASCII, 'a' = 0x61 and 'A' = 0x41; using (token & 15) gives the unit digit value
            else if (token >= '0' && token <= '9') {

                fputc(token, fd2);
                int num;

                num = token - '0';
                value[0] = token;
                if (num > 0) {
                    for (i = 1; *src >= '0' && *src <= '9'; i++) {
                        value[i] = *src;
                        num = num * 10 + *src - '0';
                        nextChar(fd2,1,stats);
                    }
                }
                else {
                    num = '0';
                }
                key = 40;
                type = Num;
                outWhile = 1;
            }
            else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':' || token == '\\') {
                key = 64;
                type = Separator;
                value[0] = token;
                outWhile = 1;
                fputc(token, fd2);
            }
            else if (token == 0)
            {
                outWhile = 1;
            }
            else {
                //printf("Error,error in %d!\n", line);
                key = 65;
                type = Other;
                value[0] = token;
                outWhile = 1;
            }
            break;
        }

        if (outWhile)
            break;
    }
    p->key = key;
    p->line = line;
    strcpy(p->value, value);

    p->type = type;
    node* q = (node*)malloc(sizeof(node));
    q->next = NULL;
    p->next = q;
    return p;
}

// Function to check if a variable name is valid
int is_valid_identifier(const char* name) {
    if (name == NULL || *name == '\0') {
        return 0; // Empty string is invalid
    }
    // First character must be a letter or underscore
    if (!isalpha(name[0]) && name[0] != '_') {
        return 0;
    }
    // Remaining characters must be letters, digits, or underscores
    for (int i = 1; name[i] != '\0'; i++) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return 0;
        }
    }
    return 1;
}

// Determine whether a token sequence is valid according to rules
int judgeValid(struct node* last, struct node* cur,struct Stats* stats)
{
    if ((last->type == Keyw && (last->key > 6 && last->key < 13)))// ±‰¡øø™Õ∑
    {
        if (strcmp(cur->value, "main") == 0)
            return 1;
        if (cur->type == Keyw)// Keyword followed by another keyword is not allowed
            return 0;
        if (cur->type == Separator)// type followed by ']', etc., is allowed
            return 1;
        if (cur->type == Var)// Type followed by variable is allowed
        {
            if (strcmp(cur->next->value, "-") == 0 && strcmp(cur->next->next->value, "=") != 0)
            {
                return 1;
            }
            
            stats->variables_checked++;
        }
        else if (cur->type == Mul)// Pointer declaration (e.g., int *ptr)
        {
            // Skip all * (multiple pointers)
            while (cur->type == Mul)
            {
                cur = cur->next;
            }
            if (cur->type == Var)
            {
                if (cur->next->type == Assign || strcmp(cur->next->value, ";") == 0 || strcmp(cur->next->value, ",") == 0 || strcmp(cur->next->value, "[") == 0)
                {
                
                    stats->variables_checked++;
                }
                else
                {
                    add_Error(stats, cur,0);
                    return 0;
                }
            }
            else
                return 0;

            cur = cur->next;

            // Handle comma-separated variable declarations
            while (strcmp(cur->value, ";") != 0 && strcmp(cur->value, "=") != 0)
            {
                if (strcmp(cur->value, ",") != 0)
                {
                    judgeValid(last, cur,stats);
                    while (strcmp(cur->value, ";") != 0 && strcmp(cur->value, ",") != 0)
                        cur = cur->next;
                }
                if(strcmp(cur->value, ";") != 0)
                    cur = cur->next;
            }
        }
        else
            return 0;
    }
    else if (last->type == Var)
    {
        if (cur->type == Var)
            return -1; // Two variable tokens together is ambiguous or invalid
        else if (strcmp(cur->value, "-") == 0 && strcmp(cur->next->value,"=") != 0)
        {
            return 0; // Subtraction-like pattern, not an assignment
        }
    }

    return 1; // Default to valid
}

/*
 * Function: add_Error
 * -------------------
 * Records an error related to an invalid identifier or token during lexical analysis.
 * Determines the file and adjusted line number by referencing the fileNodes array.
 * Dynamically resizes the error_list in the Stats structure if capacity is exceeded.
 * Logs the filename, line number, and erroneous token value into the error list.
 *
 * Parameters:
 *   stats   - Pointer to the Stats structure where errors are logged.
 *   pointer - Pointer to the node representing the erroneous token.
 *   count   - Flag indicating whether to increment the variables_checked count.
 *
 * Behavior:
 *   - Locates the corresponding file block for the given token.
 *   - Adjusts line number based on file includes or splits.
 *   - Appends a new error entry with filename, line, and token string.
 *   - Increments variables_checked if count == 0.
 */

void add_Error(Stats* stats, struct node* pointer, int count) {
    int lineCounter = 0;

    // Locate the file node that matches the current line
    for (int i = fileNodeCounter + 1; i < fileNodeMax; i++) {
        if (pointer->line >= fileNodes[i].startLine && pointer->line <= fileNodes[i].endLine) {

            // Resize error list if needed
            if (stats->errors >= stats->error_capacity) {
                stats->error_capacity = stats->error_capacity == 0 ? 4 : stats->error_capacity * 2;
                stats->error_list = realloc(stats->error_list, stats->error_capacity * sizeof(ErrorInfo));
            }

            // Record error metadata
            stats->error_list[stats->errors].filename = strdup(fileNodes[i].filename);
            stats->error_list[stats->errors].message = strdup(pointer->value);

            // Count total lines from previous file blocks with the same filename
            for (int j = fileNodeCounter + 1; j < i; j++) {
                if (strcmp(fileNodes[j].filename, fileNodes[i].filename) == 0)
                    lineCounter += fileNodes[j].endLine - 1;
            }

            // Adjust line number for include files
            if (fileNodes[i].includeFile == 0) {
                stats->error_list[stats->errors].line = lineCounter + pointer->line - fileNodes[i].startLine + 1;
            } else {
                stats->error_list[stats->errors].line = lineCounter + pointer->line - fileNodes[i].startLine + 2;
            }

            stats->errors++;

            // Track invalid variable name in stats if required
            if (count == 0) {
                stats->variables_checked++;
            }

            break;
        }
    }
}

void free_errors(Stats* stats) {
    for (int i = 0; i < stats->errors; i++) {
        free(stats->error_list[i].filename);
        free(stats->error_list[i].message);
    }
    free(stats->error_list);
    stats->error_list = NULL;
    stats->errors = 0;
    stats->error_capacity = 0;
}

/*
 * Function: process_file
 * ----------------------
 * Processes a source file by performing lexical analysis, extracting tokens,
 * and validating variable names according to specified rules.
 * Tracks statistics such as variables checked, errors detected, lines processed,
 * comments removed, and included files.
 *
 * Parameters:
 *   pre       - The prefix path or string used for handling includes.
 *   curPath   - The current file path being processed.
 *   head      - Pointer to the head node of the token linked list.
 *   fileSrc   - The source code content of the current file.
 *   fd2       - File descriptor used for outputting processed content.
 *   verbose   - Flag indicating whether to output detailed processing statistics.
 *
 * Behavior:
 *   - Initializes file tracking and statistics.
 *   - Sets up global source pointer to the file source code.
 *   - Iteratively calls the lexer to extract all tokens into a linked list.
 *   - Validates variable names and adds errors for invalid identifiers.
 *   - Outputs detailed statistics if verbose mode is enabled.
 */
void process_file(char* pre, char* curPath, struct node* head, char* fileSrc, FILE* fd2, int verbose) {
    Stats stats = { 0 };  // Initialize status tracking

    // Set file node boundaries
    fileNodes[fileNodeCounter].startLine = 1;
    fileNodes[fileNodeCounter].endLine = count_file_lines(curPath) + 1;
    fileNodes[fileNodeCounter].filename = strdup(curPath);
    if (!fileNodes[fileNodeCounter].filename) {
        perror("Failed to allocate memory for filename");
        return;
    }
    fileNodeCounter--;

    // Track filename - don't strdup again, use the same pointer
    fileNames[fileCounter] = fileNodes[fileNodeCounter + 1].filename;
    fileCounter++;

    // Set source code pointer
    src = fileSrc;

    // Begin tokenization
    struct node* pointer = head;
    pointer = next(pre, pointer, fd2, &stats);  // Initialize first token
    if (!pointer) {
        // Don't cleanup here - let the caller handle it
        return;
    }

    // Tokenize until end of file
    while (token > 0) {
        pointer = pointer->next;
        if (!next(pre, pointer, fd2, &stats)) {
            break;
        }
    }

    // Check variable naming rules
    struct node* last = head;
    pointer = head->next;
    while (pointer != NULL) {
        int judge = judgeValid(last, pointer, &stats);
        if (judge != 1) {
            add_Error(&stats, pointer, judge);
        }
        last = pointer;
        pointer = pointer->next;
    }

    stats.total_lines = line;

    // If verbose mode, print summary and error details
    if (verbose) {
        printf("Number of variables checked: %d\n", stats.variables_checked);
        printf("Number of errors detected: %d\n", stats.errors);
        
        if (stats.errors > 0) {
            printf("\nDetailed error information:\n");
            for (int i = 0; i < stats.errors; i++) {
                printf("File %s, Line %d: Invalid identifier '%s'\n", 
                       stats.error_list[i].filename,
                       stats.error_list[i].line,
                       stats.error_list[i].message);
            }
        }
        
        printf("\nNumber of comment lines removed: %d\n", stats.comments_removed);
        printf("Number of files included: %d\n", stats.included_files);
        printf("Total lines in output: %d\n", stats.total_lines);
        printf("Size of output: %ld\n", stats.total_size);

        for (int i = 0; i < fileCounter; i++) {
            printf("File: %s, Lines: %d, Size: %ld\n",
                   fileNames[i],
                   count_file_lines(fileNames[i]),
                   get_file_size(fileNames[i]));
        }
    }

    // Cleanup stats only (nodes and files cleaned up by caller)
    cleanup_stats(&stats);

}

// Modified helper functions:

void cleanup_stats(Stats* stats) {
    if (stats && stats->error_list) {
        for (int i = 0; i < stats->errors; i++) {
            free(stats->error_list[i].message);
            // Don't free filename here if it points to fileNodes
        }
        free(stats->error_list);
        stats->error_list = NULL;
        stats->errors = 0;
    }
}

