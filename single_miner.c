//To compile for a regular laptop
// gcc -o pi_miner pi_miner.c -lssl -lcrypto

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <limits.h>
#include <sys/time.h>

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
                // read in 2 characaters at a time
                buf[0] = src[c++];
                buf[1] = src[c++];
                // convert them to a interger and recast to a char (uint8)
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

int main(){

    //Initalization of Header and Hash Char
	block_header header;
    unsigned char hash1[SHA256_DIGEST_LENGTH];
    unsigned char hash2[SHA256_DIGEST_LENGTH];
    unsigned char target[32];
    hex2bin(target,"00000000839a8e6886ab5951d76f411475428afc90947ee320161bbf18eb6048");
    // hexdump(target,32);

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
    header.nonce = 2571394689; //2573394689
    //Converting from Little Endian to Big Endian for computations
    byte_swap(header.prev_block, 32);
    byte_swap(header.merkle_root, 32);

    // hex2bin(preblockhash,"000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
    // byte_swap(preblockhash,32);

    int memNum=0;

    unsigned int difference = (2573394689-header.nonce);
 
    //Begin Elasped Time Calculations
    struct timeval begin,stop;
    gettimeofday(&begin,NULL);

    clock_t start = clock(), diff; //Does not work on pi cluster
    const int MAX_TIME = 600000;

     while(1){
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

        //Compares the binary SHA256 values to see if it reached the correct hash
    	memNum = memcmp(hash2,target,SHA256_DIGEST_LENGTH);

        //Stops the loop
    	if(memNum<=0){
            printf("End here 2\n");
    		break;
    	}

        //Stops the loops when the max nonce value is reached
        if(header.nonce>=UINT_MAX){
            printf("Reached here\n");
            header.nonce=0;
            header.timestamp++;
        }else{
            header.nonce++;
        }
    		

	}

    //Calculate time and begin the information prints
	diff = clock() - start;
    gettimeofday(&stop,NULL);

	// int msec = diff*1000/(CLOCKS_PER_SEC);
    int msec = ((((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec) / 1000);

    printf("Time in microseconds: %ld microseconds\n",((stop.tv_sec - begin.tv_sec)*1000000L+stop.tv_usec) - begin.tv_usec);

    int minutes,seconds,milliseconds;
    milliseconds=msec%1000;
    minutes=msec/60000;
    seconds=(msec-(minutes*60000))/1000;

    printf("Target Second Pass Checksum: ");
    hexdump(hash2,SHA256_DIGEST_LENGTH);

	printf("Time taken %d minutes %d seconds %d milliseconds",minutes,seconds,milliseconds);
	

    printf("\nHashes: %d",(difference));
    printf("\nHashes Per Second: %d", ((difference/msec)*1000) );

    // printf("\nHashes: %d",(header.nonce));
    // printf("\nHashes Per Second: %d", ((header.nonce/msec)*1000) );

	printf("\n\n");

	// hexdump(hash2, SHA256_DIGEST_LENGTH);

	return 0;
}