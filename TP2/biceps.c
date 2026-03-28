#include <stdio.h>
#include <readline/readline.h>

#include "gescom.h"

static char** Mots;    /* le tableau des mots de la commande */
static int NMots;    /* nombre de mots de la commande */
char* prompt;



typedef struct intern_cmd_ {
    char* name;
    int (*fct)(int argc, char *argv[]);
} intern_cmd;

#define NBMAXC 10
intern_cmd* cmds;
int idx_cmds=0;



int analyse_commande(char* cmd, char* delim){

    char* token;

    char* ptr_cmd = cmd;
    char* cmd_bis = strdup(cmd);
    char* ptr_cmd_bis = cmd_bis;

    /* On récupère le nombre de parties dans la commande pour créer le tableau à la bonne taille */
    NMots=0;
    while((token = strsep(&ptr_cmd_bis, delim))){
        if(*token != '\0'){
            NMots++;
        }
    }
    free(cmd_bis);

    Mots = (char**)malloc(sizeof(char*)*(NMots+1));

    /* On remplit le tableau avec les mots de la commande */
    NMots=0;
    while((token = strsep(&ptr_cmd, delim))){
        if(*token != '\0'){
            Mots[NMots] = token;
        NMots++;
        }
    }
    Mots[NMots] = NULL;

    return NMots;
}

void interruption(){
    printf("\n\nIf you want to leave the bash, use 'exit' command\n\n");
    rl_on_new_line();       // New line
    rl_replace_line("", 0); // Del actual txt
    rl_redisplay();         // Display the prompt
}


void ajouteCom(char* cmd_name, int (*fct)()){
    
    if(idx_cmds+1>=NBMAXC){
        printf("Trop de commandes !\n");
        exit(1);
    }

    cmds[idx_cmds].name = cmd_name;
    cmds[idx_cmds].fct = fct;
    idx_cmds++;
}

void majComInt(){
    ajouteCom("exit", Sortie);
    ajouteCom("pwd", pwd);
    ajouteCom("cd", cd);
    ajouteCom("vers", vers);
    ajouteCom("history", history);
    ajouteCom("beuip", beuip);
}

void listeComInt(void){
    printf("Liste des commandes :\n");
    for(int i=0; i<idx_cmds; i++){
        printf("\t- %s\n", cmds[i].name);
    }
}

int execComInt(int argc, char **argv){

    for(int i=0; i<idx_cmds; i++){
        if(!strcmp(argv[0], cmds[i].name)){
            if(cmds[i].fct(argc, argv)){
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

int execComExt(char** argv){

    int pid = fork(), wstatus;

    if(pid == 0){       // Child
        execvp(argv[0], argv);
        exit(127);      // Si on arrive ici, erreur
    } else if(pid > 0){ // Parent
        waitpid(pid, &wstatus, 0);
        int code_retour = (WEXITSTATUS(wstatus)==127) ? 0 : 1;
        return code_retour;
    } else {
        perror("Error on fork");
        exit(1);
    }
}




void bash(char* ansi_cmd){

    char* cmd;

    cmd = readline(prompt);
    printf("%s\n",ansi_cmd);

    if(cmd == NULL){         // CTRL+D
        stop=0;
        printf("Arrêt du programme...");
        return;
    }

    if(*cmd == '\0'){ // Enter
        return;
    }

    char *token, *temp, *ptr_cmd = cmd;
    while((token = strsep(&ptr_cmd, ";"))){

        temp = strdup(token);
        analyse_commande(token," \t\n");

        if(trace){ // -DTRACE
            for(int i=0; i<NMots; i++){
                printf("%d : %s\n", i, Mots[i]);
            }
            printf("Commande : %s\n", token);
        }
        
        if(!execComInt(NMots, Mots)){
            if(!execComExt(Mots)){
                printf("COMMAND NOT FOUND\n");
            } else {
                add_history(temp);
            }
        } else {
            add_history(temp);
        }
        free(temp);

        free(Mots);
        printf("\n");
    }

    free(cmd);

}


int main(int argc, char **argv){

    for(int i=0; i<argc; i++){
        if(!strcmp(argv[i], "-DTRACE")){
            trace=1;
        }
    }

    char* user = getenv("USER");

    char name[HOST_NAME_MAX];
    if(gethostname(name, HOST_NAME_MAX)){
        printf("Impossible d'obtenir le hostname\n");
        return 0;
    }
    
    char end_prompt = (getuid()) ? '$' : '#';

    char* ansi_prompt = "\033[1;34m";
    char* ansi_user = "\033[0m\033[32m";
    char* ansi_cmd = "\033[0m";

    int len_prompt = 17 + strlen(name) + 1 + strlen(user) + 4;
    prompt = (char*)malloc(sizeof(char)*len_prompt);
    sprintf(prompt, "%s%s@%s %c %s", ansi_prompt, user, name, end_prompt, ansi_user);

    cmds = (intern_cmd*)malloc(sizeof(intern_cmd)*NBMAXC);
    majComInt();
    listeComInt();
    printf("\n");

    signal(SIGINT, interruption);
    read_history("biceps_history");

    while(stop){
        bash(ansi_cmd);
    }

    free(prompt);
    return 0;
}