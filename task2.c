#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

char *makeSnapshotName(char *path){
    char tempPath[100] = "\0";
    strcpy(tempPath,path);
    for (int i = 0; i < strlen(tempPath); ++i) {
        if(tempPath[i] == '/')
            tempPath[i] = '_';
    }
    tempPath[strlen(tempPath)-1] = '\0';
    return strdup(tempPath);
}

char *makePath(char *snapshotPath, char *dirPath){
    unsigned long pathSize = 100 + strlen("Snapshots/") + 5;
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

void parseDirectory(DIR *directory, char path[],int fd){

    //We go through every file of the directory
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;

        if (nextFolderName[0] == '.')
            continue; // if the folder is "." or ".." there is no need to parse it

        //We write info about the file in the snapshot .txt file
        char buff[4096];
        strcpy(buff,"\0");
        strcat(buff, nextFolderName);
        strcat(buff," - ");
        strcat(buff, path);
        strcat(buff, "\n");
        write(fd,buff, strlen(buff));

        //We generate the path to the next possible directory
        char tempPath[1000] = "";
        strcat(tempPath,path);
        strcat(tempPath,nextFolderName);
        strcat(tempPath,"/");

        //The next directory is initialised
        //If not null it will be parsed
        DIR *nextFolder = opendir(tempPath);
        if (nextFolder != NULL) {
            printf("%s === %d\n",tempPath,fd);
            parseDirectory(nextFolder, tempPath, fd);
        }
    }
}

int main(int argc, char* argv[]){
    if (argc <= 4) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    if (strcmp(argv[1],"-o") != 0) {
        printf("Second argument should be -o\n");
        printf("And state the output folder for the snapshots\n");
        return -1;
    }
    char path[100] = "\0";
    for (int i = 3; i < argc; ++i) {
        int fd = getFileDescriptor(makePath(argv[2],argv[i]));

        strcpy(path,argv[i]);
        DIR *directory = opendir(path);
        if (directory == NULL){
            printf("Could not open directory\n");
            return -1;
        }
        parseDirectory(directory, path, fd);
    }
    return 0;
}
