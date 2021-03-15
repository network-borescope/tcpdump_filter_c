#include <signal.h>
#include <stdarg.h>
#include <inttypes.h>
#include "dict.h"
#include "ip_to_nome_lat_lon.h"

#include <sys/stat.h> // mkdir
int mkdir(const char *pathname, mode_t mode); // mkdir header

#define MAX_LINE_SZ 2001
#define BLANKS_PER_INDENT 4
//#define CAPTURE_TIME 1

static int global_stop = 0;
static FILE *global_fout = NULL;
static List *freelist = NULL;
static int CAPTURE_TIME = 1; // default value

void signal_handler() {
    global_stop = 1;
}

// [data, hora, val_ttl, val_proto, val_ip_id ] 
typedef struct {
	char data[15];
	char hora[5];
	char min[5];
	char ttl[6];
	char proto[6];
	char val_ip_id[10];
	int ip_src_arr[4];
	int ip_dst_arr[4];
	char port_dst[8];
} DataHandler;

typedef struct {
	char lst_data[15];
	char lst_hora[5];
	int lst_min_capture;
} TimeHandler;

void info_free(Info *pi) {
	if(freelist == NULL){
		freelist = createList();
	}
  	append(freelist, pi);
}

Info *info_alloc(void) {
	Info *pi = getFirstInfo(freelist);
	if (pi) { return pi; }
  	return malloc(sizeof(Info));
}

void list_flush(List *pl, FILE *global_fout) {
	Info *pi, *pa;
	pi = pl->first;
	if (!pi) return;
	for (pa = NULL; pi; pa = pi, pi = pi->dict_next) { 
		fprintf(global_fout,"%s;%d\n",pi->key, pi->counter);
	}
	pa->dict_next = freelist->first;
	freelist->first = pl->first;
	pl->first = NULL;
}

void insert_or_increment_key(DictInfo *d, char *prev_key) {
	if (prev_key[0] == '\0') return;

	Info *pi = dictLocate(d, prev_key);
	if (!pi) {
		pi = info_alloc();
		pi->counter = 0;
		strcpy(pi->key, prev_key);
		dictInsert(d, pi);
	}
	pi->counter ++;

}

void fatal(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    if (fmt) {
        vfprintf(stderr,fmt,args);
        fprintf(stderr,"\n");
    }
    va_end(args);
    exit(99);
}

int get_number(char *dst, char *src, int len, int end) {
    for (; len; dst ++, src++, len --) {
        if (!(*src >= '0' && *src <= '9')) return 0;
        *dst = *src;
    }
    if (end) *dst = '\0';
    return 1;
}

int get_number_var(char *dst, char *src, int max_len) {
    int len;
    for (len = 0; len < max_len; dst ++, src++, len++) {
        if (!(*src >= '0' && *src <= '9')) break;
        *dst = *src;
    }
    if (!len) return 0;
    *dst = '\0';
    return 1;
}

