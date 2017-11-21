#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include "stdlib.h"
#include "string.h"
//#include "utility.h"

#define DEBUG 

void help(){
	printf("-f filename of trace\n");
	for(int i = 0; i < 3; i++){
		printf("-s%d set size(KB), associativity, block_size respectively\n", i);
		printf("-m%d set write_through(0|1 for back|through), write_allocate( 0|1 for no-alc|alc)\n", i);
	}
}

void printCacheConfig(CacheConfig cc){
	printf("size = %d\n", cc.size);
	printf("associativity = %d\n", cc.associativity);
	printf("block size = %d\n", cc.block_size);
	printf("set number = %d\n", cc.set_num);
	if(cc.write_through == 0)
		printf("write back\n");
	else
		printf("write through\n");
	if(cc.write_allocate == 0)
		printf("no write allocate\n");
	else
		printf("write allocate\n");
}

int main(int argc, char* argv[]) {
	int argCount = 1;
	CacheConfig cc[6];
	Cache L[6];
	// default
	for(int i = 1; i <= 5; i++){
		if(i == 1){
			cc[i].size = 1024;		// 32K
			cc[i].AccessTime = 1.90655475818;
		}
		else if(i == 2){
			cc[i].size = 1024*2;	// 256K
			cc[i].AccessTime = 2.13542188553;
		}
		else if(i == 3){
			cc[i].size = 1024*3;		// 8M
			cc[i].AccessTime = 5.39454845224;
		}
		else cc[i].size = 0;
		cc[i].associativity = 2;	// 8 associativity
		cc[i].block_size = 16*i;		// line size
		cc[i].set_num = cc[i].size / (cc[i].block_size*cc[i].associativity);
		cc[i].write_through = 0;
		cc[i].write_allocate = 1;
	}	
	// Access Time = Hit Time + Miss Penalty x Miss Rate
	// Access time: time between the request and when the data is available (or written)
	// Cycle time: time between requests

	// L1: Access time (ns): 1.90655475818
	// L2: Access time (ns): 2.13542188553
	// L3: Access time (ns): 5.39454845224
	// one cycle time  (ns): 0.5
	// main memory time(ns): 50.0

	FILE *fp = NULL;
	//char* debugArgs = "";
	for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
		argCount = 1;
		//printf("%d\n", argc);
        if (argv[0][1] == 's') {
        	if(argc < 4){
        		help();
        		return 0;
        	}
        	int num = atoi(&argv[0][2]);
        	cc[num].size = atoi(argv[1])*1024;
			cc[num].associativity = atoi(argv[2]);
			cc[num].block_size = atoi(argv[3]);
			cc[num].set_num = cc[num].size / cc[num].block_size;
			argCount = 4; 
        }
        if (argv[0][1] == 'm') { 
        	if(argc < 3){
        		help();
        		return 0;
        	}
        	int num = atoi(&argv[0][2]);
        	cc[num].write_through = atoi(argv[1]);
        	cc[num].write_allocate = atoi(argv[2]);
        	argCount = 3;
        	if(cc[num].write_through > 1 || cc[num].write_allocate > 1){
        		help();
        		return 0;
        	}
	    }
	    if(!strcmp(*argv, "-f")){
	    	if(argc < 2){
	    		help();
	    		return 0;
	    	}
	    	if(!(fp = fopen(argv[1], "r")))
	    		printf("unable to open trace\n");
	    	else
	    		printf("file %s opened successfullly\n", argv[1]);
	    	argCount = 2;
	    	//printf("!!!\n");
	    }
	    
	}
	//DebugInit(debugArgs);	

	Memory * MainMem=new Memory(50);
	Storage* p=MainMem;
	//char *d="ABC";
	// initialization is also in this loop
	for(int i = 5; i > 0; i--){
		if(cc[i].size != 0){
			L[i].SetLower(p);
			L[i].SetConfig(cc[i]);
			//*d=(*d)+1;
			L[i].SetName(i);
			p=&L[i];
			printf("Configurations for L%d cache:\n", i);
			printCacheConfig(cc[i]);
		}
	}

	if(fp == NULL){
		printf("no trace file\n");
		help();
		return 0;
	}

	char* request = new char[30];
	char c;
	int hit=0;
	int cycle=0;
	long long sum_hit=0;
	long long sum_cycle=0;

	while(fgets(request, 20, fp)){
		printf("%s\n", request);
		unsigned long addr = atol(&request[2]);
		printf("%llx\n", addr);
		hit=0;
		cycle=0;
		if(request[0] == 'r'){
			// read command
			L[1].HandleRequest(addr,1,1,&c,hit,cycle);
		}
		else if(request[0] == 'w'){
			c='c';
			L[1].HandleRequest(addr,1,0,&c,hit,cycle);
			// write command
		}
		sum_cycle+=cycle;
		sum_hit+=hit;
	}
}