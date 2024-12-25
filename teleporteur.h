#include <vector>
#include <queue>
#include <string>


namespace teleporteur {

// **Structure des paramètres**
struct Parametres {
    std::string affichage_type;        // Type d'affichage : SHOW_CYCLES ou SHOW_NO_CYCLES
    int nb_files;                 // Nombre de files d'attente
    std::vector<std::queue<int>> files;    // Files d'attente, chaque file est une std::queue
};

// **Prototypes des fonctions**
bool lire_et_valider_parametres(Parametres& param,bool& error_trouve); 
void afficher_etat_initial(const Parametres& param);
void neqli(Parametres& param);                     
void print_error(std::string message);
void error (Parametres param, bool error_trouve);
void afficher_cycle(int cycle, int depart, int arrivee, bool sortie, bool entree); 
std::vector<bool> mettre_a_jour_files_non_vides(const Parametres& param);            
bool toutes_files_vides(const Parametres& param);                                
int trouver_prochaine_file_faneqli(const std::vector<bool>& non_vides, 
const std::vector<bool>& done, int nb_files);

void faneqli(Parametres& param);
void afficher_cycle(int cycle, int depart, int arrivee, bool sortie, bool entree); 
void afficher_statistiques_finales();
void cas_1_neqli(int& scanner, int& deplacements, int& cycles,
 const std::vector<bool>& non_vides);
 
void cas_2_neqli(int& scanner, int& deplacements, int& cycles, 
int& robot_dans_scanner, std::queue<int>& file, std::vector<int>& somme_indices_cycles);

void cas_3_neqli(int& scanner, int& deplacements, int& cycles, 
int& robot_dans_scanner, const std::vector<bool>& non_vides);

void cas_4_neqli(int& scanner, int& deplacements,int& cycles, int& robot_dans_scanner,
 std::queue<int>& file, std::vector<int>& somme_indices_cycles);
 
void stocker_resultats(int cycles, int deplacements,
 const std::vector<int>& somme_indices_cycles, const std::vector<int>& nb_robots_initial, 
 int nb_files, int& cycles_resultat, int& deplacements_resultat, 
 std::vector<double>& attente_resultat);
 
void initialiser_variables(const Parametres& param, int& cycles, int& deplacements, 
int& scanner, int& robot_dans_scanner, 
std::vector<int>& somme_indices_cycles, std::vector<int>& nb_robots_initial);

bool toutes_files_traitees_faneqli(const std::vector<bool>& done,const Parametres& param);
void reinitialiser_files_faneqli(std::vector<bool>& done, const Parametres& param);

void cas_1_faneqli(int& scanner, int& deplacements, int& cycles, 
const std::vector<bool>& non_vides, const std::vector<bool>& done, Parametres& param);



void cas_2_faneqli(int& scanner, int& deplacements, int& cycles, 
int& robot_dans_scanner, std::queue<int>& file, std::vector<bool>& done,
 std::vector<int>& somme_indices_cycles, const std::vector<bool>& non_vides,Parametres& param);
 
void cas_3_faneqli(int& scanner, int& deplacements, int& cycles, 
int& robot_dans_scanner, const std::vector<bool>& non_vides, std::vector<bool>& done,
 Parametres& param);
 
void cas_4_faneqli(int& scanner, int& deplacements, int& cycles, 
                   int& robot_dans_scanner, const std::vector<bool>& non_vides, 
                   std::vector<bool>& done, Parametres& param, std::vector<int>& somme_indices_cycles);

} // teleporteur
