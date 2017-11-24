#include "cache.h"

void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
  stats_.access_counter++;
  hit = 0;
  time = 0;
  // Bypass?
  if (!BypassDecision()) {
    PartitionAlgorithm();
    // Miss?
    if (ReplaceDecision(addr)) {
      // Choose victim
      //printf("miss\n");
      ReplaceAlgorithm(addr, read, time);
    } 
    else { //hit
      //printf("hit\n");
      if (read == 0) {
          CacheConfig cc;
          GetConfig(cc);
          if(cc.write_through == 0) { //write back
            //hit and set the dirty bit
            set_dirty(addr);
          } 
          else if(cc.write_through == 1) { //write through
            int lower_hit, lower_time;
            lower_->HandleRequest(addr, bytes, read, content,
                          lower_hit, lower_time); 
            time += latency_.bus_latency + lower_time;
          }
      }

      // return hit & time
      hit = 1;
      time += latency_.bus_latency + latency_.hit_latency;
      stats_.access_time += time;
      return;
    }
  }
  // Prefetch?
  if (PrefetchDecision()) {
    PrefetchAlgorithm();
  } 
  else {
    stats_.miss_num++;
    stats_.fetch_num++;
    // Fetch from lower layer
    int lower_hit, lower_time;
    lower_->HandleRequest(addr, bytes, read, content,
                          lower_hit, lower_time);
    hit = 0;
    time += latency_.bus_latency + lower_time + latency_.hit_latency;
    stats_.access_time += time;
  }
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm() {
  return;
}

int Cache::ReplaceDecision(uint64_t addr) {
  CacheConfig cc;
  GetConfig(cc);
  int t=0,s=0,b=0;
  int block_size = cc.block_size;
  int set_num = cc.set_num;
  while(block_size != 1){
    block_size /= 2;
    b++;
  }
  while(set_num != 1){
    set_num /= 2;
    s++;
  }
  t = 8*sizeof(uint64_t) - s - b;

  unsigned set_index = (addr << t) >> (t + b);
  uint64_t tag = addr >> (s + b);
  for(int i = 0; i < cc.associativity; i++)
  {
    //valid and tag is equal -> hit
    if(sets[set_index].line[i].valid && sets[set_index].line[i].tag == tag){
      //update timestamp
      sets[set_index].line[i].timestamp = request_num;
      return 0;
    }
  }
  //miss
  return 1;
}

//case 1:not found  case 2:conflict miss
void Cache::ReplaceAlgorithm(uint64_t addr, int read, int &time) {
  CacheConfig cc;
  GetConfig(cc);
  //only if write and wirte non-allocate is there no replacement
  if(read == 0 && cc.write_allocate == 0)
  {
    return;
  }

  //consider both case 1 and case 2 as replace
  stats_.replace_num++;
  int t=0,s=0,b=0;
  int block_size = cc.block_size;
  int set_num = cc.set_num;
  while(block_size != 1){
    block_size /= 2;
    b++;
  }
  while(set_num != 1){
    set_num /= 2;
    s++;
  }
  t = 8*sizeof(uint64_t) - s - b;

  unsigned set_index = (addr << t) >> (t + b);
  uint64_t tag = addr >> (s + b);
  int replace_index = sets[set_index].if_full();

  if(replace_index == -1) {
    //case 2:conflict miss
    replace_index = sets[set_index].find_LRU();
    //write back and modified
    if(cc.write_through == 0 && sets[set_index].line[replace_index].dirty) {
      uint64_t old_addr = (set_index << b) 
                        + (sets[set_index].line[replace_index].tag << (s + b));
      int lower_hit, lower_time;
      char content[64];
      lower_->HandleRequest(old_addr, 1, 0, content,
                          lower_hit, lower_time);
      time += lower_time;
    }

    //sets[set_index].LRU_queue.pop();
    //sets[set_index].LRU_queue.push(replace_index);
    sets[set_index].line[replace_index].valid = true;
    if(read == 0)
      sets[set_index].line[replace_index].dirty = true;
    else
      sets[set_index].line[replace_index].dirty = false;
    sets[set_index].line[replace_index].tag = tag;
  } 
  else {
    //case 1:not found
    /*
    int i = 0;
    while(sets[set_index].line[i].valid)
      i++;
    replace_index = i;
    */
    //sets[set_index].LRU_queue.push(replace_index);
    sets[set_index].line[replace_index].timestamp = request_num;
    sets[set_index].line[replace_index].valid = true;
    if(read == 0)
      sets[set_index].line[replace_index].dirty = true;
    else
      sets[set_index].line[replace_index].dirty = false;
    sets[set_index].line[replace_index].tag = tag;
  }
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
  return;
}

void Cache::SetConfig(CacheConfig cc) {
  config_ = cc;
  sets = new cache_set[cc.set_num];
  for(int i = 0; i < cc.set_num; i++){
    sets[i].init(cc.associativity, cc.block_size);
  }
}

void Cache::GetConfig(CacheConfig &cc) {
  cc = config_;
}

void Cache::set_dirty(uint64_t addr) {
  CacheConfig cc;
  GetConfig(cc);
  int t=0,s=0,b=0;
  int block_size = cc.block_size;
  int set_num = cc.set_num;
  while(block_size != 1){
    block_size /= 2;
    b++;
  }
  while(set_num != 1){
    set_num /= 2;
    s++;
  }
  t = sizeof(uint64_t) - s - b;
  unsigned set_index = (addr << t) >> (t + b);
  uint64_t tag = addr >> (s + b);
  for(int i = 0; i < cc.associativity; i++)
  {
    //valid and tag is equal
    if(sets[set_index].line[i].valid && sets[set_index].line[i].tag == tag) {
      sets[set_index].line[i].dirty = true;
    }
  }
}