#include "stdio.h"
#include "cache.h"
#include "memory.h"
using namespace std;

const char* FILE_ARG = "file";
const char* SET_ARG = "set";
const char* RUN_ARG = "run";
const char* QUIT_ARG = "quit";
const char* HELP_ARG = "help";

char FILENAME[30];
FILE* file = NULL;
int cache_num = 0;
Cache* caches = NULL;
Memory m;
bool set_flag = false;
char content[64];

void usage();
char *trim(char *str);

int main() {
  char* command;
  command = new char[120];
  cout << "cache simulator by ycx" << endl;
  cout << "version V1.0" << endl;
  cout << "you can input 'help' for usage" << endl;
  while(1) {
    cout << ">>>";
    cin.getline(command, 100);
    command = trim(command);
    if(!strcmp(command, FILE_ARG)) {
      cout << "please input the file name or trace name" << endl;
      scanf("%s", FILENAME);
      getchar();
      file = fopen(FILENAME, "r");
      if(file == NULL) {
        cout << "open file error!" << endl;
      } else {
        cout << "open file successfully!" << endl;
      }
    } 
    else if(!strcmp(command, SET_ARG)) {
      StorageLatency msl;
      cout << "----------memory set-----------" << endl;
      cout << "please input the memory's storage latency" << endl;
      cout << "bus latency:0" << endl;
      cout << "hit latency:";
      scanf("%d",&msl.hit_latency);
      msl.bus_latency = 0;
      m.SetLatency(msl);

      cout << "----------cache set------------" << endl;
      cout << "please input the num of caches you want to set:";
      scanf("%d",&cache_num);
      caches = new Cache[cache_num];
      for(int i = cache_num - 1; i >= 0; i--) {
        //set lower
        if(i == cache_num - 1) caches[i].SetLower(&m);
        else caches[i].SetLower(&caches[i+1]);
        //set storage latency
        StorageLatency sl;
        cout << "please input the L" << (i+1) << " cache's storage latency:" << endl;
        cout << "bus latency:0" << endl;
        cout << "hit latency:";
        scanf("%d",&(sl.hit_latency));
        sl.bus_latency = 0;
        caches[i].SetLatency(sl);
        //set cache config
        CacheConfig cc;
        cout << "please input the L" << (i+1) << " cache's configuration:" << endl;
        cout << "cache size:";
        scanf("%d",&cc.size);
        cout << "associativity:";
        scanf("%d",&cc.associativity);
        cout << "set num:";
        scanf("%d",&cc.set_num);
        cout << "write through(0|1 for back|through):";
        scanf("%d",&cc.write_through);
        cout << "write allocate(0|1 for no-alc|alc):";
        scanf("%d",&cc.write_allocate);
        cc.block_size = cc.size/(cc.associativity*cc.set_num);
        caches[i].SetConfig(cc);
      }

      getchar();
      set_flag = true;
    }
    else if(!strcmp(command, RUN_ARG)) {
      if(file == NULL || set_flag == false) {
        cout << "not ready to run" << endl;
        cout << "make sure you have open the file and set the caches" << endl;
      }
      else {
        uint64_t addr;
        char read;
        int hit, time;
        int request_num = 0;
        while(fscanf(file,"%c",&read) != -1) {
          fscanf(file, "%llu", &addr);
          printf("addr:%llx\t%c\n",addr,read);
          if(read == 'r')
            caches[0].HandleRequest(addr,1,1,content,hit,time);
          else if(read == 'w')
            caches[0].HandleRequest(addr,1,1,content,hit,time);
          printf("Request %d access time: %dns\n", request_num++, time);
        }
        cout << "run over! here is the result:\n" << endl;
        for(int i = 0; i < cache_num; i++) {
          printf("-------------L%d----------------",i+1);
          caches[i].print_result();
        }
      }
    }
    else if(!strcmp(command, QUIT_ARG)) {
      break;
    }
    else if(!strcmp(command, HELP_ARG)) {
      usage();
    }
    else {
      cout << "illegal command!" << endl;
      usage();
    }
  }
}

  /*Memory m;
  Cache l1;
  l1.SetLower(&m);

  StorageStats s;
  s.access_time = 0;
  m.SetStats(s);
  l1.SetStats(s);

  StorageLatency ml;
  ml.bus_latency = 6;
  ml.hit_latency = 100;
  m.SetLatency(ml);

  StorageLatency ll;
  ll.bus_latency = 3;
  ll.hit_latency = 10;
  l1.SetLatency(ll);

  int hit, time;
  char content[64];
  l1.HandleRequest(0, 0, 1, content, hit, time);
  printf("Request access time: %dns\n", time);
  l1.HandleRequest(1024, 0, 1, content, hit, time);
  printf("Request access time: %dns\n", time);

  l1.GetStats(s);
  printf("Total L1 access time: %dns\n", s.access_time);
  m.GetStats(s);
  printf("Total Memory access time: %dns\n", s.access_time);
  return 0;*/

void usage() {
  return;
}

char *trim(char *str) {
    char *p = str;
    char *p1;
    if(p)
    {
        p1 = p + strlen(str) - 1;
        while(*p && isspace(*p)) p++;
        while(p1 > p && isspace(*p1)) *p1-- = 0;
    }
    return p;
}