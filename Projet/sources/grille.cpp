#include "grille.hpp"

epidemie::Grille::Grille( std::size_t population )
{
    int taille = int(sqrt(population));
    m_dim_x = (population+taille-1)/taille;
    m_dim_y = taille;
    std::vector<StatistiqueParCase>(m_dim_x*m_dim_y).swap(m_statistiques);
}
//
std::size_t 
epidemie::Grille::nombreTotalContaminesGrippe() const
{
    std::size_t total = 0;
    for (auto statistique : m_statistiques)
    {
        total += statistique.nombre_contaminant_grippe_et_contamine_par_agent;
        total += statistique.nombre_contaminant_seulement_grippe;
    }
    return total;
}
//
std::size_t 
epidemie::Grille::nombreTotalContaminesAgentPathogene() const
{
    std::size_t total = 0;
    for (auto statistique : m_statistiques )
    {
        total += statistique.nombre_contaminant_seulement_contamine_par_agent;
        total += statistique.nombre_contaminant_grippe_et_contamine_par_agent;
    }
    return total;
}
//
void epidemie::Grille::setStatistiques(std::vector<int> vec_statistiques) {
    for (long unsigned int i = 0; i < vec_statistiques.size() / 3; i++) {
        m_statistiques[i].nombre_contaminant_seulement_grippe = vec_statistiques[3 * i + 0];
        m_statistiques[i].nombre_contaminant_seulement_contamine_par_agent = vec_statistiques[3 * i + 1];
        m_statistiques[i].nombre_contaminant_grippe_et_contamine_par_agent = vec_statistiques[3 * i + 2];
    }
}