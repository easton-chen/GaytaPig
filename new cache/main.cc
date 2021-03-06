#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

const char* FILE_ARG = "file";
const char* SET_ARG = "set";
const char* RUN_ARG = "run";
const char* QUIT_ARG = "quit";
const char* HELP_ARG = "help";

char FILENAME[30];
FILE* file = NULL;
int numerical = 16;
int cache_num = 0;
Cache* caches = NULL;
Memory m;
bool set_flag = false;
char content[64];
unsigned long long request_num = 0; 

void usage();
char *trim(char *str);
double cal_AMAT();

int main() {
  char* command;
  command = new char[120];
  cout << "You can input 'help' for usage" << endl;
  while(1) {
    cout << ">>>";
    cin.getline(command, 100);
    command = trim(command);
    if(!strcmp(command, FILE_ARG)) {
      cout << "Please input the file name or trace name" << endl;
      scanf("%s", FILENAME);
      cout << "Please input the numerical system(10|16 for decimal|hexadecimal system):";
      scanf("%d",&numerical);
      getchar();
      if(numerical != 10 && numerical != 16) {
        cout << "error numerical system!!" << endl;
        continue;
      }
      file = fopen(FILENAME, "r");
      if(file == NULL) {
        cout << "Open file error!" << endl;
      } else {
        cout << "Open file successfully!" << endl;
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
      if(caches) delete [] caches;
      caches = new Cache[cache_num];
      for(int i = cache_num - 1; i >= 0; i--) {
        //set lower
        if(i == cache_num - 1) caches[i].SetLower(&m);
        else caches[i].SetLower(&caches[i+1]);
        //set storage latency
        StorageLatency sl;
        cout << "please input the L" << (i+1) << " cache's storage latency:" << endl;
        cout << "bus latency:";
        scanf("%d",&(sl.bus_latency));
        cout << "hit latency:";
        scanf("%d",&(sl.hit_latency));
        caches[i].SetLatency(sl);
        //set cache config
        CacheConfig cc;
        cc.level = i + 1;
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
        cout << "Algorithm(0|1|2 for LRU|LFU|LIRS):";
        scanf("%d",&cc.algorithm_sort);
        cout << "Prefetch switch(0|1 for close|open):";
        scanf("%d",&cc.prefetch_flag);
        cout << "Bypass switch(0|1 for close|open):";
        scanf("%d",&cc.bypass_switch);
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
        int run_num = 0;
        request_num = 0;
        cout << "please input the num to run:";
        scanf("%d",&run_num);
        getchar();
        for(int i = 0; i < run_num; i++) {
          fseek(file, 0, 0);

          while(fscanf(file,"%c",&read) != -1) {
          	if(numerical == 10) fscanf(file, "%llu\n", &addr);
            else if(numerical == 16) fscanf(file, "%llx\n", &addr);
          	//printf("addr:%llx\t%c\n",addr,read);
          	if(read == 'r')
              caches[0].HandleRequest(addr,1,1,content,hit,time,0);
            else if(read == 'w')
              caches[0].HandleRequest(addr,1,0,content,hit,time,0);
          //printf("Request %llu access time: %dns\n", request_num++, time);
            request_num++;
          }
        }

        
        cout << "run over! here is the result:" << endl;
        ////printf("case-------------\n\n");
        CacheConfig cc;
        caches[0].GetConfig(cc);
        StorageStats stats;
        caches[0].GetStats(stats);
        //int access_time = stats.access_time;
        //double miss_rate=(double)stats.miss_num/(double)stats.access_counter;
        //printf("%d %d %d %d %d %d %d %lf\n",cc.size,cc.set_num,cc.associativity,cc.block_size,cc.write_through,cc.write_allocate,access_time,miss_rate);
        for(int i = 0; i < cache_num; i++) {
          printf("-------------L%d----------------\n",i+1);
          caches[i].print_result();
        }
        printf("-----------Memory--------------\n");
        m.print_result();
        double AMAT;
        AMAT = cal_AMAT();
        printf("AMAT:%.04lf\n",AMAT);
        //delete [] caches;
      }
    }
    else if(!strcmp(command, QUIT_ARG)) {
      if(caches) delete [] caches;
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
  //printf("Request access time: %dns\n", time);
  l1.HandleRequest(1024, 0, 1, content, hit, time);
  //printf("Request access time: %dns\n", time);

  l1.GetStats(s);
  //printf("Total L1 access time: %dns\n", s.access_time);
  m.GetStats(s);
  //printf("Total Memory access time: %dns\n", s.access_time);
  return 0;*/

void usage() {
  printf("command\t\t\tusage\n");
  printf("file\t\topen file\n");
  printf("set\t\tset the latency and configuration of the caches\n");
  printf("run\t\trun the simulator\n");
  printf("quit\t\tquit the program\n");
  printf("help\t\thelp information like this\n");
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

double cal_AMAT() {
  double res = 0.0;
  double lower_amat; //lower amat equal to upper level's miss penalty;
  StorageStats stats;
  StorageLatency sl;
  m.GetLatency(sl);
  lower_amat = sl.hit_latency+sl.bus_latency;
  for(int i = cache_num - 1; i>=0; i--) {
    caches[i].GetStats(stats);
    caches[i].GetLatency(sl);
    lower_amat = sl.hit_latency + sl.bus_latency
                +((double)stats.miss_num/(double)stats.access_counter)*lower_amat;
    //printf("%lf\n",lower_amat);
  }
  res = lower_amat;
  return res;
}