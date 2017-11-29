#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include <math.h>
#include "storage.h"
#include "def.h"

extern unsigned long long request_num;

typedef struct CacheConfig_ {
  int size;
  int associativity;
  int set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  int block_size;
  int level;
} CacheConfig;

class Cache: public Storage {
 public:
  cache_set *sets;

  Cache() {}
  ~Cache() {
    delete [] sets;
  }

  // Sets & Gets
  void SetConfig(CacheConfig cc);
  void GetConfig(CacheConfig &cc);
  void SetLower(Storage *ll) { lower_ = ll; }
  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);

 private:
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm();
  // Replacement
  int ReplaceDecision(uint64_t addr);
  void ReplaceAlgorithm(uint64_t addr, int read, int &time);
  // Prefetching
  int PrefetchDecision();
  void PrefetchAlgorithm(uint64_t addr);

  //adding function
  void set_dirty(uint64_t addr);

  bool find(uint64_t addr);

  //bool is_full(int set_index);

  void insert(uint64_t addr);

  CacheConfig config_;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 
