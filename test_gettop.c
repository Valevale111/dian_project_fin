//test function gettop
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
  /*
   * 打印参数列表
   */
  printf("argc=%d\n",argc);
  for(int i=0;i<argc;i++) {
    printf("argv[%d]=%s ",i, argv[i]);
  }
  printf("\n"); 

  int opt,flags;
  char *avalue,*bvalue;
  flags = 0;
  char *optstring = "a:b:c"; 
  while ((opt = getopt(argc, argv, optstring)) != -1) {
    switch(opt) {
      case 'a': 
        flags = 1;
        avalue = optarg;
        break;
      case 'b': 
        flags = 2;
        bvalue = optarg;
        break;
      case 'c': 
        flags = 3;
        break;
      default:
        flags = 5;
    }
  }
  printf("flags=%d; optind=%d; avalue=%s; bvalue=%s\n", flags, optind, avalue,bvalue);

  exit(EXIT_SUCCESS);
}