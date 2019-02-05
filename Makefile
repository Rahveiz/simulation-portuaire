CFLAGS = -g -Wall -Wextra -Werror

PROGS = pcap pstop pnav pfcam pdump pclean

all: $(PROGS)

$(PROGS): port.o

port.o: port.h

clean:
	rm -f *.o $(PROGS)
	rm -rf *.dSYM