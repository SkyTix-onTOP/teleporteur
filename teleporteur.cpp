#include "teleporteur.h"

#include <iostream>
#include <iomanip> // Pour gérer l'affichage formaté
#include <cmath>   // Pour abs()

using namespace std;

namespace teleporteur
{

    static int cycles_neqli = 0, cycles_faneqli = 0;
    static int deplacements_neqli = 0, deplacements_faneqli = 0;
    static vector<double> attente_neqli, attente_faneqli;
    static const string BAD_DISPLAY_TYPE("Error: must be SHOW_CYCLES or SHOW_NO_CYCLE");
    static const string BAD_QUEUE_NB("Error: the number of queues must be strictly positive");
    static const string BAD_QUEUE_INDEX("Error: invalid queue index");

    void print_error(string message)
    {
        cout << message;
        cout << endl;
        exit(0);
    }

    void error(Parametres param, bool error_trouve)
    {
        if (param.affichage_type != "SHOW_CYCLES" && param.affichage_type != "SHOW_NO_CYCLE")
        {
            print_error("Error: must be SHOW_CYCLES or SHOW_NO_CYCLE");
        }

        if (param.nb_files <= 0)
        {
            print_error("Error: the number of queues must be strictly positive");
        }

        if (error_trouve)
        {                                              // Si une erreur a été trouvée durant la lecture
            print_error("Error: invalid queue index"); //
        }
    }

    // **Lecture des paramètres**
    bool lire_et_valider_parametres(Parametres &param, bool &error_trouve)
    {
        // Lecture du type d'affichage
        cin >> param.affichage_type;
        if (param.affichage_type != "SHOW_CYCLES" && param.affichage_type != "SHOW_NO_CYCLE")
        {
            error_trouve = true; // Marquer l'erreur
            return false;
        }

        // Lecture du nombre de files
        cin >> param.nb_files;
        if (param.nb_files <= 0)
        {
            error_trouve = true; // Marquer l'erreur
            return false;
        }

        // Initialisation des files d'attente
        param.files.resize(param.nb_files);

        // Lecture des robots et des files
        int file, sortie;
        while (true)
        {
            cin >> file >> sortie;

            // Condition de fin : -1 -1
            if (file == -1 && sortie == -1)
                break;
            if (file < 0 or file >= param.nb_files or sortie < 0 or sortie >= param.nb_files)
            {
                error_trouve = true;
                return false;
            }
            // Ajouter le robot dans la file correspondante
            param.files[file].push(sortie);
        }

        return true; // Tous les paramètres sont valides
    }

    // **Affichage de l'état initial**
    void afficher_etat_initial(const Parametres &param)
    {
        cout << "Etat initial" << endl;

        // Parcours des files pour afficher leur contenu
        for (size_t i = 0; i < param.files.size(); ++i)
        {
            cout << i << "\t";                // Numéro de la file
            queue<int> temp = param.files[i]; // Copie temporaire pour affichage
            while (!temp.empty())
            {
                cout << temp.front() << " "; // Affichage des destinations des robots
                temp.pop();
            }
            cout << endl;
        }
    }

    // **Affichage des cycles**
    void afficher_cycle(int cycle, int depart, int arrivee, bool sortie, bool entree)
    {
        // Affichage des informations sur un cycle : numéro, départ, arrivée, actions
        cout << depart << "\t" << arrivee << "\t"
             << (sortie ? 1 : 0) << " " << (entree ? 1 : 0) << endl;
    }

    // **Mise à jour des files non vides**
    vector<bool> mettre_a_jour_files_non_vides(const Parametres &param)
    {
        // Initialise un vecteur pour indiquer si chaque file est vide ou non
        vector<bool> non_vides(param.nb_files, false);
        for (size_t i = 0; i < param.files.size(); ++i)
        {
            if (!param.files[i].empty())
            {
                non_vides[i] = true;
            }
        }
        return non_vides;
    }

    // **Vérifie si toutes les files sont vides**
    bool toutes_files_vides(const Parametres &param)
    {
        for (const auto &file : param.files)
        {
            if (!file.empty())
            {
                return false;
            }
        }
        return true;
    }

