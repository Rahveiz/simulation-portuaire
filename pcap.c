# include "port.h"

int creer_shm(char m, int size) {
    key_t k ; 
    int id = 0;
    int r;
    k = ftok("/usr/include/semaphore.h", m);
    if (k == -1) { error(1, "ftok"); }
    // suppression de l'ancienne mémoire
    if((id = shmget(k, 0, 0)) != -1) { 
        r = shmctl(id, IPC_RMID, NULL);
        if (r == -1) { error(1, "shmctl"); }
    }
    id = shmget(k, size, IPC_CREAT | 0666);
    if (id == -1) { error(1, "shmget"); }
    return id;
}

int creer_semaphore(int n, char m) {
    key_t k ; 
    int id = 0;
    int r;
    k = ftok("/usr/include/semaphore.h", m);
    if (k == -1) { error(1, "ftok"); }
    // suppression de l'ancien ensemble
    if((id = semget(k, 0, 0)) != -1) { 
        r = semctl(id, 0, IPC_RMID, NULL);
        if (r == -1) { error(1, "semget"); }
    }

    id = semget(k, n, IPC_CREAT | 0666);
    if (id == -1) { error(1, "semget"); }
    return id;
}

void set_sem_val(int semid, int k, int val) {
    if (semctl (semid, k, SETVAL, val) == -1) { error(1, "semctl"); }
}
/*
int quai_libre(struct S_NAV *quais, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (!quais[i].nom) {
            return i;
        }
    }
    return -1;
}
*/
int get_quai(struct S_NAV *quais, int n, char c) {
    int i;
    for (i = 0; i < n; i++) {
        if (quais[i].nom == c) {
            return i;
        }
    }
    return -1;
} 

int main(int argc, char const *argv[]){

    prog = argv[0];
    dflag = get_debug_level();

    if (argc != 2) {
        fprintf(stderr, "usage: ./pcap <nb_quais>\n");
        exit(EXIT_FAILURE);
    }

    int nb_quais = atoi(argv[1]);
    if (nb_quais <= 0) {
        fprintf(stderr, "usage: ./pcap <nb_quais>\n");
        exit(EXIT_FAILURE);
    }
    int i;
    int free_slot;
    int quai_slot;
    int tmp;
    /* Initialisation du groupe de semaphores pour la gestion des quais
     * 0 : quais (n)
     * 1 : communication (0)
     * 2 : communication (1)
     * 3 : communication (0)
     */
    int semid = creer_semaphore(4, 'S');
    set_sem_val(semid, 0, nb_quais);
    set_sem_val(semid, 1, 0);
    set_sem_val(semid, 2, 1);
    set_sem_val(semid, 3, 0);

    
    /* Initialisation des semaphores necessaires a la synchronisation
     * pour le dechargement des conteneurs :
     * 3 ensembles de n semaphores
     * - F : flotte
     * - C : camion
     * - Q : quai (grue)
     */
    int semid_f = creer_semaphore(nb_quais, 'F');
    int semid_c = creer_semaphore(nb_quais, 'C');
    int semid_q = creer_semaphore(nb_quais, 'Q');
    for (i = 0; i < nb_quais; i++) {
        set_sem_val(semid_f, i, 1);
        set_sem_val(semid_c, i, 0);
        set_sem_val(semid_q, i, 0);
    }

    /* Creation d'un segment partage pour la communication entre la capitainerie
     * et les autres programmes
     */
    //int shmid = creer_shm('M', BUFSIZE);

    /* Creation de segments partages pour stocker les informations sur l'etat
     * actuel du systeme (notamment pour pdump)
     */
    int shmid_d = creer_shm('D', nb_quais * sizeof(struct S_NAV));
    int shmid_p = creer_shm('P', sizeof(struct S_PORT));
    
    /* Note : le buffer aurait pu etre membre de la structure S_PORT pour
     * economiser un objet IPC, mais le fait de separer les 2 permet de bien
     * distinguer les fonctionnalites (un choix de lisibilite donc)
     */
    //char *addr = attacher_shm(shmid);
    struct S_NAV *quais = attacher_shm(shmid_d);
    struct S_PORT *port = attacher_shm(shmid_p);
    for(i = 0; i < nb_quais; i++) {
        quais[i].nom = 0;
    }
    char *addr = port->buf;
    port->open = 1;
    port->n_quais = nb_quais;
    port->used_slots = 0;
    port->waiting = 0;

    print_debug(1, "Port initialisé\n");
    while (port->open || port->used_slots) {
        P(semid, 1);
        switch(addr[0]) {
            case 's': // stop
                port->open = 0;
                /* liberation des navires en attente
                 * qui vont alors refaire une demande d'amarrage
                 * (qui sera refusee car aflag a ete change)
                 */
                if (port->waiting) {
                    V_bulk(semid, 0, port->waiting);
                }
                V(semid, 2);
                print_debug(1, "Le port va bientôt fermer !\n");
                break;
            case 'a': // demande d'accostage
                if (!port->open) {
                    print_debug(1, "Le port va fermer, %c refusé\n", addr[1]);
                    sprintf(addr, "%d\n", -2);
                    V(semid, 3);
                    break;
                }
                free_slot = get_quai(quais, nb_quais, 0); // quai libre
                if (free_slot != -1) { // une place est libre
                    (port->used_slots)++;
                    quais[free_slot].nom = '+'; // slot reserve
                } else {
                    (port->waiting)++;
                }
                print_debug(1, "Navire %c -> quai %d\n", addr[1], free_slot);
                sprintf(addr, "%d", free_slot);
                V(semid, 3);
                break;
            case 'c': // camion
                quai_slot = get_quai(quais, nb_quais, addr[1]);
                sprintf(addr, "%d", quai_slot);
                print_debug(2, "Redirection de flotte de camions -> quai %d\n",
                    quai_slot);
                V(semid, 3);
                break;
            case 'd': // depart d'un navire
                tmp = atoi(addr+1); // numero de quai utilise
                print_debug(2, "Départ du navire %c\n", quais[tmp].nom);
                quais[tmp].nom = 0; // liberation du quai
                (port->used_slots)--;
                V(semid, 0);
                V(semid, 2);
                break;
        }
    }

    print_debug(1, "Fermeture du port ...\n");
    //detacher_shm(addr);
    detacher_shm(quais);
    supprimer_shm('M');
    supprimer_shm('D');
    supprimer_shm('P');
    supprimer_semaphore('S');
    supprimer_semaphore('C');
    supprimer_semaphore('Q');
    supprimer_semaphore('F');
    return 0;
}
