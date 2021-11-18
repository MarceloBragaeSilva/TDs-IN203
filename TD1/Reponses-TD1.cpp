/*


Un hello World parallèle - Réponses
  - Pour le HelloWorld qui affiche les méssages sur le terminal,
    l'ordre d'affichage de chaque tâche n'est pas fixe, à chaque
    fois l'ordre est différente.
  - En enregistrant les sorties dans les fichiers .txt, on garantie
    que le processus nº 'N' écrira dans le fichier 'Output000N.txt'
 
 

Envoi Bloquant et Non Bloquant
    La première version est sûre car chaque processus n'est pas empeché
    de suivre l'exécution du code dans le loop. 
    Par contre, dans la deuxième version il y a une commande "MPI_Wait",
    qui bloque le processus jusqu'à une condition specifique soit attendre.
    Alors, il est possible d'avoir une situation où un processus A est bloqué
    par un processus B, et B est aussi bloqué(un deadlock, par exemple) et
    donc, le programme au total se trouve bloqué.
    La sureté de la deuxième version dépends évidément du contexte, mais une
    solution serait d'utiliser des fonctions non bloquants comme "MPI_Isend" et
    "MPI_Irecv" ou une autre fonction wait comme "MPI_Waitany" qui
    ne specifie pas quel processus il doit attendre.

Circulation d'un jeton dans un anneau
  - Les messages sont affichés dans l'ordre sequentielle parce que le 
    processus nº N doit attendre l'envoi du jeton par la part du processus nº N-1,
    por ensuite l'envoyer au prochain, nº N+1. De la même façon, le processus 0 doit
    attendre l'envoi du dernier processus nº nbp-1 avant d'afficher sa message
  
  - Pour la version différente, on constante qui l'affichage n'est pas toujours dans
    l'ordre comme dans la version precédente, car chaque processus envoi son entier
    sans avoir la necessité de attendre un autre processus, l'envoi est indépendant.






*/