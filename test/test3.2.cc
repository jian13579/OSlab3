#include <iostream>
#include "vm_app.h"

using namespace std;

int main()
{
  char *p[5];
  for (int i = 0; i < 5; i++){
  p[i] = (char *) vm_extend();
  p[i][0] = 'h';
  p[i][1] = 'e';
  p[i][2] = 'l';
  p[i][3] = 'l';
  p[i][4] = 'o';
  vm_syslog(p[i], 5);
  }

  for (int j = 0; j < 5; j++){
    vm_syslog(p[j],5);
  }
}
