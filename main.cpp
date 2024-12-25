#include "teleporteur.h"

#include <iostream>

using namespace teleporteur;
using namespace std;

// **Fonction principale**
int main() {
    Parametres param;
    bool error_trouve = false;
	lire_et_valider_parametres(param, error_trouve);
	error(param, error_trouve);
    
    Parametres param1 = param;
    // Affichage de l'état initial des files
    afficher_etat_initial(param);   
    if (param.affichage_type == "SHOW_CYCLES"){
		cout << "NEQLI" << endl;
	}
    neqli(param);
	if ( param.affichage_type == "SHOW_CYCLES"){
		cout << "FANEQLI" << endl; }
	faneqli(param1);
	afficher_statistiques_finales();

    return 0;
}
