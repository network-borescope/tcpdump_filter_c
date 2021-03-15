#include "ip_to_nome_lat_lon.h"

#define MAX_LINE_SZ 200
#define MAX_NETS 8
#define MAX_REDES 78

rede cidrs[MAX_REDES];

void Debug_print(rede a){
  printf("%s, %s (%s,%s)\nNet:", a.nome, a.id, a.lat, a.lon);
  for (int i = 0; i < 4; i++) {
    printf("%d ",a.net[i] );
  }
  printf("\nMask:");
  for (int i = 0; i < 4; i++) {
    printf("%d ",a.mask[i] );
  }
  printf("\n\n");
}

void load_ips(char* filename){

  char *nets [MAX_NETS];
  int addrs [4];
  int cidr;
  int atual = 0;


  char line[MAX_LINE_SZ+4];
  FILE *file;
  file = fopen(filename,"r");

  while(fgets(line, MAX_LINE_SZ , file)){
    if (line[0] == '#'|| !line[0] || line[0]=='\n') continue;

    char* id = strtok(line, ";");
    char* nome = strtok(NULL, ";");
    char* lat = strtok(NULL, ";");
    char* lon = strtok(NULL, ";");
    char* ips = strtok(NULL, ";");

    if(ips == NULL || ips[0] == '\n' || ips[0] == '\r') continue;
    int i;
    nets[0] = strtok(ips, ",");
    for (i = 1; i < MAX_NETS; i++){
      nets[i] = strtok(NULL, ",");
    }

    //Get address string and CIDR string from command line
    i = 0;
    while(nets[i]!=NULL){
      char* addrString = strtok(nets[i],"/");
      char* cidrString = strtok(NULL,"/");
      cidr = atoi(cidrString);
      addrs[0] = atoi(strtok(addrString,"."));
      for (int j = 1; j < 4; j++) {
        addrs[j] = atoi(strtok(NULL,"."));
      }

      //Initialize the netmask and calculate based on CIDR mask
      int mask[4] = {0, 0, 0, 0};
      for(int j = 0; j< cidr; j++ ) {
        mask[(int)j/8] = mask[(int)j/8] + (1 << (7 - j % 8));
      }

      //Initialize net and binary and netmask with addr to get network
      int net[4];
      for(int j = 0; j< 4; j++ ){
        net[j]= addrs[j] & mask[j];
      }

      rede r;
      r.nome = strdup(nome);
      r.lat = strdup(lat);
      r.lon = strdup(lon);
      r.id = strdup(id);
      for (int j = 0; j < 4; j++) {
        r.net[j] = net[j];
        r.mask[j] = mask[j];
      }
      //printf("%d\n", atual);
      //Debug_print(r);
      cidrs[atual] = r;
      atual++;
      i++;
    }
  }
  fclose(file);
}

rede site_from_ip_addr(int* addr){
    // Initialize net and binary and netmask with addr to get network
    int found ;
    int net_ip [4];
    rede r;
    r.id = NULL;
    for (int i = 0; i < 4; i++) net_ip[i] = addr[i] & 255;

    for (int i = 0; i < 78; i++) {
      found = 1;
      for (int j = 0; j < 4; j++) {
        if ((net_ip[j] & cidrs[i].mask[j]) != cidrs[i].net[j]){
          found = 0;
          break;
        }
      }
      if (found) return cidrs[i];
    }
    return r;
}

rede site_from_ip(char* ip){
    char x[16];
    strcpy(x,ip);
    int addr[4];
    char* tk = strtok(x, ".");
    addr[0] = atoi(tk);
    tk = strtok(NULL, ".");
    addr[1] = atoi(tk);
    tk = strtok(NULL, ".");
    addr[2] = atoi(tk);
    tk = strtok(NULL, ".");
    addr[3] = atoi(tk);
    return site_from_ip_addr(addr);

}

void free_cidrs(){
  for (int i = 0; i < MAX_REDES; i++){
    free(cidrs[i].nome);
    free(cidrs[i].lat);
    free(cidrs[i].lon);
    free(cidrs[i].id);
  }
}

/*
int main() {
    load_ips("pop_df_lat_lon.txt");
    printf("-------------\n");
    for(int i = 0; i < MAX_REDES; i++){
      Debug_print(cidrs[i]);
    }
    //int a[] = {200,130,29,0};
    //rede s = site_from_ip("153.330.56.0");
    //rede s = site_from_ip_addr(a);
    //Debug_print(s);
    return 0;
}
*/