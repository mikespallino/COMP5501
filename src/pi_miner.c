// To compile through the Pi Cluster
// mpicc -o pi_miner pi_miner.c -lssl -lcrypto
// sudo cp pi_miner /nfs/supermam/pi_miner
// qsub pi_miner_submit.sh
// qstat

#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <limits.h>
#include <sys/time.h>

#define MAX_TIME 600000
#define BLOCK_TIME_LIMIT 600

typedef struct block_header {
        unsigned int    version;
        // Char is supposed to contain only binary data
        unsigned char   prev_block[32];
        unsigned char   merkle_root[32];
        unsigned int    timestamp;
        unsigned int    bits;
        unsigned int    nonce;
} block_header;

// Helper function to convert hex to binary (Can be optimized better)
void hex2bin(unsigned char* dest, unsigned char* src)
{
        unsigned char bin;
        int c, pos;
        char buf[3];
 
        pos=0;
        c=0;
        buf[2] = 0;
        while(c < strlen(src))
        {
                // Read in 2 characaters at a time
                buf[0] = src[c++];
                buf[1] = src[c++];
                // Convert them to a interger and recast to a char (uint8)
                dest[pos++] = (unsigned char)strtol(buf, NULL, 16);
        }
       
}
 
// Used to print out the hex of the hash. Not necessary for mining computation. Only for human readability
void hexdump(unsigned char* data, int len)
{
        int c;
       
        c=0;
        while(c < len)
        {
                printf("%.2x", data[c++]);
        }
        printf("\n");
}
 
// Function to swap the byte ordering of binary data (Can be optimized better)
void byte_swap(unsigned char* data, int len) {
        int c;
        unsigned char tmp[len];
       
        c=0;
        while(c<len)
        {
                tmp[c] = data[len-(c+1)];
                c++;
        }
       
        c=0;
        while(c<len)
        {
                data[c] = tmp[c];
                c++;
        }
}