    // **Affichage des statistiques finales**
    void afficher_statistiques_finales()
    {
        cout << "Nombre de cycles" << endl;
        cout << cycles_neqli << "\t" << cycles_faneqli << endl;

        cout << "Déplacement total" << endl;
        cout << deplacements_neqli << "\t" << deplacements_faneqli << endl;

        cout << "Attente moyenne" << endl;
        for (size_t i = 0; i < attente_neqli.size(); ++i)
        {
            cout << i << "\t";
            if (attente_neqli[i] != 0)
            {
                cout << fixed << setprecision(2) << attente_neqli[i];
            }
            else
            {
                cout << "";
            }

            if (attente_faneqli[i] != 0)
            {
                cout << "\t" << fixed << setprecision(2) << attente_faneqli[i] << endl;
            }
            else
            {
                cout << "\t" << endl;
            }
        }
    }

    // **Fonction pour stocker les résultats**
    void stocker_resultats(int cycles, int deplacements,
                           const vector<int> &somme_indices_cycles, const vector<int> &nb_robots_initial,
                           int nb_files, int &cycles_resultat, int &deplacements_resultat,
                           vector<double> &attente_resultat)
    {
        cycles_resultat = cycles;
        deplacements_resultat = deplacements;
        attente_resultat.resize(nb_files);
        for (int i = 0; i < nb_files; ++i)
        {
            attente_resultat[i] = nb_robots_initial[i] > 0
                                      ? static_cast<double>(somme_indices_cycles[i]) / nb_robots_initial[i]
                                      : 0.0;
        }
    }

    // **Initialisation des variables**
    void initialiser_variables(const Parametres &param, int &cycles, int &deplacements,
                               int &scanner, int &robot_dans_scanner, vector<int> &somme_indices_cycles,
                               vector<int> &nb_robots_initial)
    {
        cycles = 0;
        deplacements = 0;
        scanner = 0;
        robot_dans_scanner = -1;
        somme_indices_cycles.resize(param.nb_files, 0);
        nb_robots_initial.resize(param.nb_files, 0);

        for (size_t i = 0; i < param.files.size(); ++i)
        {
            nb_robots_initial[i] = param.files[i].size();
        }
    }
    // début des fontions speicalement pour Neqli

    // **Recherche de la prochaine file non vide**
    int trouver_prochaine_file(const vector<bool> &non_vides)
    {
        // Recherche la première file non vide
        for (size_t i = 0; i < non_vides.size(); ++i)
        {
            if (non_vides[i])
            {
                return i;
            }
        }
        return -1; // Aucune file non vide trouvée
    }

    // **Cas 1 : Scanner vide et file vide (NEQLI)**
    void cas_1_neqli(int &scanner, int &deplacements, int &cycles, const vector<bool> &non_vides)
    {
        int prochaine_file = trouver_prochaine_file(non_vides);
        if (prochaine_file != -1)
        {
            int depart = scanner;
            scanner = prochaine_file;
            deplacements += abs(depart - scanner);
            ++cycles;
            afficher_cycle(cycles, depart, scanner, false, false);
        }
    }

    // **Cas 2 : Scanner vide et file pleine (NEQLI)**
    void cas_2_neqli(int &scanner, int &deplacements, int &cycles,
                     int &robot_dans_scanner, queue<int> &file,
                     vector<int> &somme_indices_cycles)
    {
        robot_dans_scanner = file.front();
        file.pop();
        int depart = scanner;
        scanner = robot_dans_scanner;
        deplacements += abs(depart - scanner);
        ++cycles;
        afficher_cycle(cycles, depart, scanner, false, true);
        somme_indices_cycles[depart] += cycles;
    }

    // **Cas 3 : Scanner plein, file vide, bonne sortie (NEQLI)**
    void cas_3_neqli(int &scanner, int &deplacements, int &cycles,
                     int &robot_dans_scanner, const vector<bool> &non_vides)
    {
        robot_dans_scanner = -1;
        int prochaine_file = trouver_prochaine_file(non_vides);
        if (prochaine_file != -1)
        {
            int depart = scanner;
            scanner = prochaine_file;
            deplacements += abs(depart - scanner);
            ++cycles;
            afficher_cycle(cycles, depart, scanner, true, false);
        }
        else
        {
            ++cycles;
            afficher_cycle(cycles, scanner, scanner, true, false);
        }
    }

