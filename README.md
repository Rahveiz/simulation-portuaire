# Simulation Portuaire

Simulation portuaire utilisant les objets IPC System V (mémoire partagée
et sémaphores).

## Compilation

```sh
make all
```

## Utilisation

**Capitainerie (pcap)**
```sh
./pcap <nb_quais>
```
Crée une capitainerie avec le nombre de quais précisés en argument.
Elle reçoit les demandes d'accostage des navires et les redirige vers des quais
 libres (ou les fait attendre si le port est plein), et redirige également les 
 flottes de camions.

**Navires (pnav)**
```sh
./pnav <nom> <nb_cont> <tps_acc> <tps_dech>
```
Crée un navire allant s'accoster à la capitainerie.
Arguments :

* Nom du navire (seule la première lettre est significative)
* Nombre de conteneures
* Temps pour accoster
* Temps pour décharger un conteneur à quai

**Flotte de camions (pcam)**
```sh
./pfcam <nom_navire> <nb_camions> <temps>
```
Crée une flotte de camions allant charger les conteneurs d'un navire.
Arguments :

* Nom du navire (rappel : seule la première lettre compte)
* Nombre de camions de la flotte (cas où il y aurait trop de camions non géré)
* Temps de chargement d'un conteneur

**Arrêt du port (pstop)**
```sh
./pstop
```
Envoie un signal d'arrêt au port (ce dernier arrête alors d'accepter de
nouveaux navires et se termine une fois tous les navires accostés partis).

## programmes annexes

**Génération de dump (pdump)**
```sh
./pdump
```
Génère un dump de l'état actuel du port.


**Clean des objets IPC System V (pclean)**
```sh
./pclean
```
Supprime les objets IPC résiduels (utile en cas de crash ou d'utilisation
de SIGINT par exemple).