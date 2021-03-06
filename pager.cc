/*External Pager*/
#include "vm_pager.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <cstring>
using namespace std;


/*Work Deferral: 

1. We defer work when we create a new virtual page and we initialize a zero bit instead of zeroing the page in
vm_extend(), we defer this until we actually fault, and then if it is a zero bit, then we zero it out every single time since the fault is cheaper than the reading from disk.
2. We only write to disk, dirty pages or pages that have been written to we defer the work to when we have
confirmed that the page has actually changed content wise from the last time it was written to disk.
3. We never write zero pages to disk, and only write zero pages to disk if we have changed something.
*/


typedef vector<page_table_t> tables; //Vector of page tables

struct Vpage{
  int disk_block;
  int dirty;//use dirty bit to test write, set dirty bit when we modify page, when we swap back in set to 0, unless we modify again
  int zero;//use zero bit to read or not, we don't need to write to disk
  int resident;
  int ppage;
  int reference;
  int arenaidx;
  pid_t pid;
};

struct process{
  vector<Vpage*> pageVector;
  page_table_t ptable;
  pid_t pid;
};

process* current; //Then we look through map each time
stack<int> phys_mem;
stack<int> disk;
map<pid_t, process*> processMap;
map<pid_t, process*>::iterator it;
vector<Vpage*> clockQ; //A queue of the most recently accessed processes

unsigned long convertIdxtoaddress(int idx){
  /*Conversion from idx into the void* address*/
    unsigned long address = ((unsigned long) idx * (unsigned long) VM_PAGESIZE) + (unsigned long) VM_ARENA_BASEADDR;
    return address;
};

int convertAddresstoIdx(unsigned long address){
  /*Conversion from address into the idx of Vpage vector*/
    int idx = (int) (address - (unsigned long)(VM_ARENA_BASEADDR))/ (unsigned long) VM_PAGESIZE;
    return idx;
};

void vm_init(unsigned int memory_pages, unsigned int disk_blocks){
/*Called when the pager starts, it is the number of pages provided in physical memory and the number of disk blocks avaliable on disk*/

  for (int i = 0; i < memory_pages; i++){
    phys_mem.push(i);
  }
  for (int j = 0; j < disk_blocks; j++){
    disk.push(j);
  }
    
}

void vm_create(pid_t pid){
/*Called when a new application starts, and the data structures it needs to handle the process, and its subsequent calls to the library.*/

  process* newProcess = new process;
  page_table_t ptable;
  //set entries for the entire range
  for (int i = 0; i < (int) ((unsigned long)VM_ARENA_SIZE/ (unsigned long)VM_PAGESIZE); i++){
    ptable.ptes[i].ppage = -1;
    ptable.ptes[i].read_enable = 0;
    ptable.ptes[i].write_enable = 0;
  }
  newProcess->ptable = ptable;
  newProcess->pid = pid;
  //process* toadd = &newProcess;
  processMap.insert(pair<pid_t, process* >(pid, newProcess));
  //  current = newProcess;
};

void vm_switch(pid_t pid){
  //write error for calling this before vm_create
  //If there is a process, then we need to swap the process
  page_table_t* temp = &(processMap[pid]->ptable);
  page_table_base_register = temp;
  current = processMap[pid];
};


int clockAlgorithm(){
  Vpage* pageToDisk;
  while((clockQ.front() -> reference)){
    //Cycles through the clockQ wipping the reference bits to 0
    pageToDisk = clockQ.front();
    pageToDisk -> reference = 0;//whenever I set reference bit to 0, also set read/write to 0 for the pages in the clock algorithm
    clockQ.push_back(clockQ.front());
    clockQ.erase(clockQ.begin());
    //pages may not be owned by the current process
  process* toChange = processMap.find(pageToDisk->pid)->second;
  toChange->ptable.ptes[pageToDisk->arenaidx].write_enable = 0;
  toChange->ptable.ptes[pageToDisk->arenaidx].read_enable = 0;
  }
  
  //set dirty bit to 0, since we no longer need it, unless we actually change the contents

  pageToDisk = clockQ.front();
  //set the rw bits page to false, and set to non-resident/noppage
  int freepage = pageToDisk->ppage;
  pageToDisk->ppage = -1;
  pageToDisk->resident = -1;
  process* toChange = processMap.find(pageToDisk->pid)->second;
  toChange->ptable.ptes[pageToDisk->arenaidx].write_enable = 0;
  toChange->ptable.ptes[pageToDisk->arenaidx].read_enable = 0;
  toChange->ptable.ptes[pageToDisk->arenaidx].ppage = -1;
  if(pageToDisk->dirty == 1){
    //only write if page is not zero and it is dirty
    disk_write(pageToDisk->disk_block, freepage);
  }

  pageToDisk->dirty = 0;
  clockQ.erase(clockQ.begin());

  //set the resident bit of the evicted page to nonresident
  return freepage;
}

