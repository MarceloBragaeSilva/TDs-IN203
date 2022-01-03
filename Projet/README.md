
# Bilan Projet

## Informations

Le projet a été développé dans un environnement WSL (Windows Subsystem for Linux) en utilisant le MobaXterm comme fenêtre graphique. L'ordinateur utilisé a 4 cœurs de calcule et donc, le maximum de processus utilisés est 4. Les fichiers .cpp pour chaque partie du projet ont été nommés conformément aux consignes du Descriptif. 

Un exécutable .exe de même nom sera créé pour chacun en utilisant la commande "make all".

## 2.1 Mesure du temps:
Premièrement, toutes les simulations faites dans le projet ont été configurées avec une durée de 365 jours pour que le résultat et la quantité de travail de chaque code soient les mêmes.

Donc, pour le code base et pour une simulation d'une année, on a les temps:

- Temps avec l'affichage : 25.82 secondes "./simulation.exe"

- Temps sans l'affichage : 6.25 secondes "./simulation.exe -nw"

On voit clairement que l'affichage est responsable pour la plupart du temps d'exécution du code et donc, elle ralentit l'exécution de la simulation.

## 2.2 Parallélisation affichage contre simulation (simulation_sync_affiche_mpi)
Dans cette partie, les calculs ont été divisés entre le processus 0 (chargé de l'affichage) et le processus 1 (chargé de la simulation proprement dit). Le processus 1 est responsable pour la mesure du temps d'exécution du code.

À la fin de chaque jour calculé, le processus 1 désormais transforme les statistiques de la grille en un vecteur, et ensuite envoi ce vecteur au processus 0 avec MPI_Send. Le processus 0 reçoit le vecteur avec MPI_Recv et affiche la simulation. Il faut noter que le nombre de jours écoulés est passé comme paramètre tag, pour que les deux processus puissent savoir quand arrêter la simulation.

OBS.: Pour faire la conversion de statistiques de la grille -> vecteur de statistiques la méthode Grille::getStatistiques a été utilisée. Cependant, dans l'autre sens, cela n'était pas possible car l'accès est défini privé. À cause de cela, j'ai modifié la classe Grille en créant la méthode Grille::setStatistiques.

- Temps avec l'affichage :  "mpirun np -2 simulation_sync_affiche_mpi"

- Temps sans l'affichage :  "mpirun np -2 simulation_sync_affiche_mpi -nw"

## 2.3 Parallélisation affichage asynchrone contre simulation (simulation_async_affiche_mpi)
Dans cette partie, la simulation du processus 1 n'est pas tellement ralentie par le processus 0 car l'affichage est fait seulement quand c'est possible.

Une tag AFFICHAGE_ASYNC_TAG a été créé pour le handshake entre les processus. Le processus 0 envoi non bloquant au processus 1 qui il est disponible pour un nouvel affichage et ensuite attend un vecteur de statistiques. De l'autre côté, le processus 1 vérifie chaque jour écoulé si une message de 0 a été envoyé et dans le cas positif, il envoie les statistiques a 0.

OBS.: une MPI_Recv a été fallu pour remettre le 'flag' de MPI_IProbe a zero et pour motifs de blocage de code, le dernier jour de simulation est toujours envoyé au processus 0.

- Temps avec l'affichage :  "mpirun np -2 simulation_async_affiche_mpi"

- Temps sans l'affichage :  "mpirun np -2 simulation_async_affiche_mpi -nw"

## 2.4 Parallélisation OpenMP (simulation_async_omp)
Dans cette partie, l'utilisation de OpenMP a été faite pour paralléliser les boucles de taille significative. La difficulté est dans l'ordre d'exécution dans la grille pour chaque individu. Les classes individu, agent pathogène et grippe ont tous des attributs 'm_moteur_stochastique' qui sont des 'default_random_engine'. 

Alors par exemple, dans la boucle d'actualisation de la population (ligne 202) l'appel à personne.estContamine(grippe) représente un appel à grippe.nombreJoursIncubation()/grippe.nombreJoursSymptomatomatiques() et ces nombres sont définis en appelant une distribution gamma sur l'engine m_moteur_stochastique. Cette engine gère une séquence pseudo-aleátoire et donc, est une suite de valeurs définis lorsqu'on connaît l'ordre des graines utilisées. 

Cependant, quand on parallèle avec OpenMP, nous ne savons pas quel ordre des graines sera utilisé car chaque thread exécute dans une ordre différent et donc, le résultat de la simulation sera toujours un peu biaisé. La solution utilisée a été utilisé le directive "ordered" de OpenMP pour garantir l'exécution séquentielle de chaque individu, cependant, l'accélération obtenue n'est pas significative.

 N proc pour sim | Temps sans ordered | Temps avec ordered 
-----------------|--------------|------------
1                 |        1      |98.0604  
2                 |         2     |54.0118  
3                 |        3      |44.2169  