int header_line(DataHandler *dados, char *start) {
	char *_;
	char *p;
	
    p =  strtok(start, " ");
	if (!p) return 0;
    if (strlen(p) != 10) return 0;
#if 1
    if (!get_number(dados->data+0, p+0, 4, 0)) return 0;
    if (!get_number(dados->data+4, p+5, 2, 0)) return 0;
    if (!get_number(dados->data+6, p+8, 2, 1)) return 0;
    
#else
    if (!(p[0] >= '0' && p[0] <= '9')) return 0;
	strcpy(dados->data, p);
	dados->data[4] = dados->data[5];
	dados->data[5] = dados->data[6];
	dados->data[6] = dados->data[8];
	dados->data[7] = dados->data[9];
	dados->data[8] = '\0';
#endif

	p = strtok(NULL, " ");
	if (!p) return 0;
    if (strlen(p) < 6) return 0;
#if 1
    if (!get_number(dados->hora, p+0, 2, 1)) return 0;
    if (!get_number(dados->min , p+3, 2, 1)) return 0;
#else    
	p[2] = '\0';
	p[5] = '\0';
	strcpy(dados->hora, p);
	strcpy(dados->min, p+3);
#endif

	char *headerType = strtok(NULL, " ");
	if ( !headerType || strcmp(headerType, "IP") != 0) return 0;

	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");

	p = strtok(NULL, " "); // ttl
	if (!p) return 0;
	if (strlen(p) >= 6) fatal("Erro 4 %s\n", p);
#if 1
    if (!get_number_var(dados->ttl, p, 5)) return 0;
#else
	p[strlen(p)-1] = '\0'; // remove ","
	strcpy(dados->ttl, p); // armazena ttl
#endif

	_ = strtok(NULL, " ");

	p = strtok(NULL, " "); // ip_id
	if (!p) return 0;
	if (strlen(p) >= 11) return 0;
#if 1
    if (!get_number_var(dados->val_ip_id, p, 5)) return 0;
#else
	p[strlen(p)-1] = '\0'; // remove ","
	strcpy(dados->val_ip_id, p); // armazena ip_id
#endif
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	
	p = strtok(NULL, " "); // protocolo
	if (!p) return 0;
	p++; // remove "("
	if (strlen(p) >= 6) return 0;
	p[strlen(p)-2] = '\0'; // remove "),"
	strcpy(dados->proto, p); // armazena protocolo

	return 1;
}

int get_ips_and_port(char *start, DataHandler *dados, char **lat_lon_id, char **proto_ports, int n) {
	
	char *token = strtok(start, ":");

	if (!(token[0] >= '0' && token[0] <= '9')) return 0;
	char *p;

	char ipSrc[32];
	
	p = strtok(token, " > ");
	if (strlen(p) >= 32) fatal("Src ip maior: %s\n", p);
	strcpy(ipSrc, p);

	char ipDst[32];
	p = strtok(NULL, " > ");
	if (strlen(p) >= 32) fatal("Dst ip maior: %s\n", p);
	strcpy(ipDst, p);

	// pega o ip de origem
	token = strtok(ipSrc, ".");
    if (!token) return 0;
	dados->ip_src_arr[0] = atoi(token);
	for (int i = 1; i < 4; i++) {
		token = strtok(NULL, ".");
        if (!token) return 0;
		dados->ip_src_arr[i] = atoi(token);
	}

	rede r = site_from_ip_addr(dados->ip_src_arr);
	if (r.id == NULL) {
		printf("Error: Invalid Ip %d.%d.%d.%d\n", dados->ip_src_arr[0],dados->ip_src_arr[1],dados->ip_src_arr[2],dados->ip_src_arr[3]);
		lat_lon_id[0] = NULL;
		return 0;
	}

	lat_lon_id[0] = r.lat;
	lat_lon_id[1] = r.lon;
	lat_lon_id[2] = r.id;

	// pega o ip de destino
	token = strtok(ipDst, ".");
    if (!token) return 0;
	dados->ip_dst_arr[0] = atoi(token);
	for (int i = 1; i < 6; i++) {
        if (!token) return 0;
		if (i < 5) {
			dados->ip_dst_arr[i] = atoi(token);
		}
		else if(token) {
			//port_dst = token;
			strcpy(dados->port_dst, token);
		}
		token = strtok(NULL, ".");
	}
    
	if (dados->port_dst && proto_ports) {
		// concentra portas sem interesse na porta "0"
		char proto_port[15];
		sprintf(proto_port, "%s:%s", dados->proto, dados->port_dst);
		// {"", "", "", NULL}
		char **p;
		for (p = proto_ports; n; p++, n--) {
			if (strcmp(proto_port, *p) == 0) break;
		}
		if (!n) {
			strcpy(dados->port_dst, "0");
		}
	}
	else {
		strcpy(dados->port_dst, "0");
	}
    
    return 1;
}

int must_flush(DataHandler *dados, TimeHandler *t) {

	int min = atoi(dados->min);

	// verifica se estourou o tempo de buferizacao
	int min_capture = min - (min % CAPTURE_TIME);
	
	// houve mudança na chave?
	int res = (strcmp(dados->hora, t->lst_hora) != 0 || strcmp(dados->data, t->lst_data) != 0 || t->lst_min_capture != min_capture);

    strcpy(t->lst_data, dados->data);
    strcpy(t->lst_hora, dados->hora);
    t->lst_min_capture = min_capture;
    return res;
}

