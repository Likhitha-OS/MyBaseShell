# Simple Unix Shell Implementation

## Objective
This project involves creating a Unix shell program that acts as an intermediary between the user and the operating system. The shell, implemented in C, allows users to execute commands, manage directories, and interact with the system seamlessly. The shell provides a customized environment where several commands are built-in for efficient processing.

## Features
### Shell Prompt
- Displays a continuous prompt waiting for user input.
- The prompt includes the full path of the current working directory followed by the `>` symbol (e.g., `/usr/home/me/Desktop>`).

### Command Execution
- Reads and parses a command entered by the user.
- Executes commands directly or as built-in functions, depending on the type.

### Built-in Commands
1. **clear**: Clears the terminal screen.
2. **env**: Displays all environment variables.
3. **cd `<dir>`**: Changes the current working directory to the specified directory `<dir>`.
4. **pwd**: Prints the current working directory.
5. **mkdir `<dir>`**: Creates a directory named `<dir>`.
6. **rmdir `<dir>`**: Removes the directory named `<dir>`.
7. **history**:
   - Displays a list of the last executed commands with an offset next to each command.
   - **history `<n>`**: Displays the last `n` commands executed.
8. **exit**: Exits the shell.

### External Command Execution
- Commands not recognized as built-in are treated as external executables.
- Searches for executables in the current directory or directories specified by the `PATH` environment variable.
- Executes the file in a new process and waits for the process to complete before returning to the prompt.
- Supports commands with any number of arguments.

### Background Execution
- Commands ending with `&` are executed in the background.
- The shell immediately returns to the prompt without waiting for background commands to complete.
- Background execution is not supported for built-in commands.

## Error Handling
- Proper error messages are displayed for failed commands using `perror`.

## Implementation Details
- **Language**: C
- **Libraries Used**: Standard C libraries for system calls such as `chdir`, `getcwd`, `mkdir`, `rmdir`, `readdir`, and `stat`.
- **Environment Variables**: Managed using functions like `getenv` for fetching `PATH`.
- **History Management**: Commands are stored in a file for persistence and displayed to the user on request.

## How It Works
1. **Prompt Display**: Continuously displays the shell prompt with the current directory.
2. **Command Parsing**: Reads input, tokenizes the command, and identifies the type (built-in or external).
3. **Built-in Execution**: Executes the command directly if it matches one of the built-in commands.
4. **External Execution**: Creates a new process to execute external commands using `fork` and `exec`.
5. **Background Execution**: If `&` is detected, the shell does not wait for the process to finish.
6. **Error Handling**: Displays appropriate messages for invalid commands or operations.

## Example Commands
```sh
/home/user/Desktop> clear
/home/user/Desktop> pwd
/home/user/Desktop
/home/user/Desktop> cd Documents
/home/user/Documents> mkdir NewFolder
/home/user/Documents> ls -l
/home/user/Documents> history 5
/home/user/Documents> a.out &
```

## Requirements
- Unix-based system
- GCC Compiler

## Usage
1. Compile the shell program using:
   ```sh
   gcc simplesh.c -o simplesh
   ```
2. Run the shell:
   ```sh
   ./simplesh
   ```
3. Start entering commands at the prompt.

## Notes
- Built-in commands are implemented within the shell.
- Other commands utilize the system’s executable search paths.
- Background commands are limited to external executables.
- Ensure appropriate permissions for directory and file operations.

---

This project provides a simplified but powerful implementation of a Unix shell, enabling users to understand the inner workings of command interpretation and execution in an operating system.

