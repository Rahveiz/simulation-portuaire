# include "port.h"

int main(int argc, const char *argv[]) {

    prog = argv[0];
    dflag = get_debug_level();

    prog = argv[0];
    if (argc != 1) {
        fprintf(stderr, "usage: ./pstop\n");
        exit(EXIT_FAILURE);
    }
    (void) argv;
    int semid = get_semid('S');
    int shmid = get_shmid('M');
    char *addr = attacher_shm(shmid);
    P(semid, 2);
    memset(addr, 0, BUFSIZE);
    strcpy(addr, "stop");
    print_debug(1, "Envoi du stop à la capitainerie\n");
    V(semid, 1);
    detacher_shm(addr);
    print_debug(1, "Effectué\n");
    return 0;
}
