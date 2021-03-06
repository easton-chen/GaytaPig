#ifndef CACHE_STORAGE_H_
#define CACHE_STORAGE_H_

#include <stdint.h>
#include <stdio.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)

// Storage access stats
typedef struct StorageStats_ {
  int access_counter;
  int miss_num;
  int access_time; // In nanoseconds
  int replace_num; // Evict old lines
  int fetch_num; // Fetch lower layer
  int prefetch_num; // Prefetch
} StorageStats;

// Storage basic config
typedef struct StorageLatency_ {
  int hit_latency; // In nanoseconds
  int bus_latency; // Added to each request
} StorageLatency;

class Storage {
 public:
  Storage() {
    stats_.access_counter = 0;
    stats_.miss_num = 0;
    stats_.access_time = 0;
    stats_.replace_num = 0;
    stats_.fetch_num = 0;
    stats_.prefetch_num = 0;
  }
  ~Storage() {}

  // Sets & Gets
  void SetStats(StorageStats ss) { stats_ = ss; }
  void GetStats(StorageStats &ss) { ss = stats_; }
  void SetLatency(StorageLatency sl) { latency_ = sl; }
  void GetLatency(StorageLatency &sl) { sl = latency_; }

  // Main access process
  // [in]  addr: access address
  // [in]  bytes: target number of bytes
  // [in]  read: 0|1 for write|read
  // [i|o] content: in|out data
  // [out] hit: 0|1 for miss|hit
  // [out] time: total access time
  virtual void HandleRequest(uint64_t addr, int byte, int read,
                             char *content, int &hit, int &time, bool pre_flag) = 0;
  void print_result() {
    printf("access_counter:\t%d\n",stats_.access_counter);
    printf("miss_num:\t%d\n",stats_.miss_num);
    printf("access_time:\t%d\n",stats_.access_time);
    printf("replace_num:\t%d\n",stats_.replace_num);
    printf("fetch_num:\t%d\n",stats_.fetch_num);
    printf("prefetch_num:\t%d\n",stats_.prefetch_num);
    printf("miss rate:\t%lf\n",(double)stats_.miss_num/(double)stats_.access_counter);
  }
 protected:
  StorageStats stats_;
  StorageLatency latency_;
};

#endif //CACHE_STORAGE_H_ 
