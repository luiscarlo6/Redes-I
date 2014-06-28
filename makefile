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


all : clean $(PROG1)

$(PROG1): $(OBJS1)
		$(CC) $(CFLAGS) $(OBJS1) -o $(PROG1)

%.o: %.c  
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(RMFLAGS) $(PROG1) $(OBJS1) *.txt *.o
