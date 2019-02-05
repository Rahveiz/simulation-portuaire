# include "port.h"

/* Fonction d'affichage des erreurs */
void error (int syserr, const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    fprintf (stderr, "%s: ", prog);
    vfprintf (stderr, fmt, ap);
    fprintf (stderr, "\n");
    va_end (ap);
    if (syserr)
	perror ("");
    exit (1);
}

/* Recupere la variable d'environnement DEBUG_PORT */
int get_debug_level() {
    char *debug = getenv("DEBUG_PORT");
    return debug ? atoi(debug) : 0;
}

/* Affiche un message ou non selon la variable de debug */
void print_debug(int threshold, const char *fmt, ...)
{
    if (dflag < threshold) { return; } 
    va_list ap;
    va_start (ap, fmt);
    vfprintf (stdout, fmt, ap);
    fflush(stdout);
    va_end (ap) ;
}

/* Fonctions pour les semaphores
 * (merci Dijkstra)
 */
void P (int id, int num) {
    struct sembuf s [1] = { {num, -1, 0} } ;
    if (semop (id, s, 1) == -1) { error(1, "semop (P) -> errno = %d", errno); }
}

void V (int id, int num) {
    struct sembuf s [1] = { {num, 1, 0} } ;
    if (semop (id, s, 1) == -1) { error(1, "semop (V) -> errno = %d", errno); }
}

// Incrementation par bloc
void V_bulk (int id, int num, int nb) {
    struct sembuf s [1] = { {num, nb, 0} } ;
    if (semop (id, s, 1) == -1) { error(1, "semop (V)"); }
}

/* Gestion de la shm */
void *attacher_shm(int shmid) {
    char *addr = NULL;
    addr = shmat(shmid, NULL, 0); 
    if (addr == NULL) { error(1, "shmat"); }
    return addr;
}

void detacher_shm(void *addr) {
    if (shmdt(addr) == -1){ error(1, "shmdt"); }
}

/* Gestion des objets IPC System V */
int get_semid(char m) {
    key_t k;
    int id;
    k = ftok("/usr/include/semaphore.h", m);
    if (k == -1) { error(1, "ftok"); }
    if((id = semget(k, 0, 0)) == -1) { error(1, "semget"); }
    return id;
}

int get_shmid(char m) {
    key_t k;
    int id;
    k = ftok("/usr/include/semaphore.h", m);
    if (k == -1) { error(1, "ftok"); }
    if((id = shmget(k, 0, 0)) == -1) { error(1, "shmget"); }
    return id;
}

void supprimer_shm(char m) {
    int r;
    key_t k;
    int id;
    k = ftok("/usr/include/semaphore.h", m);
    if (k == -1) { error(1, "ftok"); }
    if((id = shmget(k, 0, 0)) != -1) { 
        r = shmctl(id, IPC_RMID, NULL);
        if (r == -1) { error(1, "shmctl (errno %d)", errno); }
    }
}

void supprimer_semaphore(char m) {
    int r;
    key_t k;
    int id;
    k = ftok("/usr/include/semaphore.h", m);
    if (k == -1) { error(1, "ftok"); }
    if((id = semget(k, 0, 0)) != -1) { 
        r = semctl(id, 0, IPC_RMID, NULL);
        if (r == -1) { error(1, "semctl (errno %d)", errno); }
    }
}