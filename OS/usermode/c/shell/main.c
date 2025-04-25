/*
// Hojuix Usermode Shell v0.1a
*/

#include <stdio.h>
#include <stdbool.h>
#include <sys/syscalls.h>

// User input
char input_data[256] = {0x00};
char input_data_cmd[64] = {0x00};
char exec_path[64] = {0x00};
int input_index = 0;
char c;

int shell_args_found = 0;

void exec_syscall_test(char* data);

int test_strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == '\0')
        {
            return 0;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

char* prompt() {
    return "[/]$";
}

int main(int argc, char** argv) {
    printf("hsh 0.1a - Build Time: %s %s\n", __DATE__, __TIME__);

    bool running = true;
    while (running) {
        // Print the prompt
        printf("%s ", prompt());

        // Receive input
        while (c != 0xa) { // Command is not finished
            c = syscall_input();
            if (c != 0x00) {
                input_data[input_index] = c;

                // Check if the entered key was a backspace, and handle it
                if (c == 0x08) {
                    input_data[input_index] == 0x00;
                    if (input_index != 0) {
                        input_data[input_index - 1] == 0x00;
                        input_index--;
                        printf(".");
                    }
                }

                // Check if anything was entered, and if not (input_index[0] is 0xa), or a backspace, don't print the character
                if (input_data[0] != 0xa && c != 0x08) {
                    printf("%c", c);
                    input_index++;
                }
            }
        }
        c = 0x00;

        // Search input data for newlines, and zero them (Yes this is very inefficient but idc)
        for (size_t i = 0; i < 256; i++) {
            if (input_data[i] == 0xa) {
                input_data[i] = 0x00;
            }
        }

        // Check if anything was entered, and if not, jump right to the clear
        if (input_index == 0) {
            printf("\n");
            goto clear_input_buffer;
        }

        // Find the root command
        for (size_t i = 0; i < 256; i++) {
            if (input_data[i] != 0x20) {
                input_data_cmd[i] = input_data[i];
            } else {
                i = 257;
            }
        }

        // At this point, a command is in the buffer
        if (test_strcmp(input_data_cmd, "help") == 0) {
            printf("Help command received!!\n");
        } else if (test_strcmp(input_data_cmd, "shutdown") == 0) {
            // Perform ACPI Shutdown
            libc_syscall(SYS_SHUTDOWN);
        } else if (test_strcmp(input_data_cmd, "exit") == 0) {
            // Exit shell the normal way
            goto exit_shell;
        } else if (test_strcmp(input_data_cmd, "open") == 0) {
            // Test open syscall
            //open_syscall_test(input_data);
        } else if (test_strcmp(input_data_cmd, "exec") == 0) {
            exec_syscall_test(input_data);
        } else {
            printf("Unknown command: %s\n", input_data_cmd);
        }

        // Clear input buffer
clear_input_buffer:
        for (size_t i = 0; i < input_index; i++) {
            input_data[i] = 0x00;
        }
        input_index = 0;
        for (size_t i = 0; i < 64; i++) {
            input_data_cmd[i] = 0x00;
        }
    }

exit_shell:
    printf("Exiting shell...\n");
    return 5;
}

void exec_syscall_test(char* data) {
    // Fetch exec path
    int arg_path_start = 0;
    for (size_t i = 0; i < 256; i++) {
        if (data[i] == 0x20) {
            arg_path_start = i + 1;
            break;
        }
    }
    for (size_t i = arg_path_start; i < 256; i++) {
        if (data[i] != 0x20) {
            exec_path[i - arg_path_start] = data[i];
        } else {
            break;
        }
    }
    
    // Issue exec syscall
    printf("Executing %s\n", exec_path);
    libc_syscall(SYS_EXEC, exec_path);
}
