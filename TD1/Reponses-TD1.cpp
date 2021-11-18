/*


Un hello World parall�le - R�ponses
  - Pour le HelloWorld qui affiche les m�ssages sur le terminal,
    l'ordre d'affichage de chaque t�che n'est pas fixe, � chaque
    fois l'ordre est diff�rente.
  - En enregistrant les sorties dans les fichiers .txt, on garantie
    que le processus n� 'N' �crira dans le fichier 'Output000N.txt'
 
 

Envoi Bloquant et Non Bloquant
    La premi�re version est s�re car chaque processus n'est pas empech�
    de suivre l'ex�cution du code dans le loop. 
    Par contre, dans la deuxi�me version il y a une commande "MPI_Wait",
    qui bloque le processus jusqu'� une condition specifique soit attendre.
    Alors, il est possible d'avoir une situation o� un processus A est bloqu�
    par un processus B, et B est aussi bloqu�(un deadlock, par exemple) et
    donc, le programme au total se trouve bloqu�.
    La suret� de la deuxi�me version d�pends �vid�ment du contexte, mais une
    solution serait d'utiliser des fonctions non bloquants comme "MPI_Isend" et
    "MPI_Irecv" ou une autre fonction wait comme "MPI_Waitany" qui
    ne specifie pas quel processus il doit attendre.

Circulation d'un jeton dans un anneau
  - Les messages sont affich�s dans l'ordre sequentielle parce que le 
    processus n� N doit attendre l'envoi du jeton par la part du processus n� N-1,
    por ensuite l'envoyer au prochain, n� N+1. De la m�me fa�on, le processus 0 doit
    attendre l'envoi du dernier processus n� nbp-1 avant d'afficher sa message
  
  - Pour la version diff�rente, on constante qui l'affichage n'est pas toujours dans
    l'ordre comme dans la version prec�dente, car chaque processus envoi son entier
    sans avoir la necessit� de attendre un autre processus, l'envoi est ind�pendant.






*/