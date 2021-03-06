# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>

int main(int nargs, char* argv[])
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Cr?er un communicateur global, COMM_WORLD qui permet de g?rer
	//       et assurer la coh?sion de l'ensemble des processus cr??s par MPI;
	//    2. d'attribuer ? chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	MPI_Init(&nargs, &argv);
	// Pour des raisons de portabilit? qui d?bordent largement du cadre
	// de ce cours, on pr?f?re toujours cloner le communicateur global
	// MPI_COMM_WORLD qui g?re l'ensemble des processus lanc?s par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour conna?tre le nombre de processus
	// qui ont ?t? lanc?s par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour conna?tre l'identifiant qui
	// m'a ?t? attribu? ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp ?tant le nombre de processus qui ont ?t? lanc?s par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);
	// Cr?ation d'un fichier pour ma propre sortie en ?criture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output(fileName.str().c_str());

	// Rajout du programme ici...
	MPI_Status status;
	int jeton;
	int tag = 1111;

	if (rank == 0) {
		jeton = 12345;

		MPI_Send(&jeton, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);
		MPI_Send(&jeton, 1, MPI_INT, 2, tag, MPI_COMM_WORLD);
	}
	if (rank == 1) {
		MPI_Recv(&jeton, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		MPI_Send(&jeton, 1, MPI_INT, 3, tag, MPI_COMM_WORLD);
	}
	if (rank == 2) {
		MPI_Recv(&jeton, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
	}
	if (rank == 3) {
		MPI_Recv(&jeton, 1, MPI_INT, 1, tag, MPI_COMM_WORLD, &status);
	}
	if(rank<4)
	std::cout << "Tache " << rank << " Jeton = " << jeton << "\n";






	output.close();
	// A la fin du programme, on doit synchroniser une derni?re fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue ?
	// tourner. Si on oublie cet instruction, on aura une plantage assur? des processus
	// qui ne seront pas encore termin?s.
	MPI_Finalize();
	return EXIT_SUCCESS;
}