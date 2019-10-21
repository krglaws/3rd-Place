#include <stdio.h>

int main()
{
  char* str = "Content-Length: 123";
  int num = -1;
  sscanf(str, "%*s %d", &num);
  printf("%d\n", num);
}
