all : biceps

biceps.o : biceps.c gescom.h
gescom.o : gescom.c gescom.h


%.o : %.c
	cc -g -c $< -o $@ -Wall -Werror



biceps: biceps.o gescom.o
	cc -o biceps biceps.o gescom.o -Wall -Werror -lreadline -pthread -g

biceps-valgrind: biceps
	valgrind --leak-check=full --track-origins=yes -s ./biceps


cln :
	rm -f *.o biceps
