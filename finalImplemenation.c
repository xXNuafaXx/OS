#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define PATH_SIZE 100
#define MAX_BUFF_SIZE 4096

int potentiallyDangerousFiles = 0;

//write all the function declarations here

// makeSnapshotName takes path and returns the name of the snapshot
// by replacing the '/' with '_'
char *makeSnapshotName(char *path);

// makeSnapshotPath takes snapshotPath and dirPath and returns the path of the snapshot
char *makeSnapshotPath(char *snapshotPath, char *dirPath);

// getFileDescriptor takes path and returns a file descriptor
int getFileDescriptor(char *path);

//  heckPermissions takes path and returns 1 if the file is readable, writable or executable
int hasNoPermissions(char *path);

// moveFile takes filename and destinationFolder and moves the file to the destinationFolder
void moveFile(const char *filename, const char *destinationFolder);

// isMalicious takes pathand returns 1 if the file is not malicious
int isMalicious(char *path);

// checkFile takes path and returns 1 if the file is safe
int checkFile(char *path);

// writeInfo takes fd, path and nextFolderPath and writes the info about the files in the snapshot file
void writeInfo(int fd, char path[], char nextFolderPath[]);

// parseDirectory takes directory, path, isolationFolder and fd and parses the directories
void parseDirectory(DIR *directory, char path[],char *isolationFolder,int fd);

char *makeSnapshotName(char *path){
    char tempPath[PATH_SIZE] = "\0";
    strcpy(tempPath,path);
    for (int i = 0; i < strlen(tempPath); ++i) {
        if(tempPath[i] == '/')
            tempPath[i] = '_';
    }
    tempPath[strlen(tempPath)-1] = '\0';
    return strdup(tempPath);
}

char *makeSnapshotPath(char *snapshotPath, char *dirPath){
    unsigned long pathSize = PATH_SIZE + strlen("Snapshots/") + 5;
    char tempPath[pathSize];
    strcpy(tempPath, snapshotPath);
    strcat(tempPath, "snapshot");
    strcat(tempPath, makeSnapshotName(dirPath));
    strcat(tempPath, ".txt");
    return strdup(tempPath);
}

int getFileDescriptor(char *path){
    int fileDescriptor = open(path,O_WRONLY | O_CREAT | O_TRUNC, 0777);
    return fileDescriptor;
}

