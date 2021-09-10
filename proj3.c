
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
int * tobinary(int num) // converts an int to binary
{
    int binaryarray[8] = {0,0,0,0,0,0,0,0};
    int index = 6; // starts 1 bit over from the right since the right most is the allocation flag
    int finalindex = 0;
    while(num > 0)
    {
        binaryarray[index] = num % 2;
        num = num / 2;
        index--;
    }
        for(int b = 0 ; b < 8;b++)
    {
        //printf("%i",binaryarray[b]);
    }
    int * ptr = binaryarray;
    return ptr;
}
int tonum(int * holder)
{
int finalnum = 0;
int binaryconvert[] = {64,32,16,8,4,2,1};
for(int i = 0; i < 7; i++)
{
    if(holder[i] == 1)
    {
        finalnum += binaryconvert[i];
    }
}
finalnum = finalnum - 2; // remove header and footer from free allocation
return finalnum;
}


    struct Allocation // this is the struct we will use to hold single byte info
    { 
    int headerflag;
    int footerflag;
    int firstaddress;
    int value[8]; // this will hold the 8 bits in a byte
    };


 int main(int argc, const char * argv[]) 
 {
     int heap = 127;
     int pointervaluemax = 126;
     int MaxLine = 80;
     char cmdhold[MaxLine];
     int quitflag = 0;
     char line[MaxLine];
    struct Allocation Allocations[heap]; // what will represent our heap
    int initializer;
    for(initializer = 0; initializer < heap; initializer++) // initialize all addresses with 0
    {
        int valueinitialize;
        for(valueinitialize = 0; valueinitialize < 8; valueinitialize++)
        {
            Allocations[initializer].value[valueinitialize] = 0;
            //printf("ALLOCATION %i VALUE NUMBER %i NOW ZERO\n",initializer,valueinitialize);
        }
    }

    int * holder = tobinary(127);
    int i;
    for(i = 0; i < 8; i++)
    {
        Allocations[0].value[i] = holder[i];
        Allocations[126].value[i] = holder[i];
        //printf("ALLOCATION 0 AND 126 VALUE %i NOW %i\n",i,holder[i]);
    }
    Allocations[0].headerflag = 20;
    Allocations[0].firstaddress = 1;
    Allocations[126].firstaddress = 1;
    Allocations[126].footerflag = 30;

          while(quitflag == 0)
     {

         printf("> ");
         fgets(line,80,stdin);
        char * command = strtok(line, " ");
            removenewline(command);
            strcpy(cmdhold,command);


         
         if (strcmp(cmdhold,"malloc") == 0) //read command
         {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int IntSize = atoi(cmdhold);
            int totalsize = IntSize + 2; // can use this later if want held in the struct
            holder = tobinary(totalsize); // convert int to binary, adding 2 to account for size of header and footer
            holder[7] = 1; // sets the allocation flag
            int i;
            int openblock;
            int foundblockholder;
            int emptyblocks;
            int firstaddy;
            for(i = 0; i < 127;i++) // runs through the heap to find the first open header block
            {
                if(Allocations[i].headerflag == 20 && Allocations[i].value[7] == 0) // if its a header block and unallocated
                {
                    //printf("FOUND A HEADER %i\n",i);
                    openblock = i;
                    foundblockholder = i;
                    firstaddy = i + 1;
                    i = 128;
                }
            }
            //overwrites the open block header with info on the new allocated block
            Allocations[openblock].headerflag = 20; // header flag
            Allocations[openblock].firstaddress = firstaddy;// setting first address value

            for(i = 0; i < 8; i++) // inserts header info
            {
                Allocations[openblock].value[i] = holder[i];
                //printf("OPENBLOCK %i INDEX %i INSERTING %i\n",openblock,i,holder[i]);
            }
            //creating the footer
            int footerindex = firstaddy + IntSize;// getting index of footer byte
            Allocations[footerindex].footerflag = 30; // setting footer flag
            Allocations[footerindex].firstaddress = firstaddy;//setting first address

            for(i = 0; i < 8; i++) // inserts footer info
            {
                Allocations[footerindex].value[i] = holder[i];
                //printf("FOOTER %i INDEX %i INSERTING %i\n",footerindex,i,holder[i]);
            }
            //now we create a new header for the remaining open blocks
             for(int i = footerindex + 1; i < 127;i++) // first we look for a footer 
            {
                if(Allocations[i].footerflag == 30 && Allocations[i].value[7] == 0) // if its a header block and unallocated
                {
                    //printf("FOUND A FOOTER %i\n",i);
                    openblock = i;
                    i = 128;
                }
            }
            int newheaderindex = footerindex + 1;
            int newblocksize = openblock - footerindex;
            //printf("NEW BLOCK %i\n",newblocksize);
            Allocations[newheaderindex].headerflag = 20;
            Allocations[newheaderindex].firstaddress = newheaderindex + 1;
            holder = tobinary(newblocksize);
            for(i = 0; i < 8; i++) // inserts new header info
            {
                Allocations[newheaderindex].value[i] = holder[i];
                //printf("NEW EMPTY ALLOCATION %i NEWHEADER %i INDEX %i INSERTING %i\n",newblocksize -2,newheaderindex,i,holder[i]);
            }

            //now we replace the old footer with new footer

            Allocations[openblock].footerflag = 30;
            Allocations[openblock].firstaddress = newheaderindex + 1;
            holder = tobinary(newblocksize);
            for(i = 0; i < 8; i++) // inserts new header info
            {
                Allocations[openblock].value[i] = holder[i];
                //printf("NEW EMPTY ALLOCATION %i REPLACEFOOTER %i INDEX %i INSERTING %i\n",newblocksize -2,openblock,i,holder[i]);
            }
            
            printf("%i\n",Allocations[foundblockholder].firstaddress);
         }

         else if (strcmp(cmdhold,"free") == 0)//write command
         {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int IntIndex = atoi(cmdhold);
                int i;
                int foundfreeheaderbefore = 0;
                int foundfreeheaderafter = 0;
              for(i = 0; i < 127;i++) // runs through the heap to find the first open header block
            {
                if(Allocations[i].headerflag == 20 || Allocations[i].footerflag == 30) // if its a header block or a footer block
                {
                    //printf("FOUND A HEADER OR FOOTER INDEX %i\n",i);
                    if(Allocations[i].firstaddress == IntIndex) // if we have a matching first address value
                    {
                        if (Allocations[i].headerflag == 20)// if it was a header, erase all values in addresses within header & footer, "if a block is freed, you must ensure that whatever was written to the block is reset back to 0"
                        {
                            int addresseraser = tonum(Allocations[i].value);
                            int z;
                            int d;
                            int onaddress = i + 1;
                            for(z = 0; z < addresseraser; z++)
                            {
                                //printf("ERASING %i\n",onaddress);
                                for(d = 0; d < 8; d++)
                                {
                                    Allocations[onaddress].value[d] = 0;
                                }
                                onaddress++;
                            }
                        }
                        //printf("MATCH INDEX TO MERGE %i\n",i);
                        Allocations[i].value[7] = 0; //changing the allocation flag to not, that way we can overwrite it in malloc
                        


                        if(Allocations[i].headerflag == 20) // backward coalescing
                        {
                            int searcher = i-1;
                            while(searcher >= 0)
                            {
                                //printf("SEARCHING %i\n",searcher);
                                if(Allocations[searcher].headerflag == 20)
                                {
                                    if(Allocations[searcher].value[7] == 0)
                                    {
                                    //printf("FOUND EMPTY BLOCK, HEADER AT %i SIZE %i\n",searcher,tonum(Allocations[searcher].value));
                                    int currentblockSize = tonum(Allocations[i].value);
                                    int oldemptyblocksize = tonum(Allocations[searcher].value);
                                    int totalnewsize = currentblockSize + oldemptyblocksize + 2; // can use this later if want held in the struct
                                    holder = tobinary(totalnewsize); // convert int to binary, adding 2 to account for size of header and footer
                                    int blem;
                                    int footertoerase = i + currentblockSize + 1;
                                    int firstaddress = searcher + 1;
                                    //printf("HERE BOY %i\n",footertoerase);
                                    for(blem = 0; blem < 8; blem++) // inserts new header info to old empty block header
                                    {
                                    Allocations[searcher].value[blem] = holder[blem];//fill in new value
                                    Allocations[i].value[blem] = 0; // cleaning current header
                                    Allocations[footertoerase].value[blem] = 0;//cleaning current footer
                                    //printf("OPENBLOCK %i INDEX %i INSERTING %i\n",searcher,blem,holder[blem]);
                                    }
                                    Allocations[searcher].firstaddress = firstaddress;
                                    Allocations[i].headerflag = 0; // erasing current header
                                    Allocations[footertoerase].footerflag = 0; // erasing current footer
                                    //printf("SEARCHER %i TOTALNEWSIZE %i\n",searcher,totalnewsize);
                                    int newfooter = searcher + totalnewsize - 1;
                                    for(blem = 0; blem < 8; blem++) // fill in new footer info
                                    {
                                        Allocations[newfooter].value[blem] = holder[blem];
                                    }
                                    Allocations[newfooter].footerflag = 30; // set flag for new footer
                                    Allocations[newfooter].firstaddress =firstaddress;

                                    int lookforheader = newfooter + 1;
                                    int moveto = newfooter + 1;
                                    int distance = 0;
                                    while(lookforheader < 127)
                                    {
                                        if(Allocations[lookforheader].headerflag == 20)
                                        {
                                            if(Allocations[lookforheader].value[7] == 0)
                                            {
                                            //printf("FOUND HEADER TO SHIFT OVER %i DISTANCE %i\n",lookforheader,distance);
                                            //printf("NEWSIZE %i\n",tonum(Allocations[lookforheader].value) + distance);
                                            int newtotalsize = tonum(Allocations[lookforheader].value) + distance + 2;
                                            holder = tobinary(newtotalsize);
                                            Allocations[lookforheader].headerflag = 0;
                                            for(blem = 0; blem < 8;blem++)
                                            {
                                                Allocations[moveto].value[blem] = holder[blem];
                                            }
                                            Allocations[moveto].firstaddress = moveto + 1;
                                            Allocations[moveto].headerflag = 20;

                                            int lookforfooter = moveto + 1;
//                                            if(Allocations[lookforfooter].footerflag == 30)
//                                            {
//                                            for(blem = 0; blem < 8;blem++)
//                                            {
//                                                Allocations[lookforfooter].value[blem] = holder[blem];
//                                            }
//                                            
//                                            
//                                            Allocations[lookforfooter].firstaddress = moveto + 1;
//                                            }
                                            }
                                            lookforheader = 128;
                                        }
                                        distance++;
                                        lookforheader++;
                                    }
                                    }
                                    searcher = -2;
                                }
                                searcher--;
                            }
                        }
                        if(Allocations[i].footerflag == 30) // forward coalescing
                        {
                            int searcher;
                            int oldheader = Allocations[i].firstaddress - 1;
                            
                            //printf("OLD HEADER %i AN SEARCHER %i\n",oldheader,i+1);
                            
                            for(searcher = i; searcher < 127; searcher++)
                            {
                                //printf("SEARCHING %i\n",searcher);
                                if(Allocations[searcher].headerflag == 20)
                                {
                                    if(Allocations[searcher].value[7] == 0)
                                    {
                                    //printf("FOUND A HEADER AT %i\n",searcher);
                                    int oldfooter = tonum(Allocations[searcher].value) + searcher + 1;

                                    //printf("OLD FOOTER %i\n",oldfooter);
                                    int newvalue = tonum(Allocations[searcher].value) + tonum(Allocations[i].value) + 2 + 2;
                                    //printf("VALUE %i\n",newvalue);
                                    int blem;
                                    holder = tobinary(newvalue);
                                    for(blem = 0;blem < 8; blem++)
                                    {
                                        Allocations[oldheader].value[blem] = holder[blem];//overwrite old val in head
                                        Allocations[oldfooter].value[blem] = holder[blem];// overwrite old val in foot
                                        Allocations[searcher].value[blem] = 0; // overwrite now useless header vals
                                        Allocations[searcher-1].value[blem] = 0;//overwrite now useless footer vals
                                    }
                                    Allocations[oldheader].firstaddress = oldheader + 1;
                                    Allocations[oldfooter].firstaddress = oldheader + 1;
                                    Allocations[searcher].headerflag = 0;
                                    Allocations[searcher-1].footerflag = 0;
                                    }
                                    searcher = 128;
                                }
                            }
                        }







                    }
                }
            }

         }
         else if (strcmp(cmdhold,"blocklist") == 0) //showmain command
         {
             int i;
             for(i = 0; i < 127; i++)
             {
                 if(Allocations[i].headerflag == 20) // if its a header block
                 {
                     if(Allocations[i].value[7] == 1) // if allocated
                     {
                         printf("%i, %i, allocated.\n",Allocations[i].firstaddress,tonum(Allocations[i].value));
                     }
                     else if(Allocations[i].value[7] == 0) // if unalocated
                     {
                        printf("%i, %i, free.\n",Allocations[i].firstaddress,tonum(Allocations[i].value));
                     }
                 }
             }
         }
         else if (strcmp(cmdhold,"writemem") == 0) //showdisk command
         {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int IntIndex = atoi(cmdhold);
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            char * str = cmdhold;
            int b;
            for(b = 0; b < strlen(str); b++)
            {
            //printf("%c\n",str[b]);
            holder = tobinary((int)str[b]);

            for(i = 0; i < 8; i++) // inserts new header info
            {
                Allocations[IntIndex].value[i] = holder[i];
                //printf("ADDRESS %i INDEX %i INSERTING %i\n",IntIndex,i,holder[i]);
            }
            IntIndex++;
            }
         }
         else if (strcmp(cmdhold,"printmem") == 0) //showptable command
         {
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int IntIndex = atoi(cmdhold);
            command = strtok (NULL, " ");
            removenewline(command);
            strcpy(cmdhold,command);
            int Intprintcount = atoi(cmdhold);
            int b;
            for(b = 0; b < Intprintcount - 1; b++)
            {
                printf("%02X ",tonum(Allocations[IntIndex].value) + 2); // 02X prints out in HEX
                IntIndex++;
            }
            printf("%02X\n",tonum(Allocations[IntIndex].value) + 2); // 02X prints out in HEX
         }
        else if (strcmp(cmdhold,"quit") == 0) //quit command
         {
             quitflag = 1;
         }
     }
     return 0;
 }