    // **Cas 4 : Scanner plein, file pleine, bonne sortie (NEQLI)**
    void cas_4_neqli(int &scanner, int &deplacements, int &cycles,
                     int &robot_dans_scanner, queue<int> &file, vector<int> &somme_indices_cycles)
    {
        robot_dans_scanner = -1;
        robot_dans_scanner = file.front();
        file.pop();
        int depart = scanner;
        scanner = robot_dans_scanner;
        deplacements += abs(depart - scanner);
        ++cycles;
        afficher_cycle(cycles, depart, scanner, true, true);
        somme_indices_cycles[depart] += cycles;
    }

    // **Algorithme NEQLI**
    void neqli(Parametres &param)
    {
        int cycles, deplacements, scanner, robot_dans_scanner;
        vector<int> somme_indices_cycles, nb_robots_initial;

        initialiser_variables(param, cycles, deplacements, scanner,
                              robot_dans_scanner, somme_indices_cycles, nb_robots_initial);

        while (!toutes_files_vides(param) || robot_dans_scanner != -1)
        {
            vector<bool> non_vides = mettre_a_jour_files_non_vides(param);

            if (robot_dans_scanner == -1 && param.files[scanner].empty())
            {
                cas_1_neqli(scanner, deplacements, cycles, non_vides);
            }
            else if (robot_dans_scanner == -1 && !param.files[scanner].empty())
            {
                cas_2_neqli(scanner, deplacements, cycles, robot_dans_scanner,
                            param.files[scanner], somme_indices_cycles);
            }
            else if (robot_dans_scanner != -1 && param.files[scanner].empty() && scanner == robot_dans_scanner)
            {
                cas_3_neqli(scanner, deplacements, cycles, robot_dans_scanner, non_vides);
            }
            else if (robot_dans_scanner != -1 && !param.files[scanner].empty() && scanner == robot_dans_scanner)
            {
                cas_4_neqli(scanner, deplacements, cycles, robot_dans_scanner,
                            param.files[scanner], somme_indices_cycles);
            }
        }

        // Stocker les résultats
        stocker_resultats(cycles, deplacements, somme_indices_cycles,
                          nb_robots_initial, param.nb_files, cycles_neqli,
                          deplacements_neqli, attente_neqli);
    }

    // début des fonctions specialment pour faneqli

    // **Recherche de la prochaine file non vide (FANEQLI)**
    int trouver_prochaine_file_faneqli(const vector<bool> &non_vides,
                                       const vector<bool> &done, int nb_files)
    {
        for (int i = 0; i < nb_files; ++i)
        {
            if (non_vides[i] && !done[i])
            {
                return i; // Retourne le plus petit indice trouvé
            }
        }
        return -1; // Aucun indice valide trouvé
    }

    // **Fonction pour vérifier si toutes les files ont été traitées (FANEQLI)**
    bool toutes_files_traitees_faneqli(const vector<bool> &done, const Parametres &param)
    {
        for (int i = 0; i < param.nb_files; ++i)
        {
            if (!done[i] && !param.files[i].empty())
            {
                return false;
            }
        }
        return true;
    }

    // **Fonction pour réinitialiser les files (FANEQLI)**
    void reinitialiser_files_faneqli(vector<bool> &done, const Parametres &param)
    {
        vector<bool> non_vides = mettre_a_jour_files_non_vides(param);
        for (int i = 0; i < param.nb_files; ++i)
        {
            done[i] = !non_vides[i];
        }
    }

    // **Fonction pour le Cas 1 : Scanner vide et file vide (FANEQLI)**
    void cas_1_faneqli(int &scanner, int &deplacements, int &cycles,
                       const vector<bool> &non_vides, const vector<bool> &done, Parametres &param)
    {
        int prochaine_file = trouver_prochaine_file_faneqli(non_vides, done, param.nb_files);
        if (prochaine_file != -1)
        {
            int depart = scanner;
            scanner = prochaine_file;
            deplacements += abs(depart - scanner);
            ++cycles;
            afficher_cycle(cycles, depart, scanner, false, false);
        }
    }

    // **Fonction pour le Cas 2 : Scanner vide et file pleine (FANEQLI)**
    void cas_2_faneqli(int &scanner, int &deplacements, int &cycles,
                       int &robot_dans_scanner, queue<int> &file, vector<bool> &done,
                       vector<int> &somme_indices_cycles, const vector<bool> &non_vides, Parametres &param)
    {

        if (!done[scanner])
        {
            robot_dans_scanner = file.front();
            file.pop();
            done[scanner] = true;
            int depart = scanner;
            scanner = robot_dans_scanner;
            deplacements += abs(depart - scanner);
            ++cycles;
            afficher_cycle(cycles, depart, scanner, false, true);
            somme_indices_cycles[depart] += cycles;
        }
        else
        {
            int prochaine_file = trouver_prochaine_file_faneqli(non_vides, done,
                                                                param.nb_files);
            if (prochaine_file != -1)
            {
                int depart = scanner;
                scanner = prochaine_file;
                deplacements += abs(depart - scanner);
                ++cycles;
                afficher_cycle(cycles, depart, scanner, false, false);
            }
        }
    }

