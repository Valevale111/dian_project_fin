//atfer testing,modify for level 0-1
//-h,-v
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
//打印参数列表
//   printf("argc=%d\n",argc);
//   for(int i=0;i<argc;i++) {
//     printf("argv[%d]=%s ",i, argv[i]);
//   }
//   printf("\n"); 

  int opt,flags;
  flags = 0;
  char *optstring = "hv"; 
  while ((opt = getopt(argc, argv, optstring)) != -1) {
    switch(opt) {
      case 'h': 
 //       printf("");
        break;
      case 'v': 
        printf("dian-player v1.0\n");
        break;
      default:
        printf("wrong input!\n");
    }
  }

  exit(EXIT_SUCCESS);
}