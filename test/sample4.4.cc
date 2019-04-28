#include <iostream>
#include "vm_app.h"

using namespace std;

int main()
{
  char *p[20];
  for (int i = 0; i < 6;i++){
    p[i] = (char *) vm_extend();
    for (int j = 0; j < 6; j++){
      p[i][j] = 'o';
      vm_yield();
    }
  }

  for (int i = 0; i < 6;i++){
    p[i][i+1] = 'w';
    vm_syslog(p[i], 10000);
    
  }
}
