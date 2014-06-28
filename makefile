#Makefile para svr_s
#Archivo usado para compilar el programa
#Autores : 	Luiscarlo Rivera 09-11020
#		Daniel Leones 09-10977

CC = gcc
RMFLAGS = -fv 
#CFLAGS = -g -pthread -pedantic -Wextra -Wall -ggdb -Wunreachable-code 
CFLAGS = -g -pthread -Wall -ggdb
OBJS1 = Operacionesvr_s.o svr_s.o Entrada_s.o
PROG1 = svr_s
PROG2 = svr_c
OBJS2 = svr_c.o Operacionesvr_c.o Entrada_c.o

all : clean $(PROG1) $(PROG2)

$(PROG1): $(OBJS1)
		$(CC) $(CFLAGS) $(OBJS1) -o $(PROG1)

$(PROG2): $(OBJS2)
		$(CC) $(CFLAGS) $(OBJS2) -o $(PROG2)

%.o: %.c  
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(RMFLAGS) $(PROG1) $(OBJS1) $(PROG2) $(OBJS2) *.o
