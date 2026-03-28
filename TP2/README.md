# Système d'exploitation : TP2

## Les fichiers :

* biceps.c *(V2.0)*
    * Le fichier principal, qui permet de lancer le terminal
* gescom.c
    * Le fichier qui contient toutes les commandes que nous avons implémenté
* creme.c
    * Le fichier qui contient toute la partie réseau du coté du terminal
* servbeuip.c
    * Le fichier qui contient toute la partie réseau coté serveur, executé par un processus fils

## L'utilisation :

La première étape sera d'éffectuer un :
    `make`

Ensuite, il vous faudra lancer l'executable :
    `./biceps.c`

Au lancement, la liste des commandes vous sera affiché dans votre terminal.
Pour la partie réseau, pour simplifier les commandes et ne pas avoir 2 commandes différentes (beuip et msg), j'ai tout regroupé sous beuip. Ainsi que ça soit pour lancer, arrêter ou envoyer, la commande de base reste beuip.

> TP réalisé par : VALTER Mathéo