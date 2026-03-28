#include "gescom.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stop=1;
static int child_pid = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////


int Sortie(int argc, char *argv[]){
    printf("Arrêt du programme...");
    write_history("biceps_history");
    if(child_pid != -1){
        kill(child_pid, SIGINT);
        waitpid(child_pid, NULL, 0);
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

        char* pseudo = (argc >= 3) ? argv[2] : "Anonyme";

        /* creation d'un nouveau processus */
        if ((child_pid = fork()) == -1) { /* erreur */
            perror("fork");
            return 0;
        }

        if (child_pid == 0) { /* code du fils */

            /* on change de code ! */
            execlp("./servbeuip", "servbeuip", pseudo, NULL);
            
            /* ici si on execute le code c'est qu'il y a une erreur !!*/
            perror("execlp servbeuip");
            return 0;

        } else { /* code du pere */
            printf("Serveur lancé !\n");
            return 1;
        }

    } else if(strcmp(argv[1], "stop") == 0){

        kill(child_pid, SIGINT);
        waitpid(child_pid, NULL, 0);
        return 1;

    } else if((strcmp(argv[1], "3") == 0) || (strcmp(argv[1], "4") == 0) || (strcmp(argv[1], "5") == 0)){
        return send_serveur(argc, argv);
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