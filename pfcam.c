# include "port.h"

int main(int argc, char const *argv[]) {
    
    prog = argv[0];
    dflag = get_debug_level();

    if (argc != 4) {
        fprintf(stderr, "usage: ./pfcam <nom_navire> <nb_camions> <temps>\n");
        exit(EXIT_FAILURE);
    }
    
    int nc = atoi(argv[2]);
    int tc = atoi(argv[3]);
    if (nc <= 0 || tc < 0) {
        fprintf(stderr, "usage: ./pfcam <nom_navire> <nb_camions> <temps>\n");
        exit(EXIT_FAILURE);
    }
    int n_quai;
    int i;
    int semid = get_semid('S');
    int semid_f = get_semid('F');
    int semid_c = get_semid('C');
    int semid_q = get_semid('Q');
    int shmid_p = get_shmid('P');
    int shmid_d = get_shmid('D');
    struct S_PORT *port = attacher_shm(shmid_p);
    char *addr = port->buf;
    struct S_NAV *quai = attacher_shm(shmid_d);
    
    // demande le quai correspondant a la capitainerie
    P(semid, 2);
    sprintf(addr, "c%s", argv[1]);
    print_debug(2, "Demande de redirection au quai\n");
    V(semid, 1);
    // recupere le numero de quai
    P(semid, 3);
    n_quai = atoi(addr);
    V(semid, 2);
    if (n_quai == -1) {
        print_debug(1, "Aucun navire correspondant n'est à quai\n");
        exit(1);
    }
    print_debug(1, "-> Quai %d\n", n_quai);
    
    // protection pour l'acces concurrent au sein des flottes
    P(semid_f, n_quai);
    // chargement (1 camion a la fois)
    V(semid_q, n_quai); // demande de decharger un premier conteneur
    for (i = 0; i < nc; i++) {
        // attente qu'un conteneur soit disponible
        P(semid_c, n_quai);
        // demande de decharger le conteneur suivant
        if (i != nc - 1) { // sauf si c'est le dernier
            V(semid_q, n_quai);
        }
        // chargement
        if (usleep(tc*1000) == -1) { error(1, "usleep"); }        
        print_debug(2, "Un camion quiite le port\n");
    }
    // liberation de l'acces pour une autre flotte
    V(semid_f, n_quai);

    detacher_shm(port);
    detacher_shm(quai);
    return 0;
}
