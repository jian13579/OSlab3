#include <iostream>
#include "vm_app.h"

using namespace std;

int main()
{
  char *p;
  p = (char *) vm_extend();

  int a = vm_syslog(p,1);
  int b = vm_syslog(p,20000);
  p[200] = 'h';
  p[6000] = 'e';
  p[6001] = 'l';
  p[7000] = 'l';
  p[7643] = 'o';
  p[8000] = '!';

  return a;
}
