#include "processor.h"
char* src1;          // Source code string pointer (points to the current processing position)

char* extract_path_prefix(const char* path) {
    if (path == NULL || *path == '\0') {
        return NULL; // Handle null pointer or empty string
    }

    // Find the position of the last separator (supporting cross-platform)
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last_sep = NULL;

    // Get the position of the last separator
    if (last_slash && last_backslash) {
        last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;
    }
    else if (last_slash) {
        last_sep = last_slash;
    }
    else {
        last_sep = last_backslash;
    }

    // If no separator is found, return an empty string
    if (!last_sep) {
        char* result = malloc(1);
        if (result) *result = '\0';
        return result;
    }

    // Calculate prefix length (including the separator)
    size_t prefix_len = last_sep - path + 1;

    // Allocate memory and copy the prefix
    char* prefix = malloc(prefix_len + 1); // +1 for the null terminator
    if (!prefix) {
        return NULL; // Memory allocation failed
    }

    strncpy(prefix, path, prefix_len);
    prefix[prefix_len] = '\0'; // Ensure null termination

    return prefix;
}

void free_list(struct node* head) {
    while (head != NULL) {
        struct node* temp = head;
        head = head->next;
        free(temp);
    }
}

int main(int argc, char* argv[]) {
    char* path = NULL;
    char* output_file = NULL;
    int verbose = 0;
    char* src1 = NULL;
    FILE* input = NULL;
    FILE* output = stdout;    // Default output
    FILE* terminal = NULL;   // For terminal output when needed
    struct node* head = NULL;
    char* prefix = NULL;
    int ret = EXIT_SUCCESS;

    // Argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in") == 0) {
            if (i + 1 < argc) path = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
            if (i + 1 < argc) output_file = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (!path) {
            path = argv[i];
        }
    }

    if (!path) {
        fprintf(stderr, "Input file required\n");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    // Setup output streams
    if (output_file) {
        output = fopen(output_file, "w");
        if (!output) {
            perror("Error opening output file");
            ret = EXIT_FAILURE;
            goto cleanup;
        }
    }

    // Allocate memory
    if (!(src1 = malloc(SIZE + 1))) {
        perror("Could not allocate memory for source area");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    if (!(input = fopen(path, "r"))) {
        perror("Error opening input file");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    if (!(head = malloc(sizeof(struct node)))) {
        perror("Could not allocate memory for node");
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    head->next = NULL;

    int bytes_read = fread(src1, 1, SIZE, input);
    if (bytes_read <= 0) {
        perror("Error reading input file");
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    src1[bytes_read] = '\0';

    prefix = extract_path_prefix(path);
    if (!prefix) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    // Check if we need terminal output
    int show_terminal = (!isatty(fileno(stdout)));

    if (show_terminal) {
        terminal = fopen("/dev/tty", "w");
        if (!terminal) {
            terminal = stderr;
        }
    }

    // Process the file
    process_file(prefix, path, head, src1, output, verbose);
    
    // If needed, show output on terminal too
    if (show_terminal && terminal) {
        process_file(prefix, path, head, src1, terminal, 0);
    }

cleanup:
    if (terminal && terminal != stderr) fclose(terminal);
    if (output && output != stdout) fclose(output);
    if (input) fclose(input);
    if (head) free_list(head);
    free(src1);
    free(prefix);

    return ret;
}
