
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
            for(int i = 0; i < sizeof(command); i++)
            {

            if((position = strchr(command,'\n')) != NULL)
            {
                *position = '\0';
            }
            }

    return command;
}
 int main(int argc, const char * argv[]) 
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
//printf("%i\n", clientfd);


          while(quitflag == 0)
     {

         printf("> ");
         fgets(line,80,stdin);
        char * command = strtok(line, " ");
            removenewline(command);
            strcpy(cmdhold,command);


         
         if (strcmp(cmdhold,"Prices") == 0) //prices command
         {
             /*
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            char CSVFILE[80]; // which csvfile to check
            int size = strlen(cmdhold);
            sprintf(CSVFILE,"%d",size); // get length of string and add to beginning
            strcat(CSVFILE,cmdhold);

            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            char DATE[20]; // date to check
            size = strlen(cmdhold);
            sprintf(DATE,"%d",size); // get length of string and add to beginning
            strcat(DATE,cmdhold);
            
            printf("CHECKING FILE %s FOR DATE %s\n", CSVFILE,DATE);
            printf("%s\n", argv);
            */
           char message[256] = "";
           int messagelength = 0;
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                messagelength+= strlen(command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcat(message," ");
                    messagelength++;
                }
               }
               char finalmessage[256] = ""; // final message to add length to
               sprintf(finalmessage,"%d",messagelength);
               strcat(finalmessage,message);
               //printf("SENDING %s\n", finalmessage);
               write(clientfd,finalmessage,MAXLINE);
               read(clientfd,finalmessage,MAXLINE);
               if(strcmp(finalmessage,"Unknown") == 0 || strcmp(finalmessage,"Invalid syntax") == 0)
               {
                   printf("%s\n",finalmessage);
               }
               else
               {
               double price = atof(finalmessage);
               printf("%4.2f\n",price);
               }
               
         }

         else if (strcmp(cmdhold,"MaxProfit") == 0)//maxprofit command
         {
           char message[256] = "";
           char csvfile[20] = "";
           int messagelength = 0;
               while(command != NULL)
               {
                removenewline(command);
                strcat(message,command);
                messagelength+= strlen(command);
                command = strtok (NULL, " ");
                if(command != NULL)
                {
                    strcpy(csvfile,command);
                    strcat(message," ");
                    messagelength++;
                }
               }
               char finalmessage[256] = ""; // final message to add length to
               sprintf(finalmessage,"%d",messagelength);
               strcat(finalmessage,message);
               //printf("SENDING %s\n", finalmessage);
               write(clientfd,finalmessage,MAXLINE);
               read(clientfd,finalmessage,MAXLINE);
               if(strcmp(finalmessage,"Unknown") == 0)
               {
                   printf("%s\n",finalmessage);
               }
               else
               {
               double maxprofit = atof(finalmessage);
                removenewline(csvfile);
               printf("Maximum Profit for %s: %4.2f\n",csvfile,maxprofit);
               }
         }
       
        else if (strcmp(cmdhold,"quit") == 0) //quit command
         {
             quitflag = 1;
         }
     }
     return 0;
 }