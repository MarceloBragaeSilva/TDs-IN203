#include <cstdlib>
#include <random>
#include <iostream>
#include <fstream>
#include "contexte.hpp"
#include "individu.hpp"
#include "graphisme/src/SDL2/sdl2.hpp"
#include <chrono>
#include <mpi.h>

#define DAYS_OF_SIM 365
#define INTERVAL_AFFICHAGE 10
#define AFFICHAGE_ASYNC_TAG 9999

//Diviser le COMM_WORLD en un comm pour l'affichage et un autre pour la sim
MPI_Comm sub_comm;

void majStatistique(epidemie::Grille& grille, std::vector<epidemie::Individu> const& individus)
{
    for (auto& statistique : grille.getStatistiques())
    {
        statistique.nombre_contaminant_grippe_et_contamine_par_agent = 0;
        statistique.nombre_contaminant_seulement_contamine_par_agent = 0;
        statistique.nombre_contaminant_seulement_grippe = 0;
    }
    auto [largeur, hauteur] = grille.dimension();
    auto& statistiques = grille.getStatistiques();
    for (auto const& personne : individus)
    {
        auto pos = personne.position();

        std::size_t index = pos.x + pos.y * largeur;
        if (personne.aGrippeContagieuse())
        {
            if (personne.aAgentPathogeneContagieux())
            {
                statistiques[index].nombre_contaminant_grippe_et_contamine_par_agent += 1;
            }
            else
            {
                statistiques[index].nombre_contaminant_seulement_grippe += 1;
            }
        }
        else
        {
            if (personne.aAgentPathogeneContagieux())
            {
                statistiques[index].nombre_contaminant_seulement_contamine_par_agent += 1;
            }
        }
    }
}

void afficheSimulation(sdl2::window& ecran, epidemie::Grille const& grille, std::size_t jour)
{
    auto [largeur_ecran, hauteur_ecran] = ecran.dimensions();
    auto [largeur_grille, hauteur_grille] = grille.dimension();
    auto const& statistiques = grille.getStatistiques();
    sdl2::font fonte_texte("./graphisme/src/data/Lato-Thin.ttf", 18);
    ecran.cls({ 0x00,0x00,0x00 });
    // Affichage de la grille :
    std::uint16_t stepX = largeur_ecran / largeur_grille;
    unsigned short stepY = (hauteur_ecran - 50) / hauteur_grille;
    double factor = 255. / 15.;

    for (unsigned short i = 0; i < largeur_grille; ++i)
    {
        for (unsigned short j = 0; j < hauteur_grille; ++j)
        {
            auto const& stat = statistiques[i + j * largeur_grille];
            int valueGrippe = stat.nombre_contaminant_grippe_et_contamine_par_agent + stat.nombre_contaminant_seulement_grippe;
            int valueAgent = stat.nombre_contaminant_grippe_et_contamine_par_agent + stat.nombre_contaminant_seulement_contamine_par_agent;
            std::uint16_t origx = i * stepX;
            std::uint16_t origy = j * stepY;
            std::uint8_t red = valueGrippe > 0 ? 127 + std::uint8_t(std::min(128., 0.5 * factor * valueGrippe)) : 0;
            std::uint8_t green = std::uint8_t(std::min(255., factor * valueAgent));
            std::uint8_t blue = std::uint8_t(std::min(255., factor * valueAgent));
            ecran << sdl2::rectangle({ origx,origy }, { stepX,stepY }, { red, green,blue }, true);
        }
    }

    ecran << sdl2::texte("Carte population grippee", fonte_texte, ecran, { 0xFF,0xFF,0xFF,0xFF }).at(largeur_ecran / 2, hauteur_ecran - 20);
    ecran << sdl2::texte(std::string("Jour : ") + std::to_string(jour), fonte_texte, ecran, { 0xFF,0xFF,0xFF,0xFF }).at(0, hauteur_ecran - 20);
    ecran << sdl2::flush;
}

