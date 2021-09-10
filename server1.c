#define MAXLINE 256
#include <unistd.h> 
#include <stdio.h>  
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
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

    struct LineOfCSV // this is the struct we will use to hold single line info
    { 
    char DATE[20];
    double OPEN;
    double HIGH;
    double LOW;
    double CLOSE;
    double ADJCLOSE;
    int VOLUME;
    };
struct LineOfCSV compiledcsvone[550];
struct LineOfCSV compiledcsvtwo[550];
int countone = 0;
int counttwo = 0;
void compilecsv(char * csvfile, char * csvfile2)
{
FILE* fileptr;
int bufflength = MAXLINE;
char buff[bufflength];
char * command;
removenewline(command);
int counter = 0;


fileptr = fopen(csvfile, "r");
while (fgets(buff,bufflength,fileptr))
{
    command = strtok(buff, ",");
    command = removenewline(command);
    while(command != NULL)
    {

        //compiledcsvone[counter].DATE = malloc(sizeof(char) * (strlen(command) + 1));
        strcpy(compiledcsvone[counter].DATE,command);
        //printf("DATE %s\n", command);
        command = strtok(NULL,",");
        compiledcsvone[counter].OPEN = atof(command);
        //printf("OPEN %s\n", command);
        command = strtok(NULL,",");
        compiledcsvone[counter].HIGH = atof(command);
        //printf("HIGH %s\n", command);
        command = strtok(NULL,",");
        compiledcsvone[counter].LOW = atof(command);
        //printf("LOW %s\n", command);
        command = strtok(NULL,",");
        compiledcsvone[counter].CLOSE = atof(command);
        //printf("CLOSE %s\n", command);
        command = strtok(NULL,",");
        compiledcsvone[counter].ADJCLOSE = atof(command);
        //printf("Adj CLOSE %s\n", command);
        command = strtok(NULL,",");
        compiledcsvone[counter].VOLUME = atoi(command);
        //printf("VOLUME %s\n", command);
        command = strtok(NULL,",");
        //printf("COUNTER %i\n", counter);
        counter++;
    }
}


fileptr;
bufflength = MAXLINE;
buff[bufflength];
countone = counter;
counter = 0;


fileptr = fopen(csvfile2, "r");
while (fgets(buff,bufflength,fileptr))
{
    command = strtok(buff, ",");
    command = removenewline(command);
    while(command != NULL)
    {

        //compiledcsvtwo[counter].DATE = malloc(sizeof(char) * (strlen(command) + 1));
        strcpy(compiledcsvtwo[counter].DATE,command);
        //printf("DATE %s\n", command);
        command = strtok(NULL,",");
        compiledcsvtwo[counter].OPEN = atof(command);
        //printf("OPEN %s\n", command);
        command = strtok(NULL,",");
        compiledcsvtwo[counter].HIGH = atof(command);
        //printf("HIGH %s\n", command);
        command = strtok(NULL,",");
        compiledcsvtwo[counter].LOW = atof(command);
        //printf("LOW %s\n", command);
        command = strtok(NULL,",");
        compiledcsvtwo[counter].CLOSE = atof(command);
        //printf("CLOSE %s\n", command);
        command = strtok(NULL,",");
        compiledcsvtwo[counter].ADJCLOSE = atof(command);
        //printf("Adj CLOSE %s\n", command);
        command = strtok(NULL,",");
        compiledcsvtwo[counter].VOLUME = atoi(command);
        //printf("VOLUME %s\n", command);
        command = strtok(NULL,",");
        //printf("COUNTER %i\n", counter);
        counter++;
    }
}
counttwo = counter;
}



void echo(int connfd);
char csvfileone[20] = "";
char csvfiletwo[20] = "";

