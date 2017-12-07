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
#include <algorithm>
#include <map>
#include <deque>
using namespace std;

class LIRS_block {
public:
	uint64_t tag;
	int resident;	//-1 for not valid, other for line_index
	bool LIR;		//true for LIR type
	LIRS_block() {
		tag = 0;
		resident = -1;
		LIR = false;
	}
	bool operator==(const LIRS_block &b) {
		return tag == b.tag;
	}
};

class cache_line {
public:
	bool valid;
	bool dirty;
	uint64_t tag;
	int block_size;
	//char* block;
	unsigned long long timestamp;

	cache_line() {
		valid = false;
		dirty = false;
		tag = 0;
		block_size = 0;
		//block = NULL;
	}

	~cache_line() {
		//printf("delete blocks\n");
		//delete [] block;
	}
	void init(int bsize) {
		valid = false;
		dirty = false;
		tag = 0;
		block_size = bsize;
		//block = new char[bsize];
		//memset(block, 0, bsize);
		timestamp = 0;
	}
};


class cache_set {
public:
	int line_num;
	cache_line *line;
	map<uint64_t, int> tag_frequecy;

	int LIR_num,HIR_num;
	int warm_up_flag;
	deque<class LIRS_block> S_queue;
	deque<class LIRS_block> Q_queue;
	//queue<int> LRU_queue; 	//the tail of the queue should be replaced
								//only happen when queue's size = line num

	cache_set() {
		line_num = 0;
		line = NULL;
		LIR_num = 6;
		HIR_num = 2;
		warm_up_flag = 6;
	}

