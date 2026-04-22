#include "gescom.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stop=1;
int trace=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

char buf[LBUF+1];
struct sockaddr_in Sock; /* pour les operations du serveur : mise a zero */
struct elt* e_list = NULL;
int sid, idx=0;

// Socket UDP
pthread_t thread_serveur;
int serveur_actif = 0;
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

// Socket TCP
pthread_t thread_serveur_tcp;
int sid_tcp;
char mon_pseudo[LPSEUDO + 1];
char* repertoire_public = "pub";

////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Sortie(int argc, char *argv[]){
    printf("Arrêt du programme...");
    write_history("biceps_history");
    if(serveur_actif == 1){
        response_deco();
    }
    stop=0;
    return 1;
}

int pwd(int argc, char *argv[]){

    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != NULL) {
        printf("%s\n", path);
        return 1;
    } else {
        perror("getcwd() error");
        return 0;
    }
}

int cd(int argc, char *argv[]){

    if(argc==1){
        char* root = getenv("HOME");
        if(root == NULL){
            perror("cd home error");
            return 0;
        } else {
            return 1;
        }
    }

    char path[PATH_MAX] = "";

    for(int i=0; i<argc-1; i++){
        strcat(path, argv[i+1]);
        strcat(path, "/");
    }

    if(chdir(path) == 0){
        return 1;
    } else {
        perror("cd() error");
        return 0;
    }

}

int vers(int argc, char *argv[]){
    printf("biceps 2.0\n");
    return 1;
}

int history(int argc, char *argv[]){
    HIST_ENTRY** tab = history_list();
    if(tab){
        for(int i=0; tab[i]; i++){
            printf("%d : %s\n", i + history_base, tab[i]->line);
        }
    }
    return 1;
}

