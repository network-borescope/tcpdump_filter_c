#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Rede{
	char *nome;
	int net[4];
  int mask[4];
  char* lat;
  char* lon;
  char* id;
}rede;

void Debug_print(rede a);
void load_ips(char* filename);
rede site_from_ip_addr(int* addr);
rede site_from_ip(char* ip);
void free_cidrs();