	~cache_set() {
		//printf("delete lines\n");
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
		return res_min;//(res_max+2*res_min)/3;
	}
	void init(int associativity, int bsize) {
		line_num = associativity;
		line = new cache_line[line_num];
		for(int i = 0; i < line_num; i++)
		{
			line[i].init(bsize);
		}
		tag_frequecy.clear();
		Q_queue.clear();
		S_queue.clear();
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
		index = if_full();
		if(index != -1) return index;
		unsigned long long use_time=0xffffffffffffffff;
		for(int i=0;i < line_num; i++)
		{
			if(line[i].valid && line[i].timestamp < use_time)
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
		index = if_full();
		if(index != -1) return index;
		int least_frequency = MAX;
		for(int i=0; i < line_num; i++)
		{
			if(line[i].valid && tag_frequecy[line[i].tag] < least_frequency)
			{
				index = i;
				least_frequency = tag_frequecy[line[i].tag];
			}
		}
		return index;
	}

	//return value: -1 for no replacement, other for line number 
	int update_LIRS(uint64_t tag) {
		int res = -1;
		if(warm_up_flag > 0) {
			LIRS_block lb;
			lb.tag = tag;
			deque<class LIRS_block>::iterator di;
			di = find(S_queue.begin(), S_queue.end(),lb);
			if(di != S_queue.end()) {
				lb = *di;
				S_queue.erase(di);
				S_queue.push_front(lb);
			}
			else {
				lb.resident = if_full();
				lb.LIR = true;
				res = lb.resident;
				warm_up_flag--;
				S_queue.push_front(lb);
			}
		}
		else {
			LIRS_block lb;
			lb.tag = tag;
			deque<class LIRS_block>::iterator di;
			di = find(S_queue.begin(), S_queue.end(),lb);
			if(di != S_queue.end()) {
				//in the S queue
				lb = *di;
				if(lb.LIR) {
					//a LIR block in the S queue, move it to the front
					S_queue.erase(di);
					S_queue.push_front(lb);
					//trimming
					while(S_queue.back().LIR == false)
						S_queue.pop_back();
				}
				else {
					//a HIR block in the S queue, move it to the front 
					//and set it as a LIR block. Then remove the end LIR
					//block from the S queue, set this block as the HIR
					//block and move it to the end of the Q queue
					if(lb.resident != -1) {
						//in the cache
						//remove it from Q queue.
						deque<class LIRS_block>::iterator qi;
						qi = find(Q_queue.begin(), Q_queue.end(), lb);
						Q_queue.erase(qi);
						S_queue.erase(di);
						//add the S queue's end block to the Q queue. 
						LIRS_block removed_block = S_queue.back();
						removed_block.LIR = false;
						S_queue.pop_back();
						Q_queue.push_front(removed_block);
						//trimming
						while(S_queue.back().LIR == false)
							S_queue.pop_back();
						//set lb as LIR and move it to the front of S queue
						lb.LIR = true;
						S_queue.push_front(lb);
					}
					else {
						//not in the cache, need to replace
						//remove the front  from Q queue.
						S_queue.erase(di);
						LIRS_block replaced_block;
						replaced_block = Q_queue.back();
						//if replaced block is in the S queue, set it as non-resident
						deque<class LIRS_block>::iterator replaced_i;
						replaced_i = find(S_queue.begin(), S_queue.end(), replaced_block);
						if(replaced_i != S_queue.end()) (*replaced_i).resident = -1;
						
						for(int i = 0; i < line_num; i++) {
							if(line[i].tag == replaced_block.tag)
								res = i;
						}
						if(res == -1) cout << "error1!!!" << endl;
						Q_queue.pop_back();
						//add the S queue's end block to the Q queue. 
						LIRS_block removed_block = S_queue.back();

						S_queue.pop_back();
						removed_block.LIR = false;
						Q_queue.push_front(removed_block);
						//trimming
						while(S_queue.back().LIR == false)
							S_queue.pop_back();
						//set lb as LIR and move it to the front of S queue
						lb.LIR = true;
						lb.resident = res;
						S_queue.push_front(lb);
					}
				}
			}
			else {
				//not in the S queue
				if(Q_queue.size() < HIR_num) {
					//still have valid block
					/*LIRS_block removed_block = S_queue.back();
					S_queue.pop_back();
					removed_block.LIR = false;
					Q_queue.push_front(removed_block);
					//trimming
					while(S_queue.back().LIR == false)
						S_queue.pop_back();*/

					//add new block to the front of the S queue.
					deque<class LIRS_block>::iterator qi;
					qi = find(Q_queue.begin(), Q_queue.end(), lb);
					if(qi == Q_queue.end()) {
						//not find in Q queue
						lb.LIR = false;
						lb.resident = if_full();
						S_queue.push_front(lb);
						Q_queue.push_front(lb);
						res = lb.resident;
					}
					else {
						//found,only need to push in S queue
						lb = *qi;
						S_queue.push_front(lb);
					}
				}
				else {
					deque<class LIRS_block>::iterator qi;
					qi = find(Q_queue.begin(), Q_queue.end(), lb);
					if(qi == Q_queue.end()) {
						//conflict miss, need to replace
						LIRS_block replaced_block;
						replaced_block = Q_queue.back();
						//if replaced block is in the S queue, set it as non-resident
						deque<class LIRS_block>::iterator replaced_i;
						replaced_i = find(S_queue.begin(), S_queue.end(), replaced_block);
						if(replaced_i != S_queue.end()) (*replaced_i).resident = -1;
						for(int i = 0; i < line_num; i++) {
							if(line[i].tag == replaced_block.tag)
								res = i;
						}
						if(res == -1) cout << "error2!!!" << endl;
						Q_queue.pop_back();

						lb.resident = res;
						lb.LIR = false;
						Q_queue.push_front(lb);
						S_queue.push_front(lb);
					}
					else {
						//in Q queue, not in S Queue
						lb = (*qi);
						Q_queue.erase(qi);
						Q_queue.push_front(lb);
						S_queue.push_front(lb);
					}
				}
			}
		}
		//printf("res:%d\n",res);
		//print_LIRS();
		//printf("\n");
		return res;
	}

	void print_LIRS() {
		printf("------------------S_queue------------------\n");
		for (int i = 0; i < S_queue.size(); i++) {
			printf("tag:%llx\tresident:%d\tLIR:%d\n", S_queue[i].tag,S_queue[i].resident,S_queue[i].LIR);
		}
		printf("------------------Q_queue------------------\n");
		for (int i = 0; i < Q_queue.size(); i++) {
			printf("tag:%llx\tresident:%d\tLIR:%d\n", Q_queue[i].tag,Q_queue[i].resident,Q_queue[i].LIR);
		}
	}
};

#endif //CACHE_DEF_H_
