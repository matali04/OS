# Système d'exploitation : TP3 (Réseau Multi-Threadé)

## Les fichiers :

* biceps.c *(V3.0)*
    * Le fichier principal, qui permet de lancer le terminal interactif.
* gescom.c
    * Le fichier central qui contient désormais toutes les commandes implémentées et toute la partie réseau (UDP et TCP) unifiée dans le même processus grâce au multi-threading.
* gescom.h
    * Le fichier d'en-tête contenant les déclarations de fonctions, les inclusions de bibliothèques et la structure de la liste chaînée.

*(Note : Les anciens fichiers creme.c et servbeuip.c ont été supprimés suite à la fusion de l'architecture).*

## L'utilisation :

La première étape sera d'effectuer un :
    `make`

Ensuite, il vous faudra lancer l'exécutable :
    `./biceps`

Au lancement, la liste des commandes vous sera affichée dans votre terminal.
Pour la partie réseau, pour simplifier les commandes et ne pas avoir plusieurs commandes différentes, j'ai tout regroupé sous la commande beuip. Ainsi, que ça soit pour lancer, arrêter, envoyer des messages ou télécharger des fichiers, la commande de base reste beuip. Exemples :
    `beuip start [pseudo]`
    `beuip message all test`
    `beuip ls pseudo`
    `beuip get pseudo fichier`

Le code intègre intégralement la partie réseau (serveurs TCP et UDP via threads partagés), la gestion dynamique des adresses IP (getifaddrs), les messages (privés et broadcasts), l'échange de fichiers, ainsi que l'arrêt propre des serveurs avec la commande `beuip stop` (ou automatiquement à l'arrêt du terminal).

---

## État d'avancement et Tests :

Pour ce TP, j'ai inclus toutes les parties demandées, y compris les fonctionnalités bonus. 

Cependant, il est à noter que je n'ai pas pu régler tous les petits soucis d'affichage spécifiques à cette partie bonus. De plus, nous n'avons pas pu retester le protocole en conditions réelles avec d'autres utilisateurs car nous n'avions plus accès au serveur de test à la fin de la séance. 

Malgré cela, toute la logique réseau (Mutex, sockets, processus fils, redirections) est correctement en place, gérée de manière sécurisée, et normalement tout marche !

> *TP réalisé par : VALTER Mathéo*
NOM: VALTER
PRENOM: MATHEO