void writeInfo(int fd, char path[], char nextFolderPath[]) {
    char buffer[MAX_BUFF_SIZE];
    strcpy(buffer, "\0");
    strcat(buffer, "Parrent folder: ");
    strcat(buffer, path);
    strcat(buffer, "\nFile: ");
    strcat(buffer, nextFolderPath);
    strcat(buffer, "\n");
    write(fd, buffer, strlen(buffer));
    struct stat fileStat;

    // Open the file
    int file = open(nextFolderPath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (file == -1) {
        write(fd, "File permissions can not be obtained\n\n\n", strlen("File permissions can not be obtained\n\n\n"));
        perror("open");
        return;
    }
    if (fstat(file, &fileStat) == -1) {
        perror("fstat");
        return;
    }
    // Buffer for access rights information
    int size = 0;

    // Access rights
    size += sprintf(buffer + size, "Access rights: ");

    // Owner permissions
    size += sprintf(buffer + size, (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IXUSR) ? "x" : "-");

    // Group permissions
    size += sprintf(buffer + size, (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IXGRP) ? "x" : "-");

    // Others permissions
    size += sprintf(buffer + size, (fileStat.st_mode & S_IROTH) ? "r" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    size += sprintf(buffer + size, (fileStat.st_mode & S_IXOTH) ? "x" : "-");

    size += sprintf(buffer + size, "\n\n\n");

    // Write the buffer to the file
    if (write(fd, buffer, size) == -1) {
        perror("write");
        return;
    }
}

char *makeTempPath(char *path, char *nextFolderName){
    char tempPath[PATH_SIZE] = "\0";
    strcpy(tempPath, path);
    strcat(tempPath, "/");
    strcat(tempPath, nextFolderName);
    return strdup(tempPath);
}

int hasNoPermissions(char *path){
    struct stat st;
    if (lstat(path, &st) == -1) {
        if (errno == ENOENT) {
            printf("File %s does not exist.\n", path);
        }
    }
    //Check if it's readable, writable or executable in any group
    if (st.st_mode & S_IRWXU || st.st_mode & S_IRWXG || st.st_mode & S_IRWXO) {
        return 0;
    }
    return 1;
}

void moveFile(const char *filename, const char *destinationFolder) {
    char command[1024];
    // Construct the command to move the file using system function
    sprintf(command, "mv %s %s/", filename, destinationFolder);

    // Execute the command
    int result = system(command);

    // Check if the command was executed successfully
    if (result == 0) {
        perror("File moved successfully.\n");
    } else {
        perror("Failed to move the file.\n");
    }
}

int isMalicious(char *path){
    int pfd[2];
    int pid;
    if(pipe(pfd) == -1){
        perror("Pipe creation error\n");
        exit(1);
    }
    pid = fork();
    if(pid == -1){
        perror("Fork error\n");
        exit(1);
    } else if(pid == 0){

        close(pfd[0]);

        int give_access=chmod(path, 777); //giving read access to the "malicious file"
        char argument[100];  //making the command to run the script
        snprintf(argument, sizeof(argument), "./analysis.sh \'%s'", path);

        int file_status = system(argument); //executing the script
        write(pfd[1], &file_status, sizeof(file_status));

        give_access = chmod(path, 000); //changing again the acces rights to 0
        close(give_access);
        close(pfd[1]);
        exit(0);
    }
    close(pfd[1]);
    int result;
    read(pfd[0], &result, sizeof(result));
    close(pfd[0]);
    return result;
}

int checkFile(char *path){
    if(hasNoPermissions(path) && isMalicious(path))
        return 0;
    return 1;
}
// Checking for differences between snapshots is not implemented
// by updating the snapshot file every time we get the same expected behaviour
// while not wasting time on unnecessary checks as even the smallest change in the file
// would result in having to overwrite the snapshot file
void parseDirectory(DIR *directory, char path[],char *isolationFolder,int fd){

    //We go through every file of the directory
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;

        if (strcmp(nextFolderName, ".") == 0 || strcmp(nextFolderName, "..") == 0)
            continue; // if the folder is "." or ".." there is no need to parse it

        char *tempPath = makeTempPath(path, nextFolderName);

        if(checkFile(tempPath) == 0){
            potentiallyDangerousFiles++;
            moveFile(tempPath,isolationFolder);
            continue;
        }else{
            writeInfo(fd,path,tempPath);
        }

        //The next directory is initialised and if not null it will be parsed
        DIR *nextFolder = opendir(tempPath);
        if (nextFolder != NULL) {
            parseDirectory(nextFolder, tempPath, isolationFolder, fd);
        }
    }
}

int main(int argc, char* argv[]){
    if (argc <= 5) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    if (strcmp(argv[1],"-o") != 0) {
        printf("Second argument should be -o\n");
        printf("And the third should the output folder for the snapshots \n");
        return -1;
    }
    if (strcmp(argv[3],"-s") != 0) {
        printf("Fourth argument should be -o\n");
        printf("And the fifth should state the isolation folder for malicious files\n");
        return -1;
    }
    char path[PATH_SIZE] = "\0";

    int status;
    for (int i = 5; i < argc; ++i) {
        pid_t childPid;
        childPid = fork();
        if(childPid == -1) {
            perror("fork\n");
            return -1;
        }else if(childPid == 0) {

            strcpy(path, argv[i]);
            DIR *directory = opendir(path);
            if (directory == NULL) {
                perror(path);
                return -1;
            }
            int fd = getFileDescriptor(makeSnapshotPath(argv[2], argv[i]));
            parseDirectory(directory, path,argv[4], fd);
            close(fd);
            printf("Snapshot for %s created\n", path);
            printf("Child has %d files with potential danger\n", potentiallyDangerousFiles);
            return 0;
        }
    }
    for (int i = 0; i < argc-5; ++i) {
        int pidChild = wait(&status);
        if(WIFEXITED(status)) {
            printf("Child process %d terminated with PID %d\n\n\n", i, pidChild);
        } else {
            printf("Child exited abnormally\n");
        }
    }
    return 0;
}
