
#define MAXLINE 256
#include <unistd.h> 
#include <stdio.h>  
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>

// ics 53
// Winter 2021
int LISTENQ = 1;
typedef struct sockaddr SA;


int open_listenfd(char *port)
{
struct addrinfo hints, *listp, *p;
int listenfd, optval=1;
/* Get a list of potential server addresses */
memset(&hints, 0, sizeof(struct addrinfo));
hints.ai_socktype = SOCK_STREAM; /* Accept connect. */
hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* …on any IP addr */
hints.ai_flags |= AI_NUMERICSERV; /* …using port no. */
getaddrinfo(NULL, port, &hints, &listp);
/* Walk the list for one that we can bind to */
for (p = listp; p; p = p->ai_next) {
/* Create a socket descriptor */
if ((listenfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) < 0)
continue; /* Socket failed, try the next */
/* Eliminates "Address already in use" error from bind */
setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
(const void *)&optval , sizeof(int));
/* Bind the descriptor to the address */
if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
break; /* Success */
close(listenfd); /* Bind failed, try the next */
}
/* Clean up */
freeaddrinfo(listp);
if (!p) /* No address worked */
return -1;
/* Make it a listening socket ready to accept conn. requests */
if (listen(listenfd, LISTENQ) < 0) {
close(listenfd);
return -1;
}
printf("server started\n");
return listenfd;
}

  char * removenewline(char * command)
{
            char* position;
            int i;
            for(i = 0; i < sizeof(command); i++)
            {

            if((position = strchr(command,'\n')) != NULL)
            {
                *position = '\0';
            }
            }

    return command;
}



sem_t mutex;

    struct FileInUse // this is the struct we will use to hold single line info
    {
    int inUse; //flag to let us know if we can overwrite this info or not
    char filename[20]; // name of the file 
    FILE* fileptr;
    int readflag; // flag if it was a read
    int appendflag; // flag if it was an append
    pthread_t callingThread; // keeps ID of thread that called file us (for multiple reads)
    };
struct FileInUse FileDatabase[4]; // each client can only have one file open, regardless if to read or append, there will be no more than 4 clients


void echo(int connfd);
void sigchld_handler(int sig);
void *thread(void *vargp);
int main(int argc, char **argv)
{

int initialize;
for(initialize = 0; initialize < 4; initialize++) // initialize the database
{
    FileDatabase[initialize].inUse = 10;
    FileDatabase[initialize].readflag = 0;
    FileDatabase[initialize].appendflag = 0;
}




sem_init(&mutex, 0 , 1);

int listenfd, *connfdp;
socklen_t clientlen;
struct sockaddr_storage clientaddr;
pthread_t tid;
listenfd = open_listenfd(argv[1]);
while (1) {
clientlen=sizeof(struct sockaddr_storage);
connfdp = malloc(sizeof(int));
*connfdp = accept(listenfd, (SA *) &clientaddr, &clientlen);
pthread_create(&tid, NULL, thread, connfdp);
}
}
/* Thread routine */
void *thread(void *vargp)
{
int connfd = *((int *)vargp);
pthread_detach(pthread_self());
free(vargp);
echo(connfd);
close(connfd);
return NULL;
}





void sigchld_handler(int sig)
{
while (waitpid(-1, 0, WNOHANG) > 0);
return;
}



int readflag = 0; // keeps track if a file is open to read
int appendflag = 0;
FILE* filereadptr;
FILE* fileappendptr;
char appendname[50];
char readname[50];



void openfileread (char * filename) //open file for reading
{
if (fopen(filename,"r") != NULL) // if file exists and can be successfully opened
{
    int fileptrindex = -1;
    int indexsearch = 0;
    while(fileptrindex == -1 && indexsearch < 4)
    {
        if(FileDatabase[indexsearch].inUse == 10)
        {
            //printf("FOUND OPEN STRUCT AT INDEX %i\n", indexsearch);
            fileptrindex = 1;
        }
        else
        {
            indexsearch++;
        }
    }
    FileDatabase[indexsearch].fileptr = fopen(filename, "r"); // set file descriptor
    strcpy(FileDatabase[indexsearch].filename,filename); //set filename
    FileDatabase[indexsearch].readflag = 1; // set readflag
    FileDatabase[indexsearch].callingThread = pthread_self(); // set thread id
    FileDatabase[indexsearch].inUse = 20; // set in use flag
    return;
}
}
int fileopentoappend = 0;
void openfileappend (char * filename) //open file for appending
{


    int fileptrindex = -1;
    int indexsearch = 0;
    while (indexsearch < 4 && fileopentoappend == 0)
    {
        //printf("CHECKING INDEX %i\n", indexsearch);
        // if it is a file in use
        if(FileDatabase[indexsearch].inUse == 20)
        {
            //if the file in use is the file we are trying AND it is open to append or read
            if(strcmp(filename,FileDatabase[indexsearch].filename) == 0 && FileDatabase[indexsearch].appendflag == 1 || FileDatabase[indexsearch].readflag == 1)
            {
                //printf("FILE ALREADY IN USE INDEX %i\n", indexsearch);
                fileopentoappend = 1;
            }
            else
            {
                indexsearch++;
            }
            

        }
        else
        {
            indexsearch++;
        }
        
    }

if(fileopentoappend == 0)
{
    indexsearch = 0;
    while(fileptrindex == -1 && indexsearch < 4)
    {
        if(FileDatabase[indexsearch].inUse == 10)
        {
            //printf("FOUND OPEN STRUCT AT INDEX %i\n", indexsearch);
            fileptrindex = 1;
        }
        else
        {
            indexsearch++;
        }
    }
    FileDatabase[indexsearch].fileptr = fopen(filename, "a"); // set file descriptor
    strcpy(FileDatabase[indexsearch].filename,filename); //set filename
    FileDatabase[indexsearch].appendflag = 1; // set appendflag
    FileDatabase[indexsearch].callingThread = pthread_self(); // set thread id
    FileDatabase[indexsearch].inUse = 20; // set in use flag

}
    return;
}


