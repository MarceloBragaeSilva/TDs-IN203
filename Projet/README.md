
# Bilan Projet

BRAGA E SILVA, Marcelo
Le 03 janvier 2022

## Informations

Ce projet fait partie du cours IN203 - Programmation Parallèle de l'école ENSTA Paris, avec le but de paralléliser avec MPI et OpenMP une simulation stochastique de la co–circulation d’un virus (de la grippe par exemple) et d’un second agent pathogène, en interaction dans une population humaine virtuelle. La parallélisation a été développée dans un environnement WSL (Windows Subsystem for Linux) en utilisant le MobaXterm comme fenêtre graphique.  L'ordinateur utilisé a 4 cœurs de calcule et donc, le maximum de processus utilisés est 4. Les fichiers .cpp pour chaque partie du projet ont été nommés conformément aux consignes du [Descriptif](https://github.com/MarceloBragaeSilva/TDs-IN203/blob/main/Projet/Sujet.pdf) fourni par le prof M. Juvigny. 

Un exécutable .exe de même nom sera créé pour chacun en utilisant la commande "make all".

## 2.1 Mesure du temps:
Premièrement, toutes les simulations faites dans le projet ont été configurées avec une durée de 365 jours pour que le résultat et la quantité de travail de chaque code soient les mêmes. Les temps mentionnés sont la moyenne de 5 itérations pour chaque simulation.

Donc, pour le [code base fourni](https://github.com/MarceloBragaeSilva/TDs-IN203/blob/main/Projet/sources/simulation.cpp) et pour une simulation d'une année, on a les temps:       

- Temps avec l'affichage : 32.65 secondes, commande "./simulation.exe"

- Temps sans l'affichage : 6.03 secondes, commande "./simulation.exe -nw"

On voit clairement que l'affichage est responsable pour la plupart du temps d'exécution du code et donc, elle ralentit l'exécution de la simulation.

## 2.2 Parallélisation affichage contre simulation - [.cpp](https://github.com/MarceloBragaeSilva/TDs-IN203/blob/main/Projet/sources/simulation_sync_affiche_mpi.cpp)
Dans cette partie, les calculs ont été divisés entre le processus 0 (chargé de l'affichage) et le processus 1 (chargé de la simulation proprement dit). Le processus 1 est responsable pour la mesure du temps d'exécution du code.

À la fin de chaque jour calculé, le processus 1 désormais transforme les statistiques de la grille en un vecteur, et ensuite envoi ce vecteur au processus 0 avec MPI_Send. Le processus 0 reçoit le vecteur avec MPI_Recv et affiche la simulation. Il faut noter que le nombre de jours écoulés est passé comme paramètre tag, pour que les deux processus puissent savoir quand arrêter la simulation.

OBS.: Pour faire la conversion de statistiques de la grille -> vecteur de statistiques la méthode Grille::getStatistiques a été utilisée. Cependant, dans l'autre sens, cela n'était pas possible car l'accès est défini privé. À cause de cela, j'ai modifié la classe Grille en créant la méthode Grille::setStatistiques.

- Commande "mpirun np -2 simulation_sync_affiche_mpi.exe"

- Commande "mpirun np -2 simulation_sync_affiche_mpi.exe -nw"

Simulation        | Temps (s)          | Accélération
------------------|--------------------|---------
Avec l'affichage  |        28.33       |  1.15
Sans l'affichage  |        5.98        |  1.001

La simulation sans affichage n'a pas changé en relation à précédent, comme espéré car le programme MPI sans affichage traite le programme comme le programme base. Cependant, on voit qu' avec affichage le temps diminue de quelques secondes, parce qu’avant un seul cœur calculait l’affichage et la simulation.

## 2.3 Parallélisation affichage asynchrone contre simulation - [.cpp](https://github.com/MarceloBragaeSilva/TDs-IN203/blob/main/Projet/sources/simulation_async_affiche_mpi.cpp)
Dans cette partie, la simulation du processus 1 n'est pas tellement ralentie par le processus 0 car l'affichage est fait seulement quand c'est possible.

Une tag AFFICHAGE_ASYNC_TAG a été créé pour le handshake entre les processus. Le processus 0 envoi non bloquant au processus 1 qui il est disponible pour un nouvel affichage et ensuite attend un vecteur de statistiques. De l'autre côté, le processus 1 vérifie dans chaque jour écoulé si une message de 0 a été envoyé et dans le cas positif, il envoie les statistiques a 0.

OBS.: une MPI_Recv a été fallu pour remettre le 'flag' de MPI_IProbe a zero et pour motifs de blocage de code, le dernier jour de simulation est toujours envoyé au processus 0.

- Commande "mpirun np -2 simulation_async_affiche_mpi.exe"

- Commande "mpirun np -2 simulation_async_affiche_mpi.exe -nw"

Simulation        | Temps (s)          | Accélération
------------------|--------------------|---------
Avec l'affichage  |        7.12        |  4.58
Sans l'affichage  |        6.05        |  0.996

Ici on voit une énorme accélération du temps avec affichage, car en moyenne, le programme affiche l'écran une fois à chaque 6 jours calculés. Désormais, c'est le processus 1 qui règle prioritairement le temps d'exécution. Il faut noter que le framerate est beaucoup plus faible.

## 2.4 Parallélisation OpenMP - [.cpp](https://github.com/MarceloBragaeSilva/TDs-IN203/blob/main/Projet/sources/simulation_async_omp.cpp)
Dans cette partie, l'utilisation de OpenMP a été faite pour paralléliser les boucles de taille significative. La difficulté est dans l'ordre d'exécution dans la grille pour chaque individu. Les classes individu, agent pathogène et grippe ont tous des attributs 'm_moteur_stochastique' qui sont des 'default_random_engine'. 

Alors par exemple, dans la boucle d'actualisation de la population (ligne 202) l'appel à personne.estContamine(grippe) représente un appel à grippe.nombreJoursIncubation()/grippe.nombreJoursSymptomatomatiques() et ces nombres sont définis en appelant une distribution gamma sur l'engine m_moteur_stochastique. Cette engine gère une séquence pseudo-aleátoire et donc, est une suite de valeurs définis lorsqu'on connaît l'ordre des graines utilisées. 

Cependant, quand on parallèle avec OpenMP, nous ne savons pas quel ordre des graines sera utilisé car chaque thread exécute dans une ordre différent et donc, le résultat de la simulation sera toujours un peu biaisé. La solution utilisée a été utilisé le directive "ordered" de OpenMP pour garantir l'exécution séquentielle de chaque individu et donc, le même résultat qu'au debút, cependant, l'accélération obtenue n'est pas significative.

OBS.: nombre de threads OMP a été configuré utilisant "export OMP_NUM_THREADS=1" (ou 2,3,..)

**Tableau avec affichage**
 OMP_NUM_THREADS | Temps sans ordered | Temps avec ordered | Acc sans ordered | Acc avec ordered
-----------------|--------------------|--------------------|------------------|------------------
1                |        6.58        |7.38                |        4.96      |4.42
2                |        4.55        |8.43                |        7.17      |3.87
3                |        3.45        |9.87                |        9.46      |3.31
4                |        3.20        |9.92                |       10.20      |3.29

Nous regardons un effet contraire aux simulation avec et sans 'ordered' quant le nombre de thread OMP augmente. Si on ne considère pas les résultats différents, ce code obtient une accélération significative de 10, qui montre qu' au minimum, presque 90% du code est parallélisable (loi d'Amdahl).

**Tableau sans affichage**
 OMP_NUM_THREADS | Temps sans ordered | Temps avec ordered | Acc sans ordered | Acc avec ordered
-----------------|--------------------|--------------------|------------------|------------------
1                |        6.18        |6.42                |        5.28      |5.08
2                |        4.14        |7.56                |        7.89      |4.32
3                |        3.26        |8.02                |       10.01      |4.07
4                |        2.75        |8.28                |       11.87      |3.94

Désormais on peut voir que les temps sans affichage finalement ont réduit, car il y a plus d'un cœur pour calculer la simulation.

## 2.5 Parallélisation MPI de la simulation - [.cpp](https://github.com/MarceloBragaeSilva/TDs-IN203/blob/main/Projet/sources/simulation_async_mpi.cpp)
Dans cette partie, les threads s'occupant de la simulation ont sont propre MPI_Comm pour échanger des donnés, le 'sub_comm'. À la vue du processus 0 (affichage), rien n'a changé. Pour la simulation, la stratégie adoptée a été couper le nombre total d' individus dans 'contexte.taux_population' et chaque thread aura son 'sub_taux_population' et fera les calculs de ces individus.

Cependant, la grille utilisée doit être la même pour les threads (sinon il n'aura pas d'interaction entre les threads). Alors, avant de calculer les contaminations (boucle ligne 234), on sauvegarde les statistiques dans un vecteur (comme mentionné avant), on appelle la fonction MPI_Allreduce pour que tous les threads ajoutent ces statistiques dans ce vecteur et ensuite actualise la grille. De cette manière, la grille est la même pour les threads et après chaque thread calcule la contamination de son subpopulation.

Obs: comme dans la section 2.4, les résultats ne sont pas les mêmes de la simulation initiale.


**Tableau avec affichage**
 Threads MPI pour simulation| Temps              | Accélération
----------------------------|--------------------|------------------
1                           |        6.50        |        5.02     
2                           |        5.38        |        6.06     
3                           |        5.09        |        6.41     

**Tableau sans affichage**
 Threads MPI pour simulation| Temps              | Accélération
----------------------------|--------------------|------------------
1                           |        6.42        |        5.08     
2                           |        4.47        |        7.30     
3                           |        4.62        |        7.07  

Si on compare les temps des section 2.4 et 2.5, la parallélisation avec OpenMP était plus rapide qu' avec MPI.
