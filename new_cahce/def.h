#ifndef CACHE_DEF_H_
#define CACHE_DEF_H_

#define TRUE 1
#define FALSE 0
#define MAX 100000000

#include <queue>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <map>
using namespace std;

class cache_line {
public:
	bool valid;
	bool dirty;
	uint64_t tag;
	int block_size;
	char* block;
	unsigned long long timestamp;

	cache_line() {
		valid = false;
		dirty = false;
		tag = 0;
		block_size = 0;
		block = NULL;
	}

	~cache_line() {
		delete [] block;
	}
	void init(int bsize) {
		valid = false;
		dirty = false;
		tag = 0;
		block_size = bsize;
		block = new char[bsize];
		memset(block, 0, bsize);
		timestamp = 0;
	}
};


class cache_set {
public:
	int line_num;
	cache_line *line;
	map<uint64_t, int> tag_frequecy;
	//queue<int> LRU_queue; 	//the tail of the queue should be replaced
							//only happen when queue's size = line num

	cache_set() {
		line_num = 0;
		line = NULL;
	}

	~cache_set() {
		delete [] line;
	}

	int threshold_frequency() {
		int res_max = 0;
		int res_min = MAX;
		for(int i = 0; i<line_num; i++) {
			if(line[i].valid) {
				int temp = tag_frequecy[line[i].tag];
				res_max = temp > res_max ? temp : res_max;
				res_min = temp < res_min ? temp : res_min;
			}
			else {
				//have no-valid line ,frequency considered as 0
				return 0;
			}
		}
		return (res_max+2*res_min)/3;
	}
	void init(int associativity, int bsize) {
		line_num = associativity;
		line = new cache_line[line_num];
		for(int i = 0; i < line_num; i++)
		{
			line[i].init(bsize);
		}
		tag_frequecy.clear();
		//while(!LRU_queue.empty()) LRU_queue.pop();
	}
	int if_full(){
		for(int i = 0; i < line_num; i++)
		{
			if(line[i].valid == 0)
				return i;
		}
		return -1;
	}
	//only called when this set is in L1 cache and this set is full
	int find_LRU(){
		int index=0;
		unsigned long long use_time=0xffffffffffffffff;
		for(int i=0;i < line_num; i++)
		{
			if(line[i].timestamp < use_time)
			{
				index = i;
				use_time = line[i].timestamp;
			}
		}
		return index;
	}
	//only called when this set is in L2 cache and this set is full
	int find_LFU(){
		int index;
		int least_frequency = MAX;
		for(int i=0; i < line_num; i++)
		{
			if(tag_frequecy[line[i].tag] < least_frequency)
			{
				index = i;
				least_frequency = tag_frequecy[line[i].tag];
			}
		}
		return index;
	}
};

#endif //CACHE_DEF_H_
