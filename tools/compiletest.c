#include <stdio.h>

int main(int argc, char *argv[])
{
 char line[1024];
 int i;
 for (i = 1; i < argc; ++i)
 {
  if (argv[i][0] == '-')
  {
   if (argv[i][1] == 'o')
    ++i;
  }
  else
  {
   FILE *f = fopen(argv[i], "r");
   int k = 0;
   while (fgets(line, 1024, f))
   {
    ++k;
    printf("%s:\n  %d error 615\n", argv[i], k);
   }
   fclose(f);
  }
 }
 return 1;
}

