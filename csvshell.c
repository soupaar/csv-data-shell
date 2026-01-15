#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ROWS 1000
#define MAX_COLS 20
#define MAX_CELL_SIZE 256
#define MAX_LINE 4096

// Global table (Keeping state)
typedef struct 
{
    char data[MAX_ROWS][MAX_COLS][MAX_CELL_SIZE];
    char headers[MAX_COLS][MAX_CELL_SIZE];
    int num_rows;
    int num_cols;
} Table;

Table current_table = {0};

// Main loop funciton
void csv_loop(void);

/**
 * Read a line of input from the user.
 * @return a dynamically allocated string containing the user's input,
 *          or NULL on EOF (Ctrl+D)
 * Note: Caller must free() the returned string
 */
#define CSV_RL_BUFSIZE 1024
char *csv_read_line(void)
{
    int bufsize = CSV_RL_BUFSIZE;
    int position = 0;

    // Malloc means 'Memory Allocation'.
    // (Reserve a specific block of memory on the system's heap at runtime).
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    // Checks for allocation fails
    if(!buffer) {
        fprintf(stderr, "csv: allocation failure");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If we hit EOF or new line, replace with null char and return.
        if (c == EOF || c == '\n') {
        buffer[position] = '\0';
        return buffer;
        } else {
        buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += CSV_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            
            // Checks for allocation fails
            if (!buffer) {
                fprintf(stderr, "csv: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * Split a line into separate words/tokens.
 * @param line the line to split (will be modified by strtok)
 * @return a NULL-terminated array of strings (tokens)
 * Note: Caller must free() the returned array
 */
#define CSV_TOK_BUFSIZE 64
#define CSV_TOK_DELIM " \t\r\n"
char **csv_split_line(char *line)
{
    int bufsize = CSV_TOK_BUFSIZE;
    int position = 0;
    // ** denotes a pointer to a pointer of the data type.
    char **tokens = malloc(sizeof(char) * bufsize);
    // * denotes a pointer to a data type.
    char *token;

    if(!tokens) {
        fprintf(stderr, "csv: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, CSV_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += CSV_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "csv: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CSV_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// Function declarations for builtin shell commands:
int csv_load(char **args);
int csv_show(char **args);
int csv_filter(char **args);
int csv_sort(char **args);
int csv_save(char **args);
int csv_count(char **args);
int csv_columns(char **args);
int csv_help(char **args);
int csv_exit(char **args);

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = 
{
    "load",
    "show",
    "filter",
    "sort",
    "save",
    "count",
    "columns",
    "help",
    "exit"

};

// Array of function pointers
int (*builtin_func[]) (char **) = {
    &csv_load,
    &csv_show,
    &csv_filter,
    &csv_sort,
    &csv_save,
    &csv_count,
    &csv_columns,
    &csv_help,
    &csv_exit
};

int csv_num_builtins() 
{
    // Total size / size of one element = number of elements
    return sizeof(builtin_str) / sizeof(char *);
}

/**
 * Remove leading and trailing whitespace from a string.
 * @param str the string to trim (modified in place)
 */
void trim(char *str)
{
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
}

// Builtin function implementations.
/**
 * Load a CSV file into memory.
 * @param args command arguments where args[1] is the filename
 * @return 1 to continue shell loop
 */
int csv_load(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "csv: expected argument to \"load\"\n");
        return 1;
    }

    FILE *file = fopen(args[1], "r");
    if (!file) {
        perror("csv");
        return 1;
    }

    char line[MAX_LINE];

    // Read header
    if (fgets(line, MAX_LINE, file)) {
        line[strcspn(line, "\n")] = 0;
        char *token = strtok(line, ",");
        current_table.num_cols = 0;

        while (token != NULL && current_table.num_cols < MAX_COLS) {
            trim(token);
            strcpy(current_table.headers[current_table.num_cols], token);
            current_table.num_cols++;
            token = strtok(NULL, ",");
        }
    }

    // Read data rows
    current_table.num_rows = 0;
    while (fgets(line, MAX_LINE, file) && current_table.num_rows < MAX_ROWS) {
        line[strcspn(line, "\n")] = 0;
        char *token = strtok(line, ",");
        int col = 0;

        while (token != NULL && col < current_table.num_cols) {
            trim(token);
            strcpy(current_table.data[current_table.num_rows][col], token);
            col++;
            token = strtok(NULL, ",");
        }
        current_table.num_rows++;
    }

    fclose(file);
    printf("Loaded %d rows, %d columns\n", current_table.num_rows, current_table.num_cols);
    return 1;
}

/**
 * Displys the current table.
 * @param args command argumnts (not used)
 * @return 1 to continue shell loop
*/
int csv_show(char **args)
{
    int i;
    int max_rows;
    int row, col;

    // Check if there is any data loaded
    if (current_table.num_rows == 0) {
        printf("No data loaded. Use 'load <file.csv>' first.\n");
        return 1;
    }

    // Print column headers
    for (i = 0; i < current_table.num_cols; i++) {
        printf("%-15s", current_table.headers[i]);
    }
    printf("\n");

    // Print separator line
    for (i = 0; i < current_table.num_cols; i++) {
        printf("%-15s", "---------------");
    }
    printf("\n");

    // Decide how many rows to show (max 20)
    max_rows = current_table.num_rows > 20 ? 20 : current_table.num_rows;

    // Print data rows
    for (row = 0; row < max_rows; row++) {
        for (col = 0; col < current_table.num_cols; col++) {
            printf("%-15s", current_table.data[row][col]);
        }
        printf("\n");
    }

    if (current_table.num_rows > 20) {
        printf("...(%d more rows)\n", current_table.num_rows - 20);
    }
    return 1;
}

/**
 * Loop through current_table.headers
 * Compare each header with the column name using strcmp()
 * Return the index if found, return -1 if not found
 */
int find_column(char *column_name) {
    int i;  // Declare variable first
    
    // Loop through all columns
    for (i = 0; i < current_table.num_cols; i++) { 
        // Compare column_name with current_table.headers[i]
        if (strcmp(current_table.headers[i], column_name) == 0) { 
            return i;
        }
    }
    
    return -1; 
}

/**
 * Compare val1 and val2 using the operator
 * Operators: ">", "<", "==", "!="
 * For == and !=, use strcmp()
 * For > and <, try converting to numbers with atof() or atoi()
 * Return 1 if comparison is true, 0 if false
 */
int compare_values(char *val1, char *op, char *val2) {
    double num1, num2; 
    
    // Check for ==
    if (strcmp(op, "==") == 0) {
        // Return 1 if val1 equals val2
        return (strcmp(val1, val2) == 0);
    }
    
    // Check for !=
    if (strcmp(op, "!=") == 0) {
        // Return 1 if val1 NOT equals val2 
        return (strcmp(val1, val2) != 0);  // Return 1 when they DON'T match
    }
    
    // Check for > 
    if (strcmp(op, ">") == 0) {
        num1 = atof(val1);
        num2 = atof(val2);
        return (num1 > num2);
    }
    
    // Check for <
    if (strcmp(op, "<") == 0) {
        num1 = atof(val1);
        num2 = atof(val2);
        return (num1 < num2);  // Return 1 when num1 < num2
    }
    
    // Unknown operator
    fprintf(stderr, "csv: unknown operator: %s\n", op);
    return 0;
}

/**
 * Keep only rows matching a condition.
 * @param args command arguments where:
 *              args[1] = column name (e.g., "age")
 *              args[2] = operator (e.g., ">", "<", "==", "!=")
 *              args[3] = value to compare (e.g., "25")
 * @return 1 to continue shell loop
 */
int csv_filter(char **args)  /* Remove semicolon before { */
{
    int col_index;
    Table temp_table;
    int row, col; 
    int new_row_count;

    // Check arguments
    if (args[1] == NULL || args[2] == NULL || args[3] == NULL) {
        fprintf(stderr, "csv: usage: filter <column> <op> <value>\n");
        return 1;
    }

    col_index = find_column(args[1]);

    if (col_index == -1) {
        fprintf(stderr, "csv: column '%s' not found\n", args[1]);
        return 1;
    }

    // Copy headers to temp table
    for (col = 0; col < current_table.num_cols; col++) { 
        strcpy(temp_table.headers[col], current_table.headers[col]);
    }
    temp_table.num_cols = current_table.num_cols;  // Copy the count

    // Filter rows
    new_row_count = 0;
    for (row = 0; row < current_table.num_rows; row++) {
        // Get value from the column we're filtering by
        if (compare_values(current_table.data[row][col_index], args[2], args[3])) {
            
            // Copy ALL columns of this matching row
            for (col = 0; col < current_table.num_cols; col++) {
                strcpy(temp_table.data[new_row_count][col], current_table.data[row][col]);
            }
            
            new_row_count++;
        }
    }

    // Copy temp_table back to current_table
    for (row = 0; row < new_row_count; row++) {
        for (col = 0; col < temp_table.num_cols; col++) {
            strcpy(current_table.data[row][col], temp_table.data[row][col]);
        }
    }
    current_table.num_rows = new_row_count;

    printf("Filtered to %d rows\n", new_row_count); 
    return 1;
}

/**
 * Sort the table by a column.
 * @param args command arguments where args[1] is column name
 * @return 1 to continue shell loop
 */
int csv_sort(char **args)
{
    int col_index;
    int i, j;
    char temp[MAX_CELL_SIZE];
    int col;
    double val_j, val_j1;

    // Check if column name was provided
    if (args[1] == NULL) {
        fprintf(stderr, "csv: expected argument to \"sort\"\n");
        return 1;
    }

    // Find the column index
    col_index = find_column(args[1]);

    if (col_index == -1) {
        fprintf(stderr, "csv: column '%s' not found\n", args[1]);
        return 1;
    }

    // Bubble sort - outer loop
    for (i = 0; i < current_table.num_rows - 1; i++) {
        
        // Inner loop - compare adjacent rows
        for (j = 0; j < current_table.num_rows - i - 1; j++) {
            
            // Compare values in the sort column 
            val_j = atof(current_table.data[j][col_index]);
            val_j1 = atof(current_table.data[j + 1][col_index]);
            
            // If row j is greater than row j+1, swap them 
            if (val_j > val_j1) {
                
                // Swap entire rows j and j+1 
                for (col = 0; col < current_table.num_cols; col++) {
                    strcpy(temp, current_table.data[j][col]);
                    strcpy(current_table.data[j][col], current_table.data[j + 1][col]);
                    strcpy(current_table.data[j + 1][col], temp);
                }
            }
        }
    }

    printf("Sorted by %s\n", args[1]);
    return 1;
}

/**
 * Save current table to a CSV file.
 * @param args command arguments where args[1] is filename
 * @return 1 to continue shell loop
 */
int csv_save(char **args)
{
    FILE *file;
    int row, col;

    // args is an array of strings that contains the commands and its arguments
    if (args[1] == NULL) {
        fprintf(stderr, "csv: expected argument to \"save\"\n");
        return 1;
    }

    // Open file for writing
    file = fopen(args[1], "w"); // "w" means write mode

    if (!file) {
        perror("csv");
        return 1;
    }

    // Write headers and data
    for (col = 0; col < current_table.num_cols; col++) {
        fprintf(file, "%s", current_table.headers[col]);
        // Add comas
        if (col < current_table.num_cols - 1) {
            fprintf(file, ",");
        }
    }
    // Add newline after headers
    fprintf(file, "\n");

    // Write data rows
    for (row = 0; row < current_table.num_rows; row++) {
        for (col = 0; col < current_table.num_cols; col++) {
            fprintf(file, "%s", current_table.data[row][col]);
                    if (col < current_table.num_cols - 1) {
            fprintf(file, ",");
            }
        }
        // Add newline after each new row
        fprintf(file, "\n");
    }

    fclose(file);
    printf("saved %d rows to %s\n", current_table.num_rows, args[1]);
    return 1;
}

/**
 * Display number of rows in current table.
 * @param args command arguments (not used)
 * @return 1 to continue shell loop
 */
int csv_count(char **args)
{
    if (current_table.num_rows == 0) {
        printf("No data loaded.\n");
        return 1;
    }

    printf("%d rows\n", current_table.num_rows);
    return 1;
}

/**
 * Display column names.
 * @param args command arguments (not used)
 * @return 1 to continue shell loop
 */
int csv_columns(char **args)
{
    int i;

    // Print how many columns there are
    printf("Columns (%d): ", current_table.num_cols);

    // Print each column name
    for (i = 0; i < current_table.num_cols; i++) {
        printf("%s", current_table.headers[i]);
        if (i < current_table.num_cols - 1) {
            printf(", ");
        }
    }

    printf("\n");
    return 1;
}

/**
 * Display help message with available commands.
 * @param args command arguments (not used)
 * @retun 1 to continue shell loop
 */
int csv_help(char **args)
{
    int i;
    printf("CSV Data Shell\n");
    printf("Type commands and arguments, and hit enter.\n");
    printf("The following commands are built in:\n");

    // Loop through and print each command name
    for(i = 0; i < csv_num_builtins(); i++) {
        printf("    %s\n", builtin_str[i]);
    }

    printf("Use 'help' to see this message again.\n");
    return 1;
}

/**
 * Exit the shell.
 * @param args command arguments (not used)
 * @return 0 to signal shell loop to terminate
 */
int csv_exit(char **args)
{
    return 0;
}

/**
 * Execute a command.
 * @param args parsed command arguments (args[0] is command name)
 * @return status code (0 = exit, 1 = continue)
 */
int csv_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    // Loop through builtin commands
    for (i = 0; i < csv_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            // Call the matching function
            return (*builtin_func[i])(args);
        }
    }

    // Unknown command
    printf("csv: unknown command: %s\n", args[0]);
    printf("Type 'help' for available commands\n");
    return 1;
}

/**
 * Main shell loop (Read-Parse-Execute).
 * Loops forever until user exits by:
 *      1. Printing prompt "csv> "
 *      2. Reading line from user (csv_read_line)
 *      3. Parsing line into tokens (csv_split_line)
 *      4. Executing command (csv_execute)
 *      5. Freeing memory
 *      6. Repeating
 */
void csv_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        // Print prompt
        printf("csv> ");

        // Read line from user
        line = csv_read_line();

        // Check for EOF
        if (!line) break;

        // Parse line into tokens
        args = csv_split_line(line);

        // Execute command
        status = csv_execute(args);

        // Free memory
        free(line);
        free(args);
    } while (status);
}

/**
 * Entry point of the program.
 * @param argc argument count (not used)
 * @param argv argument vector (not used)
 * @return EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
  printf("╔════════════════════════════════════════╗\n");
  printf("║     Welcome to CSV Data Shell!         ║\n");
  printf("╚════════════════════════════════════════╝\n\n");
  
  printf(" A simple tool for analyzing CSV files\n\n");
  
  printf("Quick Start:\n");
  printf("  1. Load a file:    load mydata.csv\n");
  printf("  2. View data:      show\n");
  printf("  3. Filter rows:    filter age > 25\n");
  printf("  4. Sort data:      sort name\n");
  printf("  5. Save results:   save output.csv\n\n");
  
  printf("Type 'help' to see all commands\n");
  printf("Type 'exit' to quit\n\n");
  
  csv_loop();
  
  printf("\nThanks for using CSV Data Shell!\n");
  printf("Goodbye! \n");
  
  return EXIT_SUCCESS;
}
