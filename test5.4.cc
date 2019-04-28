#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "vm_app.h"
using namespace std;

int main()
{
  // Creating first child
  int n1 = fork();

  // Creating second child. First child
  // also executes this line and creates
  // grandchild.
  int n2 = fork();

  char *p;
  p = (char *) vm_extend();
  p[0] = 'h';
  p = (char *) vm_extend();
  p[1] = 'e';
  p = (char *) vm_extend();
  p[2] = 'l';
  p = (char *) vm_extend();
  p[3] = 'l';
  p = (char *) vm_extend();
  
  return 0;
} 
