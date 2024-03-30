#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
//Description: The proposed project combines functionalities for monitoring a directory to manage differences between two captures (snapshots) of it.
// The user will be able to observe and intervene in the changes in the monitored directory.

//Directory Monitoring:
//The user can specify the directory to be monitored as an argument in the command line, and the program will track
// changes occurring in it and its subdirectories, parsing recursively each entry from the directory.
//With each run of the program, the snapshot of the directory will be updated, storing the metadata of each entry.

void parseDirectory(DIR *directory, char path[]){

    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;

        if (nextFolderName[0] != '.'){

            char tempPath[1000] = "";
            strcat(tempPath,path);
            strcat(tempPath,nextFolderName);
            strcat(tempPath,"/");
            printf("%s - %s\n", nextFolderName, tempPath);
            DIR *nextFolder = opendir(tempPath);
            if (nextFolder != NULL)
            {
                parseDirectory(nextFolder,tempPath);
            }
        }
    }
}



int main(int argc, char* argv[]){
    /*if (argc != 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }*/
    char *path = "/home/florin/Desktop/os/CA/";
    DIR *directory = opendir(path);
    if (directory == NULL){
        printf("Could not open directory\n");
        return -1;
    }
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;
        if (nextFolderName[0] != '.'){
            char tempPath[1000] = "";
            strcat(tempPath,path);
            strcat(tempPath,nextFolderName);
            strcat(tempPath,"/");
            printf("%s - %s\n", nextFolderName, tempPath);

            DIR *nextFolder = opendir(tempPath);
            if (nextFolder != NULL)
            {
                //printf("===%s===\n",tempPath);
                parseDirectory(nextFolder, tempPath);
            }

        }

    }

    //parseDirectory(directory,strcat(path,"/"));

    return 0;
}
