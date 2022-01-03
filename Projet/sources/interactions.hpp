#ifndef _EPIDEMIE_INTERACTION_HPP_
#define _EPIDEMIE_INTERACTION_HPP_

namespace epidemie 
{
struct Interactions 
{
    double beta = 20 ;// Modificateur du paramètre de transmission de la grippe
    double alfa = 1.5 ;// Paramètre de variabilite d'acquisition de la grippe par l'agent pathogène
    double teta = 1 ;// Paramètre modifiant transmission de la grippe par interaction avec l'agent pathogène
    double mi = 1 ;// Paramètre d'interaction sur l'immunite croisee.

    double tauxTransmissionGrippe( double taux_base, 
                                   bool contaminant_a_agent,
                                   bool contamine_infecte_agent, bool contamine_immunise_agent ) const
    {
        double taux = taux_base * beta;
        if (contaminant_a_agent)
            taux *= teta;
        if (contamine_infecte_agent)
            taux *= alfa;
        if (contamine_immunise_agent)
            taux *= mi;
        return taux;
    }
};


}

#endif