int vm_fault(void *addr, bool write_flag){
 
  int size = current->pageVector.size(); 
  //Find vpage given the address
  unsigned long address = (unsigned long) addr; //The current vpage we divide by 2000 to get to the address
  int vpageidx = convertAddresstoIdx(address);
  
  if(vpageidx < 0 || vpageidx >= size){
    return -1;
  }
  
  Vpage* toUpdate = (current->pageVector.at(vpageidx));
  toUpdate-> arenaidx = vpageidx;
  int ppage_num;//current ppage we are on

  if(toUpdate->resident == -1){
    /*Either evict or get new ppage block*/
  
    if(phys_mem.empty()){
      ppage_num = clockAlgorithm();
    }
    else{
      ppage_num = phys_mem.top();
      phys_mem.pop();
    }

    toUpdate->ppage = ppage_num;
    current->ptable.ptes[vpageidx].ppage = ppage_num;
    if(toUpdate->zero){
      //If it hasn't been zeroed, I am zeroing it
      memset((char *) ((unsigned long) pm_physmem + (ppage_num * VM_PAGESIZE)), 0, VM_PAGESIZE);
    }else{
      disk_read(toUpdate->disk_block, toUpdate->ppage);
    }
    clockQ.push_back(toUpdate);
  }

  /*updating this new page*/
  toUpdate -> reference = 1;
  //  toUpdate -> ppage = ppage_num;
  toUpdate -> resident = 1;
  //update page_table_t
  if(write_flag || toUpdate->dirty){
    current->ptable.ptes[vpageidx].write_enable = 1;
    current->ptable.ptes[vpageidx].read_enable = 1;
    toUpdate->dirty = 1;
    toUpdate->zero = 0;
  }else{
    current->ptable.ptes[vpageidx].read_enable = 1;
  }
  
  return 0;
}

void vm_destroy(){
  /*Deallocates all of the memory, and memory of the current process
    (page tables, physical pages, and disk blocks), released physical pages need to go back onto the disk block.*/

  //Get each process in the map
  pid_t a = current->pid;
  
  for (it = processMap.begin(); it != processMap.end();it++)
  {
    /*We check for the process that is equal to the pid*/
    if(it->first != a){
      continue;
    }
    /*We delete the Vpages owned by the current process and the vpages that are owned by the same process in the clockQ.*/
    vector<Vpage *>::iterator it2;
    process* temp = it-> second;
    vector<Vpage *>::iterator it3;
    
    for (it2 = temp->pageVector.begin(); it2 != temp->pageVector.end();it2++)
    {
      Vpage* temp2 = *it2;
      for(it3 = clockQ.begin(); it3 != clockQ.end();it3++){
	if(temp2 == *it3){
	  clockQ.erase(it3);
      	  break;
      	}
      }
      
      if(temp2->ppage > -1){
	 phys_mem.push(temp2->ppage);	
      }
    
      disk.push(temp2->disk_block);
      delete temp2;
      
    }

    delete temp;
    processMap.erase(it);
  }
  
};



void* vm_extend(){

  //Check if the current has a vpage
  if(disk.empty()){
    return NULL;
  }

  //create vpage
  Vpage* x = new Vpage;

  //get disk block
  x->disk_block = disk.top();
  disk.pop();
  x->zero = 1;
  x->dirty = 0;
  x->resident = -1;
  x->ppage = -1;
  x->reference = 0;
  //store vpage in process
  current->pageVector.push_back(x);
  int idx = current->pageVector.size() - 1;
  x->pid = current->pid;
  void* address = (void*) (( idx * (unsigned long) VM_PAGESIZE) + (unsigned long) VM_ARENA_BASEADDR);
  
  return address; 
};


int vm_syslog(void *message, unsigned int len){

/*Find the message address, and given the length of the thing
 and iterate through each of the Vpages and then grab it from physmem */

/*If len is 0, if message is outside of what is currently allocated in arena*/
  int size = current->pageVector.size();
  unsigned long cap = (unsigned long) size * (unsigned long) VM_PAGESIZE + (unsigned long) VM_ARENA_BASEADDR;
  
  unsigned long addr = (unsigned long) message;
  unsigned long max = addr + len;

  if(len <= 0 || max > cap || addr < (unsigned long) VM_ARENA_BASEADDR){
    return -1;
  }
  int firstpage = convertAddresstoIdx(addr);
  int lastpage = convertAddresstoIdx(max-1);

  unsigned long offset = addr - convertIdxtoaddress(firstpage);
  unsigned long end_offset = max - convertIdxtoaddress(lastpage);

  string s;
  for (int vpageidx = firstpage; vpageidx <= lastpage; vpageidx++){

    unsigned long faultaddress = convertIdxtoaddress(vpageidx);
    void* void_fault = (void *) faultaddress; 
    vm_fault(void_fault, false);

    //get the actual page
    Vpage* toaccess = current->pageVector.at(vpageidx);
    int ppage_num = toaccess->ppage;

    unsigned long start = ((unsigned long) ppage_num * (unsigned long) VM_PAGESIZE);
    unsigned long end = start + (unsigned long) VM_PAGESIZE;
    
    if(vpageidx == firstpage){
      start = start + offset;
    }    
    
    //calculating end page
    if(vpageidx == lastpage){
      end = (len+offset) % ((unsigned long) VM_PAGESIZE) + ((unsigned long) ppage_num * (unsigned long) VM_PAGESIZE);//formula is top of (len-offset)%PageSize
    }

    //The case where the end page is the exact length of the page
    if(end % ((unsigned long) VM_PAGESIZE) == 0){
      end = VM_PAGESIZE + ((unsigned long) ppage_num * (unsigned long) VM_PAGESIZE);
    }

    //appending the strings
    for (unsigned long idx = start; idx < end; idx++){
     s.append(string(1, ((char *) pm_physmem)[idx]));
    }

  }

  cout << "syslog \t\t\t" << s << endl;

  return 0;
};
