
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

struct Pages // this is the struct we will use to hold page info
{
    int startingindex; // we might end up not needing this line
    int interactedwith; //tracks how recently used the page was
    int VP;
    int valid;
    int dirty;
    int value[8]; // this will hold the 8 values in a page, initialized with -1.
};

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
int tobinary(int num) // converts an int to binary
{
    int binaryarray[7] = {0};
    int indexholder[3] = {0,0,0};
    int index = 0;
    int finalindex = 0;
    while(num > 0)
    {
        binaryarray[index] = num % 2;
        num = num / 2;
        index++;
    }
    index = 3; // we want just 3 of the 6 bits to determine the VP
    while(index < 6)
    {
        if(index == 3)
        {
            if(binaryarray[index] == 1)
            {
                finalindex +=1;
            }
        }
        if(index == 4)
        {
            if(binaryarray[index] == 1)
            {
                finalindex +=2;
            }
        }
        if(index == 5)
        {
            if(binaryarray[index] == 1)
            {
                finalindex +=4;
            }
        }
        index++;
    }
    return finalindex;
}

int main(int argc, const char * argv[])
{
    int FIFOFlag; // flag for FIFO
    int LRUFlag ;// flag for LRU
    if(argc > 1)
    {
        if(strcmp(argv[1],"LRU") == 0 || strcmp(argv[1],"lru") == 0) // catch argv and set the flag, if empty it defaults to FIFO
        {
            LRUFlag = 1;
        }
        else
        {
            FIFOFlag = 1;
        }
        
    }
    else
    {
        FIFOFlag = 1;
    }
    
    int MaxLine = 80;
    char cmdhold[MaxLine];
    int quitflag = 0;
    char line[MaxLine];
    int IndexTracker = 0;
    int interactedwith = 0;
    
    int currentpage; // this will hold the page we're on, so we can set it for the next page
    
    struct Pages virtualmemory[8]; // virtual memory has 8 pages
    struct Pages mainmemory[4]; //main memory has 4 pages
    int i;
    int j;
    int startingindex = 0;
    for(i = 0; i < 4; i++) // initialize mainmemory
    {
        mainmemory[i].valid = 0;
        mainmemory[i].dirty = 0;
        mainmemory[i].startingindex = startingindex;
        for(j = 0; j < 8; j++)
        {
            mainmemory[i].value[j] = -1;
            startingindex++;
            //printf("mainmemory %i index %i is %i\n", i, j, mainmemory[i].value[j]);
        }
    }
    startingindex = 0;
    for(i = 0; i < 8; i++) // initialize mainmemory
    {
        virtualmemory[i].valid = 0;
        virtualmemory[i].dirty = 0;
        virtualmemory[i].VP = i;
        virtualmemory[i].startingindex = startingindex;
        for(j = 0; j < 8; j++)
        {
            virtualmemory[i].value[j] = -1;
            startingindex++;
            //printf("virtualmemory %i index %i is %i\n", i, j, virtualmemory[i].value[j]);
        }
    }
    
    while(quitflag == 0)
    {
        
        printf("> ");
        fgets(line,80,stdin);
        char * command = strtok(line, " ");
        removenewline(command);
        strcpy(cmdhold,command);
        
        
        
        if (strcmp(cmdhold,"read") == 0) //read command
        {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int virtualaddress = atoi(cmdhold);
            int PTE = tobinary(virtualaddress);
            
            int valueindex = virtualaddress;
            
            while(valueindex >= 8)
            {
                valueindex -= 8;
            }
            
            if(FIFOFlag == 1) // if we are in FIFO mode
            {
                
                if(virtualmemory[PTE].valid == 0 && IndexTracker < 4)
                {
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    printf("%i\n",virtualmemory[PTE].value[valueindex]); //print out the value currently at given address
                    virtualmemory[PTE].valid = 1; //set it to valid 1 since its being moved to physical memory
                    //printf("GOING INTO PYSICAL MEMORY %i\n",IndexTracker);
                    mainmemory[IndexTracker] = virtualmemory[PTE];//setting point in physical to page in virtual
                    if(IndexTracker < 4)
                    {
                        IndexTracker++; // upping the index tracker
                    }
                }
                else if(virtualmemory[PTE].valid == 0 && IndexTracker >= 4)
                {
                    //printf("REACHED END OF PHYSICAL MEMORY, FIFO INITIATED\n");
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    
                    mainmemory[0].valid = 0; // setting virtual memory that was in physical's valid back to 0 since its going back to virtual
                    
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 8) // this is because we have to write to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[0].VP == virtualmemory[incriment].VP)
                        {
                            mainmemoryreadindex = incriment;
                        }
                        incriment++;
                    }
                    virtualmemory[mainmemoryreadindex] = mainmemory[0];//EVICTED
                    
                    mainmemory[0] = mainmemory[1]; //pushing 0 index out
                    mainmemory[1] = mainmemory[2];
                    mainmemory[2] = mainmemory[3];
                    virtualmemory[PTE].valid = 1; // setting valid for new main memory to be at index 3
                    mainmemory[3] = virtualmemory[PTE];
                    printf("%i\n",mainmemory[3].value[valueindex]); //print out the value currently at given address
                    //IndexTracker--; //decriment the tracker by 1
                }
                else
                {
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 4) // this is because we have to read to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[incriment].VP == virtualmemory[PTE].VP)
                        {
                            mainmemoryreadindex = incriment;
                            incriment = 4; // in the case that the index is 0, sometimes random var is 0 so stop the search;
                        }
                        incriment++;
                    }
                    printf("%i\n",mainmemory[mainmemoryreadindex].value[valueindex]); //if all goes well
                }
            }
            
            if(LRUFlag == 1)
            {
                if(virtualmemory[PTE].valid == 0 && IndexTracker < 4)
                {
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    virtualmemory[PTE].valid = 1; //set it to valid 1 since its being moved to physical memory
                    //printf("GOING INTO PYSICAL MEMORY %i\n",IndexTracker);
                    interactedwith++; //up the number that tracks recent interaction
                    //printf("INTERACTION UP ONE %i\n",interactedwith);
                    mainmemory[IndexTracker] = virtualmemory[PTE];//setting point in physical to page in virtual
                    mainmemory[IndexTracker].interactedwith = interactedwith;
                    printf("%i\n",mainmemory[IndexTracker].value[valueindex]); //print out the value currently at given address
                    if(IndexTracker < 4)
                    {
                        IndexTracker++; // upping the index tracker
                    }
                }
                else if(virtualmemory[PTE].valid == 0 && IndexTracker >= 4)
                {
                    //printf("REACHED END OF PHYSICAL MEMORY, LRU INITIATED\n");
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    
                    int LeastInteractionNum = mainmemory[0].interactedwith;
                    int IndexCheck;
                    int LeastInteractionIndex = 0;
                    
                    for(IndexCheck = 0; IndexCheck < 4; IndexCheck++) // compare all values in mainmemory to find least interacted with
                    {
                        if(mainmemory[IndexCheck].interactedwith < LeastInteractionNum)
                        {
                            LeastInteractionNum = mainmemory[IndexCheck].interactedwith;
                            LeastInteractionIndex = IndexCheck;
                        }
                    }
                    
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 8) // this is because we have to read to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[LeastInteractionIndex].VP == virtualmemory[incriment].VP)
                        {
                            //printf("INDEX IS %i\n",mainmemoryreadindex);
                            mainmemoryreadindex = incriment;
                        }
                        incriment++;
                    }
                    
                    mainmemory[LeastInteractionIndex].valid = 0;
                    virtualmemory[mainmemoryreadindex] = mainmemory[LeastInteractionIndex];
                    
                    //virtualmemory[mainmemory[LeastInteractionIndex].VP].valid = 0; // setting virtual memory that was in physical's valid back to 0 since its going back to virtual
                    
                    virtualmemory[PTE].valid = 1; // setting valid for new main memory to be at index 3
                    interactedwith++;
                    //printf("INTERACTION UP ONE %i\n",interactedwith);
                    //virtualmemory[PTE].interactedwith = interactedwith;
                    mainmemory[LeastInteractionIndex] = virtualmemory[PTE];
                    mainmemory[LeastInteractionIndex].interactedwith = interactedwith;
                    printf("%i\n", mainmemory[LeastInteractionIndex].value[valueindex]);
                    //IndexTracker--; //decriment the tracker by 1
                }
                else
                {
                    
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 4) // this is because we have to read to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[incriment].VP == virtualmemory[PTE].VP)
                        {
                            mainmemoryreadindex = incriment;
                            incriment = 4; // in case matching index is 0
                        }
                        incriment++;
                    }
                    interactedwith++;
                    //printf("INTERACTION UP ONE %i\n",interactedwith);
                    //virtualmemory[PTE].interactedwith = interactedwith;
                    mainmemory[mainmemoryreadindex].interactedwith = interactedwith;
                    printf("%i\n",mainmemory[mainmemoryreadindex].value[valueindex]); //if all goes well
                    
                }
            }
            
        }
        
        else if (strcmp(cmdhold,"write") == 0)//write command
        {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int virtualaddress = atoi(cmdhold);
            int PTE = tobinary(virtualaddress);
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int num = atoi(cmdhold);
            
            int valueindex = virtualaddress;
            int mainmemorywriteindex;
            
            
            while(valueindex >= 8)
            {
                valueindex -= 8;
            }
            
            if(FIFOFlag == 1) // if we are in FIFO mode
            {
                
                if(virtualmemory[PTE].valid == 0 && IndexTracker < 4)
                {
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    virtualmemory[PTE].valid = 1; //set it to valid 1 since its being moved to physical memory
                    //printf("GOING INTO PYSICAL MEMORY %i\n",IndexTracker);
                    mainmemory[IndexTracker] = virtualmemory[PTE];//setting point in physical to page in virtual
                    mainmemory[IndexTracker].dirty = 1;
                    mainmemory[IndexTracker].value[valueindex] = num;
                    if(IndexTracker < 4)
                    {
                        IndexTracker++; // upping the index tracker
                    }
                }
                else if(virtualmemory[PTE].valid == 0 && IndexTracker >= 4)
                {
                    //printf("REACHED END OF PHYSICAL MEMORY, FIFO INITIATED\n");
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    
                    mainmemory[0].valid = 0; // setting virtual memory that was in physical's valid back to 0 since its going back to virtual
                    
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 8) // this is because we have to write to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[0].VP == virtualmemory[incriment].VP)
                        {
                            mainmemoryreadindex = incriment;
                        }
                        incriment++;
                    }
                    virtualmemory[mainmemoryreadindex] = mainmemory[0];//EVICTED
                    
                    mainmemory[0] = mainmemory[1]; //pushing 0 index out
                    mainmemory[1] = mainmemory[2];
                    mainmemory[2] = mainmemory[3];
                    virtualmemory[PTE].valid = 1; // setting valid for new main memory to be at index 3
                    mainmemory[3] = virtualmemory[PTE];
                    mainmemory[3].dirty = 1;
                    mainmemory[3].value[valueindex] = num;
                    //IndexTracker--; //decriment the tracker by 1
                }
                else // all goes well
                {
                    
                    int incriment = 0;
                    while(incriment < 4) // this is because we have to write to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[incriment].VP == virtualmemory[PTE].VP)
                        {
                            mainmemorywriteindex = incriment;
                            incriment = 4; // in case index is 0
                        }
                        incriment++;
                    }
                    
                    //virtualmemory[PTE].value[valueindex] = num;
                    mainmemory[mainmemorywriteindex].value[valueindex] = num;
                    mainmemory[mainmemorywriteindex].dirty = 1;
                }
                
            }
            if(LRUFlag == 1)
            {
                if(virtualmemory[PTE].valid == 0 && IndexTracker < 4)
                {
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    virtualmemory[PTE].valid = 1; //set it to valid 1 since its being moved to physical memory
                    interactedwith++; //up the number that tracks recent interaction
                    //printf("INTERACTION UP ONE %i\n",interactedwith);
                    mainmemory[IndexTracker] = virtualmemory[PTE];//setting point in physical to page in virtual
                    mainmemory[IndexTracker].interactedwith = interactedwith;
                    mainmemory[IndexTracker].dirty = 1;
                    mainmemory[IndexTracker].value[valueindex] = num;
                    if(IndexTracker < 4)
                    {
                        IndexTracker++; // upping the index tracker
                    }
                }
                else if(virtualmemory[PTE].valid == 0 && IndexTracker >= 4)
                {
                    //printf("REACHED END OF PHYSICAL MEMORY, LRU INITIATED\n");
                    printf("A Page Fault Has Occurred\n"); // print page fault message
                    
                    int LeastInteractionNum = mainmemory[0].interactedwith;
                    int IndexCheck;
                    int LeastInteractionIndex = 0;
                    
                    for(IndexCheck = 0; IndexCheck < 4; IndexCheck++) // compare all values in mainmemory to find least interacted with
                    {
                        if(mainmemory[IndexCheck].interactedwith < LeastInteractionNum)
                        {
                            LeastInteractionNum = mainmemory[IndexCheck].interactedwith;
                            LeastInteractionIndex = IndexCheck;
                        }
                    }
                    
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 8) // this is because we have to read to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[LeastInteractionIndex].VP == virtualmemory[incriment].VP)
                        {
                            mainmemoryreadindex = incriment;
                        }
                        incriment++;
                    }
                    
                    mainmemory[LeastInteractionIndex].valid = 0;
                    virtualmemory[mainmemoryreadindex] = mainmemory[LeastInteractionIndex];
                    
                    //virtualmemory[mainmemory[LeastInteractionIndex].VP].valid = 0; // setting virtual memory that was in physical's valid back to 0 since its going back to virtual
                    
                    virtualmemory[PTE].valid = 1; // setting valid for new main memory to be at index 3
                    interactedwith++;
                    //printf("INTERACTION UP ONE %i\n",interactedwith);
                    //virtualmemory[PTE].interactedwith = interactedwith;
                    mainmemory[LeastInteractionIndex] = virtualmemory[PTE];
                    mainmemory[LeastInteractionIndex].interactedwith = interactedwith;
                    mainmemory[LeastInteractionIndex].dirty = 1;
                    mainmemory[LeastInteractionIndex].value[valueindex] = num;
                    //IndexTracker--; //decriment the tracker by 1
                    
                }
                else // all goes well
                {
                    int incriment = 0;
                    int mainmemoryreadindex;
                    while(incriment < 4) // this is because we have to read to mainmemory too if the virtual values are supposed to be in there
                    {
                        if (mainmemory[incriment].VP == virtualmemory[PTE].VP)
                        {
                            mainmemoryreadindex = incriment;
                            incriment = 4; // in case matching index is 0
                        }
                        incriment++;
                    }
                    interactedwith++;
                    //printf("INTERACTION UP ONE %i\n",interactedwith);
                    //virtualmemory[PTE].interactedwith = interactedwith;
                    mainmemory[mainmemoryreadindex].interactedwith = interactedwith;
                    mainmemory[mainmemoryreadindex].dirty = 1;
                    mainmemory[mainmemoryreadindex].value[valueindex] = num;
                }
                
            }
        }
        else if (strcmp(cmdhold,"showmain") == 0) //showmain command
        {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int ppn = atoi(cmdhold);
            int goflag = 1;
            
//            if(ppn > 3 || ppn < 0)
//            {
//                printf("INVALID PPN\n");
//                goflag = 0;
//            }
            
            if(goflag == 1)
            {
                //printf("VALID BIT IS: %i\n",mainmemory[ppn].valid);
                int d;
                int indexprint;
                if(ppn == 0) // since mainmemory address values are static, ill use this.
                {
                    indexprint = 0;
                }
                else if(ppn == 1)
                {
                    indexprint = 8;
                }
                else if(ppn == 2)
                {
                    indexprint = 16;
                }
                else if(ppn == 3)
                {
                    indexprint = 24;
                }
                for(d = 0; d < 8; d++)
                {
                    printf("%i:%i\n",indexprint,mainmemory[ppn].value[d]);
                    indexprint++;
                }
            }
            
        }
        else if (strcmp(cmdhold,"showdisk") == 0) //showdisk command
        {
            //SHOWDISK IS NOT SUPPOSED TO SHOW VALUES UNTIL THE BLOCK IS KICKED FROM MAINMEMORY IN FIFO/LRU
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int ppn = atoi(cmdhold);
//            if(ppn > 7 || ppn < 0)
//            {
//                printf("INVALID PPN\n");
//            }
//            else
//            {
                int showdiskindex;
                int startingindexincriment = virtualmemory[ppn].startingindex;
                for(showdiskindex = 0; showdiskindex < 8;showdiskindex++)
                {
                    printf("%i:%i\n",startingindexincriment,virtualmemory[ppn].value[showdiskindex]);
                    startingindexincriment++;
                }
//            }
        }
        else if (strcmp(cmdhold,"showptable") == 0) //showptable command
        {
            
            /*
             NOTE:
             this code almost works correctly, just need PPNum to print out the right way after something has been read/written to
             */
            int VPNum = 0;
            int ValidBit;
            int DirtyBit;
            int PPNum;
            while(VPNum < 8)
            {
                int VPindexcheck;
                for(VPindexcheck = 0; VPindexcheck < 4; VPindexcheck++)
                {
                    if(mainmemory[VPindexcheck].VP == virtualmemory[VPNum].VP)
                    {
                        PPNum = VPindexcheck;
                        ValidBit = virtualmemory[VPNum].valid;
                        DirtyBit = mainmemory[VPindexcheck].dirty;
                        VPindexcheck = 4;
                    }
                    else
                    {
                        ValidBit = virtualmemory[VPNum].valid;
                        DirtyBit = mainmemory[VPindexcheck].dirty;
                        PPNum = VPNum;
                        
                    }
                }
                
                printf("%i:%i:%i:%i\n",VPNum,ValidBit,DirtyBit,PPNum);
                VPNum++;
            }
        }
        else if (strcmp(cmdhold,"quit") == 0) //quit command
        {
            quitflag = 1;
        }
    }
    
    return 0;
}
