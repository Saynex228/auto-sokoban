/* Auteur : Tuila Abdelkarim (Modifié pour Version Autonome)
 * Date : 06/11/2025
 * Description : SAÉ 1.02 - Sokoban Autonome
 * Gestion stricte des majuscules (poussée) et minuscules (déplacement simple)
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define TAILLE_DEPLACEMENT 1000
#define TAILLE 12
#define MUR '#'
#define CAISSE '$'
#define VIDE ' '
#define CIBLE '.'
#define SOKOBAN '@'
#define CAISSE_CIBLE '*'
#define SOKOBAN_CIBLE '+'

typedef char typeDeplacements[TAILLE_DEPLACEMENT];
typedef char t_Plateau[TAILLE][TAILLE];
typedef typeDeplacements t_tabDeplacement;

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

void tenter_deplacement(t_Plateau plateau, t_Plateau plateauInitial, int *nbDeplacements, char action);

int main() {
    t_Plateau plateauInitial;
    t_Plateau plateau;
    typeDeplacements tab;
    char nomFichier[30];
    char nomFichierDep[30];

    int nbDeplacementsTotal = 0;
    int nbDeplacementsValides = 0;

    printf("Tapez le nom de la partie (ex: niveau1.sok) : ");
    if (scanf("%29s", nomFichier) != 1) return 1;

    printf("Tapez le nom des deplacements (ex: niveau1.dep) : ");
    if (scanf("%29s", nomFichierDep) != 1) return 1;

    chargerPartie(plateau, nomFichier);
    copier_plateau(plateau, plateauInitial);
    chargerDeplacements(tab, nomFichierDep, &nbDeplacementsTotal);

    for(int i = 0; i < nbDeplacementsTotal; i++){
        system("clear");

        afficher_entete(tab, nomFichier, nbDeplacementsValides, i);
        afficher_plateau(plateau);

        tenter_deplacement(plateau, plateauInitial, &nbDeplacementsValides, tab[i]);

        usleep(500000);
    }

    system("clear");
    afficher_entete(tab, nomFichier, nbDeplacementsValides, nbDeplacementsTotal);
    afficher_plateau(plateau);

    if(gagne(plateau)) {
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

void tenter_deplacement(t_Plateau plateau, t_Plateau plateauInitial, int *nbDeplacements, char action) {
    int sokobanY = 0, sokobanX = 0;
    trouver_sokoban(plateau, &sokobanY, &sokobanX);

    int dy = 0, dx = 0;
    char direction = tolower(action);
    bool demande_poussee = isupper(action);

    switch(direction) {
        case 'h': dy = -1; break;
        case 'b': dy = +1; break;
        case 'g': dx = -1; break;
        case 'd': dx = +1; break;
        default: return;
    }

    int nextY = sokobanY + dy;
    int nextX = sokobanX + dx;

    if(nextX < 0 || nextX >= TAILLE || nextY < 0 || nextY >= TAILLE) return;

    char caseCible = plateau[nextY][nextX];

    if (demande_poussee) {
        if (caseCible != CAISSE && caseCible != CAISSE_CIBLE) {
            return;
        }

        int afterBoxY = nextY + dy;
        int afterBoxX = nextX + dx;

        if(afterBoxX < 0 || afterBoxX >= TAILLE || afterBoxY < 0 || afterBoxY >= TAILLE) return;

        char derriereCaisse = plateau[afterBoxY][afterBoxX];

        if (derriereCaisse == VIDE || derriereCaisse == CIBLE) {
            if(derriereCaisse == CIBLE) plateau[afterBoxY][afterBoxX] = CAISSE_CIBLE;
            else plateau[afterBoxY][afterBoxX] = CAISSE;

            restaurer_position_sokoban(plateau, plateauInitial, sokobanY, sokobanX);
            placer_sokoban(plateau, plateauInitial, nextY, nextX);

            (*nbDeplacements)++;
        }
    }
    else {
        if (caseCible == CAISSE || caseCible == CAISSE_CIBLE) {
            return;
        }

        if (caseCible == MUR) {
            return;
        }

        if (caseCible == VIDE || caseCible == CIBLE) {
            restaurer_position_sokoban(plateau, plateauInitial, sokobanY, sokobanX);
            placer_sokoban(plateau, plateauInitial, nextY, nextX);
            (*nbDeplacements)++;
        }
    }
}

void afficher_entete(t_tabDeplacement tab, char nomFichier[], int nbDeplacements, int index_actuel) {
    printf("====================================================\n");
    printf("              SOKOBAN AUTONOME\n");
    printf("====================================================\n");
    printf(" Fichier : %s\n", nomFichier);
    printf(" Mouvements valides : %d\n", nbDeplacements);
    printf("----------------------------------------------------\n");
    printf(" Prochain(s) : ");
    for(int k=0; k<15 && (index_actuel+k) < TAILLE_DEPLACEMENT && tab[index_actuel+k] != '\0'; k++){
        if(k==0) printf("[%c] ", tab[index_actuel+k]);
        else printf("%c", tab[index_actuel+k]);
    }
    printf("\n====================================================\n");
}

void afficher_plateau(t_Plateau plateau) {
    for(int i = 0; i < TAILLE; i++) {
        for(int j = 0; j < TAILLE; j++) {
            char s = plateau[i][j];
            if(s == CAISSE_CIBLE) s = CAISSE;
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
