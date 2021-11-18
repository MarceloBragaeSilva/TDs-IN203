# include <chrono>
# include <random>
# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>
#include <chrono>

// Attention , ne marche qu'en C++ 11 ou sup�rieur :
double approximate_pi(unsigned long nbSamples)
{
	typedef std::chrono::high_resolution_clock myclock;
	myclock::time_point beginning = myclock::now();
	myclock::duration d = beginning.time_since_epoch();
	unsigned seed = d.count();
	std::default_random_engine generator(seed);
	std::uniform_real_distribution <double> distribution(-1.0, 1.0);
	unsigned long nbDarts = 0;
	// Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
	for (unsigned sample = 0; sample < nbSamples; ++sample) {
		double x = distribution(generator);
		double y = distribution(generator);
		// Test if the dart is in the unit disk
		if (x * x + y * y <= 1) nbDarts++;
	}
	// Number of nbDarts throwed in the unit disk
	double ratio = double(nbDarts) / double(nbSamples);
	return 4 * ratio;
}

int main(int nargs, char* argv[])
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Cr�er un communicateur global, COMM_WORLD qui permet de g�rer
	//       et assurer la coh�sion de l'ensemble des processus cr��s par MPI;
	//    2. d'attribuer � chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	MPI_Init(&nargs, &argv);
	// Pour des raisons de portabilit� qui d�bordent largement du cadre
	// de ce cours, on pr�f�re toujours cloner le communicateur global
	// MPI_COMM_WORLD qui g�re l'ensemble des processus lanc�s par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour conna�tre le nombre de processus
	// qui ont �t� lanc�s par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour conna�tre l'identifiant qui
	// m'a �t� attribu� ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp �tant le nombre de processus qui ont �t� lanc�s par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);
	// Cr�ation d'un fichier pour ma propre sortie en �criture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output(fileName.str().c_str());

	// Rajout de code....
	MPI_Status status;
	//MPI_Request request[nbp-1];
	int* index;
	bool flag;
	int tag = 123;
	MPI_Request request;
	if (rank == 0) {
		auto starting_time = std::chrono::high_resolution_clock::now();
		double pi = 0;
		double pi_i;
		for (int i = 0; i < nbp - 1; i++) {
			MPI_Irecv(&pi_i, 1, MPI_DOUBLE, MPI_ANY_SOURCE, tag, globComm, &request);
			MPI_Wait(&request, &status);
			pi += pi_i;
		}
		pi = pi / (nbp - 1);
		std::cout << "Valeur Pi = " << pi << "\n";
		auto elapsed_time = std::chrono::high_resolution_clock::now() - starting_time;
		std::cout << "Temps en ticks = " << elapsed_time.count() << "\n\n";

	}
	else {
		double pi_local = approximate_pi(10000);
		MPI_Isend(&pi_local, 1, MPI_DOUBLE, 0, tag, globComm, &request);
	}


	output.close();
	// A la fin du programme, on doit synchroniser une derni�re fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue �
	// tourner. Si on oublie cet instruction, on aura une plantage assur� des processus
	// qui ne seront pas encore termin�s.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
