#include "proc_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

int is_number(const char* str) {
    if (str == NULL || *str == '\0') return 0;
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

int list_process_directories(void) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Failed to open /proc");
        return -1;
    }

    struct dirent *entry;
    int count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    while ((entry = readdir(dir)) != NULL) {
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    if (closedir(dir) != 0) {
        perror("Failed to close /proc directory");
        return -1;
    }

    printf("Found %d process directories\n", count);
    printf("SUCCESS: Process directories listed!\n");
    return 0;
}

int read_file_with_syscalls(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file with open()");
        return -1;
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    if (bytes_read == -1) {
        perror("Error reading file with read()");
        close(fd);
        return -1;
    }

    if (close(fd) != 0) {
        perror("Failed to close file descriptor");
        return -1;
    }

    return 0;
}

int read_file_with_library(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file with fopen()");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    if (fclose(file) != 0) {
        perror("Failed to close file");
        return -1;
    }

    return 0;
}

int read_process_info(const char* pid) {
    char filepath[256];

    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);
    printf("\n--- Process Information for PID %s ---\n", pid);
    if (read_file_with_syscalls(filepath) != 0) {
        printf("ERROR: Failed to read process status\n");
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);
    printf("\n--- Command Line ---\n");
    if (read_file_with_syscalls(filepath) != 0) {
        printf("ERROR: Failed to read process cmdline\n");
        return -1;
    }

    printf("\nSUCCESS: Process information read!\n");
    return 0;
}

int show_system_info(void) {
    const int MAX_LINES = 10;
    char line[256];
    int count;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);
    FILE *cpu = fopen("/proc/cpuinfo", "r");
    if (!cpu) {
        perror("Failed to open /proc/cpuinfo");
        return -1;
    }

    count = 0;
    while (fgets(line, sizeof(line), cpu) && count < MAX_LINES) {
        printf("%s", line);
        count++;
    }
    fclose(cpu);

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);
    FILE *mem = fopen("/proc/meminfo", "r");
    if (!mem) {
        perror("Failed to open /proc/meminfo");
        return -1;
    }

    count = 0;
    while (fgets(line, sizeof(line), mem) && count < MAX_LINES) {
        printf("%s", line);
        count++;
    }
    fclose(mem);

    printf("SUCCESS: System information displayed!\n");
    return 0;
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}
