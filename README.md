# CSV Data Shell

A command-line data processing shell written in C for analyzing CSV files with filtering, sorting, and data export capabilities.

## Features

- **Load CSV files** into memory
- **View data** with formatted table output
- **Filter rows** by column values using comparison operators (`>`, `<`, `==`, `!=`)
- **Sort data** by any column
- **Save results** to new CSV files
- **Count rows** and view column information

## Demo
```
╔════════════════════════════════════════╗
║     Welcome to CSV Data Shell!         ║
╚════════════════════════════════════════╝

 A simple tool for analyzing CSV files

Quick Start:
  1. Load a file:    load mydata.csv
  2. View data:      show
  3. Filter rows:    filter age > 25
  4. Sort data:      sort name
  5. Save results:   save output.csv
```

## Example Usage
```bash
csv> load employees.csv
Loaded 100 rows, 5 columns

csv> show
name           age    department     salary         years
-------------------------------------------------------------------------------
Alice          28     Engineering    85000          3
Bob            35     Marketing      75000          7
...

csv> filter salary > 80000
Filtered to 42 rows

csv> sort name
Sorted by name

csv> save high_earners.csv
Saved 42 rows to high_earners.csv
```

## Technical Details

- **Language**: C
- **Data Structure**: In-memory 3D array for table storage
- **Algorithms**: Bubble sort for sorting, linear search for filtering
- **Architecture**: Read-Parse-Execute loop modeled after Unix shells
- **Memory Management**: Dynamic buffer allocation with realloc
- **File I/O**: CSV parsing and writing with proper formatting

## Building and Running

### Prerequisites
- GCC compiler
- macOS, Linux, or Unix-like environment

### Compile
```bash
gcc -o csvshell csvshell.c
```

### Run
```bash
./csvshell
```

## Available Commands

| Command | Description | Example |
|---------|-------------|---------|
| `load <file>` | Load a CSV file | `load data.csv` |
| `show` | Display current table | `show` |
| `filter <col> <op> <val>` | Filter rows | `filter age > 25` |
| `sort <column>` | Sort by column | `sort name` |
| `save <file>` | Save to CSV | `save output.csv` |
| `count` | Show row count | `count` |
| `columns` | Show column names | `columns` |
| `help` | Show all commands | `help` |
| `exit` | Quit shell | `exit` |

## Project Structure
```
csvshell.c              # Main source file (~600 lines)
├── Data structures     # Table struct for in-memory storage
├── I/O functions       # Read line, parse tokens
├── Built-in commands   # Load, show, filter, sort, save, etc.
├── Helper functions    # find_column, compare_values, trim
└── Main loop           # Read-Parse-Execute cycle
```

## Implementation Highlights

- **Function pointers** for extensible command dispatch
- **String manipulation** with strtok for CSV parsing
- **Type conversion** for numeric comparisons in filtering
- **Row-level operations** for sorting entire table rows
- **Buffer management** with dynamic reallocation

## Author

**Naikya Chandana**
- GitHub: [@naikya-chandana](https://github.com/naikya-chandana)
- LinkedIn: [Naikya Chandana](https://www.linkedin.com/in/naikya-chandana)

## License

This project is open source and available for educational purposes.
