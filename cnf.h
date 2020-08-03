#ifndef CNF_H__
#define CNF_H__

#define MAX_CNF_KEYSTR 100
#define MAX_CNF_LINE 100

int cnf_init(void);
void cnf_free(void);
int cnf_load(char* cnfpath);
int cnf_save(char* cnfpath);
int cnf_getstr(const char* key, char *str, const char* Default);
int cnf_setstr(const char* key, char *str);
int cnf_debug(int id, char *data);

#endif
