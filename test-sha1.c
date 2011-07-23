
#include "wrap-sha1.h"


int main()
{
  char *target = "/home/i/a.out";
  char *rela = "/a.out/";
  char digest[41];
  char hash[41];

  get_hash(target, rela, digest, 41, hash, 41);
  printf("--%s--\n", digest);
  printf("==%s--\n", hash);

  return 0;
}
