# include "port.h"

int main(int argc, char const *argv[]) {
    
    prog = argv[0];
    dflag = get_debug_level();

    if (argc != 5) {
        fprintf(stderr, "usage: ./pnav <nom> <nb_cont> <tps_acc> <tps_dech>\n");
        exit(EXIT_FAILURE);
    }
    char nom_navire = argv[1][0];
    int nb_cont = atoi(argv[2]);
    int ta = atoi(argv[3]);
    int td = atoi(argv[4]);
    (void) nom_navire;
    int n_quai;
    if (nb_cont <= 0 || ta < 0 || td < 0) {
        fprintf(stderr, "usage: ./pnav <nom> <nb_cont> <tps_acc> <tps_dech>\n");
        exit(EXIT_FAILURE);
    }
    int semid = get_semid('S');
    int semid_q = get_semid('Q');
    int semid_c = get_semid('C');
    //int shmid = get_shmid('M');
    int shmid_d = get_shmid('D');
    int shmid_p = get_shmid('P');
    struct S_NAV *port_stats = attacher_shm(shmid_d);
    struct S_PORT *port = attacher_shm(shmid_p);
    char *addr = port->buf;
    // demande d'accostage a la capitainerie
    P(semid, 2);
    sprintf(addr, "a%s", argv[1]);
    V(semid, 1);
    // recuperation du numero de quai
    P(semid, 3);
    print_debug(2, "Reception du quai\n");
    n_quai = atoi(addr);
    V(semid, 2);
    if (n_quai < 0) { // aucune place disponible
        while (n_quai < 0) {
            if (n_quai == -2) {
                error(0, "La capitainerie va fermer, impossible d'accoster\n");
            }
            print_debug(1, "Aucun quai disponible, attente ...\n");
            P(semid, 0); // le navire attend qu'une place se libere
            P(semid, 2); // on redemande
            (port->waiting)--;
            sprintf(addr, "a%s", argv[1]);
            V(semid, 1);
            // recuperation du numero de quai
            P(semid, 3);
            n_quai = atoi(addr);
            print_debug(2, "Reception du quai\n");
            V(semid, 2);
        }
    } else {
        P(semid, 0);
    }
    print_debug(1, "Amarrage au quai %d\n", n_quai);
    if (usleep (ta * 1000) == -1) { error(1, "usleep"); }

    // remplissage du segment shm avec les infos du navire
    port_stats[n_quai].conteneurs_default = nb_cont;
    port_stats[n_quai].conteneurs_current = nb_cont;
    port_stats[n_quai].nom = argv[1][0];
    print_debug(1, "Amarrage terminé\n");
    // attente de dechargement
    while(port_stats[n_quai].conteneurs_current > 0) {
        // attente de demande de dechargement de la part des camions
        P(semid_q, n_quai);
        // attente si le quai n'est pas libre (conteneur non charge)
        if (usleep(td * 1000) == -1) { error(1, "usleep"); }
        print_debug(2, "Conteneur déchargé\n");
        // signalement au camion que le conteneur est decharge
        V(semid_c, n_quai);
        (port_stats[n_quai].conteneurs_current)--;
    }
    
    // annonce du depart du navire
    P(semid, 2);
    sprintf(addr, "d%d", n_quai);
    V(semid, 1);
    print_debug(1, "Le navire quitte le port\n");
    //detacher_shm(addr);
    detacher_shm(port_stats);
    detacher_shm(port);
    return 0;
}
