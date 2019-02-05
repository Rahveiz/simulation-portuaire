# include "port.h"

int main(int argc, const char *argv[]) {
    prog = argv[0];
    dflag = get_debug_level();
    if (argc != 1) {
        fprintf(stderr, "usage: ./pdump\n");
        exit(EXIT_FAILURE);
    }
    int shmid_d = get_shmid('D');
    int shmid_p = get_shmid('P');
    struct S_NAV *port_stats = attacher_shm(shmid_d);
    struct S_PORT *port = attacher_shm(shmid_p);
    int i;

    /* Etape 2 : affichage des infos globales du systeme */
    print_debug(0, "===-- Le port SALUX --===\n");
    print_debug(0, "Etat actuel : %s\n", 
        port->open ? "ouvert" : "va bientôt fermer");
    print_debug(0, "Quais  : %d / %d\n", port->used_slots, port->n_quais);
    print_debug(0, "Navires en attente : %d\n", port->waiting);

    /* ~ What shall we do with a drunken sailor ? */
    printf("                            _.--.\n");
    printf("                        _.-'_:-'||\n");
    printf("                    _.-'_.-::::'||\n");
    printf("               _.-:'_.-::::::'  ||\n");
    printf("             .'`-.-:::::::'     ||\n");
    printf("            /.'`;|:::::::'      ||_\n");
    printf("           ||   ||::::::'     _.;._'-._\n");
    printf("           ||   ||:::::'  _.-!oo @.!-._'-.\n");
    printf("           \\'.  ||:::::.-!()oo @!()@.-'_.|\n");
    printf("            '.'-;|:.-'.&$@.& ()$%%-'o.'\\U||\n");
    printf("              `>'-.!@%%()@'@_%%-'_.-o _.|'||\n");
    printf("               ||-._'-.@.-'_.-' _.-o  |'||\n");
    printf("               ||=[ '-._.-\\U/.-'    o |'||\n");
    printf("               || '-.]=|| |'|      o  |'||\n");
    printf("               ||      || |'|        _| ';\n");
    printf("               ||      || |'|    _.-'_.-'\n");
    printf("               |'-._   || |'|_.-'_.-'\n");
    printf("                '-._'-.|| |' `_.-'\n");
    printf("                    '-.||_/.-'\n");

    /* Etape 3 : affichage des infos specifiques a chaque quai */
    for (i = 0; i < port->n_quais; i++) {
        print_debug(0, "=========== QUAI N° %d ===========\n", i);
        if (!port_stats[i].nom) {
            print_debug(0, "Aucun navire à quai\n");
            print_debug(0, "\n\n");
            continue;
        }
        print_debug(0, "Navire à quai : %c\n", port_stats[i].nom);
        print_debug(0, "Conteneurs restants : %d / %d\n",
            port_stats[i].conteneurs_current, port_stats[i].conteneurs_default);
        print_debug(0, "\n\n");
    }
    detacher_shm(port_stats);
    detacher_shm(port);
    return 0;
}
