# TP2 de BRAGA E SILVA Marcelo

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`





## Mandelbrot 

*Expliquer votre stratégie pour faire une partition équitable des lignes de l'image entre chaque processus*

La stratégie adopté c'était de faire H/nbp itérations de la fonction 'computeMandelbrotSetRow' pour chaque
processus, en utilisant la fonction 'MPI_Gather' où le processus 0 est le root et agregue les calcules
locaux de chaque processus dans le vector 'pixels' et après sauvegarde l'image'.

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |            98.7747  
avec -O3   |            14.4217
1          |            98.0604  
2          |            54.0118  
3          |            44.2169  
4          |            39.7886  
8          |            45.8549  


*Discuter sur ce qu'on observe, la logique qui s'y cache.*

On peut voir que le temps de calcule sequentielle et avec 1 processus est presque le même, mais quand on
augmente le nombre le temps diminue, mais pas linearement. Le temps avec 8 processus était plus grand qu'avec
4 à mon avis, car mon ordinateur à 4 slots disponibles au total, alors, pour utiliser 8, j'ai eu besoin d'utiliser
la commande '--oversubscribe' pour ignorer le nombre de slots disponibles, et cela peut ralentir l'éxécution.




*Expliquer votre stratégie pour faire une partition dynamique des lignes de l'image entre chaque processus*

La stratégie adopté c'était la proposée (Maitre-Esclave) où le maitre envoie au nbp-1 esclaves une tâche et chaque
esclave calcule une ligne, l'envoi au maitre, et reçois une nouvelle ligne à calculer. Cependant, je n'ai pas trouvé
à temps une façon de bien organiser l'image assemblé, alors, des fois le code ne mets pas l'image dans l'ordre correcte.

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |           98.7747   
1          |  pas possible, il faut au minimum 1 maitre et un esclave            
2          |          119.523    
3          |           62.7844   
4          |           44.4714   
8          |           23.8546   

*Discuter sur ce qu'on observe, la logique qui s'y cache.*
Contrairement au code de partition équitable, cette code devient plus rapide même avec 8 processus et avec 2 processus
il est plus lent que le code séquentiel. Cela est le cas car il faut deux communications entre la tâche maitre et esclave pour
chaque ligne de l'image, en plus d'avoir seulement un processus qui fait réelement les calcules (comme le code séquentiel).



## Produit matrice-vecteur



*Expliquer la façon dont vous avez calculé la dimension locale sur chaque processus, en particulier quand le nombre de processus ne divise pas la dimension de la matrice.*

La dimension locale se calcule par la division de la dimension N de la matrice A par le nombre de processus. Quand le nombre
de processus ne divise pas la dimension N, les premiers taches auront comme N_loc le truncate(N/nbp) et la dernier tache aura le reste de la division.