void simulation(bool affiche, int rank, int nbp) {

    epidemie::ContexteGlobal contexte;
    // contexte.deplacement_maximal = 1; <= Si on veut moins de brassage
    // contexte.taux_population = 400'000;
    //contexte.taux_population = 1'000;
    contexte.interactions.beta = 60;
    std::size_t jours_ecoules = 0;

    MPI_Status status;

    if (rank == 0) {  //AFFICHAGE EN SYNCHRONE

        constexpr const unsigned int largeur_ecran = 1280, hauteur_ecran = 1024;
        sdl2::window ecran("Simulation epidemie de grippe", { largeur_ecran,hauteur_ecran });
        epidemie::Grille grille{ contexte.taux_population };
        std::vector<int> vec_statistiques(3 * grille.getStatistiques().size());
        bool quitting = false;

        //int flag = 1;
        MPI_Request request;
        while (!quitting) {
            if (affiche) {
                //MPI_Recv
                MPI_Isend(0, 0, MPI_INT, 1, AFFICHAGE_ASYNC_TAG, MPI_COMM_WORLD, &request);
                MPI_Recv(vec_statistiques.data(), vec_statistiques.size(), MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                grille.setStatistiques(vec_statistiques);

                jours_ecoules = status.MPI_TAG;

                afficheSimulation(ecran, grille, jours_ecoules);
                if (jours_ecoules >= DAYS_OF_SIM) quitting = true;
            }
            else if (!affiche) quitting = true;
        }


    }

    else if (rank >= 1) {   //SIMULATION: each thread in this group will reserve a fraction
                            // of the population

        std::uniform_real_distribution<double> porteur_pathogene(0., 1.);
        std::vector<epidemie::Individu> population;

        int sub_taux_population = contexte.taux_population / (nbp - 1);
        size_t it_debut = (rank - 1) * sub_taux_population;
        size_t it_fin = rank * sub_taux_population - 1;
        //Dernier processus prend le reste
        if (rank == nbp - 1) {
            sub_taux_population += (contexte.taux_population % (nbp - 1));
            it_fin += (contexte.taux_population % (nbp - 1));
        }
        std::cout << "Debut " << it_debut << " Fin " << it_fin << "\n";
        population.reserve(sub_taux_population);
        unsigned int graine_aleatoire = 1 + it_debut;

        epidemie::Grille grille{ contexte.taux_population };
        std::vector<int> vec_statistiques(3 * grille.getStatistiques().size());

        auto [largeur_grille, hauteur_grille] = grille.dimension();
        // L'agent pathogene n'evolue pas et reste donc constant...
        epidemie::AgentPathogene agent(graine_aleatoire++);

        // Initialisation de la population initiale :
        for (std::size_t i = it_debut; i < it_fin; ++i) {
            std::default_random_engine motor(100 * (i + 1));
            population.emplace_back(graine_aleatoire++, contexte.esperance_de_vie, contexte.deplacement_maximal);
            population.back().setPosition(largeur_grille, hauteur_grille);
            if (porteur_pathogene(motor) < 0.2)
                population.back().estContamine(agent);
            
        }
        //std::cout << rank <<"HERE\n\n";
        //std::size_t jours_ecoules = 0;
        int         jour_apparition_grippe = 0;
        size_t      nombre_immunises_grippe = (contexte.taux_population * 23) / 100;
        sdl2::event_queue queue;

        bool quitting = false;

        int flag = 0;
        int teste = 0;
        std::ofstream output("Courbe.dat");
        if (rank == 0) {
            output << "# jours_ecoules \t nombreTotalContaminesGrippe \t nombreTotalContaminesAgentPathogene()" << std::endl;
        }
        epidemie::Grippe grippe(0);

        std::cout << "Debut boucle epidemie" << std::endl << std::flush;
        while (!quitting) {
            auto events = queue.pull_events();
            for (const auto& e : events) {
                if (e->kind_of_event() == sdl2::event::quit)
                    quitting = true;
            }
            if (jours_ecoules % 365 == 0){// Si le premier Octobre (debut de l'annee pour l'epidemie ;-) )
                grippe = epidemie::Grippe(jours_ecoules / 365);
                jour_apparition_grippe = grippe.dateCalculImportationGrippe();
                grippe.calculNouveauTauxTransmission();


                // 23% des gens sont immunises. On prend les 23% premiers
                /*for (int ipersonne = 0; ipersonne < nombre_immunises_grippe; ++ipersonne)
                {
                    population[ipersonne].devientImmuniseGrippe();
                }
                for (int ipersonne = nombre_immunises_grippe; ipersonne < int(contexte.taux_population); ++ipersonne)
                {
                    population[ipersonne].redevientSensibleGrippe();
                }*/
                for (size_t ipersonne = it_debut; ipersonne < it_fin; ++ipersonne)
                {   if(ipersonne < nombre_immunises_grippe)
                        population[ipersonne - it_debut].devientImmuniseGrippe();
                    else
                        population[ipersonne - it_debut].redevientSensibleGrippe();
                }
            }
            if (jours_ecoules % 365 == std::size_t(jour_apparition_grippe))
            {
                /*for (int ipersonne = nombre_immunises_grippe; ipersonne < nombre_immunises_grippe + 25; ++ipersonne)
                {
                    population[ipersonne].estContamine(grippe);
                }*/
                if (nombre_immunises_grippe >= it_debut && nombre_immunises_grippe < it_fin - 25) {
                    for (size_t ipersonne = nombre_immunises_grippe; ipersonne < nombre_immunises_grippe + 25; ++ipersonne)
                    {
                        population[ipersonne - it_debut].estContamine(grippe);
                        //std::cout << "Personne " << ipersonne << " contamine\n";
                    }
                }
            }


            // Mise a jour des statistiques pour les cases de la grille :
            majStatistique(grille, population);
            // Tous les threads doivent voir et calculer sur la meme grille
            for (long unsigned int i = 0; i < grille.getStatistiques().size(); i++) {
                vec_statistiques[3 * i + 0] = grille.getStatistiques().at(i).nombre_contaminant_seulement_grippe;
                vec_statistiques[3 * i + 1] = grille.getStatistiques().at(i).nombre_contaminant_seulement_contamine_par_agent;
                vec_statistiques[3 * i + 2] = grille.getStatistiques().at(i).nombre_contaminant_grippe_et_contamine_par_agent;
            }
            MPI_Allreduce(MPI_IN_PLACE, vec_statistiques.data(), vec_statistiques.size(), MPI_INT, MPI_SUM, sub_comm);
            grille.setStatistiques(vec_statistiques);

            // On parcout la sub_population pour voir qui est contamine et qui ne l'est pas, d'abord pour la grippe puis pour l'agent pathogene
            std::size_t compteur_grippe = 0, compteur_agent = 0, mouru = 0;
            for (auto& personne : population) {
                if (personne.testContaminationGrippe(grille, contexte.interactions, grippe, agent)) {
                    compteur_grippe++;
                    personne.estContamine(grippe);
                }
                if (personne.testContaminationAgent(grille, agent)){
                    compteur_agent++;
                    personne.estContamine(agent);
                }
                // On verifie si il n'y a pas de personne qui veillissent de veillesse et on genere une nouvelle personne si c'est le cas.
                if (personne.doitMourir()) {
                    mouru++;
                    unsigned nouvelle_graine = jours_ecoules + personne.position().x * personne.position().y;
                    personne = epidemie::Individu(nouvelle_graine, contexte.esperance_de_vie, contexte.deplacement_maximal);
                    personne.setPosition(largeur_grille, hauteur_grille);
                }
                personne.veillirDUnJour();
                personne.seDeplace(grille);
            }
            
            //Seulement Processus 1 va communiquer avec le groupe Affichage
            // Probleme actuel : il envoie toujours le jour d'avant le jour courant
            if (rank == 1) {
                if (affiche) {
                    //std::cout << flag << std::endl;
                    MPI_Iprobe(0, AFFICHAGE_ASYNC_TAG, MPI_COMM_WORLD, &flag, &status);
                    if (flag) {
                        //capture the ISend message to make flag go back to 0, if not, flag stays always at 1
                        MPI_Recv(0, 0, MPI_INT, 0, AFFICHAGE_ASYNC_TAG, MPI_COMM_WORLD, &status);
                        /*for (long unsigned int i = 0; i < grille.getStatistiques().size(); i++) {
                            vec_statistiques[3 * i + 0] = grille.getStatistiques().at(i).nombre_contaminant_seulement_grippe;
                            vec_statistiques[3 * i + 1] = grille.getStatistiques().at(i).nombre_contaminant_seulement_contamine_par_agent;
                            vec_statistiques[3 * i + 2] = grille.getStatistiques().at(i).nombre_contaminant_grippe_et_contamine_par_agent;
                        }*/
                        MPI_Send(vec_statistiques.data(), vec_statistiques.size(), MPI_INT, 0, jours_ecoules, MPI_COMM_WORLD);
                        if (jours_ecoules == DAYS_OF_SIM) flag = 1;
                        teste++;
                    }
                }
                output << jours_ecoules << "\t" << grille.nombreTotalContaminesGrippe() << "\t"
                    << grille.nombreTotalContaminesAgentPathogene() << std::endl;
            }
            //std::cout << "Proc "<<rank<<" jour "<<jours_ecoules<<"\n";

            if (jours_ecoules >= DAYS_OF_SIM) {
                quitting = true;                          //FOR A YEAR
                //necessary cause proc0 gets stuck at MPI_Recv, so, we force to send the last days of simulation if it not yet (reusing 'flag')
                if (affiche && rank == 1 && !flag) MPI_Send(vec_statistiques.data(), vec_statistiques.size(), MPI_INT, 0, jours_ecoules, MPI_COMM_WORLD);
            }
            jours_ecoules += 1;
        }// Fin boucle temporelle

        output.close();
        //std::cout << "N sends = " << teste << "\nN jours = "<<jours_ecoules<<"\n\n";
    }

}


int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    //MPI_Comm globComm;
    //MPI_Comm_dup(MPI_COMM_WORLD, &globComm);

    int nbp;
    MPI_Comm_size(MPI_COMM_WORLD, &nbp);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int color = 1; //sim
    if (rank == 0) color = 0; //afic
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &sub_comm);



    // parse command-line
    bool affiche = true;
    for (int i = 0; i < argc; i++) {
        std::cout << i << " " << argv[i] << "\n";
        if (std::string("-nw") == argv[i]) affiche = false;
    }

    
    //Seulement Proc 1 mesure le temps
    if (rank == 1) {
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

    sdl2::init(argc, argv);
    {
        simulation(affiche, rank, nbp);
    }
    sdl2::finalize();

        std::cout << "Processus " << rank << "termine\n";
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Temps calcul : " << elapsed_seconds.count() << std::endl;
    }
    else {
        sdl2::init(argc, argv);
        {
            simulation(affiche, rank, nbp);
        }
        sdl2::finalize();

        std::cout << "Processus " << rank << "termine\n";
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