int main(int argc, char **argv)
{
compilecsv(argv[1],argv[2]);
strcpy(csvfileone,argv[1]);
strcpy(csvfiletwo, argv[2]);
int listenfd, connfd;
socklen_t clientlen;
struct sockaddr_storage clientaddr; /* Enough room for any addr */
char client_hostname[MAXLINE], client_port[MAXLINE];
listenfd = open_listenfd(argv[3]);
while (1) {
clientlen = sizeof(struct sockaddr_storage); /* Important! */
connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
getnameinfo((SA *) &clientaddr, clientlen,
client_hostname, MAXLINE, client_port, MAXLINE, 0);
//printf("Connected to (%s, %s)\n", client_hostname, client_port);
echo(connfd);
close(connfd);
}
exit(0);
}
void echo(int connfd)
{
size_t n;
char message[MAXLINE];
char commandtype[20] = "";
char CSVFile[20] = "";
char CSVFILENOTAIL[20] = "";
char Date[20] = "";
while((n = read(connfd, message, MAXLINE)) != 0) {
//printf("server received %d bytes\n", (int)n);
//printf("RECIEVED %s\n", message);
char * command = strtok(message, " ");
if(strstr(command, "Prices") != NULL)
{
    command = strtok (NULL, " ");
    removenewline(command);
    strcpy(CSVFile, command);
    command = strtok (NULL, " ");
    removenewline(command);
    strcpy(Date, command);
    strcpy(CSVFILENOTAIL,CSVFile);
    strcat(CSVFile,".csv");
    int csvfound = 0;
    int allgoodflag = 0;

            int datetest;
            int fourflag = 0;
            int sevflag = 0;
        for(datetest = 0; datetest < strlen(command); datetest++)
        {
            if(command[datetest] == '-')
            {
                if(datetest == 4)
                {
                for(datetest = 4; datetest < strlen(command); datetest++)
                {
                    if(command[datetest] == '-')
                    {
                        if(datetest == 7)
                        {
                            fourflag = 1;
                            sevflag = 1;
                            datetest = strlen(command) + 1;
                        }
                    }
                }
                }
            }
        }
        if(fourflag == 0 || sevflag == 0)
        {
          write(connfd,"Invalid syntax",MAXLINE);
        }
    if(fourflag == 1 && sevflag == 1)
    {
    if(strcmp(CSVFile,csvfileone) == 0) // if the CSVFILE from the command matches our first one loaded into server
    {
        int count;
        int foundflag = 0; // in case value isnt found
        csvfound = 1;
        for(count = 0; count < countone; count++)
        {
            if(strcmp(compiledcsvone[count].DATE,Date) == 0)
            {
                char holder[20];
                sprintf(holder,"%f",compiledcsvone[count].CLOSE); 
                write(connfd,holder,MAXLINE);
                foundflag = 1;
                printf("Prices %s %s\n", CSVFILENOTAIL,Date);
            }
        }
        if(foundflag == 0)
        {
            printf("Prices %s %s\n", CSVFILENOTAIL,Date);
            write(connfd,"Unknown",MAXLINE);
        }
    }
    if(strcmp(CSVFile,csvfiletwo) == 0) // if the CSVFILE from the command matches our second one loaded into server
    {
        int count;
        int foundflag = 0; // in case value isnt found;
        csvfound = 1;
        for(count = 0; count < counttwo; count++)
        {
            if(strcmp(compiledcsvtwo[count].DATE,Date) == 0)
            {
                char holder[20];
                sprintf(holder,"%f",compiledcsvtwo[count].CLOSE); // get length of string and add to beginning
                write(connfd,holder,MAXLINE);
                foundflag = 1;
                printf("Prices %s %s\n", CSVFILENOTAIL,Date);
            }
        }
        if(foundflag == 0)
        {
            printf("Prices %s %s\n", CSVFILENOTAIL,Date);
            write(connfd,"Unknown",MAXLINE);
        }
    }
            if(csvfound == 0)
        {
            write(connfd,"Invalid syntax",MAXLINE);
        }
    }

}
else if(strstr(command, "MaxProfit") != NULL)
{
    int foundflag = 0;
    command = strtok (NULL, " ");
    removenewline(command);
    strcpy(CSVFile, command);
    printf("MaxProfit %s\n", CSVFile);
    strcat(CSVFile,".csv");
        if(strcmp(CSVFile,csvfileone) == 0) // if the CSVFILE from the command matches our first one loaded into server
    {
                 foundflag = 1;
                 double lowBuy = 0;
                 double highSell = 0;
                 double maxprofit = 0;
                 int x = 0;
                 double lowholder;
                 int lowcount;
                 for(x; x < 504; x++){
                     if (x == 1){
                         lowholder = compiledcsvone[x].CLOSE;
                         highSell = compiledcsvone[x].CLOSE;
                         lowcount = x;
                        //printf("LOWBUY %f HIGHSELL %f\n", lowBuy,highSell);

                     }
                     else {
                         if (lowholder > compiledcsvone[x].CLOSE)
                         {
                             lowcount = x;
                             lowholder = compiledcsvone[x].CLOSE;

                         }
                         if (highSell < compiledcsvone[x].CLOSE)
                         {
                             highSell = compiledcsvone[x].CLOSE;
                             if(x >= lowcount)
                             {
                                lowBuy = lowholder;
                             }
                         }
                     }
                        //printf("LOWBUY %f HIGHSELL %f\n", lowBuy,highSell);

                 }
                 maxprofit = highSell - lowBuy;
                 char holder[20];
                sprintf(holder,"%f",maxprofit); // get length of string and add to beginning
                write(connfd,holder,MAXLINE);
    }
        if(strcmp(CSVFile,csvfiletwo) == 0) // if the CSVFILE from the command matches our first one loaded into server
    {
                 foundflag = 1;
                 double lowBuy = 0;
                 double highSell = 0;
                 double maxprofit = 0;
                 int x = 0;
                 double lowholder;
                 int lowcount;
                 for(x; x < 504; x++){
                     if (x == 1){
                         lowholder = compiledcsvtwo[x].CLOSE;
                         highSell = compiledcsvtwo[x].CLOSE;
                         lowcount = x;
                        //printf("LOWBUY %f HIGHSELL %f\n", lowBuy,highSell);

                     }
                     else {
                         if (lowholder > compiledcsvtwo[x].CLOSE)
                         {
                             lowcount = x;
                             lowholder = compiledcsvtwo[x].CLOSE;

                         }
                         if (highSell < compiledcsvtwo[x].CLOSE)
                         {
                             highSell = compiledcsvtwo[x].CLOSE;
                             if(x >= lowcount)
                             {
                                lowBuy = lowholder;
                             }
                         }
                     }
                        //printf("LOWBUY %f HIGHSELL %f\n", lowBuy,highSell);

                 }
                 maxprofit = highSell - lowBuy;
                 char holder[20];
                sprintf(holder,"%f",maxprofit); // get length of string and add to beginning
                write(connfd,holder,MAXLINE);
    }
    if(foundflag == 0)
    {
    write(connfd,"Unknown",MAXLINE);
    }
}
}
}