    // **Fonction pour le Cas 3 : Scanner plein, file vide, bonne sortie (FANEQLI)**
    void cas_3_faneqli(int &scanner, int &deplacements, int &cycles,
                       int &robot_dans_scanner, const vector<bool> &non_vides, vector<bool> &done,
                       Parametres &param)
    {
        robot_dans_scanner = -1;
        int prochaine_file = trouver_prochaine_file_faneqli(non_vides, done, param.nb_files);
        if (prochaine_file != -1)
        {
            int depart = scanner;
            scanner = prochaine_file;
            deplacements += abs(depart - scanner);
            ++cycles;
            afficher_cycle(cycles, depart, scanner, true, false);
        }
        else
        {
            ++cycles;
            afficher_cycle(cycles, scanner, scanner, true, false);
        }
    }
    // fonction pour le cas 4
    void cas_4_faneqli(int &scanner, int &deplacements, int &cycles,
                       int &robot_dans_scanner, const vector<bool> &non_vides,
                       vector<bool> &done, Parametres &param,
                       vector<int> &somme_indices_cycles)
    {
        if (robot_dans_scanner != -1)
        {
            if (!done[scanner])
            {
                robot_dans_scanner = param.files[scanner].front();
                param.files[scanner].pop();
                int depart = scanner;
                scanner = robot_dans_scanner;
                deplacements += abs(depart - scanner);
                ++cycles;
                afficher_cycle(cycles, depart, scanner, true, true);
                somme_indices_cycles[depart] += cycles;
                done[depart] = true;
            }
            else
            {
                // File déjà traitée
                int prochaine_file = trouver_prochaine_file_faneqli(non_vides, done, param.nb_files);
                if (prochaine_file != -1)
                {
                    robot_dans_scanner = -1;
                    int depart = scanner;
                    scanner = prochaine_file;
                    deplacements += abs(depart - scanner);
                    ++cycles;
                    afficher_cycle(cycles, depart, scanner, true, false);
                }
            }
        }
    }

    // **Algorithme FANEQLI**
    void faneqli(Parametres &param)
    {
        int cycles, deplacements, scanner, robot_dans_scanner;
        vector<int> somme_indices_cycles, nb_robots_initial;

        // Initialiser les variables
        initialiser_variables(param, cycles, deplacements, scanner,
                              robot_dans_scanner, somme_indices_cycles, nb_robots_initial);

        // Ajout du vecteur done pour gérer les tours
        vector<bool> done(param.nb_files, false);

        while (!toutes_files_vides(param) || robot_dans_scanner != -1)
        {
            vector<bool> non_vides = mettre_a_jour_files_non_vides(param);

            if (toutes_files_traitees_faneqli(done, param))
            {
                reinitialiser_files_faneqli(done, param);
            }
            if (robot_dans_scanner == -1 && param.files[scanner].empty())
            {
                cas_1_faneqli(scanner, deplacements, cycles, non_vides, done, param);
            }
            else if (robot_dans_scanner == -1 && !param.files[scanner].empty())
            {
                cas_2_faneqli(scanner, deplacements, cycles, robot_dans_scanner,
                              param.files[scanner], done, somme_indices_cycles, non_vides, param);
            }
            else if (robot_dans_scanner != -1 && param.files[scanner].empty() && scanner == robot_dans_scanner)
            {
                cas_3_faneqli(scanner, deplacements, cycles,
                              robot_dans_scanner, non_vides, done, param);
            }
            else if (robot_dans_scanner != -1)
            {
                cas_4_faneqli(scanner, deplacements, cycles,
                              robot_dans_scanner, non_vides, done, param, somme_indices_cycles);
            }
        }
        stocker_resultats(cycles, deplacements, somme_indices_cycles, nb_robots_initial,
                          param.nb_files, cycles_faneqli, deplacements_faneqli, attente_faneqli);
    }

} // teleporteur
