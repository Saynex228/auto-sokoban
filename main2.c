/* Auteur : Tuila Abdelkarim (Modifié pour Version Autonome)
 * Date : 06/11/2025
 * Description : SAÉ 1.02 - Sokoban Autonome
 * Gestion stricte des majuscules (poussée) et minuscules (déplacement simple)
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Pour usleep [cite: 28]
#include <fcntl.h>
#include <ctype.h>  // Pour toupper/tolower [cite: 39]

/* Constantes pour le plateau */
#define TAILLE_DEPLACEMENT 1000
#define TAILLE 12
#define MUR '#'
#define CAISSE '$'
#define VIDE ' '
#define CIBLE '.'
#define SOKOBAN '@'
#define CAISSE_CIBLE '*'
#define SOKOBAN_CIBLE '+'

/* Types personnalisés */
typedef char typeDeplacements[TAILLE_DEPLACEMENT];
typedef char t_Plateau[TAILLE][TAILLE];
typedef typeDeplacements t_tabDeplacement;

/* Prototypes */
void chargerPartie(t_Plateau plateau, char fichier[]);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);
void afficher_entete(t_tabDeplacement tab, char nomFichier[], int nbDeplacements, int index_actuel);
void afficher_plateau(t_Plateau plateau);
void trouver_sokoban(t_Plateau plateau, int *y, int *x);
bool est_sur_cible(t_Plateau plateau_initial, int y, int x);
void restaurer_position_sokoban(t_Plateau plateau, t_Plateau plateauInitial, int sokobanY, int sokobanX);
void placer_sokoban(t_Plateau plateau, t_Plateau plateauInitial, int y, int x);
bool gagne(t_Plateau plateau);
void copier_plateau(t_Plateau source, t_Plateau destination);

/* Fonction principale de déplacement autonome avec logique stricte */
void tenter_deplacement(t_Plateau plateau, t_Plateau plateauInitial, int *nbDeplacements, char action);

int main() {
    t_Plateau plateauInitial;
    t_Plateau plateau;
    typeDeplacements tab;
    char nomFichier[30];
    char nomFichierDep[30];

    int nbDeplacementsTotal = 0; // Nombre de char dans le fichier
    int nbDeplacementsValides = 0; // Nombre de mouvements réellement effectués

    // 1. Demander les fichiers [cite: 8, 9]
    printf("Tapez le nom de la partie (ex: niveau1.sok) : ");
    if (scanf("%29s", nomFichier) != 1) return 1;

    printf("Tapez le nom des deplacements (ex: niveau1.dep) : ");
    if (scanf("%29s", nomFichierDep) != 1) return 1;

    // Chargement
    chargerPartie(plateau, nomFichier);
    copier_plateau(plateau, plateauInitial);
    chargerDeplacements(tab, nomFichierDep, &nbDeplacementsTotal);

    // 2. Boucle automatique [cite: 17]
    for(int i = 0; i < nbDeplacementsTotal; i++){
        system("clear"); // Ou "cls" sous Windows

        // Affichage avant le mouvement
        afficher_entete(tab, nomFichier, nbDeplacementsValides, i);
        afficher_plateau(plateau);

        // Tentative de mouvement selon la lettre
        tenter_deplacement(plateau, plateauInitial, &nbDeplacementsValides, tab[i]);

        // Pause de 0.25 secondes (250 000 microsecondes)
        usleep(500000);
    }

    // Affichage final pour le dernier état
    system("clear");
    afficher_entete(tab, nomFichier, nbDeplacementsValides, nbDeplacementsTotal);
    afficher_plateau(plateau);

    // 3. Bilan final [cite: 20]
    if(gagne(plateau)) {
        // [cite: 21, 22, 23]
        printf("\nLa suite de deplacements %s est bien une solution pour\n", nomFichierDep);
        printf("la partie %s.\n", nomFichier);
        printf("Elle contient %d deplacements effectues.\n", nbDeplacementsValides);
    }
    else {
        printf("\nLa suite de deplacements %s N'EST PAS une solution pour\n", nomFichierDep);
        printf("la partie %s.\n", nomFichier);
    }

    return 0;
}

/*
 * Logique stricte de déplacement :
 * Majuscule -> Doit pousser (sinon rien)
 * Minuscule -> Déplacement simple (sinon rien)
 */
