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
// ics 53
// Winter 2021
  int tokenize(char *line); 

  struct Jobs { // this is the struct we will use to hold the information that defines a job
   int jobid; 
   int processid;
   char status[20];
   char proccessname[80];
   int InUseFlag;
   };
   struct Jobs JobArray[5];
   int sigflag;
   int IndexToUse = 0;
   int tobgpid;
   int tofgpid;
   int tokillpid;
   int percentflag = 0;
   pid_t shellpid;
   pid_t shellpgid;
   int CurrentJobs = 0;


    int Maxjob = 5;
    int MaxLine = 80;


 void signal_handler(int signum) // this will handle passed signals
 {
     if(signum == SIGTSTP)
     {
    strcpy(JobArray[IndexToUse].status, "Stopped");
    JobArray[IndexToUse].InUseFlag = 1;
     }


     if(signum == SIGCONT)
     {
         int foundverflag;
         for(int i = 0; i < Maxjob; i++)
         {
             if(JobArray[i].processid == tobgpid) // checks for matching process id
             {
                kill(tobgpid,SIGCONT);
                strcpy(JobArray[i].status, "Background");
                foundverflag = 1;
             }
             if(JobArray[i].jobid == tobgpid) // check for matching job id
             {
                kill(JobArray[i].processid,SIGCONT);
                strcpy(JobArray[i].status, "Background");
                foundverflag = 1;
             }
         }
         if (foundverflag != 1)
         {
             printf("NO JOB EXISTS WITH THAT PID OR JID\n");
         }

     }

     if(signum == SIGINT)
     {
        int foundverflag;
         for(int i = 0; i < Maxjob; i++)
         {
             if(JobArray[i].processid == tokillpid) // checks for matching process id
             {
                kill(tokillpid,SIGCONT);
                kill(tokillpid,SIGINT);
                waitpid(tokillpid,NULL,WUNTRACED);
                kill(tokillpid,SIGINT);
                JobArray[i].InUseFlag = 0;
                foundverflag = 1;
             }
             if(JobArray[i].jobid == tokillpid) // check for matching job id
             {
                kill(JobArray[i].processid,SIGCONT);
                kill(JobArray[i].processid,SIGINT);
                waitpid(JobArray[i].processid,NULL,WUNTRACED);
                kill(JobArray[i].processid,SIGINT);
                JobArray[i].InUseFlag = 0;
                foundverflag = 1;
             }
         }
         if (foundverflag != 1)
         {
             printf("NO JOB EXISTS WITH THAT PID OR JID\n");
         }
     }


     if(signum == -1) // this will handle foreground requests
     {
         int foundverflag;
         if(percentflag == 1)
         {
         for(int i = 0; i < Maxjob;i++)
         {
             if(JobArray[i].jobid == tofgpid)
             {
                 tofgpid = JobArray[i].processid;
             }
         }
         percentflag = 0;
         }
         for(int i = 0; i < Maxjob; i++)
         {
             if(JobArray[i].processid == tofgpid) // checks for matching process id
             {
                signal(SIGTTOU,SIG_IGN); //ignores signal since it is sent by the child
                strcpy(JobArray[i].status, "Foreground");
                printf("foreground %d\n",shellpgid);
                tcsetpgrp(0,tofgpid);
                kill(tofgpid,SIGCONT);
                pid_t fgtokill = waitpid(tofgpid,NULL,WUNTRACED); //wait for process to finish
                tcsetpgrp(0,shellpgid);
                foundverflag = 1;
                CurrentJobs--;

        for(int i = 0; i < Maxjob; i++) //mark the foreground process as done
        {
            if(JobArray[i].processid == fgtokill)
            {
                JobArray[i].InUseFlag = 0;
            }

        } 
             }
         }
         if (foundverflag != 1)
         {
             printf("NO JOB EXISTS WITH THAT PID OR JID\n");
         }
     }
 }

