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

    //We go through every file of the directory
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;

        //We write info about the file in snapshot.txt ===TBD===


        if (nextFolderName[0] != '.'){
            //We generate the path to the next possible directory
            char tempPath[1000] = "";
            strcat(tempPath,path);
            strcat(tempPath,nextFolderName);
            strcat(tempPath,"/");

            //Debug purposes
            printf("%s - %s\n", nextFolderName, tempPath);

            //The next directory is initialised
            //If not null it will be parsed
            DIR *nextFolder = opendir(tempPath);
            if (nextFolder != NULL) {
                parseDirectory(nextFolder, tempPath);
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

    parseDirectory(directory, path);

    return 0;
}
