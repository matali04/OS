#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>



#define PORT 9998
#define LBUF 512

typedef struct {
    char ip[16];
    char pseudo[50];
} Client;

char buf[LBUF+1];
struct sockaddr_in Sock; /* pour les operation du serveur : mise a zero */
Client table_clients[255];
int sid, idx=0;




char* addrip(unsigned long A){
  static char b[16];
  sprintf(b,"%u.%u.%u.%u",(unsigned int)(A>>24&0xFF),(unsigned int)(A>>16&0xFF),
         (unsigned int)(A>>8&0xFF),(unsigned int)(A&0xFF));
  return b;
}

void fill_sock(int sin_family, in_addr_t s_addr, uint16_t sin_port){
    bzero(&Sock, sizeof(Sock));
    Sock.sin_family = sin_family;
    Sock.sin_addr.s_addr = s_addr;
    Sock.sin_port = sin_port;
}

void interrupt(int S){
    switch(S) {
        case SIGINT :

            fill_sock(AF_INET, htons(PORT), inet_addr("192.168.88.255"));

            sprintf(buf, "0BEUIP");
            sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));

            printf("\n\nDéconnexion du serveur\n");
            exit(0);
        default :
            fprintf(stderr,"Signal reçu inconnu : %d\n",S);
    }
}


void add_table(){
    // Ajout à la table
    strcpy(table_clients[idx].ip, addrip(ntohl(Sock.sin_addr.s_addr)));
    strncpy(table_clients[idx].pseudo, buf + 6, 49);
    table_clients[idx].pseudo[49] = '\0';
    printf("Ajout dans la table : %s - %s (%s)\n", addrip(ntohl(Sock.sin_addr.s_addr)), buf + 6, buf);
    idx++;
}

void response0(){
    int client_trouve = 0;

    for(int i = 0; i < idx; i++){
        // On cherche la personne qui vient de se déco
        if(strcmp(table_clients[i].ip, addrip(ntohl(Sock.sin_addr.s_addr))) == 0){
            client_trouve = 1;
            
            // On décale les éléments après du tableau
            for(int j = i; j < idx - 1; j++){
                table_clients[j] = table_clients[j+1];
            }
            
            idx--;
            break;
        }
    }

    if(client_trouve){
        printf("%s a été retiré de la table.\n\n", addrip(ntohl(Sock.sin_addr.s_addr)));
    } else {
        printf("%s non trouvée dans la table.\n\n", addrip(ntohl(Sock.sin_addr.s_addr)));
    }
}

void response12(char* pseudo){
    // On cherche le client pour l'ajouter ou non à la table
    int trouve = 0;
    for(int i=0; i<idx; i++){
        if(strcmp(table_clients[i].pseudo, buf + 6) == 0 || strcmp(table_clients[i].pseudo, "Mr.Hilaire") == 0){
            trouve=1;
            break;
        }
    }
    if(trouve == 0){
        add_table();
    } else {
        printf("Déjà présent dans la table : %s - %s (%s)\n", addrip(ntohl(Sock.sin_addr.s_addr)), buf + 6, buf);
    }

    if (buf[0] == '1'){
        sprintf(buf, "2BEUIP%s", pseudo);
        sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
        printf("Envoie ACK\n");
    }
    printf("\n");
}

void response4(int msg_length){
    int trouve =0;
    char* pseudo_recu = buf + 6;
    int taille_pseudo = strlen(pseudo_recu);
    if(6 + taille_pseudo + 1 < msg_length){ // if (4BEUIP + taille_pseudo + 1 < taille_msg_tt)
        
        char* message_recu = pseudo_recu + taille_pseudo + 1;
        char temp_buf[LBUF+1];

        fill_sock(AF_INET, htons(PORT), 0);
        for(int i = 0; i < idx; i++){
            if(strcmp(table_clients[i].pseudo, pseudo_recu) == 0){
                Sock.sin_addr.s_addr = inet_addr(table_clients[i].ip);
                trouve = 1;
                break;
            }
        }

        if(trouve){
            sprintf(temp_buf, "9BEUIP%s", message_recu);
            sendto(sid, temp_buf, strlen(temp_buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));

            printf("Envoie message privé \n");
            printf("  Pour : %s\n", pseudo_recu);
            printf("  Message : %s\n", message_recu);
        } else {
            printf("Erreur : Pseudo '%s' introuvable dans la table.\n", pseudo_recu);
        }

    } else {
        printf("Erreur : Code 4 recu de '%s' mais le message est vide ou tronque.\n", pseudo_recu);
    }
    printf("\n");
}

