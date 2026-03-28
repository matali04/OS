#include "creme.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

char buf[LBUF+1];
int trace=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* addrip(unsigned long A){
    static char b[16];
    sprintf(b,"%u.%u.%u.%u",(unsigned int)(A>>24&0xFF),(unsigned int)(A>>16&0xFF),(unsigned int)(A>>8&0xFF),(unsigned int)(A&0xFF));
    return b;
}

int send_serveur(int argc, char*argv[]){
    int sid;
    struct sockaddr_in Sock;

    /* creation du socket */
    if((sid=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0){
        perror("socket");
        return(0);
    }

    /* initialisation de SockConf pour le bind */
    bzero(&Sock, sizeof(Sock));
    Sock.sin_family = AF_INET;
    Sock.sin_port = htons(PORT);
    Sock.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(strcmp(argv[1], "3")==0){ // Affichage des couples dans la table
        
        sprintf(buf, "3BEUIP");
        sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
        return 1;

    } else if (strcmp(argv[1], "4")==0 && argc == 4){ // Envoyer un message privé

        char* pseudo = argv[2];
        char* message = argv[3];
        int len_pseudo = strlen(pseudo);
        int len_message = strlen(message);

        sprintf(buf, "4BEUIP%s", argv[4]);

        int debut_message = 6 + len_pseudo + 1;
        strcpy(buf + debut_message, message);
        int taille_totale = debut_message + len_message;

        sendto(sid, buf, taille_totale, 0, (struct sockaddr *)&Sock, sizeof(Sock));
        return 1;

    } else if (strcmp(argv[1], "5")==0 && argc == 3){

        sprintf(buf, "5BEUIP%s", argv[2]);
        sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
        return 1;

    } else {
        fprintf(stderr,"Utilisation :\n");
        fprintf(stderr,"-   Démarrer le serveur : %s strat serveur_name\n", argv[0]);
        fprintf(stderr,"-   Éteindre le serveur : %s stop\n", argv[0]);
        fprintf(stderr,"-   Affichage des couples dans la table : %s 3\n", argv[0]);
        fprintf(stderr,"-   Envoyer un message privé : %s 4 pseudo message\n", argv[0]);
        fprintf(stderr,"-   Envoyer un message public : %s 5 message\n", argv[0]);
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////