# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <sys/sem.h>
# include <sys/shm.h>
# include <unistd.h>
# include <sys/types.h>
# include <errno.h>
# include <string.h>
# define BUFSIZE 256

void error (int syserr, const char *fmt, ...);
int get_debug_level();
void print_debug(int threshold, const char *fmt, ...);
void P (int id, int num);
void V (int id, int num);
void V_bulk (int id, int num, int nb);
void *attacher_shm(int shmid);
void detacher_shm(void *addr);
int get_semid(char m);
int get_shmid(char m);
void supprimer_shm(char m);
void supprimer_semaphore(char m);

const char *prog;
int dflag;

struct S_PORT {
    int open;
    int n_quais;
    int used_slots;
    int waiting;
    char buf[BUFSIZE];
};
struct S_NAV {
    char nom;
    int conteneurs_default;
    int conteneurs_current;
};
