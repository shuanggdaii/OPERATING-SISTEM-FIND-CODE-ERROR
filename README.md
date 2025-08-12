# OPERATING-SISTEM-FIND-CODE-ERROR
Develop an application that, given a file containing C code, processes it as follows:
1) resolves #include directives, that is, includes the contents of the files that are arguments to the #include directive;
2) checks for variables with invalid names, or invalid identifiers (e.g., x-ray, two&four, 5brothers).
3) removes all comments;
4) produces an output file containing the modified code, that is, the input file extended with the include statements and without comments.
5) produces the processing statistics reported in the "Specifications" section.

Assumptions
It can be assumed that:
1) files included with the #include directive are stored in the CWD.
2) the file containing the C code provided as input consists only of the main function code block and that there are no other functions.
3) all local variables are declared at the beginning of the main function on contiguous lines, and global variables are declared before the main function on contiguous lines.
4) the data types used in the variable declarations are correct.

Specifications
1) Input: The program requires three input parameters:
-i, --in (double-dash notation) to specify the input file
-o --out (double-dash notation) to specify the output file
-v, --verbose (double-dash notation) to output processing statistics.
The input file, i.e., the file containing the C code to be processed, is a required parameter and can be passed as an argument to the -i or --in option. For example,
myPreCompiler input_file_name.c
In this case, the file input_file_name.c will be processed by the program and the output will be sent to stdout (see the Output section).
myPreCompiler -i input_file_name.c
myPreCompile --in=input_file_name.c
In this case, the file input_file_name.c will be processed by the program, and the -o, --out options can be used to specify the output file (see the Output section).

2) Output: Two output modes must be provided. In the first, the name of the output file is specified as an input parameter to the program, as an argument to the -o, --out options. In the second, if the -o option is not used, the result of processing the input file is sent to stdout.
Execution Examples
myPreCompiler -i input_filename.c -o output_filename
At the end of the program, the file output_filename will contain the processed code.
myPreCompiler -i input_filename.c
myPreCompiler input_filename.c
Before terminating the program, it sends the processing results to stdout.
myPreCompiler -i input_filename.c > output_filename
Before terminating the program, it sends the processing results to stdout, which is redirected to the file output_filename.

The myPreCompiler program must also be able to return the following processing statistics to standard output:
number of variables checked
number of errors detected
for each error detected, the name of the file in which it was detected, and the line number in the file.
Number of comment lines removed
Number of files included
For the input file, the size in bytes and the number of lines (preprocessing)
For each included file, the size in bytes and the number of lines (preprocessing)
Number of lines and output size
The above output must be enabled/disabled using the -v, --verbose options. For example,
myPreCompiler -v -c input_filename.c -o output_filename
returns the above statistics to standard output, while
myPreCompiler -c input_filename.c -o output_filename
does not return the above statistics to standard output.

(Optional: Return processing statistics to standard error instead of standard output so you can redirect the processed file and processing statistics to two different files when selecting the second output mode.)

3) Errors
The developed application must handle the following errors:
error in input parameters, options, and related arguments specified on the command line
error opening input files or specified as an include argument
error closing file
error reading from file - for example, empty or corrupt file
error writing to file
4) Program Structure and Memory Usage
The program must not be monolotic but composed of a main program and various functions that perform the main functionality and support features.
The program must be organized into at least three files: a file containing the main program, at least one file containing the functions, and at least one header file.
When choosing data structures, dynamic memory allocation must be prioritized.
For the exam, the code must be compiled from the command line using gcc.
5) Testing
The program must be tested using
a pool of files provided by the instructor (see attached files - more will follow).
