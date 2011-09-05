
#include "path.h"
#include "fss.h"


int main()
{
  char src[MAX_PATH_LEN];
  char dst[MAX_PATH_LEN];

  char *dst_const = "/hom/ei";
  char *src_const = "asdf/";
  strncpy(dst, dst_const, strlen(dst_const)+1);
  strncpy(src, src_const, strlen(src_const)+1);

  
  printf("--%s--\n", pathncat(dst, src, MAX_PATH_LEN));
  printf("==%s--\n", src);

  return 0;
}
