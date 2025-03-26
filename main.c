/* Simple Operating System Framework in C
 * Demonstrates basic file operations: List, Move, Rename, Delete, Create, Write, Read, Mkdir, Rmdir, Copy, CD
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_FILENAME_LENGTH 50
#define MAX_FILES 100

/* Simple file structure */
typedef struct
{
    char name[MAX_FILENAME_LENGTH];
    char content[1024];
    bool exists;
    bool isDirectory;
    int permissions; /* Simple permissions: 1=read, 2=write, 4=execute */
} File;

/* OS State */
typedef struct
{
    File files[MAX_FILES];
    int fileCount;
    bool running;
    char currentDirectory[MAX_FILENAME_LENGTH];
} OSState;

/* Function prototypes */
void initializeOS(OSState *os);
void showPrompt(OSState *os);
void processCommand(OSState *os, char *command);
void listFiles(OSState *os);
void moveFile(OSState *os, char *source, char *destination);
void renameFile(OSState *os, char *oldname, char *newname);
void deleteFile(OSState *os, char *filename);
void createFile(OSState *os, char *filename);
void writeToFile(OSState *os, char *filename, char *content);
void readFile(OSState *os, char *filename);
void makeDirectory(OSState *os, char *dirname);
void removeDirectory(OSState *os, char *dirname);
void changeDirectory(OSState *os, char *dirname);
void setPermissions(OSState *os, char *filename, int permissions);
void copyFile(OSState *os, char *source, char *destination);
void showHelp();
bool isDirectoryEmpty(OSState *os, char *dirname);

int main()
{
    OSState os;
    char command[MAX_COMMAND_LENGTH];

    /* Initialize the OS */
    initializeOS(&os);

    printf("Simple OS v0.1\n");
    printf("Type 'help' for a list of commands\n");

    while (os.running)
    {
        showPrompt(&os);

        /* Get user input */
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL)
        {
            break;
        }

        /* Remove newline character */
        command[strcspn(command, "\n")] = 0;

        /* Process the command */
        processCommand(&os, command);
    }

    printf("OS shutting down...\n");
    return 0;
}

void initializeOS(OSState *os)
{
    os->fileCount = 0;
    os->running = true;
    strcpy(os->currentDirectory, "/");

    /* Create root directory */
    strcpy(os->files[0].name, "/");
    os->files[0].content[0] = '\0';
    os->files[0].exists = true;
    os->files[0].isDirectory = true;
    os->files[0].permissions = 7; /* rwx */

    /* Create a few sample files */
    strcpy(os->files[1].name, "readme.txt");
    strcpy(os->files[1].content, "Welcome to SimpleOS!");
    os->files[1].exists = true;
    os->files[1].isDirectory = false;
    os->files[1].permissions = 6; /* rw- */

    strcpy(os->files[2].name, "sample.txt");
    strcpy(os->files[2].content, "This is a sample file.");
    os->files[2].exists = true;
    os->files[2].isDirectory = false;
    os->files[2].permissions = 6; /* rw- */

    /* Create a sample directory */
    strcpy(os->files[3].name, "docs");
    os->files[3].content[0] = '\0';
    os->files[3].exists = true;
    os->files[3].isDirectory = true;
    os->files[3].permissions = 7; /* rwx */

    os->fileCount = 4;
}

void showPrompt(OSState *os)
{
    printf("%s> ", os->currentDirectory);
}

void processCommand(OSState *os, char *command)
{
    char cmd[MAX_COMMAND_LENGTH];
    char arg1[MAX_FILENAME_LENGTH];
    char arg2[MAX_FILENAME_LENGTH];

    /* Parse the command */
    if (sscanf(command, "%s %s %s", cmd, arg1, arg2) < 1)
    {
        return;
    }

    /* Handle commands */
    if (strcmp(cmd, "list") == 0 || strcmp(cmd, "ls") == 0)
    {
        listFiles(os);
    }
    else if (strcmp(cmd, "move") == 0 || strcmp(cmd, "mv") == 0)
    {
        moveFile(os, arg1, arg2);
    }
    else if (strcmp(cmd, "rename") == 0)
    {
        renameFile(os, arg1, arg2);
    }
    else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "rm") == 0)
    {
        deleteFile(os, arg1);
    }
    else if (strcmp(cmd, "rmdir") == 0)
    {
        removeDirectory(os, arg1);
    }
    else if (strcmp(cmd, "create") == 0)
    {
        createFile(os, arg1);
    }
    else if (strcmp(cmd, "write") == 0)
    {
        char content[1024];
        printf("Enter content: ");
        fgets(content, sizeof(content), stdin);
        content[strcspn(content, "\n")] = 0;
        writeToFile(os, arg1, content);
    }
    else if (strcmp(cmd, "read") == 0 || strcmp(cmd, "cat") == 0)
    {
        readFile(os, arg1);
    }
    else if (strcmp(cmd, "mkdir") == 0)
    {
        makeDirectory(os, arg1);
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        changeDirectory(os, arg1);
    }
    else if (strcmp(cmd, "chmod") == 0)
    {
        int permissions = atoi(arg2);
        setPermissions(os, arg1, permissions);
    }
    else if (strcmp(cmd, "copy") == 0 || strcmp(cmd, "cp") == 0)
    {
        copyFile(os, arg1, arg2);
    }
    else if (strcmp(cmd, "help") == 0)
    {
        showHelp();
    }
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
    {
        os->running = false;
    }
    else
    {
        printf("Unknown command: %s\n", cmd);
    }
}

