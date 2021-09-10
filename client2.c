
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <netdb.h>
//Khalil Alkassis 26122346
//Shril Panchigar 83816304
// ics 53
// Winter 2021

int LISTENQ = 1;
typedef struct sockaddr SA;

int open_clientfd(char *hostname, char *port) {
int clientfd;
struct addrinfo hints, *listp, *p;
/* Get a list of potential server addresses */
memset(&hints, 0, sizeof(struct addrinfo));
hints.ai_socktype = SOCK_STREAM; /* Open a connection */
hints.ai_flags = AI_NUMERICSERV; /* …using numeric port arg. */
hints.ai_flags |= AI_ADDRCONFIG; /* Recommended for connections */
getaddrinfo(hostname, port, &hints, &listp);
/* Walk the list for one that we can successfully connect to */
for (p = listp; p; p = p->ai_next) {
/* Create a socket descriptor */
if ((clientfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) < 0)
continue; /* Socket failed, try the next */
/* Connect to the server */
if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
break; /* Success */
close(clientfd); /* Connect failed, try another */
}
/* Clean up */
freeaddrinfo(listp);
if (!p) /* All connects failed */
return -1;
else /* The last connect succeeded */
return clientfd;
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
int readflag = 0; // green light for reading
int appendflag = 0; // green light for appending
int closeflag = 0;
 int main(int argc, char * argv[]) 
 {    
     int MAXLINE = 256;
     char cmdhold[MAXLINE];
     int quitflag = 0;
     char line[MAXLINE];


int clientfd;
char *host, *port, buf[MAXLINE];
host = argv[1];
port = argv[2];
clientfd = open_clientfd(host, port);


          while(quitflag == 0)
     {

         printf("> ");
         fgets(line,80,stdin);
        char * command = strtok(line, " ");
            removenewline(command);
            strcpy(cmdhold,command);


         
         if (strcmp(cmdhold,"openRead") == 0) 
         {
             if(readflag == 0 && appendflag == 0)
             {
           char message[256] = "";
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                }
               }
               //printf("SENDING %s.\n", message);
               write(clientfd,message,MAXLINE); // send request
               readflag = 1;
               closeflag = 1;
             }
             else
             {
                 printf("A file is already open\n");
             }
             

         }

         else if (strcmp(cmdhold,"openAppend") == 0)
         {
             char appenderror[256] = "";
             if(appendflag == 0 && readflag == 0)
             {
           char message[256] = "";
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                }
               }
               //printf("SENDING %s\n", finalmessage);
               write(clientfd,message,MAXLINE); // send request
               read(clientfd,message,MAXLINE); // read response
               if(strcmp(message,"") == 0)
               {
               appendflag = 1;
               closeflag = 1;
               }
               else
               {
                   printf("%s\n", message);
               }
               
             }
             else
             {
               char message[256] = "";
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                }
               }
                 printf("A file is already open\n");
                 write(clientfd,"APPENDERROR",MAXLINE);
                 read(clientfd,appenderror,MAXLINE);
                 write(clientfd,message,MAXLINE);
             }
             
         }
        else if (strcmp(cmdhold,"read") == 0)
         {

         if(readflag == 1)
         {
           char message[256] = "";
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                }
               }
               write(clientfd,message,MAXLINE); // send request
               read(clientfd,message,MAXLINE); // read response
               if(strcmp(message,"") != 0) // print error message if there is one
               {
               printf("%s\n",message);
               }
         }
         else
         {
             printf("File not open\n");
         }

         }
        else if (strcmp(cmdhold,"append") == 0)
         {
             if(appendflag == 1)
             {
           char message[256] = "";
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                }
               }
               write(clientfd,message,MAXLINE); // send request
               read(clientfd,message,MAXLINE); // read response
                if(strcmp(message,"") != 0) // print error message if there is one
               {
               printf("%s\n",message);
               }
             }
             else
             {
                 printf("File not open\n");
             }
             
         }
        else if (strcmp(cmdhold,"close") == 0)
         {

           char message[256] = "";
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                }
               }
               write(clientfd,message,MAXLINE); // send request
               closeflag = 0;
               readflag = 0;
               appendflag = 0;
         }
       
        else if (strcmp(cmdhold,"quit") == 0) //quit command
         {
             if(closeflag == 0)
             {
             quitflag = 1;
             }
             else
             {
                 printf("CANNOT QUIT WITHOUT CLOSING FILE\n");
             }
             
         }
     }
     return 0;
 }