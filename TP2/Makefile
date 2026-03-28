all : biceps servbeuip

biceps.o : biceps.c gescom.h
gescom.o : gescom.c gescom.h creme.h
creme.o : creme.c creme.h


%.o : %.c
	cc -g -c $< -o $@ -Wall -Werror


servbeuip : servbeuip.c
	cc -Wall -Werror -o servbeuip servbeuip.c



biceps: biceps.o gescom.o creme.o
	cc -o biceps biceps.o gescom.o creme.o -Wall -Werror -lreadline -g

biceps-valgrind: biceps
	valgrind --leak-check=full --track-origins=yes -s ./biceps


cln :
	rm -f *.o biceps servbeuip
