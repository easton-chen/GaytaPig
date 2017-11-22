#ifndef CACHE_DEF_H_
#define CACHE_DEF_H_

#define TRUE 1
#define FALSE 0

#include <queue>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
using namespace std;

class cache_line {
public:
	bool valid;
	bool dirty;
	uint64_t tag;
	int block_size;
	char* block;

	cache_line() {
		valid = false;
		dirty = false;
		tag = 0;
		block_size = 0;
		block = NULL;
	}

	void init(int bsize) {
		valid = false;
		dirty = false;
		tag = 0;
		block_size = bsize;
		block = new char[bsize];
		memset(block, 0, bsize);
	}
};


class cache_set {
public:
	int line_num;
	cache_line *line;
	queue<int> LRU_queue; 	//the tail of the queue should be replaced
							//only happen when queue's size = line num

	cache_set() {
		line_num = 0;
		line = NULL;
	}

	void init(int associativity, int bsize) {
		line_num = associativity;
		line = new cache_line[line_num];
		for(int i = 0; i < line_num; i++)
		{
			line[i].init(bsize);
		}
		while(!LRU_queue.empty()) LRU_queue.pop();
	}
};

#endif //CACHE_DEF_H_