void listFiles(OSState *os)
{
    printf("Files in %s:\n", os->currentDirectory);
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists)
        {
            /* Check if file is in current directory */
            char *filename = os->files[i].name;
            char *lastSlash = strrchr(filename, '/');

            /* Handle files in root directory */
            if (strcmp(os->currentDirectory, "/") == 0)
            {
                /* Show only files directly in root, not in subdirectories */
                if (lastSlash == filename && strlen(filename) > 1)
                {
                    /* Skip files in subdirectories */
                    continue;
                }
            }
            else
            {
                /* For non-root directories, check if file is in current directory */
                char dirPath[MAX_FILENAME_LENGTH];
                sprintf(dirPath, "%s/", os->currentDirectory);

                /* Skip files not in this directory */
                if (strncmp(filename, dirPath, strlen(dirPath)) != 0 ||
                    strchr(filename + strlen(dirPath), '/') != NULL)
                {
                    continue;
                }
            }

            char permStr[4] = "---";
            if (os->files[i].permissions & 4)
                permStr[0] = 'r';
            if (os->files[i].permissions & 2)
                permStr[1] = 'w';
            if (os->files[i].permissions & 1)
                permStr[2] = 'x';

            /* Display just the filename, not the full path */
            char displayName[MAX_FILENAME_LENGTH];
            if (lastSlash != NULL)
            {
                strcpy(displayName, lastSlash + 1);
            }
            else
            {
                strcpy(displayName, filename);
            }

            printf("  %s %s%s\n", permStr,
                   os->files[i].isDirectory ? "[DIR] " : "",
                   displayName);
        }
    }
}

void moveFile(OSState *os, char *source, char *destination)
{
    int sourceIndex = -1;
    int destDirIndex = -1;

    /* Find the source file */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, source) == 0)
        {
            sourceIndex = i;
            break;
        }
    }

    if (sourceIndex == -1)
    {
        printf("File not found: %s\n", source);
        return;
    }

    /* Check if destination exists and is a directory */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, destination) == 0)
        {
            if (os->files[i].isDirectory)
            {
                destDirIndex = i;
            }
            else
            {
                printf("Destination exists and is not a directory: %s\n", destination);
                return;
            }
            break;
        }
    }

    /* Create a new file at the destination */
    if (os->fileCount >= MAX_FILES)
    {
        printf("Cannot move file: maximum number of files reached\n");
        return;
    }

    /* Create the new file with the same content */
    char newName[MAX_FILENAME_LENGTH];
    if (destDirIndex != -1)
    {
        /* Move to directory */
        sprintf(newName, "%s/%s", destination, source);
    }
    else
    {
        /* Rename */
        strcpy(newName, destination);
    }

    strcpy(os->files[os->fileCount].name, newName);
    strcpy(os->files[os->fileCount].content, os->files[sourceIndex].content);
    os->files[os->fileCount].exists = true;
    os->files[os->fileCount].isDirectory = os->files[sourceIndex].isDirectory;
    os->files[os->fileCount].permissions = os->files[sourceIndex].permissions;
    os->fileCount++;

    /* Mark the original as deleted */
    os->files[sourceIndex].exists = false;

    printf("Moved %s to %s\n", source, newName);
}

void renameFile(OSState *os, char *oldname, char *newname)
{
    int fileIndex = -1;

    /* Find the file */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, oldname) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        printf("File not found: %s\n", oldname);
        return;
    }

    /* Rename the file */
    strcpy(os->files[fileIndex].name, newname);
    printf("Renamed %s to %s\n", oldname, newname);
}