int *sortjobarray(int jobnumarray[])
{

for (int i = 0; i < Maxjob; i++)                     //Loop for descending ordering
	{
		for (int j = 0; j < Maxjob; j++)             //Loop for comparing other values
		{
			if (jobnumarray[j] > jobnumarray[i])                //Comparing other array elements
			{
                //printf("%i IS LESS THAN %i",jobnumarray[j],jobnumarray[i]);
				int tmp = jobnumarray[i];         //Using temporary variable for storing last value
				jobnumarray[i] = jobnumarray[j];            //replacing value
				jobnumarray[j] = tmp;             //storing last value
			}
		}
	}
    return jobnumarray;
}





 int main(int argc, const char * argv[]) 
 {
     signal(SIGTSTP,signal_handler); // handles ctrl Z signal
     int MaxArgc = 80;


     int AvailableJobID = 0; //instructor said on piazza job ids are not reused, they simply count upwards 
    
     int quitflag = 0;
    char line[MaxLine];
    char cmdget[MaxLine];
    char *argvholder[MaxArgc];
    pid_t pid = getpid();
    shellpid = getpid();
    //printf("PID: %i\n", pid);
    





     while(quitflag == 0)
     {
         if(pid == 0) // child process
         {
             char*array[80]; // this array holds each word from argv
             char lineholder [500];

            int numat = 0; //will hold index so we can go through array later







            //took this part from the last HW, it tokenizes argv and puts it in array
            char * command = strtok(line, " ");
            array[numat] = command;
            numat++;
            while (command != NULL)
            {
            command = strtok (NULL, " ");
            array[numat] = command;
            numat++;
            }
            

            //each token in array has a \n variable at the end
            //this checks each token for \n and if it is there, replaces it with \0
            char*pos;
            for(int i = 0; i < (numat-1); i++)
            {
            strcpy(lineholder,array[i]);
            if((pos = strchr(lineholder,'\n')) != NULL)
            {
                *pos = '\0';
            }
            strcpy(array[i],lineholder);
            }


            
            //this tries to uses execv or execvp and if both fail we get invalid command print out
            strcpy(lineholder,array[0]);
            if(execv(lineholder,array) < 0)
            {
                if(execvp(lineholder, array) < 0)
                {
                printf("INVALID COMMAND : %s\n", lineholder);
                }

            }
             return 0;
         }


         else // parent process
         {
         printf("prompt> ");
         fgets(line,MaxLine,stdin);
         strncpy(cmdget,line,6); //at first i checked the whole line but who knows if someones file is called jobs.c for example and it gets flagged as jobs
         
         if (strstr(cmdget,"jobs") != NULL)
         {
            int printorder[Maxjob];
            for(int i = 0; i < Maxjob; i++) //initialize the array
            {
                printorder[i] = -1;
            }
             for(int i = 0; i < Maxjob; i++) //runs through array and prints jobs
             {
                 if(JobArray[i].InUseFlag == 1)
                 {
                     printorder[i] = JobArray[i].jobid;
                     //printf("[%i] (%i) %s %s", JobArray[i].jobid,JobArray[i].processid,JobArray[i].status,JobArray[i].proccessname);
                 }
             }
             int *finalsort = sortjobarray(printorder);
             int indexhold = 0;
             for(int i = 0; i < Maxjob; i++)
             {
                 for(int j = 0; j < Maxjob; j++)
                 {
                     if(JobArray[j].jobid == finalsort[indexhold])
                     {
                         printf("[%i] (%i) %s %s", JobArray[j].jobid,JobArray[j].processid,JobArray[j].status,JobArray[j].proccessname);
                     }
                 }
                 indexhold++;
             }
         }

         else if (strstr(cmdget,"bg") != NULL)
         {

             char * command = strtok(line, " ");
             command = strtok (NULL, " ");
            char* position;
            for(int i = 0; i < sizeof(command); i++)
            {

            if((position = strchr(command,'\n')) != NULL)
            {
                *position = '\0';
            }
            }
            pid_t tobg = atoi(command);
            tobgpid = tobg;
            signal_handler(SIGCONT); // handles sigcont signal


         }
         else if (strstr(cmdget,"fg") != NULL)
         {
             shellpgid = tcgetpgrp(0);
             if(strstr(line,"%") != NULL)
             {
                 percentflag = 1;
             }
             char * command = strtok(line, " ");
             command = strtok (NULL, " %"); //checks for space or % symbol
            char* position;
            for(int i = 0; i < sizeof(command); i++)
            {

            if((position = strchr(command,'\n')) != NULL)
            {
                *position = '\0';
            }
            }
            pid_t tofg = atoi(command);
            tofgpid = tofg;
            signal_handler(-1);
         }
         else if (strstr(cmdget,"kill") != NULL)
         {
            char * command = strtok(line, " ");
            command = strtok (NULL, " "); //checks for space or % symbol
            char* position;
            for(int i = 0; i < sizeof(command); i++)
            {

            if((position = strchr(command,'\n')) != NULL)
            {
                *position = '\0';
            }
            }
            pid_t tokill = atoi(command);
            tokillpid = tokill;
             signal_handler(SIGINT);
         }
         else if (strstr(cmdget,"quit") != NULL)
         {
             quitflag = 1;
         }
         else
         {


    
    pid_t waitresult = waitpid(-1,NULL,WNOHANG); // sets use flag to 0 (not in use) to open them for use later
    if(waitresult > 0)
    {
        CurrentJobs--;
        for(int i = 0; i < 5; i++)
        {
            if(JobArray[i].processid == waitresult)
            {
                JobArray[i].InUseFlag = 0;
            }
        }
    }



             if(CurrentJobs >= Maxjob) // checks if current job count exceeds max jobs
            {
                printf("TOO MANY RUNNING JOBS\n");
            }
            else
            {
             pid = fork();
             if (pid != 0)
             CurrentJobs++; // add to current job count
             AvailableJobID++; // increments job ID

             for(int i = 0; i < Maxjob; i++)
             {
                 if(JobArray[i].InUseFlag != 1)
                 {
                     IndexToUse = i;
                     i = Maxjob;
                 }
                 else
                 {
                     IndexToUse++;
                 }
             }


            if (pid != 0)
            {
            //printf("LINE IS %s", line);
            //printf("INSERTING IN JOB ARRAY INDEX: %i\n",IndexToUse);
            JobArray[IndexToUse].jobid = AvailableJobID;
            //printf("JOBID: %i\n", JobArray[IndexToUse].jobid);
            JobArray[IndexToUse].processid = pid;
            //printf("PROCESSID: %i\n", JobArray[IndexToUse].processid);
            strcpy(JobArray[IndexToUse].proccessname, line);
            //printf("PROCESSNAME: %s", JobArray[IndexToUse].proccessname); //NOTICE PROCESS NAME HAS ATTACHED \n
            JobArray[IndexToUse].InUseFlag = 1;
            //printf("IN USE FLAG IS: %i\n", JobArray[IndexToUse].InUseFlag);
            if(strstr(JobArray[IndexToUse].proccessname,"&"))
            {
                strcpy(JobArray[IndexToUse].status, "Background");
            }
            else
            {
            strcpy(JobArray[IndexToUse].status, "Foreground");
            JobArray[IndexToUse].InUseFlag = 0;
            }
            //printf("STATUS: %s\n", JobArray[IndexToUse].status);
            
            
            }
            
            }
         }
         }
     }

     return 0;
 }