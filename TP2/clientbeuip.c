/*****
* Exemple de serveur UDP
* socket en mode non connecte
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 9998
#define LBUF 512

char buf[LBUF+1];

char* addrip(unsigned long A){
    static char b[16];
    sprintf(b,"%u.%u.%u.%u",(unsigned int)(A>>24&0xFF),(unsigned int)(A>>16&0xFF),(unsigned int)(A>>8&0xFF),(unsigned int)(A&0xFF));
    return b;
}

int main(int argc, char*argv[]){
    int sid;
    struct sockaddr_in Sock;


    if(argc < 3){
        fprintf(stderr,"Utilisation :\n");
        fprintf(stderr,"-   Affichage des couples dans la table : %s 127.0.0.1 3\n", argv[0]);
        fprintf(stderr,"-   Envoyer un message privé : %s 127.0.0.1 4 pseudo message\n", argv[0]);
        fprintf(stderr,"-   Envoyer un message public : %s 127.0.0.1 5 message\n", argv[0]);
        return(1);
    }

    /* creation du socket */
    if((sid=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0){
        perror("socket");
        return(2);
    }

    /* initialisation de SockConf pour le bind */
    bzero(&Sock, sizeof(Sock));
    Sock.sin_family = AF_INET;
    Sock.sin_port = htons(PORT);
    Sock.sin_addr.s_addr = inet_addr(argv[1]);

    if(strcmp(argv[2], "3")==0){ // Affichage des couples dans la table
        
        sprintf(buf, "3BEUIP");
        sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));

    } else if (strcmp(argv[2], "4")==0 && argc == 5){ // Envoyer un message privé

        char* pseudo = argv[3];
        char* message = argv[4];
        int len_pseudo = strlen(pseudo);
        int len_message = strlen(message);

        sprintf(buf, "4BEUIP%s", argv[3]);

        int debut_message = 6 + len_pseudo + 1;
        strcpy(buf + debut_message, message);
        int taille_totale = debut_message + len_message;

        sendto(sid, buf, taille_totale, 0, (struct sockaddr *)&Sock, sizeof(Sock));
    } else if (strcmp(argv[2], "5")==0 && argc == 4){
        sprintf(buf, "5BEUIP%s", argv[3]);
        sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
    }

    return 0;
}