void* serveur_udp(void* p){
    char* pseudo = (char*) p;
    signal(SIGINT,interrupt);

    // Création du socket
    if((sid=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0){
        perror("socket");
        return NULL;
    }

    int broadcastEnable = 1;
    if(setsockopt(sid, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0){
        perror("Erreur setsockopt broadcast");
        return NULL;
    }

    // Socket pour le port
    fill_sock(AF_INET, htonl(INADDR_ANY), htons(PORT));
    if(bind(sid, (struct sockaddr *) &Sock, sizeof(Sock)) == -1){
        perror("bind");
        return NULL;
    }

    printf("################# Le serveur UDP est attache au port %d #################\n\n\n", PORT);

    sprintf(buf, "1BEUIP%s", pseudo);
    struct ifaddrs* ifaddrs;
    char host[NI_MAXHOST];
    getifaddrs(&ifaddrs);
    struct ifaddrs* ifa = ifaddrs;
    while(ifa!=NULL){
        if(ifa->ifa_addr != NULL){ // L'interface existe
            if(ifa->ifa_addr->sa_family == AF_INET){ // IPv4
                if(ifa->ifa_broadaddr != NULL){ // Broadcast
                    if(getnameinfo(ifa->ifa_broadaddr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0){
                        if(strncmp(host, "127.", 4) != 0){ // On ignore locale
                            // Socket pour le broadcast de connexion
                            fill_sock(AF_INET, inet_addr(host), htons(PORT));
                            sendto(sid, buf, strlen(buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
                        }
                    }
                }
            }
        }
        ifa = ifa->ifa_next;
    }
    freeifaddrs(ifaddrs);

    while(1){
        serveur(pseudo);
    }
    return NULL;
}

int beuip(int argc, char *argv[]){

    if(argc <= 1){
        fprintf(stderr,"Utilisation :\n");
        fprintf(stderr,"-   Démarrer le serveur : %s strat serveur_name\n", argv[0]);
        fprintf(stderr,"-   Éteindre le serveur : %s stop\n", argv[0]);
        fprintf(stderr,"-   Affichage des couples dans la table : %s 3\n", argv[0]);
        fprintf(stderr,"-   Envoyer un message privé : %s 4 pseudo message\n", argv[0]);
        fprintf(stderr,"-   Envoyer un message public : %s 5 message\n", argv[0]);
        return 0;
    }

    if(strcmp(argv[1], "start") == 0){

        if(argc >= 3){
            strcpy(mon_pseudo, argv[2]);
        } else {
            strcpy(mon_pseudo, "Anonyme");
        }

        pthread_create(&thread_serveur, NULL, serveur_udp, (void*)mon_pseudo);
        pthread_create(&thread_serveur_tcp, NULL, serveur_tcp, (void*)repertoire_public);
        serveur_actif = 1;
        usleep(50000); // Pour l'affichage
        return 1;

    } else if(strcmp(argv[1], "stop") == 0){

        response_deco();
        serveur_actif = 0;
        return 1;

    } else if(strcmp(argv[1], "ls") == 0 && argc >= 3){

        demandeListe(argv[2]);
        return 1;

    } else if(strcmp(argv[1], "get") == 0 && argc >= 4){
        
        demandeFichier(argv[2], argv[3]);
        return 1;

    } else if(strcmp(argv[1], "list") == 0){
        commande('3', NULL, NULL);
        return 1;
    } else if(strcmp(argv[1], "message") == 0 && strcmp(argv[2], "all") != 0 && argc >= 4){
        commande('4', argv[3], argv[2]);
        return 1;
    } else if(strcmp(argv[1], "message") == 0 && strcmp(argv[2], "all") == 0 && argc >= 4){
        commande('5', argv[3], NULL);
        return 1;
    } else {
        fprintf(stderr,"Utilisation :\n");
        fprintf(stderr,"-   Démarrer le serveur : beuip strat serveur_name\n");
        fprintf(stderr,"-   Éteindre le serveur : beuip stop\n");
        fprintf(stderr,"-   Affichage des couples dans la table : beuip 3\n");
        fprintf(stderr,"-   Envoyer un message privé : beuip 4 pseudo message\n");
        fprintf(stderr,"-   Envoyer un message public : beuip 5 message\n");
        return 0;
    }
}

void commande(char octet1, char* message, char* pseudo){

    switch(octet1){

        case '3': // Si 3 on affiche les couples dans la table
            response3();
            break;

        case '4': // Si 4 on envoie un message privé
            response4(pseudo, message);
            break;

        case '5': // Si 5 envoie à tout le monde
            response5(message);
            break;

        default:
            printf("Error : code inconnu\n");
            fprintf(stderr,"Utilisation :\n");
            fprintf(stderr,"-   Démarrer le serveur : beuip strat serveur_name\n");
            fprintf(stderr,"-   Éteindre le serveur : beuip stop\n");
            fprintf(stderr,"-   Affichage des couples dans la table : beuip 3\n");
            fprintf(stderr,"-   Envoyer un message privé : beuip 4 pseudo message\n");
            fprintf(stderr,"-   Envoyer un message public : beuip 5 message\n");
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void response_deco(){
    char temp_buf[LBUF+1] = "0BEUIP";

    struct ifaddrs* ifaddrs;
    char host[NI_MAXHOST];
    getifaddrs(&ifaddrs);
    struct ifaddrs* ifa = ifaddrs;
    while(ifa!=NULL){
        if(ifa->ifa_addr != NULL){ // L'interface existe
            if(ifa->ifa_addr->sa_family == AF_INET){ // IPv4
                if(ifa->ifa_broadaddr != NULL){ // Broadcast
                    if(getnameinfo(ifa->ifa_broadaddr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0){
                        if(strncmp(host, "127.", 4) != 0){ // On ignore locale
                            // Socket pour le broadcast de connexion
                            struct sockaddr_in dest_addr;
                            bzero(&dest_addr, sizeof(dest_addr));
                            dest_addr.sin_family = AF_INET;
                            dest_addr.sin_port = htons(PORT);
                            sendto(sid, temp_buf, strlen(temp_buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
                        }
                    }
                }
            }
        }
        ifa = ifa->ifa_next;
    }
    freeifaddrs(ifaddrs);

    pthread_cancel(thread_serveur);
    pthread_join(thread_serveur, NULL);
    close(sid);

    pthread_cancel(thread_serveur_tcp);
    pthread_join(thread_serveur_tcp, NULL);
    close(sid_tcp);
}

void response0(){
    pthread_mutex_lock(&verrou);
    supprimeElt(addrip(ntohl(Sock.sin_addr.s_addr)));
    pthread_mutex_unlock(&verrou);
}

void response12(char* pseudo){
    char* pseudo_recu = buf + 6;
    char* ip_recue = addrip(ntohl(Sock.sin_addr.s_addr));

    if (strcmp(pseudo_recu, pseudo) == 0) {
        return; // Moi-même
    }

    int existe = 0;
    pthread_mutex_lock(&verrou);
    struct elt* e = e_list;
    while(e != NULL){
        if(strcmp(e->nom, pseudo_recu) == 0 || strcmp(e->adip, ip_recue) == 0){
            existe = 1;
            break;
        }
        e = e->next;
    }

    if(!existe){
        ajouteElt(pseudo_recu, ip_recue);
    } else {
        printf("Utilisateur déjà connu : %s (%s)\n", pseudo_recu, ip_recue);
    }
    pthread_mutex_unlock(&verrou);

    if (buf[0] == '1'){
        char temp_buf[LBUF+1];
        sprintf(temp_buf, "2BEUIP%s", pseudo);
        
        struct sockaddr_in dest_addr;
        bzero(&dest_addr, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        dest_addr.sin_addr.s_addr = inet_addr(ip_recue);
        
        sendto(sid, temp_buf, strlen(temp_buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        printf("Envoie ACK à %s\n", pseudo_recu);;
    }
    printf("\n");
}

void response3(){
    pthread_mutex_lock(&verrou);
    listeElts();
    pthread_mutex_unlock(&verrou);
}

void response4(char* pseudo, char* message){
    char temp_buf[LBUF+1];

    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    pthread_mutex_lock(&verrou);
    if(e_list == NULL){
        printf("Aucun utilisateur n'est connu");
        return;
    }
    struct elt* e = e_list;
    while(e != NULL && strcmp(pseudo, e->nom) != 0){
        e = e->next;
    }

    if(e != NULL){
        sprintf(temp_buf, "9BEUIP%s", message);
        Sock.sin_addr.s_addr = inet_addr(e->adip);
        sendto(sid, temp_buf, strlen(temp_buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));

        printf("Envoie message privé \n");
        printf("  Pour : %s\n", pseudo);
        printf("  Message : %s\n", message);
    } else {
        printf("Erreur : Pseudo '%s' introuvable dans la table.\n", pseudo);
    }
    pthread_mutex_unlock(&verrou);

    printf("\n");
}

void response5(char* message){
    char temp_buf[LBUF+1];
    sprintf(temp_buf, "9BEUIP%s", message);

    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    pthread_mutex_lock(&verrou);
    struct elt* e = e_list;
    while(e != NULL){
        Sock.sin_addr.s_addr = inet_addr(e->adip);
        sendto(sid, temp_buf, strlen(temp_buf), 0, (struct sockaddr *)&Sock, sizeof(Sock));
        printf("Message envoyé à %s\n", e->nom);
        e = e->next;
    }
    pthread_mutex_unlock(&verrou);
}

void manage_response(char* buf, char* pseudo, int msg_length){
   
    switch(buf[0]){
        case '0': // Si 0 déconnexion d'un utilisateur
            printf("Message de %s : déconnexion\n\n", addrip(ntohl(Sock.sin_addr.s_addr)));
            response0();
            break;
            
        case '1': // Broadcast
        case '2': // ACK
            response12(pseudo);
            break;

        case '9': // Si 9 on affiche le message privé reçu
            printf("Message de %s : %s\n\n", addrip(ntohl(Sock.sin_addr.s_addr)), buf+6);
            break;

        default:
            printf("Error : code inconnu\n");
            break;
    }

}


void ajouteElt(char* pseudo, char* adip){

    struct elt* e_new = (struct elt*)malloc(sizeof(struct elt));
    if(e_new == NULL){
        return;
    }

    strncpy(e_new->nom, pseudo, LPSEUDO);
    e_new->nom[LPSEUDO] = '\0';
    strncpy(e_new->adip, adip, 15);
    e_new->adip[15] = '\0';
    e_new->next = NULL;

    if(e_list == NULL || strcmp(pseudo, e_list->nom) < 0){
        e_new->next = e_list;
        e_list = e_new;
        return;
    }

    struct elt* e = e_list;
    while(e->next != NULL && strcmp(pseudo, e->next->nom) > 0){
        e = e->next;
    }

    e_new->next = e->next;
    e->next = e_new;
    printf("Ajout dans la liste : %s - %s\n", adip, pseudo);
}

void supprimeElt(char* adip){
    if(e_list==NULL){
        printf("Aucun Utilisateur dans la liste");
        return;
    }

    if(strcmp(e_list->adip, adip) == 0){
        struct elt* temp = e_list;
        e_list = e_list->next;
        free(temp);
        printf("%s a été retiré de la liste.\n", adip);
        return;
    }

    struct elt* e = e_list;
    while(e->next != NULL && strcmp(e->next->adip, adip) != 0){
        e = e->next;
    }

    if(e->next == NULL){
        printf("IP %s non trouvée dans la liste.\n", adip);
    } else {
        struct elt* temp = e->next;
        e->next = e->next->next;
        free(temp);
        printf("%s a été retiré de la liste.\n", adip);
    }
}

void listeElts(void){
    printf("TABLE :\n");
    if(e_list == NULL) {
        printf("    (La liste est vide)\n");
    }
    struct elt* e = e_list;
    while(e != NULL){
        printf("    - %s - %s\n", e->adip, e->nom);
        e = e->next;
    }
    printf("\n");
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* service_tcp(void* p){

    pthread_detach(pthread_self()); 
    
    long nsock = (long)p;
    envoiContenu((int)nsock);
    close((int)nsock);
    
    pthread_exit(NULL);
}

void* serveur_tcp(void* rep){

    char* repertoire = (char*)rep;
    struct sockaddr_in Sin;
    int ln, nsock;
    long param;
    pthread_t thid;

    if((sid_tcp=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0){
        perror("socket tcp");
        return NULL;
    }

    int optval=1;
    setsockopt(sid_tcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    bzero(&Sin, sizeof(Sin));
    Sin.sin_family = AF_INET;
    Sin.sin_addr.s_addr = htonl(INADDR_ANY);
    Sin.sin_port = htons(PORT);

    if(bind(sid_tcp, (struct sockaddr*)&Sin, sizeof(Sin)) < 0){
        perror("bind tcp");
        return NULL;
    }

    if(listen(sid_tcp, 5) < 0){
        perror("listen tcp");
        return NULL;
    }

    printf("################# Serveur TCP attache au port %d (Dossier : %s) #################\n\n", PORT, repertoire);

    while(1){
        ln = sizeof(Sin);
        if((nsock=accept(sid_tcp, (struct sockaddr*)&Sin, (socklen_t*)&ln)) < 0){
            perror("accept tcp");
            continue;
        }
        
        param = nsock;
        if(pthread_create(&thid, NULL, service_tcp, (void*)param) != 0){
            fprintf(stderr,"Erreur creation thread TCP !\n");
        }
    }
    return NULL;
}

void demandeListe(char* pseudo){

    char target_ip[16] = "";
    int trouve=0;

    pthread_mutex_lock(&verrou);
    struct elt* e = e_list;
    while(e != NULL){
        if(!strcmp(e->nom, pseudo)){
            strcpy(target_ip, e->adip);
            trouve=1;
            break;
        }
        e = e->next;
    }
    pthread_mutex_unlock(&verrou);

    if(!trouve){
        printf("Erreur : Le pseudo '%s' est introuvable.\n", pseudo);
        return;
    }

    int sid_client;
    struct sockaddr_in client_addr;

    if((sid_client=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0){
        perror("socket TCP client");
        return;
    }

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(target_ip);
    client_addr.sin_port = htons(PORT);

    if(connect(sid_client, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        perror("connect TCP");
        close(sid_client);
        return;
    }

    char cmd='L';
    write(sid_client, &cmd, 1);

    char temp_buf[LBUF+1];
    int n;
    
    printf("\n--- Fichiers partagés par %s ---\n", pseudo);
    
    while((n=read(sid_client, temp_buf, LBUF)) > 0){
        temp_buf[n] = '\0';
        printf("%s", temp_buf);
    }
    printf("----------------------------------\n\n");

    close(sid_client);
}

void envoiContenu(int fd){

    char req;
    if(read(fd, &req, 1) <= 0){
        return;
    }

    if(req == 'L'){
        pid_t pid = fork();
        
        if(pid == 0){ 
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);

            execlp("ls", "ls", "-l", repertoire_public, NULL);
            perror("execlp ls");
            exit(1);
            
        } else if(pid > 0){ 
            waitpid(pid, NULL, 0);
        } else {
            perror("fork TCP");
        }

    } else if(req == 'F'){
        char nomfic[256];
        int i=0;
        char c;
        
        while(read(fd, &c, 1) > 0 && c != '\n' && i < 255){
            nomfic[i++] = c;
        }
        nomfic[i] = '\0';

        char chemin[PATH_MAX + 256];
        snprintf(chemin, sizeof(chemin), "%s/%s", repertoire_public, nomfic);

        if(access(chemin, F_OK) != 0){
            char* err = "ERR:NOFILE\n";
            write(fd, err, strlen(err));
            return;
        }

        pid_t pid = fork();
        
        if(pid == 0){ 
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            
            execlp("cat", "cat", chemin, NULL);
            perror("execlp cat");
            exit(1);
            
        } else if(pid > 0){
            waitpid(pid, NULL, 0);
        } else {
            perror("fork TCP");
        }
    }
}

void demandeFichier(char* pseudo, char* nomfic){

    char chemin_local[PATH_MAX + 256];
    snprintf(chemin_local, sizeof(chemin_local), "%s/%s", repertoire_public, nomfic);

    if(access(chemin_local, F_OK) == 0){
        printf("Erreur : Le fichier '%s' existe déjà dans votre répertoire '%s'.\n", nomfic, repertoire_public);
        return;
    }

    char target_ip[16] = "";
    int trouve=0;

    pthread_mutex_lock(&verrou);
    struct elt* e = e_list;
    while(e != NULL){
        if(!strcmp(e->nom, pseudo)){
            strcpy(target_ip, e->adip);
            trouve=1;
            break;
        }
        e = e->next;
    }
    pthread_mutex_unlock(&verrou);

    if(!trouve){
        printf("Erreur : Pseudo '%s' introuvable.\n", pseudo);
        return;
    }

    int sid_client;
    struct sockaddr_in client_addr;

    if((sid_client=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("socket TCP client");
        return;
    }

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(target_ip);
    client_addr.sin_port = htons(PORT);

    if(connect(sid_client, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        perror("connect TCP");
        close(sid_client);
        return;
    }

    char req_buf[512];
    snprintf(req_buf, sizeof(req_buf), "F%s\n", nomfic);
    write(sid_client, req_buf, strlen(req_buf));

    char temp_buf[LBUF];
    int n, is_first_chunk=1;
    FILE* f = NULL;

    while((n=read(sid_client, temp_buf, LBUF)) > 0){
        if(is_first_chunk){
            if(!strncmp(temp_buf, "ERR:NOFILE", 10)){
                printf("Erreur : Le fichier '%s' n'existe pas chez '%s'.\n", nomfic, pseudo);
                close(sid_client);
                return;
            }
            f = fopen(chemin_local, "w");
            if(!f){
                perror("fopen local");
                close(sid_client);
                return;
            }
            is_first_chunk=0;
        }
        fwrite(temp_buf, 1, n, f);
    }

    if(f != NULL){
        fclose(f);
        printf("Fichier '%s' téléchargé avec succès dans '%s/'.\n", nomfic, repertoire_public);
    }
    
    close(sid_client);
}