void deleteFile(OSState *os, char *filename)
{
    int fileIndex = -1;

    /* Find the file */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, filename) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    /* Mark the file as deleted */
    os->files[fileIndex].exists = false;
    printf("Deleted %s\n", filename);
}

void createFile(OSState *os, char *filename)
{
    if (os->fileCount >= MAX_FILES)
    {
        printf("Cannot create file: maximum number of files reached\n");
        return;
    }

    /* Check if file already exists */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, filename) == 0)
        {
            printf("File already exists: %s\n", filename);
            return;
        }
    }

    /* Create new file */
    strcpy(os->files[os->fileCount].name, filename);
    os->files[os->fileCount].content[0] = '\0';
    os->files[os->fileCount].exists = true;
    os->files[os->fileCount].isDirectory = false;
    os->files[os->fileCount].permissions = 6; /* rw- by default */
    os->fileCount++;

    printf("Created file: %s\n", filename);
}

void writeToFile(OSState *os, char *filename, char *content)
{
    int fileIndex = -1;

    /* Find the file */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, filename) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    /* Write to the file */
    strcpy(os->files[fileIndex].content, content);
    printf("Content written to %s\n", filename);
}

void readFile(OSState *os, char *filename)
{
    int fileIndex = -1;

    /* Find the file */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, filename) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    /* Display the file content */
    printf("Content of %s:\n%s\n", filename, os->files[fileIndex].content);
}

void makeDirectory(OSState *os, char *dirname)
{
    if (os->fileCount >= MAX_FILES)
    {
        printf("Cannot create directory: maximum number of files reached\n");
        return;
    }

    /* Handle special case for absolute path */
    char fullPath[MAX_FILENAME_LENGTH];
    if (dirname[0] == '/')
    {
        /* Absolute path */
        strcpy(fullPath, dirname);
    }
    else
    {
        /* Relative path - combine with current directory */
        if (strcmp(os->currentDirectory, "/") == 0)
        {
            sprintf(fullPath, "/%s", dirname);
        }
        else
        {
            sprintf(fullPath, "%s/%s", os->currentDirectory, dirname);
        }
    }

    /* Check if directory already exists */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, fullPath) == 0)
        {
            printf("Directory/file already exists: %s\n", fullPath);
            return;
        }
    }

    /* Create new directory */
    strcpy(os->files[os->fileCount].name, fullPath);
    os->files[os->fileCount].content[0] = '\0';
    os->files[os->fileCount].exists = true;
    os->files[os->fileCount].isDirectory = true;
    os->files[os->fileCount].permissions = 7; /* rwx by default for directories */
    os->fileCount++;

    printf("Created directory: %s\n", fullPath);
}

void changeDirectory(OSState *os, char *dirname)
{
    /* Handle special case for root */
    if (strcmp(dirname, "/") == 0)
    {
        strcpy(os->currentDirectory, "/");
        return;
    }

    /* Handle special case for parent directory */
    if (strcmp(dirname, "..") == 0)
    {
        /* Find the last slash in the current path */
        char *lastSlash = strrchr(os->currentDirectory, '/');
        if (lastSlash != NULL && lastSlash != os->currentDirectory)
        {
            /* Truncate the path at the last slash */
            *lastSlash = '\0';
        }
        else
        {
            /* Already at root */
            strcpy(os->currentDirectory, "/");
        }
        return;
    }

    /* Find the directory */
    int dirIndex = -1;
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, dirname) == 0)
        {
            if (os->files[i].isDirectory)
            {
                dirIndex = i;
                break;
            }
            else
            {
                printf("%s is not a directory\n", dirname);
                return;
            }
        }
    }

    if (dirIndex == -1)
    {
        printf("Directory not found: %s\n", dirname);
        return;
    }

    /* Change to the directory */
    if (strcmp(os->currentDirectory, "/") == 0)
    {
        sprintf(os->currentDirectory, "/%s", dirname);
    }
    else
    {
        sprintf(os->currentDirectory, "%s/%s", os->currentDirectory, dirname);
    }
}

void setPermissions(OSState *os, char *filename, int permissions)
{
    /* Find the file */
    int fileIndex = -1;
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, filename) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    /* Check if permissions are valid (0-7) */
    if (permissions < 0 || permissions > 7)
    {
        printf("Invalid permissions: %d (must be 0-7)\n", permissions);
        return;
    }

    /* Set the permissions */
    os->files[fileIndex].permissions = permissions;
    printf("Changed permissions of %s to %d\n", filename, permissions);
}