void response5(){
    char* pseudo_recu = buf + 6;
    char temp_buf[LBUF+1];

    sprintf(temp_buf, "9BEUIP%s", pseudo_recu);
    fill_sock(AF_INET, htons(PORT), 0);
    for(int i=0; i<idx; i++){
        if(strcmp("Mr.Hilaire", table_clients[i].pseudo) != 0){
            Sock.sin_addr.s_addr = inet_addr(table_clients[i].ip);
            sendto(sid, temp_buf, strlen(temp_buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
            printf("Message envoyé à %s\n", table_clients[i].pseudo);
        }
    }
}

void manage_response(char* buf, char* pseudo, int msg_length){
   
    switch(buf[0]){
        case '0': // Si 0 déconnexion d'un utilisateur
            printf("Message de %s : déconnexion\n\n", addrip(ntohl(Sock.sin_addr.s_addr)));
            response0();
            break;
            
        case '1': // Broadcast
        case '2': // ACK
            if(strcmp("Mr.Hilaire", buf + 6) != 0){
                response12(pseudo);
            }
            break;

        case '3': // Si 3 on affiche les couples dans la table
            printf("TABLE :\n");
            for(int i=0; i<idx; i++){
                printf("    - %s - %s\n", table_clients[i].ip, table_clients[i].pseudo);
            }
            printf("\n");
            break;

        case '4': // Si 4 on envoie un message privé
            response4(msg_length);
            break;

        case '5': // Si 5 envoie à tout le monde
            response5();
            break;

        case '9': // Si 9 on affiche le message privé reçu
            printf("Message de %s : %s\n\n", addrip(ntohl(Sock.sin_addr.s_addr)), buf+6);
            break;

        default:
            printf("Error : code inconnu\n");
            break;
    }

}


void serveur(char* pseudo){
    int n;
    socklen_t ls = sizeof(Sock);

    /* on attend un message */
    bzero(buf, sizeof(buf));
    if((n=recvfrom(sid,(void*)buf,LBUF,0,(struct sockaddr *)&Sock,&ls)) == -1) 
        perror("recvfrom");
    else {
        // on sécurise la fin du buffer
        buf[n] = '\0';

        printf("-------> MESSAGE RECU : %s\n\n", buf);

        if(strncmp(buf + 1, "BEUIP", 5) == 0){ // Message valide

            if(strcmp(addrip(ntohl(Sock.sin_addr.s_addr)), "127.0.0.1") != 0 && (buf[0] == '3' || buf[0] == '4' || buf[0] == '5')){
                printf("ALERTE : TENTATIVE D'USURPATION !!!!! \n\n");
            } else {
                manage_response(buf, pseudo, n);
            }

        } else {
            printf("Error with BEUIP : %s (%s)\n\n", addrip(ntohl(Sock.sin_addr.s_addr)), buf);
        }

    }
}


int main(int argc, char*argv[]){
    signal(SIGINT,interrupt);

    // Vérification de la commande
    if(argc !=2){
        fprintf(stderr,"Utilisation : %s nom_serveur\n", argv[0]);
        return(1);
    }

    // Création du socket
    if((sid=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0){
        perror("socket");
        return(2);
    }

    int broadcastEnable = 1;
    if(setsockopt(sid, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0){
        perror("Erreur setsockopt broadcast");
        return(3);
    }

    // Socket pour le port
    fill_sock(AF_INET, htonl(INADDR_ANY), htons(PORT));
    if(bind(sid, (struct sockaddr *) &Sock, sizeof(Sock)) == -1){
        perror("bind");
        return(4);
    }

    // Socket pour le broadcast de connexion
    fill_sock(AF_INET, inet_addr("192.168.88.255"), htons(PORT));
    sprintf(buf, "1BEUIP%s", argv[1]);
    sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));

    printf("################# Le serveur est attache au port %d #################\n\n\n",PORT);

    while(1){
        serveur(argv[1]);
    }
    return 0;
}