void do_flush(DataHandler *dados, TimeHandler* t, DictInfo *d) {
    // salva o que foi armazenado
    if (global_fout != NULL) {
        List *pl = d->content;
        for (int i = 0; i < DICT_SIZE; i++, pl++) {
            list_flush(pl, global_fout);
        }
        fclose(global_fout);
        system("sh rsync_to_server.sh");
    }
    // monta o nome do novo arquivo
    char fname[40];
    sprintf(fname, "data/%s_%s_%s.csv", dados->data, dados->hora, dados->min);
    printf("%s\n", fname);

    strcpy(t->lst_data, dados->data);
    strcpy(t->lst_hora, dados->hora);
    global_fout = fopen(fname, "a");    
}

void forge_key(char *key, DataHandler *dados, char *lat_lon_id[]) {
	// key = 'yyyy-mm-dd hh:nn:00; lat; lon; 1; id_cliente; val_ttl; proto; port_dst; 0
	char temp_date[128];
	temp_date[4] = '-';
	temp_date[7] = '-';
	temp_date[10] = '\0';
	for (int i = 0; i < strlen(dados->data); i++){
		if (i < 4) { // ano
			temp_date[i] = dados->data[i];
		}
		else if (i < 6) { // mes
			temp_date[i+1] = dados->data[i];
		}
		else { // dia
			temp_date[i+2] = dados->data[i];
		}
	}
	sprintf(key, "%s %s:%s:00;%s;%s;1;%s;%s;%s;%s;0", temp_date, dados->hora, dados->min, lat_lon_id[0], lat_lon_id[1], lat_lon_id[2], dados->ttl, dados->proto, dados->port_dst);
}

void check_key(DataHandler *dados, TimeHandler *t, DictInfo *d, char *prev_key, char *lat_lon_id[]) {

	// pega o minuto da hora

	//char aux[5] = {0};
	//strncpy(aux, dados->hora+3,2);
	//int min = atoi(aux);
	int min = atoi(dados->min);
	// verifica se estourou o tempo de buferizacao
	int min_capture = min - (min % CAPTURE_TIME);
	
	// houve mudança na chave?
	if(strcmp(dados->hora, t->lst_hora) != 0 || strcmp(dados->data, t->lst_data) != 0 || t->lst_min_capture != min_capture) {
		// salva o que foi armazenado
		if (global_fout != NULL) {
			List *pl = d->content;
			for (int i = 0; i < DICT_SIZE; i++, pl++) {
				list_flush(pl, global_fout);
			}
			fclose(global_fout);
			system("sh rsync_to_server.sh");
		}
		// monta o nome do novo arquivo
		char fname[40];
		sprintf(fname, "data/%s_%s_%s.csv", dados->data, dados->hora, dados->min);
		printf("%s\n", fname);

		strcpy(t->lst_data, dados->data);
		strcpy(t->lst_hora, dados->hora);
		t->lst_min_capture = min_capture;
		global_fout = fopen(fname, "a");
	}
    
#if 1
    forge_key(prev_key, dados, lat_lon_id);
#else
	// key = 'yyyy-mm-dd hh:nn:00; lat; lon; 1; id_cliente; val_ttl; proto; port_dst; 0
	char temp_date[20];
	temp_date[4] = '-';
	temp_date[7] = '-';
	temp_date[10] = '\0';
	for (int i = 0; i < strlen(dados->data); i++){
		if (i < 4) { // ano
			temp_date[i] = dados->data[i];
		}
		else if (i < 6) { // mes
			temp_date[i+1] = dados->data[i];
		}
		else { // dia
			temp_date[i+2] = dados->data[i];
		}
	}
	sprintf(prev_key, "%s %s:%s:00;%s;%s;1;%s;%s;%s;%s;0", temp_date, dados->hora, dados->min, lat_lon_id[0], lat_lon_id[1], lat_lon_id[2], dados->ttl, dados->proto, dados->port_dst);
#endif
}


