# include "port.h"

int main(int argc, const char *argv[]) {
    prog = argv[0];
    dflag = get_debug_level();

    if (argc != 1) {
        fprintf(stderr, "usage: ./pclean\n");
        exit(EXIT_FAILURE);
    }
    print_debug(1, "Suppression des objets IPC System V\n");
    supprimer_shm('M');
    supprimer_shm('D');
    supprimer_shm('P');
    supprimer_semaphore('S');
    supprimer_semaphore('C');
    supprimer_semaphore('F');
    supprimer_semaphore('Q');
    return 0;
}
