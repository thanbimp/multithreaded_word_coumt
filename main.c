#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


int forkTarget=1;
pthread_mutex_t lock;
//global variables

int *myThread(char input[])
{
    int i=0;
    int words=0;
    while (i<strlen(input)){
        if ( input[i] == ' ' || input[i] == '\n'){
            words++; //increment word count on space and newline apperance
        }
        i++; //increment string index counter
    }
    return (void *) words;

}

void myFun(char *path, FILE *fp) {
    int forkCounter = 0; //used to make correct amount of forks
    void *wordsCount;
    int wordsSum=0;
    pthread_t threadId;
    char *threadParts[5];
    int pid;
    pid = fork();
    do {
        if (pid != 0) { //do only when in child process
            printf("Process with ID :%d for file :%s created\n",pid,path);
            char *buffer = 0;
            long length;
            FILE *f = fopen(path, "rb"); //open current file

            if (f) {
                fseek(f, 0, SEEK_END);
                length = ftell(f);
                fseek(f, 0, SEEK_SET);
                buffer = malloc(length);
                if (buffer) {
                    fread(buffer, 1, length, f); //copy file to string
                }
                fclose(f);
            }
            if (buffer) { //start splitting string into 5 smaller strings
                int parts;
                int finalPart;
                int length=strlen(buffer);
                parts=length/5; //determine length of first 4 parts
                finalPart=length-(parts*5)+parts; //determine length of 5th part
                int currChar=0;
                for (int j = 0; j < 4; ++j) { //copy first 4 parts
                    threadParts[j] = (char*) malloc (parts);
                    strncpy(threadParts[j],buffer + currChar,parts);
                    currChar=currChar+parts;
                }
                threadParts[4] = (char*) malloc (finalPart);
                strncpy(threadParts[4],buffer + currChar,finalPart); //copy final (and bigger part)
            }
            for (int j = 0; j < 5; ++j) { //create 5 threads,one for each string
                printf("Thread:%d created\n",j);
                pthread_mutex_lock(&lock);
                pthread_create(&threadId, NULL, &myThread,threadParts[j]); //give one part of string to one thread
                pthread_join(threadId,&wordsCount);
                printf("Thread:%d terminated\n",j);
                pthread_mutex_unlock(&lock);
                wordsSum=wordsSum+(int)wordsCount; //sum returned word counts from threads
            }
            fprintf(fp," %s , %d\n",path,wordsSum); //append to file
            forkCounter++; //increment counter for flow control
            kill(pid, SIGKILL); //kill the forked process
            printf("Process with ID :%d for file :%s terminated\n",pid,path);
        }
    }while (forkCounter < forkTarget);
}


int main(int argc, char const *argv[])
{
    FILE *fp;
    int run=0; //used to determine if sucess message should be printed
    char folder[PATH_MAX]; //String that will hold final path
    struct stat info; // new stat struct
    stat(argv[1],&info); //get stats for path at argv[1]
    int i=0;
    int filecounter=0;
    if(argv[1]==NULL){ //if argv[1] is empty
        getcwd(folder, sizeof(folder)); //get current working directory'
        printf("No directory specified,running on current directory!\n");
    }
    else if (S_ISDIR(info.st_mode)){ //else if argv[1] is not empty and has folder path
        strcpy(folder,argv[1]); // copy argv[1] to string;
    }
    else{ //if invalid folder specified
        printf("Invalid Folder\n");
        return 0; //exit
    }
    fp =  fopen("output.txt", "wb");
    DIR *directory; //new DIR
    struct dirent *dir; //new dirent struct
    directory=opendir(folder); //open directory from string
    //start counting files
    while((dir = readdir(directory)) != NULL) {
        if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..") ) //skip . and ..
        {

        } else if(dir->d_type==DT_REG) { //increment counter on normal file appearance
            filecounter++;
        }
    }
    char *filesList[filecounter];
    rewinddir(directory);
//Put file names into the array
    while((dir = readdir(directory)) != NULL) {
        if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..") ) //skip . and ..
        {

        } else if(dir->d_type==DT_REG) { //increment counter on normal file appearance
            filesList[i] = (char*) malloc (strlen(dir->d_name)+1);
            strncpy (filesList[i],dir->d_name, strlen(dir->d_name) );
            i++;
        }
    }
    rewinddir(directory);
    int ASCIIFilecounter=0;
    for (int j = 0; j < filecounter; ++j) { //repeat for each file in folder
        char pathname[PATH_MAX];
        sprintf( pathname, "%s/%s", folder, filesList[j] );
        FILE *file = fopen(pathname, "r"); //open current file
        int c = fgetc(file);
        int terminate=0;
        while ((c = fgetc(file)) != EOF && terminate == 0)
        {
            if (c < 0 || c > 127)
            {
                terminate=1; //if a character in the file is out of ASCII range,set to one

            }
        }
        if (terminate == 0){
            ASCIIFilecounter++; //if all characters are ASCII,increment the counter
        }
        fclose(file);
    }
    rewinddir(directory);
    char *ASCIIfilesList[ASCIIFilecounter];
    int counter=0;
    for (int j = 0; j < filecounter; j++) {
        char pathname[PATH_MAX]; //repeat for each file in folder
        sprintf(pathname, "%s/%s", folder, filesList[j]);
        FILE *file = fopen(pathname, "r"); //open current file
        int c = fgetc(file);
        int terminate = 0;
        while ((c = fgetc(file)) != EOF && terminate == 0) {
            if (c < 0 || c > 127) {
                terminate = 1;//if a character in the file is out of ASCII range,set to one

            }
        }
        if (terminate == 0) {
            ASCIIfilesList[counter] = (char *) malloc(strlen(filesList[j]));
            strncpy(ASCIIfilesList[counter], filesList[j], strlen(filesList[j])); //if all characters are ASCII,copy the filename to the array
            counter++;
        }
        fclose(file);
    }
    for (int i = 0; i < ASCIIFilecounter; ++i) { //call myFun for each ASCII file in folder
        run=1;
        char pathname[PATH_MAX];
        sprintf(pathname, "%s/%s", folder, ASCIIfilesList[i]); //create a string in the format "/folder/file" to pass it to myFun
        myFun(pathname,fp);
    }
    if(run==1) {
        printf("The output file has been created!\n");  //only prints if myFun is run
    }
    return 0;
}