void http_user_agent_line(char *start, char *prev_key){
	start[10] = '\0';
	char *aux = start;
	if (strcmp(aux, "User-Agent") == 0) {
		char *user_agent = start + 12;

		int len_user_agent = strlen(user_agent);
		user_agent[len_user_agent-1] = '\0'; // remove o "0"

		char str_len_user_agent[4];
		sprintf(str_len_user_agent, "%d", len_user_agent);

		prev_key[strlen(prev_key)-1] = '\0';
		// key = 'yyyy-mm-dd hh:nn:00; lat; lon; 1; id_cliente; val_ttl; proto; port_dst; id_user_agent
		strcat(prev_key, str_len_user_agent);
	}
}

int main(int argc, char *argv[]) { 
    int64_t counter = 0;
    
	if (argc > 1) {
		int aux = strtol(argv[1], NULL, 10); // string to int
		if (60 % aux == 0) CAPTURE_TIME = aux;
		printf("Capture Time: %d\n", CAPTURE_TIME);
	}
    

    system("sh preserve_log.sh");
    
    FILE *flog = fopen("filter.log","wt");
    if (!flog) return 1;

	system("sh rsync_to_server.sh");

	load_ips("pop_df_lat_lon.txt");

	mkdir("data", 0777); // cria diretorio

	char line[MAX_LINE_SZ+4];
	char prev_key[MAX_KEY+1] = "";

	//char *proto_ports[] = {"17:53", "6:53", "6:80"};
	int n = 3; // quantidade de proto:port de interesse

	char *lat_lon_id[3];

	DictInfo *d = createDict();
	freelist = createList();

	DataHandler *dados = (DataHandler*)malloc(sizeof(DataHandler));
	TimeHandler *time = (TimeHandler*)malloc(sizeof(TimeHandler));
	// valores iniciais
	strcpy(time->lst_data, "");
	strcpy(time->lst_hora, "");
	time->lst_min_capture = -1;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	
	while (fgets(line, MAX_LINE_SZ, stdin)) {
		if (global_stop) break;
        
        fprintf(flog, "%s[-- %lX --]\n", line, counter); fflush(flog); counter++;

		char *start = line;
		int inner = 0;
		while (*start == ' ' || *start == '\t') {
			if (*start == '\t') { inner += 8; }
			else { inner++; }
			start++;
		}
		if (!start[0] || start[0]=='\n') { continue; }
		inner /= BLANKS_PER_INDENT;
		
		// primeiro nao branco
		if (inner == 0) {
            
            if (prev_key) {
                fseek(flog, 0L, SEEK_SET);
                fflush(flog);
            }
			insert_or_increment_key(d, prev_key);
			prev_key[0] = '\0';

			if (!header_line(dados, start)) continue;
            
		}
		else if (prev_key) {
			if (inner == 1) {
				//get_ips_and_port(start, dados, lat_lon_id, proto_ports, n);
				if (get_ips_and_port(start, dados, lat_lon_id, NULL, n) && lat_lon_id[0]) {
#if 0
					check_key(dados, time, d, prev_key, lat_lon_id);
#else
                    if (must_flush(dados,time)) {
                        do_flush(dados, time, d);
                    }
                    forge_key(prev_key, dados, lat_lon_id);
#endif                    
				} else {
                    prev_key[0] = '\0';
                }
			}
			// procurando User-Agent no corpo de requisicoes HTTP
			else {
				//http_user_agent_line(start, prev_key);
			}
		}
	}

    fclose(flog);
    system("rm filter.log");
    
	// existe algo a ser contabilizado do registro anterior?
	insert_or_increment_key(d, prev_key);
	prev_key[0] = '\0';
    
	// salva o residuo
    if (global_fout != NULL) {
        List *pl = d->content;
		for (int i = 0; i < DICT_SIZE; i++, pl++) {
			list_flush(pl, global_fout);
		}
		fclose(global_fout);
	}
    
	free(time);
	free(dados);
	freeList(freelist);
	free(d);
	free_cidrs();
    return 0;
}
