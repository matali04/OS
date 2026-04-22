#ifndef GESCOM_H
#define GESCOM_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <readline/history.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PORT 9998
#define LBUF 512

extern int trace;

/* le chainage des couples (pseudo, IPV4) */
#define LPSEUDO 23          /* longueur maxi du pseudo */
struct elt {                /* stucture d'un element */
    char nom[LPSEUDO+1];    /* nom de l'element */
    char adip[16];          /* IPv4 de l'element */
    struct elt* next;       /* adresse du prochain element */
};

extern int stop;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Sortie(int argc, char *argv[]);

int pwd(int argc, char *argv[]);

int cd(int argc, char *argv[]);

int vers(int argc, char *argv[]);

int history(int argc, char *argv[]);

void* serveur_udp(void* p);

int beuip(int argc, char *argv[]);

void commande(char octet1, char* message, char* pseudo);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* addrip(unsigned long A);

void fill_sock(int sin_family, in_addr_t s_addr, uint16_t sin_port);

void interrupt(int S);

void response_deco();

void response0();

void response12(char* pseudo);

void response3();

void response4(char* pseudo, char* message);

void response5(char* message);

void manage_response(char* buf, char* pseudo, int msg_length);

void ajouteElt(char* pseudo, char* adip);

void supprimeElt(char* adip);

void listeElts(void);

void serveur(char* pseudo);

void* service_tcp(void* p);

void* serveur_tcp(void * rep);

void demandeListe(char* pseudo);

void envoiContenu(int fd);

void demandeFichier(char* pseudo, char* nomfic);



















#endif