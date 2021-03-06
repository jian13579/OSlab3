#include <iostream>
#include "vm_app.h"

int main(){

  char *p[5];

  for (int i = 0; i < 5;i++){
    p[i] = (char *) vm_extend();
  }

  for (int i = 0; i < VM_PAGESIZE-2; i++){
    p[0][i] = '1';
  }
  for (int i = 0; i < VM_PAGESIZE-2; i++){
    p[1][i] = '1';
  }
  for (int i = 0; i < VM_PAGESIZE-2; i++){
    p[2][i] = '1';
  }
  for (int i = 0; i < VM_PAGESIZE-2; i++){
    p[3][i] = '1';
  }
  for (int i = 0; i < VM_PAGESIZE-2; i++){
    p[4][i] = '1';
  }

  p[0][VM_PAGESIZE-1] = '2';
  p[1][VM_PAGESIZE-1] = '2';
  p[2][VM_PAGESIZE-1] = '2';
  p[3][VM_PAGESIZE-1] = '2';
  p[4][VM_PAGESIZE-1] = '2';
  //p[VM_PAGESIZE] = '2';

  vm_syslog(p[1],VM_PAGESIZE);
  vm_syslog(p[2],VM_PAGESIZE);
  vm_syslog(p[3],VM_PAGESIZE);
  vm_syslog(p[4],VM_PAGESIZE);
  vm_syslog(p[0],VM_PAGESIZE);
  vm_syslog(p[0]-10, 10);
  vm_syslog(p[0]+100, 0xffffffff);
}
