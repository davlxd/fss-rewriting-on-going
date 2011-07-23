
#include "path.h"
#include "fss.h"


int main()
{
  char src[MAX_PATH_LEN];
  char dst[MAX_PATH_LEN];

  char *dst_const = "/home/i/";
  char *src_const = "/dd/asd";
  strncpy(dst, dst_const, strlen(dst_const)+1);
  strncpy(src, src_const, strlen(src_const)+1);

  pathncat(src, dst, MAX_PATH_LEN);
  
  printf("--%s--\n", dst);
  printf("==%s--\n", src);

  return 0;
}
