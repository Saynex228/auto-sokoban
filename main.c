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

/**
 * @brief Charge l'état initial du plateau de jeu à partir d'un fichier.
 * @param plateau Le tableau 2D qui recevra les données du jeu.
 * @param fichier Le nom du fichier source (.sok).
 */
void chargerPartie(t_Plateau plateau, char fichier[]);

/**
 * @brief Charge la séquence de déplacements à partir d'un fichier.
 * @param t Le tableau où la séquence de déplacements sera stockée.
 * @param fichier Le nom du fichier source (.dep).
 * @param nb Pointeur vers le nombre total de déplacements lus.
 */
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);

/**
 * @brief Affiche l'entête de la simulation (nom du fichier, compteurs, prochains mouvements).
 * @param tab Le tableau des déplacements.
 * @param nomFichier Le nom du fichier de partie.
 * @param nbDeplacements Le nombre de mouvements valides effectués.
 * @param index_actuel L'index du mouvement en cours.
 */
void afficher_entete(t_tabDeplacement tab, char nomFichier[], int nbDeplacements, int index_actuel);

/**
 * @brief Affiche l'état actuel du plateau.
 * @param plateau Le tableau 2D représentant l'état du jeu.
 */
void afficher_plateau(t_Plateau plateau);

/**
 * @brief Recherche les coordonnées du Sokoban.
 * @param plateau Le tableau 2D du jeu.
 * @param y Pointeur pour stocker la ligne du Sokoban.
 * @param x Pointeur pour stocker la colonne du Sokoban.
 */
void trouver_sokoban(t_Plateau plateau, int *y, int *x);

/**
 * @brief Vérifie si la position donnée était initialement une cible.
 * @param plateau_initial L'état initial du plateau (pour vérifier les cibles fixes).
 * @param y Ligne à vérifier.
 * @param x Colonne à vérifier.
 * @return Vrai si la position est une cible.
 **/
bool est_sur_cible(t_Plateau plateau_initial, int y, int x);

/**
 * @brief Restaure la case où se trouvait Sokoban (VIDE ou CIBLE).
 * @param plateau L'état actuel du plateau.
 * @param plateauInitial L'état initial du plateau.
 * @param sokobanY Ligne de l'ancienne position de Sokoban.
 * @param sokobanX Colonne de l'ancienne position de Sokoban.
 */
void restaurer_position_sokoban(t_Plateau plateau, t_Plateau plateauInitial, int sokobanY, int sokobanX);

/**
 * @brief Place le Sokoban à une nouvelle position (SOKOBAN ou SOKOBAN_CIBLE).
 * @param plateau L'état actuel du plateau.
 * @param plateauInitial L'état initial du plateau.
 * @param y Nouvelle ligne.
 * @param x Nouvelle colonne.
 */
void placer_sokoban(t_Plateau plateau, t_Plateau plateauInitial, int y, int x);

/**
 * @brief Vérifie si la partie est gagnée (toutes les caisses sur les cibles).
 * @param plateau L'état actuel du plateau.
 * @return Vrai si la partie est gagnée.
 */
bool gagne(t_Plateau plateau);

/**
 * @brief Copie un plateau dans un autre.
 * @param source Le plateau source.
 * @param destination Le plateau de destination.
 */
void copier_plateau(t_Plateau source, t_Plateau destination);

/**
 * @brief Tente d'effectuer un déplacement.
 * Gère le mouvement simple et la poussée de caisse.
 * @param plateau L'état actuel du plateau.
 * @param plateauInitial L'état initial (pour les cibles).
 * @param nbDeplacements Pointeur vers le compteur de mouvements valides.
 * @param action Le caractère de l'action (h/b/g/d pour simple, H/B/G/D pour pousser).
 */
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
    scanf("%s", nomFichier);

    printf("Tapez le nom des deplacements (ex: niveau1.dep) : ");
    scanf("%s", nomFichierDep);

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
    printf(" Mouvements : %d\n", nbDeplacements);
    printf("----------------------------------------------------\n");
    printf(" Prochain : ");
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