int main(int argc, char **argv){

    //Initialization of MPI
    int rank,size;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Request req;
    MPI_Status status;

    //Initialization of variables
    int memNum=0;
    unsigned int difference=0;
    unsigned int nonceSizePerRank=0;
    unsigned int nonceStart=0;
    double start=0;
    double diff=0;
    int elaspedTime=0;
    int tag;
    int root_process=0;
    int foundHash=-1;
    int foundHashAny=-1;

    //Initalization of Header and Hash Char
	block_header header;
    unsigned char hash1[SHA256_DIGEST_LENGTH];
    unsigned char hash2[SHA256_DIGEST_LENGTH];

    //The hash value lower than the target is currently hard coded due to limitations
    unsigned char target[32];
    hex2bin(target,"00000000839a8e6886ab5951d76f411475428afc90947ee320161bbf18eb6048");

    //Initialization of SHA256 variables
    SHA256_CTX sha256_pass1, sha256_pass2;

	
    //The Block Header values are obtained and hardcoded due to it being previously existing
    // and easier to verify working conditions

    //Header Version: Always the value 1
    header.version = 1; 
    // Header Previous Block: Hash of the previous block
    hex2bin(header.prev_block,"000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
    //Header Merkle Root: Hash value of the merkle root
    hex2bin(header.merkle_root,"0e3e2357e806b6cdb1f70b54c3a3a17b6714ee1f0e68bebb44a74b1efd512098");
    //Header Timestamp = Time stamp
    header.timestamp = 1231469665;
    //Header Bits: The difficulty rating for the hashing process
    header.bits = 486604799;
    //Header Nonce: The value that will increment to determine the final hash. Currently set to be
    // 4million away from the correct hash due to slow processing limitations
    header.nonce = 2572394689; //2573394689

    //Converting from Little Endian to Big Endian for computations
    byte_swap(header.prev_block, 32);
    byte_swap(header.merkle_root, 32);

    difference = (2576394689-header.nonce);//2576394689
 
    //Begin Elasped Time Calculations
    struct timeval begin,stop;
    gettimeofday(&begin,NULL);
    
    start = MPI_Wtime();

    if(rank==0){
        printf("\n\n\n---------------BLOCK HEADER---------------\n");
        printf("Version:\t\t1\n");
        printf("Previous Block Hash:\t000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f\n");
        printf("Merkle Root:\t\t0e3e2357e806b6cdb1f70b54c3a3a17b6714ee1f0e68bebb44a74b1efd512098\n");
        printf("Time:\t\t\t1231469665\n");
        printf("nBits:\t\t\t486604799\n");
        printf("Nonce:\t\t\t2572394689\n");
        printf("------------------------------------------\n\n");
        printf("------------------------------------------\n");
        printf("Node %d starting...\n",rank);
        printf("Size %d\n",size);
        printf("Nonce difference %u\n",difference);
        printf("------------------------------------------\n\n");

        nonceSizePerRank=difference/(size-1);

        start = MPI_Wtime();
        int rankID;

        //Sending necessary data to other Computation Nodes
        for(rankID=1;rankID<size;rankID++){
            nonceStart = ((header.nonce + (nonceSizePerRank * (rankID-1))))+1;
            printf("------------------------------------------\n");
            printf("Node %d has nonceStart: %u\n",rankID,nonceStart);
            printf("Node %d has nonceSizePerRank: %u\n",rankID,nonceSizePerRank);
            printf("------------------------------------------\n\n");
            tag=1;

            //To send two pieces of information, we call MPI_Send twice
            MPI_Send(&nonceStart,1,MPI_UNSIGNED,rankID,tag,MPI_COMM_WORLD);
            tag=2;
            MPI_Send(&nonceSizePerRank,1,MPI_UNSIGNED,rankID,tag,MPI_COMM_WORLD);
        }

        tag=1;
        rankID=1;

        printf("------------------------------------------\nNode %d beginning listener...\n------------------------------------------\n\n",rank);
        // printf("Node %d beginning listener...\n",rank);
        // printf("------------------------------------------\n\n");

        //Loop will continue until the 10 minute mark which is the average time to mine a block. Cannot update
        // to a new block
        while(elaspedTime<BLOCK_TIME_LIMIT){
            //Obtains all the foundHash values and picks the max value and sets it into foundHashAny
            MPI_Allreduce(&foundHash,&foundHashAny,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);

            //Once foundHashAny is found, it prints the final hash and time information
            if(foundHashAny>=0){
                unsigned int tempNonce=0;
                //Receive the nonce value from the node of foundHashAny value
                MPI_Recv(&tempNonce,1,MPI_UNSIGNED,foundHashAny,tag,MPI_COMM_WORLD,&status);
                printf("Hash was found! Continuing with Node %d\n\n",rank);
                header.nonce=tempNonce;

                //SHA256 of the header
                SHA256_Init(&sha256_pass1);
                SHA256_Update(&sha256_pass1, (unsigned char*)&header, sizeof(block_header));
                SHA256_Final(hash1, &sha256_pass1);

                //Convert back from Big Endian to Little Endian
                byte_swap(hash1, SHA256_DIGEST_LENGTH);
                byte_swap(hash1, SHA256_DIGEST_LENGTH);
                
                //SHA256 of the previous Hash
                SHA256_Init(&sha256_pass2);
                SHA256_Update(&sha256_pass2, hash1, SHA256_DIGEST_LENGTH);
                SHA256_Final(hash2, &sha256_pass2);
                
                //Convert back from Big Endian to Little Endian
                byte_swap(hash2, SHA256_DIGEST_LENGTH);
                break;
            }
            //Get the time for checking if it has past the 10 minute mark
            gettimeofday(&stop,NULL);
            elaspedTime = ((((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec) / 1000000);
            //printf("elaspedTime difference: %u\n",elaspedTime);
            //diff = MPI_Wtime()-start;
        }

        printf("******************************************\nNode %d completed\n******************************************\n\n",rank);
        // printf("Node %d completed\n",rank); 
        // printf("******************************************\n\n");

        //Calculate time and begin the information prints
        gettimeofday(&stop,NULL);
        int msec = ((((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec) / 1000);
        int minutes,seconds,milliseconds;

        milliseconds=msec%1000;
        minutes=msec/60000;
        seconds=(msec-(minutes*60000))/1000;

        printf("-------------------RESULT-----------------\n");
        printf("Time taken %d minutes %d seconds %d milliseconds",minutes,seconds,milliseconds);
        
        //Should the time exceed 10 minute mark, it stops everything
        if(((MPI_Wtime() - start)*1000)>=MAX_TIME){
            foundHash=0;
            MPI_Allreduce(&foundHash,&foundHashAny,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
            printf("Could not complete within time limit\n");
            MPI_Finalize();
            return 0;
        }


        printf("\nRequired Hashes: %u\n",(difference));

        // printf("\nHashes: %d",(header.nonce));
        // printf("\nHashes Per Second: %d", ((header.nonce/msec)*1000) );

        printf("Target Second Pass Checksum: ");
        hexdump(hash2,SHA256_DIGEST_LENGTH);
        printf("------------------------------------------\n\n");

        printf("\n\n");
        
    }else{
        unsigned int nonceRankStart;
        unsigned int nonceSegment;
        unsigned int nodeMSec=0;


        //Receive the two pieces of information, Start value and the segment value from Node 0
        tag=1;
        MPI_Recv(&nonceRankStart,1,MPI_UNSIGNED,root_process,tag,MPI_COMM_WORLD,&status);
        tag=2;
        MPI_Recv(&nonceSegment,1,MPI_UNSIGNED,root_process,tag,MPI_COMM_WORLD,&status);

        unsigned int nonceMax = nonceRankStart+nonceSegment;
        header.nonce=nonceRankStart;

        // printf("------------------------------------------\nNode %d has nonceRankStart: %u\nNode %d has nonceSegment: %u\nNode %d has nonceMax %u\nNode %d starting...\n\n------------------------------------------\n",rank,nonceRankStart,rank,nonceSegment,rank,nonceMax,rank);
        // printf("Node %d has nonceSegment: %u\n",rank,nonceSegment);
        // printf("Node %d has nonceMax %u\n",rank,nonceMax);

        printf("Node %d starting...\n\n",rank);
        //Compute the hashes until the node reached its range
        while(header.nonce<nonceMax){
            //printf("Node %d at nonce value: %d\n",rank,header.nonce);
            SHA256_Init(&sha256_pass1);
            SHA256_Update(&sha256_pass1, (unsigned char*)&header, sizeof(block_header));
            SHA256_Final(hash1, &sha256_pass1);
               
            byte_swap(hash1, SHA256_DIGEST_LENGTH);

            byte_swap(hash1, SHA256_DIGEST_LENGTH);
               
            SHA256_Init(&sha256_pass2);
            SHA256_Update(&sha256_pass2, hash1, SHA256_DIGEST_LENGTH);
            SHA256_Final(hash2, &sha256_pass2);
               
            byte_swap(hash2, SHA256_DIGEST_LENGTH);

            //Compares the binary SHA256 values to see if it reached the correct hash
            memNum = memcmp(hash2,target,SHA256_DIGEST_LENGTH);

            //If the value is smaller or equal to each other, then stop the process and send the information back
            if(memNum<=0){
                gettimeofday(&stop,NULL);
                nodeMSec = ((((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec) / 1000);
                // printf("##########################################\nHash found...Node %d\n##########################################\n\n",rank);
                // printf("Hash found...Node %d\n",rank);
                // printf("##########################################\n\n");
                foundHash=rank;
                tag=1;
                unsigned int someNonce = header.nonce;
                //Flag up to tell every Computation Node to stop their process
                MPI_Allreduce(&foundHash,&foundHashAny,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
                // printf("Reduced...\n");
                //Send back the nonce value to the root node
                MPI_Send(&someNonce,1,MPI_UNSIGNED,root_process,tag,MPI_COMM_WORLD);
                int nodeMinutes,nodeSeconds,nodeMilliseconds;

                //Print out the time and hashes made
                nodeMilliseconds=nodeMSec%1000;
                nodeMinutes=nodeMSec/60000;
                nodeSeconds=(nodeMSec-(nodeMinutes*60000))/1000;

                difference=header.nonce-nonceRankStart;
                printf("##########################################\n<<<<<Hash was obtained from Node %d>>>>>\n##########################################\n\n",rank);
                printf("*******************NODE %d*****************\nNode %d completed\nTime taken %d minutes %d seconds %d milliseconds\nHashes: %u\nHashes Per millisecond: %u\n******************************************\n\n",rank,rank,nodeMinutes,nodeSeconds,nodeMilliseconds,difference,(difference/nodeMSec));
                // printf("<<<<<Hash was obtained from Node %d>>>>>\n",rank);
                // printf("##########################################\n\n");
                break;
            }else{
                header.nonce++;
            }

            //Call reduce since the Nodes need to check if it needs to stop due to hash found
            MPI_Allreduce(&foundHash,&foundHashAny,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
            
            //Stops the Node
            if(foundHashAny>=0){
                gettimeofday(&stop,NULL);
                nodeMSec = ((((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec) / 1000);
                // printf("------------------------------------------\nfoundHashAny: %d\nHash was found...stopping Node %d\n------------------------------------------\n\n",foundHashAny,rank);
                // printf("foundHashAny: %d\n",foundHashAny);
                // printf("Hash was found...stopping Node %d\n",rank);
                // printf("------------------------------------------\n\n");
                break;
            }
            
            // if(header.nonce>=UINT_MAX){
            //     printf("Reached here\n");
            //     header.nonce=0;
            //     header.timestamp++;
            // }else{
            //     header.nonce++;
            // }
        }



        // int nodeMinutes,nodeSeconds,nodeMilliseconds;

        // nodeMilliseconds=nodeMSec%1000;
        // nodeMinutes=nodeMSec/60000;
        // nodeSeconds=(nodeMSec-(nodeMinutes*60000))/1000;

        // difference=header.nonce-nonceRankStart;

        // printf("*******************NODE %d*****************\nNode %d completed\nTime taken %d minutes %d seconds %d milliseconds\nHashes: %u\nHashes Per millisecond: %u\n******************************************\n\n",rank,rank,nodeMinutes,nodeSeconds,nodeMilliseconds,difference,(difference/nodeMSec));
        



        printf("Node %d completed\n",rank);

        

        // printf("Time taken %d minutes %d seconds %d milliseconds",nodeMinutes,nodeSeconds,nodeMilliseconds);

        

        // printf("\nHashes: %u",difference);
        // printf("\nHashes Per millisecond: %u\n\n", (difference/nodeMSec) );

        // printf("******************************************\n\n");
	}
    
    MPI_Finalize();


	// diff = clock() - start;
    // gettimeofday(&stop,NULL);

	// int msec = diff*1000/(CLOCKS_PER_SEC);
    // int msec = ((((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec) / 1000);

    // printf("Time in microseconds: %ld microseconds\n",((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec);

    

	// hexdump(hash2, SHA256_DIGEST_LENGTH);

	return 0;
}