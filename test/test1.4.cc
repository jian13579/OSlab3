#include <iostream>
#include "vm_app.h"

using namespace std;

int main()
{
  char *p;
  p = (char *) vm_extend();
  p[1] = 'h';
  p[2] = 'e';
  p[3] = 'l';
  p[4] = 'l';
  p[5] = 'o';
  p[6] = '!';
  vm_syslog(p+1, 6);
  char *r;
  r = (char *) vm_extend();
  r[2] = 'h';
  vm_syslog(r,9000);
  char *s;
  s = (char *) vm_extend();
  s[0]= 'a';
  vm_syslog(s,1);
  char *v;
  v = (char *) vm_extend();
  v[0] = '2';
  vm_syslog(v,1);
  char *q;
  q = (char *) vm_extend();
  q[0] = 'y';
  vm_syslog(q,1);
  p[6] = 'o';
  char *t;
  t = (char *) vm_extend();
  t[200] = 'h';
  t[2001]= 'i';
  vm_syslog(t,3000);
  char *l;
  l = (char *) vm_extend();
  l[2003]='e';
  vm_syslog(l,3000);
  r[12] = 'c';
  r[10000] = 'o';
  vm_syslog(r,12000);
  s[34] = 'c';
  s[10002] = 'p';
  vm_syslog(s,12000);
  p[3] ='o';
  vm_syslog(p,200);
  vm_syslog(s,2000);
  vm_syslog(r,2000);
  vm_syslog(v,2000);
  vm_syslog(q,2000);
  vm_syslog(s,2000);
}