void tenter_deplacement(t_Plateau plateau, t_Plateau plateauInitial, int *nbDeplacements, char action) {
    int sokobanY = 0, sokobanX = 0;
    trouver_sokoban(plateau, &sokobanY, &sokobanX);

    int dy = 0, dx = 0;
    char direction = tolower(action); // On normalise pour trouver la direction
    bool demande_poussee = isupper(action); // Vrai si 'G', 'D', etc.

    // Mapping h/b/g/d (haut, bas, gauche, droite) vers coordonnées
    switch(direction) {
        case 'h': dy = -1; break; // Haut
        case 'b': dy = +1; break; // Bas
        case 'g': dx = -1; break; // Gauche
        case 'd': dx = +1; break; // Droite
        default: return; // Caractère inconnu
    }

    int nextY = sokobanY + dy;
    int nextX = sokobanX + dx;

    // Vérification limites
    if(nextX < 0 || nextX >= TAILLE || nextY < 0 || nextY >= TAILLE) return;

    char caseCible = plateau[nextY][nextX];

    // --- CAS 1 :  (MAJUSCULE) ---
    if (demande_poussee) {
        // Si ce n'est PAS une caisse, le mouvement est invalide selon vos règles
        if (caseCible != CAISSE && caseCible != CAISSE_CIBLE) {
            return;
        }

        // Tentative de poussée
        int afterBoxY = nextY + dy;
        int afterBoxX = nextX + dx;

        // Vérif limites derrière la caisse
        if(afterBoxX < 0 || afterBoxX >= TAILLE || afterBoxY < 0 || afterBoxY >= TAILLE) return;

        char derriereCaisse = plateau[afterBoxY][afterBoxX];

        if (derriereCaisse == VIDE || derriereCaisse == CIBLE) {
            // Déplacement de la caisse
            if(derriereCaisse == CIBLE) plateau[afterBoxY][afterBoxX] = CAISSE_CIBLE;
            else plateau[afterBoxY][afterBoxX] = CAISSE;

            // Déplacement du Sokoban
            restaurer_position_sokoban(plateau, plateauInitial, sokobanY, sokobanX);
            placer_sokoban(plateau, plateauInitial, nextY, nextX);

            (*nbDeplacements)++;
        }
    }
    // --- CAS 2 :  (MINUSCULE) ---
    else {
        // Si c'est une caisse, le mouvement est invalide (car minuscule = pas de poussée)
        if (caseCible == CAISSE || caseCible == CAISSE_CIBLE) {
            return;
        }

        // Si c'est un mur
        if (caseCible == MUR) {
            return;
        }

        // Déplacement libre
        if (caseCible == VIDE || caseCible == CIBLE) {
            restaurer_position_sokoban(plateau, plateauInitial, sokobanY, sokobanX);
            placer_sokoban(plateau, plateauInitial, nextY, nextX);
            (*nbDeplacements)++;
        }
    }
}

/* Affiche l'entête avec l'indication du prochain coup */
void afficher_entete(t_tabDeplacement tab, char nomFichier[], int nbDeplacements, int index_actuel) {
    printf("====================================================\n");
    printf("              SOKOBAN AUTONOME\n");
    printf("====================================================\n");
    printf(" Fichier : %s\n", nomFichier);
    printf(" Mouvements valides : %d\n", nbDeplacements);
    printf("----------------------------------------------------\n");
    printf(" Prochain(s) : ");
    // Affiche les 10 prochains caractères pour voir ce qui arrive
    for(int k=0; k<15 && (index_actuel+k) < TAILLE_DEPLACEMENT && tab[index_actuel+k] != '\0'; k++){
        if(k==0) printf("[%c] ", tab[index_actuel+k]); // Le coup actuel entre crochets
        else printf("%c", tab[index_actuel+k]);
    }
    printf("\n====================================================\n");
}

/* --- Fonctions inchangées ou utilitaires --- */

void afficher_plateau(t_Plateau plateau) {
    for(int i = 0; i < TAILLE; i++) {
        for(int j = 0; j < TAILLE; j++) {
            char s = plateau[i][j];
            if(s == CAISSE_CIBLE) s = CAISSE;       // Affichage visuel simplifié
            else if(s == SOKOBAN_CIBLE) s = SOKOBAN;
            printf("%c", s);
        }
        printf("\n");
    }
}

void trouver_sokoban(t_Plateau plateau, int *y, int *x) {
    for(int i = 0; i < TAILLE; i++) {
        for(int j = 0; j < TAILLE; j++) {
            if(plateau[i][j] == SOKOBAN || plateau[i][j] == SOKOBAN_CIBLE) {
                *y = i; *x = j; return;
            }
        }
    }
}

bool est_sur_cible(t_Plateau plateau_initial, int y, int x) {
    char symbole = plateau_initial[y][x];
    return (symbole == CIBLE || symbole == CAISSE_CIBLE || symbole == SOKOBAN_CIBLE);
}

void restaurer_position_sokoban(t_Plateau plateau, t_Plateau plateauInitial, int sokobanY, int sokobanX) {
    if(est_sur_cible(plateauInitial, sokobanY, sokobanX)) plateau[sokobanY][sokobanX] = CIBLE;
    else plateau[sokobanY][sokobanX] = VIDE;
}

void placer_sokoban(t_Plateau plateau, t_Plateau plateauInitial, int y, int x) {
    if(est_sur_cible(plateauInitial, y, x)) plateau[y][x] = SOKOBAN_CIBLE;
    else plateau[y][x] = SOKOBAN;
}

bool gagne(t_Plateau plateau) {
    for(int i = 0; i < TAILLE; i++) {
        for(int j = 0; j < TAILLE; j++) {
            if(plateau[i][j] == CAISSE) return false;
        }
    }
    return true;
}

void copier_plateau(t_Plateau source, t_Plateau destination) {
    for(int i = 0; i < TAILLE; i++)
        for(int j = 0; j < TAILLE; j++)
            destination[i][j] = source[i][j];
}

void chargerPartie(t_Plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}



void chargerDeplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){

                t[*nb] = dep;
                (*nb)++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
}