void copyFile(OSState *os, char *source, char *destination)
{
    int sourceIndex = -1;
    int destDirIndex = -1;

    /* Find the source file */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, source) == 0)
        {
            sourceIndex = i;
            break;
        }
    }

    if (sourceIndex == -1)
    {
        printf("File not found: %s\n", source);
        return;
    }

    /* Check if destination exists and is a directory */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, destination) == 0)
        {
            if (os->files[i].isDirectory)
            {
                destDirIndex = i;
            }
            else
            {
                printf("Destination exists and is not a directory: %s\n", destination);
                return;
            }
            break;
        }
    }

    /* Create a new file at the destination */
    if (os->fileCount >= MAX_FILES)
    {
        printf("Cannot copy file: maximum number of files reached\n");
        return;
    }

    /* Create the new file with the same content */
    char newName[MAX_FILENAME_LENGTH];
    if (destDirIndex != -1)
    {
        /* Copy to directory */
        sprintf(newName, "%s/%s", destination, source);
    }
    else
    {
        /* Copy with new name */
        strcpy(newName, destination);
    }

    /* Check if destination file already exists */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, newName) == 0)
        {
            printf("Destination file already exists: %s\n", newName);
            return;
        }
    }

    strcpy(os->files[os->fileCount].name, newName);
    strcpy(os->files[os->fileCount].content, os->files[sourceIndex].content);
    os->files[os->fileCount].exists = true;
    os->files[os->fileCount].isDirectory = os->files[sourceIndex].isDirectory;
    os->files[os->fileCount].permissions = os->files[sourceIndex].permissions;
    os->fileCount++;

    printf("Copied %s to %s\n", source, newName);
}

/* Check if a directory is empty */
bool isDirectoryEmpty(OSState *os, char *dirname)
{
    /* Check if any file or directory has this directory as its parent */
    char dirPath[MAX_FILENAME_LENGTH];

    /* Ensure the directory path ends with a slash for proper prefix matching */
    if (strcmp(dirname, "/") == 0)
    {
        strcpy(dirPath, "/");
    }
    else
    {
        sprintf(dirPath, "%s/", dirname);
    }

    int dirPathLen = strlen(dirPath);

    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists &&
            strncmp(os->files[i].name, dirPath, dirPathLen) == 0 &&
            strcmp(os->files[i].name, dirname) != 0)
        {
            /* Found a file/directory inside this directory */
            return false;
        }
    }

    return true;
}

/* Remove a directory if it's empty */
void removeDirectory(OSState *os, char *dirname)
{
    int dirIndex = -1;

    /* Handle special case for absolute path */
    char fullPath[MAX_FILENAME_LENGTH];
    if (dirname[0] == '/')
    {
        /* Absolute path */
        strcpy(fullPath, dirname);
    }
    else
    {
        /* Relative path - combine with current directory */
        if (strcmp(os->currentDirectory, "/") == 0)
        {
            sprintf(fullPath, "/%s", dirname);
        }
        else
        {
            sprintf(fullPath, "%s/%s", os->currentDirectory, dirname);
        }
    }

    /* Find the directory */
    for (int i = 0; i < os->fileCount; i++)
    {
        if (os->files[i].exists && strcmp(os->files[i].name, fullPath) == 0)
        {
            dirIndex = i;
            break;
        }
    }

    if (dirIndex == -1)
    {
        printf("Directory not found: %s\n", fullPath);
        return;
    }

    /* Check if it's a directory */
    if (!os->files[dirIndex].isDirectory)
    {
        printf("%s is not a directory\n", fullPath);
        return;
    }

    /* Check if the directory is empty */
    if (!isDirectoryEmpty(os, fullPath))
    {
        printf("Cannot remove directory: %s is not empty\n", fullPath);
        return;
    }

    /* Mark the directory as deleted */
    os->files[dirIndex].exists = false;
    printf("Removed directory: %s\n", fullPath);
}

void showHelp()
{
    printf("Available commands:\n");
    printf("  list / ls              : List all files\n");
    printf("  create [filename]      : Create a new file\n");
    printf("  write [filename]       : Write content to a file\n");
    printf("  read / cat [filename]  : Display file content\n");
    printf("  move / mv [src] [dest] : Move a file\n");
    printf("  rename [old] [new]     : Rename a file\n");
    printf("  delete / rm [filename] : Delete a file\n");
    printf("  copy / cp [src] [dest] : Copy a file\n");
    printf("  mkdir [dirname]        : Create a new directory\n");
    printf("  rmdir [dirname]        : Remove an empty directory\n");
    printf("  cd [dirname]           : Change to directory\n");
    printf("  chmod [file] [perm]    : Change file permissions (0-7)\n");
    printf("  help                   : Show this help\n");
    printf("  exit / quit            : Exit the OS\n");
}