void echo(int connfd)
{

size_t n;
char message[MAXLINE];

while((n = read(connfd, message, MAXLINE)) != 0) {
//printf("server received %d bytes\n", (int)n);
//printf("RECIEVED %s\n", message);
//char * command = strtok(message, " ");


if(strstr(message, "APPENDERROR") != NULL)
{
char errorhold[256];
write(connfd,"",MAXLINE);
read(connfd,errorhold,MAXLINE);
printf("%s\n",errorhold);
printf("A file is already open for appending\n");
}

if(strstr(message, "openRead") != NULL)
{
printf("%s\n",message);
char * command = strtok(message, " ");
command = strtok (NULL, " ");
removenewline(command);
openfileread(command); // open file to read
}
else if(strstr(message, "openAppend") != NULL)
{


//sem_wait(&mutex);
printf("%s\n",message);
char * command = strtok(message, " ");
command = strtok (NULL, " ");
removenewline(command);
openfileappend(command); // open file to append
if(fileopentoappend == 1)
{
    fileopentoappend = 0;
    //printf("SENDING IN USE MESSAGE\n");
    printf("The file is open by another client.\n");
    write(connfd,"The file is open by another client.",MAXLINE);
}
else
{
    write(connfd,"",MAXLINE);
}

//sem_post(&mutex);



}
else if(strstr(message, "read") != NULL)
{
printf("%s\n",message);
char * command = strtok(message, " ");
command = strtok (NULL, " ");
removenewline(command);
int readamount = atoi(command);
if(readamount <= 200)//max bytes read at once = 200
{
int readindexfinder = 0;
int stilllooking = -1;
while(stilllooking == -1 && readindexfinder < 4)
{
    if(FileDatabase[readindexfinder].inUse == 20 && FileDatabase[readindexfinder].readflag == 1 && FileDatabase[readindexfinder].callingThread == pthread_self())
    {
        stilllooking = 1;
        //printf("FOUND A MATCH AT INDEX %i\n", readindexfinder);
    }
    else
    {
        readindexfinder++;
    }
    
}


char buff[readamount];
if(fgets(buff,readamount + 1,FileDatabase[readindexfinder].fileptr) != NULL)
{
write(connfd,buff,MAXLINE);
}
else
{
    strcpy(buff,""); //If the file contains no remaining bytes then nothing should be printed on the screen of the client.
    write(connfd,buff,MAXLINE);
}

}
else
{
    char error[50] = "MAX BYTE READ LIMIT IS 200";
    write(connfd,error,MAXLINE);
}

}
else if(strstr(message, "append") != NULL) // There will be no whitespace in the string that the user wants to append to a file. VIA PIAZZA
{
sem_wait(&mutex);
printf("%s\n",message);

//sleep(10);
char * command = strtok(message, " ");
command = strtok (NULL, " ");
//removenewline(command);
if(strlen(command) <= 200)//max bytes read at once = 200
{

int appendindexfinder = 0;
int stilllooking = -1;
while(stilllooking == -1 && appendindexfinder < 4)
{
    if(FileDatabase[appendindexfinder].inUse == 20 && FileDatabase[appendindexfinder].appendflag == 1 && FileDatabase[appendindexfinder].callingThread == pthread_self())
    {
        stilllooking = 1;
        //printf("FOUND A MATCH AT INDEX %i\n", appendindexfinder);
    }
    else
    {
        appendindexfinder++;
    }
    
}

    char holder[200];
    strcpy(holder, command);
    fprintf(FileDatabase[appendindexfinder].fileptr, "%s", holder);
    write(connfd,"",MAXLINE);
}
else
{
    char error[50] = "MAX BYTE APPEND LIMIT IS 200";
    write(connfd,error,MAXLINE);
}


sem_post(&mutex);
}
else if(strstr(message, "close") != NULL)
{
sem_wait(&mutex);
printf("%s\n",message);
char * command = strtok(message, " ");
command = strtok (NULL, " ");
removenewline(command);

int closeindexfinder = 0;
int stilllooking = -1;
while(stilllooking == -1 && closeindexfinder < 4)
{
    if(FileDatabase[closeindexfinder].inUse == 20 && FileDatabase[closeindexfinder].callingThread == pthread_self())
    {
        stilllooking = 1;
        //printf("FOUND A MATCH AT INDEX %i\n", closeindexfinder);
    }
    else
    {
        closeindexfinder++;
    }
    
}

fclose(FileDatabase[closeindexfinder].fileptr);
FileDatabase[closeindexfinder].inUse = 10;
FileDatabase[closeindexfinder].readflag = 0;
FileDatabase[closeindexfinder].appendflag = 0;

sem_post(&mutex);
}
}
}