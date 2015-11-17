#include "cachelab.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long ul;

typedef struct {
  int v;
  ul tag;
} block;

//block* make_empty_block(int b){
//  block* rv=malloc(sizeof(block));
//  rv.v=0;
//  ul* temp = malloc(sizeof(ul)*b);
//  int i;
//  for (i=0;i<b;i++)
//    temp[i]=0;
//  return rv;
//}
typedef struct {
  int n_set;
  int E_val;
  int** lru_tbl;
  block** sets;
} cache;

cache* make_cache(int s, int e){
  cache* rv = malloc(sizeof(cache));
  int c = pow(2,s);
  rv->n_set=c;
  block** sets = malloc(sizeof(block*)*c);
  rv->E_val = e;
  int i,j;
  int** lru_tbl = malloc(sizeof(int*)*c);
  for (i = 0; i < c; i++){
    int* lru_set = malloc(sizeof(int)*e);
    block* nv =malloc(sizeof(block)*e);
    for (j=0;j<e;j++){
      block temp;
      temp.v=0;
      temp.tag=0;
      nv[j]=temp;
      lru_set[j] = 0;
    }
    sets[i]=nv;
    lru_tbl[i]=lru_set;
  }
  rv->lru_tbl = lru_tbl;
  rv->sets = sets;
  return rv;
}

void free_cache(cache *c){
  int i;
  for (i=0; i < c->n_set; i++) {
    free(c->sets[i]);
    free(c->lru_tbl[i]);
  }
  free(c->sets);
  free(c->lru_tbl);
  free(c);
}

int find_lru_index(cache* c, ul s) {
  int i, index = 0;
  int* lru_set = c->lru_tbl[s];
  int len = c->E_val;
  for (i = 0; i < len; i++) {
    if (lru_set[index] > lru_set[i])
      index = i;
  }
  return index;
}

int lru_max(cache* c, ul s){
  int i, index = 0;
  int* lru_set = c->lru_tbl[s];
  int len = c->E_val;
  for (i = 0; i < len; i++) {
    if (lru_set[index] < lru_set[i])
      index = i;
  }
  return lru_set[index];
}

void evict(cache* c,ul set){
  // evicts LRU block in cache for designated set
  // updates LRU table (to 0)
  int ev = find_lru_index(c,set);
  c->sets[set][ev].v=0;
  c->sets[set][ev].tag=0;
  c->lru_tbl[set][ev]=0;
}

void store(cache* c, ul set, ul tag){
  // stores 'value' in set based on lru table
  // updates LRU table (to 1)
  int ev = find_lru_index(c,set);
  c->sets[set][ev].v = 1;
  c->sets[set][ev].tag = tag;
  c->lru_tbl[set][ev]=lru_max(c,set) + 1;
}


int check_cache(cache* c,ul set,ul tag,char type){
  int i,valid;
  for (i=0;i<c->E_val;i++){
    valid = c->sets[set][i].v;
    if(valid==0){
      // if(type=='S')
      //c->lru_tbl[set][i]+=1;
      return 0;  
    } else if (valid==1) {
      if (tag==c->sets[set][i].tag){
	c->lru_tbl[set][i]= lru_max(c,set) + 1; // updates in the case of a hit
	return 1;
      } else if (i == c->E_val - 1)
	return -1;
    } else {
      continue;
    }
  }
  return 2; //should never get here
}

//int find_lru_index(cache* c, ul s) {
//  int i, index = 0;
//  int* lru_set = c->lru_tbl[s];
//  int len = c->E_val;
//  for (i = 0; i < len; i++) {
//    if (lru_set[index] > lru_set[i])
//      index = i;
//  }
//  return index;
//}

// int find_lru_index(int* a, int len) {
//   int i, index = 0;
//   for (i = 0; i < len; i++) {
//     if (a[index] > a[i])
//       index = i;
//   }
//   return index;
// }

int main(int argc, char *argv[])
{
  //	if (argc < 9) {
  //		fprintf(stderr, "Not enough inputs\n");
  //		exit(1);
  //	}
  int hitCount, missCount,evictionCount;
  hitCount = missCount = evictionCount = 0;
  int i = 1; // index for walking through commandline inputs
  int h = 0, v = 0; // checked to see if h and v flags were set
  int s, e, b;  // to be set by user
  FILE* t; // file to be read from
  while (i != argc) {
    if (!(strcmp(argv[i], "-vh") || !(strcmp(argv[i], "-hv")))) {
      // h and v
      h = v = 1;
      i++;
    } else if (!strcmp(argv[i], "-h")) {
      // print usage info
      h = 1;
      i++;
    } else if (!strcmp(argv[i], "-v")) {
      // displays trace info
      v = 1;
      i++;
    } else if (!strcmp(argv[i], "-s")) {
      s = atoi(argv[i+1]);
      i += 2;
    } else if (!strcmp(argv[i], "-E")) {
      e = atoi(argv[i+1]);
      i += 2;
    } else if (!strcmp(argv[i], "-b")) {
      b = atoi(argv[i+1]);
      i += 2;
    } else if (!strcmp(argv[i], "-t")) {
      t = fopen(argv[i+1], "r");
      i += 2;
    }
  }
  if (h)
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>");
  cache* w_cache = make_cache(s,e);
  char type;
  ul addr;
  int id;
  while (fscanf(t,"%c %lx,%d",&type,&addr,&id)!=EOF)
    if (type == 'S' || type == 'L' || type == 'M'){
      ul t_filter = ((~0) >> (s+b)) << (s+b);
      ul s_filter = ((~0) << (32-s)) >> (32-b-s);
      //unsigned long b_filter = (~0) >> (32-b);
      ul tag = (t_filter & addr) >> (s + b);
      ul set = ((s_filter^t_filter) & addr) >> b;
      //unsigned long b_off = b_filter & addr;
      int hm = check_cache(w_cache,set,tag,type);
      switch (hm){
      case 0:
	store(w_cache,set,tag);
	missCount++;
	if (v){
	  printf("%c %lx,%d miss",type,addr,id);
	  if (type == 'M')
	    printf(" hit");
	  printf("\n");
	}
	break;
      case 1:
	hitCount++;
	if(v){
	  printf("%c %lx,%d hit",type,addr,id);
	  if(type =='M')
	    printf(" hit");
	  printf("\n");
	}
	break;
      case (-1):
	missCount++;
	evictionCount++;
	evict(w_cache,set);
	store(w_cache,set,tag);
	if(v){
	  printf("%c %lx,%d miss eviction",type,addr,id);
	  if(type =='M')
	    printf(" hit");
	  printf("\n");
	}
	break;
      default:
        fprintf(stderr,"issue with switch reading lines");
        exit(1);
      }
      if (type == 'M')
	hitCount++;
    } else 
	  continue;
  free_cache(w_cache);
  printSummary(hitCount, missCount, evictionCount);
  return 0;
}

// while read != -1
// use getline(&str, &len, *FILE);
// str = (str) read();
// (long int) address = strtoi(str,NULL,16);
// Use address to determine cache location
// run into hit/miss/evict, update variables
// go back to loop
// save M, L, S to an integer value
// int mode = 1, 2, 3 means M, L, S respectively
// x = add
// tag = (x>>b) >>s & ~(0x7FFFFF>>b>>s);
// set = add << (32 - s - b) >>
// such that we are look at C[S][i]
// use a for loop, we walk from i = 0 til the end
// for (i = 0; i < E; i++)
// if (C[S][i].v == 0)
// 	miss and modify without evicting

