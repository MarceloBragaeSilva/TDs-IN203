

# TP4 de BRAGA E SILVA Marcelo

`pandoc -s --toc tp4.md --css=./github-pandoc.css -o tp4.html`




## Open MP et MPI 



Nombre Processus MPI    | Nt OpenMP	 | Temps(s) | 
------------------------|------------|----------
2					    |2			 | 103.7	|
2					    |4			 | 107.0	|
2					    |8			 | 108.4	|
2					    |20			 | 111.9	|
3					    |2			 | 61.7	    |
3					    |4			 | 61.1		|
3					    |8			 | 62.8		|
3					    |20			 | 65.0		|
4					    |2			 | 43.8		|
4					    |4			 | 43.9		|
4					    |8			 | 44.6		|
4					    |20			 | 47.2		|
8					    |2			 | 22.8		|
8					    |4			 | 22.9		|
8					    |8			 | 24.1		|
8					    |20			 | 28.4		|

Meilleur temps: 8 processes MPI et Nt = 2.
OpenMP n'a pas reduit le temps de calcule. Pourquoi ??
Utilsé comme "# pragma omp parallel for schedule(dynamic)" avant la boucle




## Open MP + MPI x TBB

Nt    | OpenMP + MPI	 | TBB | 
------|------------------|------
2	  |		103.7		 | 8.04|
4	  |		107.0		 | 8.36|
8	  |		108.4		 | 8.37|
20	  |		111.9		 | 8.45|

Pour le code OpenMP + MPI: nbp = 2
Pour le code TBB		 : num_threads = 2
Encore une fois, voir pourquoi le changement de 'Nt' né change rien dans le temps de calcule.

## Raytracer

Temps sequentielle: 3.625 s

Methode					  | Grain size	|Temps(s)| 
--------------------------|-------------|--------
tbb:simple_partitioner	  |		1		| 0.865 |
tbb::auto_partitioner	  |		1		| 0.798 |
tbb::static_partitioner	  |		1		| 1.111 |
tbb:simple_partitioner	  |		2		| 0.847 |
tbb::auto_partitioner	  |		2		| 0.843 |
tbb::static_partitioner	  |		2		| 1.075 |
tbb:simple_partitioner	  |		4		| 0.810 |
tbb::auto_partitioner	  |		4		| 0.807 |
tbb::static_partitioner	  |		4		| 1.113 |
tbb:simple_partitioner	  |		20		| 0.810 |
tbb::auto_partitioner	  |		20		| 0.824 |
tbb::static_partitioner	  |		20		| 1.096 |
tbb:simple_partitioner	  |		100		| 0.845 |
tbb::auto_partitioner	  |		100		| 0.819 |  
tbb::static_partitioner	  |		100		| 1.124 | 

Ici on a les temps les plus hauts lorque le partitioner utilisé est le statique.
Si on utilise l'auto partitioner, le temps s'approche dus temps obtenu avec le simple partitioner.

# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
