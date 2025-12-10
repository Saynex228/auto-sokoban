/* Auteur : Tuila Abdelkarim
 * Date : 06/11/2025
 * Description : Jeu Sokoban avec zoom, historique et annulation
 * Version : 2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

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

/* alias pour historique (в вашем коде использовалось t_tabDeplacement) */
typedef typeDeplacements t_tabDeplacement;

/* Les fonctions fournies par l'université (signatures conservées) */
void chargerPartie(t_Plateau plateau, char fichier[]);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);

/* Les fonctions obligatoires (signatures harmonisées avec définitions) */
void afficher_entete(t_tabDeplacement tab, char nomFichier[],
                     int nbDeplacements);
void afficher_plateau(t_Plateau plateau);
void deplacer(typeDeplacements tab, t_Plateau plateau,
              t_Plateau plateauInitial, int *nbDeplacements);
bool gagne(t_Plateau plateau);

/* Les fonctions auxiliaires */
void copier_plateau(t_Plateau source, t_Plateau destination);
void trouver_sokoban(t_Plateau plateau, int *y, int *x);
bool est_sur_cible(t_Plateau plateau_initial, int y, int x);
void traiter_victoire(t_Plateau plateau, int nbDeplacements, char nomFichier[], char nomFichierDep[]);
void deplacer_simple(t_Plateau plateau, t_Plateau plateauInitial,
                     int sokobanY, int sokobanX, int nouvelleY,
                     int nouvelleX, char destination);
void deplacer_caisse(t_Plateau plateau, t_Plateau plateauInitial,
                     int sokobanY, int sokobanX, int nouvelleY,
                     int nouvelleX, int dy, int dx, char destination);
void restaurer_position_sokoban(t_Plateau plateau, t_Plateau plateauInitial,
                                int sokobanY, int sokobanX);
void placer_sokoban(t_Plateau plateau, t_Plateau plateauInitial,
                    int y, int x);

/*
 * Fonction principale du jeu Sokoban
 * Gère la boucle de jeu et les interactions utilisateur
 */
int main(void) {
    t_Plateau plateauInitial;
    t_Plateau plateau;
    typeDeplacements tab;
    char nomFichier[256];
    char nomFichierDep[256];
    int nbDeplacementsTotal = 0;
    int nbDeplacements = 0;

    printf("Tapez un nom de fichier : ");
    if (scanf("%255s", nomFichier) != 1) return 1;

    printf("Tapez un nom de fichier Deplacement : ");
    if (scanf("%255s", nomFichierDep) != 1) return 1;

    chargerPartie(plateau, nomFichier);
    copier_plateau(plateau, plateauInitial);
    chargerDeplacements(tab, nomFichierDep, &nbDeplacementsTotal);

    for (int i = 0; i < nbDeplacementsTotal; i++) {
        system("clear");
        afficher_entete(tab, nomFichier, nbDeplacements);
        afficher_plateau(plateau);

        /* On appelle deplacer qui prend tout le tableau et utilise l'indice nbDeplacements */
        deplacer(tab, plateau, plateauInitial, &nbDeplacements);
    }

    traiter_victoire(plateau, nbDeplacements, nomFichier, nomFichierDep);
    return 0;
}

/*
 * Affiche l'en-tete du jeu avec les informations et les commandes
 */
void afficher_entete(t_tabDeplacement tab, char nomFichier[],
                     int nbDeplacements) {
    printf("====================================================\n");
    printf("                       SOKOBAN\n");
    printf("====================================================\n");
    printf("                    Partie : %s\n", nomFichier);
    printf("                    Deplacements : %d\n", nbDeplacements);
    printf("----------------------------------------------------\n");
    printf("Histoire des deplacements: ");
    for (int i = 0; i < nbDeplacements; i++) {
        printf(" %c", tab[i]);
    }
    printf("\n====================================================\n");
}

/*
 * Affiche le plateau de jeu
 */
void afficher_plateau(t_Plateau plateau) {
    char symbole;
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            symbole = plateau[i][j];
            if (symbole == CAISSE_CIBLE) {
                symbole = CAISSE;
            } else if (symbole == SOKOBAN_CIBLE) {
                symbole = SOKOBAN;
            }
            printf("%c", symbole);
        }
        printf("\n");
    }
    printf("\n");
    printf("Oublie pas de fermer ta session \n");
}

/*
 * Trouve la position du Sokoban sur le plateau
 */
void trouver_sokoban(t_Plateau plateau, int *y, int *x) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (plateau[i][j] == SOKOBAN ||
                plateau[i][j] == SOKOBAN_CIBLE) {
                *y = i;
                *x = j;
                return;
            }
        }
    }
}

/*
 * Verifie si une position correspond a une cible sur le plateau initial
 */
bool est_sur_cible(t_Plateau plateau_initial, int y, int x) {
    char symbole = plateau_initial[y][x];
    return (symbole == CIBLE || symbole == CAISSE_CIBLE ||
            symbole == SOKOBAN_CIBLE);
}

/*
 * Restaure la case de depart du Sokoban apres deplacement
 */
void restaurer_position_sokoban(t_Plateau plateau, t_Plateau plateauInitial,
                                int sokobanY, int sokobanX) {
    if (est_sur_cible(plateauInitial, sokobanY, sokobanX)) {
        plateau[sokobanY][sokobanX] = CIBLE;
    } else {
        plateau[sokobanY][sokobanX] = VIDE;
    }
}

/*
 * Place le Sokoban a une nouvelle position
 */
void placer_sokoban(t_Plateau plateau, t_Plateau plateauInitial,
                    int y, int x) {
    if (est_sur_cible(plateauInitial, y, x)) {
        plateau[y][x] = SOKOBAN_CIBLE;
    } else {
        plateau[y][x] = SOKOBAN;
    }
}

/*
 * Deplace le Sokoban seul sans caisse
 */
void deplacer_simple(t_Plateau plateau, t_Plateau plateauInitial,
                     int sokobanY, int sokobanX, int nouvelleY,
                     int nouvelleX, char destination) {
    (void)destination; /* paramètre non utilisé, suppression предупреждения */
    restaurer_position_sokoban(plateau, plateauInitial, sokobanY, sokobanX);
    placer_sokoban(plateau, plateauInitial, nouvelleY, nouvelleX);
}

/*
 * Deplace le Sokoban et une caisse
 */
void deplacer_caisse(t_Plateau plateau, t_Plateau plateauInitial,
                     int sokobanY, int sokobanX, int nouvelleY,
                     int nouvelleX, int dy, int dx, char destination) {
    int nouvelleCaisseY = nouvelleY + dy;
    int nouvelleCaisseX = nouvelleX + dx;

    if (nouvelleCaisseX < 0 || nouvelleCaisseX >= TAILLE ||
        nouvelleCaisseY < 0 || nouvelleCaisseY >= TAILLE) {
        return;
    }

    char derriereCaisse = plateau[nouvelleCaisseY][nouvelleCaisseX];

    if (derriereCaisse == VIDE || derriereCaisse == CIBLE) {
        if (derriereCaisse == CIBLE) {
            plateau[nouvelleCaisseY][nouvelleCaisseX] = CAISSE_CIBLE;
        } else {
            plateau[nouvelleCaisseY][nouvelleCaisseX] = CAISSE;
        }

        placer_sokoban(plateau, plateauInitial, nouvelleY, nouvelleX);
        restaurer_position_sokoban(plateau, plateauInitial,
                                   sokobanY, sokobanX);
    }
    (void)destination; /* suppression предупреждения о неиспользуемом параметре */
}

/*
 * Deplace le Sokoban selon la touche pressee et memorise le deplacement
 * Теперь эта функция использует массив deplacements et индекс nbDeplacements.
 */
void deplacer(typeDeplacements tab, t_Plateau plateau,
              t_Plateau plateauInitial, int *nbDeplacements) {
    if (*nbDeplacements >= TAILLE_DEPLACEMENT) return;

    int sokobanY = 0, sokobanX = 0;
    trouver_sokoban(plateau, &sokobanY, &sokobanX);

    char touche = tab[*nbDeplacements];
    int dx = 0;
    int dy = 0;
    switch (touche) {
        case 'q': dx = -1; break;
        case 'd': dx = +1; break;
        case 'z': dy = -1; break;
        case 's': dy = +1; break;
        default: return;
    }

    int nouvelleX = sokobanX + dx;
    int nouvelleY = sokobanY + dy;

    if (nouvelleX < 0 || nouvelleX >= TAILLE ||
        nouvelleY < 0 || nouvelleY >= TAILLE) {
        return;
    }

    char destination = plateau[nouvelleY][nouvelleX];

    if (destination == MUR) {
        return;
    } else if (destination == VIDE || destination == CIBLE) {
        deplacer_simple(plateau, plateauInitial, sokobanY, sokobanX,
                        nouvelleY, nouvelleX, destination);
        (*nbDeplacements)++;
        tab[*nbDeplacements - 1] = touche;
    } else if (destination == CAISSE || destination == CAISSE_CIBLE) {
        deplacer_caisse(plateau, plateauInitial, sokobanY, sokobanX,
                        nouvelleY, nouvelleX, dy, dx, destination);

        if (plateau[nouvelleY][nouvelleX] == SOKOBAN ||
            plateau[nouvelleY][nouvelleX] == SOKOBAN_CIBLE) {
            (*nbDeplacements)++;
            if (touche >= 'a' && touche <= 'z') {
                touche = (char)(touche - ('a' - 'A'));
            }
            tab[*nbDeplacements - 1] = touche;
        }
    }
}

/*
 * Verifie si toutes les caisses sont placees sur les cibles
 */
bool gagne(t_Plateau plateau) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (plateau[i][j] == CAISSE) {
                return false;
            }
        }
    }
    return true;
}

/*
 * Copie le contenu d'un plateau dans un autre
 */
void copier_plateau(t_Plateau source, t_Plateau destination) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            destination[i][j] = source[i][j];
        }
    }
}

/*
 * Traite la victoire du joueur et propose de sauvegarder
 */
void traiter_victoire(t_Plateau plateau, int nbDeplacements, char nomFichier[], char nomFichierDep[]) {
    if (gagne(plateau)) {
        printf("La suite de deplacements %s est bien une solution pour la partie %s.\n", nomFichierDep, nomFichier);
        printf("Elle contient %d deplacements.\n", nbDeplacements);
    } else {
        printf("\nLa suite de deplacements %s N'EST PAS une solution pour la partie %s.\n", nomFichierDep, nomFichier);
    }
}

/*
 * Fonctions de chargement/enregistrement (ajustées pour signatures correctes)
 */
void chargerPartie(t_Plateau plateau, char fichier[]) {
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL) {
        printf("ERREUR SUR FICHIER\n");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne = 0; ligne < TAILLE; ligne++) {
            for (int colonne = 0; colonne < TAILLE; colonne++) {
                if (fread(&plateau[ligne][colonne], sizeof(char), 1, f) != 1) {
                    plateau[ligne][colonne] = VIDE;
                }
            }
            /* lire fin de ligne si présent */
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrerPartie(t_Plateau plateau, char fichier[]) {
    FILE * f;
    char finDeLigne = '\n';

    f = fopen(fichier, "w");
    if (f == NULL) return;
    for (int ligne = 0; ligne < TAILLE; ligne++) {
        for (int colonne = 0; colonne < TAILLE; colonne++) {
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]) {
    FILE * f;

    f = fopen(fic, "w");
    if (f == NULL) return;
    fwrite(t, sizeof(char), nb, f);
    fclose(f);
}

void chargerDeplacements(typeDeplacements t, char fichier[], int * nb) {
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f == NULL) {
        printf("FICHIER NON TROUVE\n");
        return;
    } else {
        if (fread(&dep, sizeof(char), 1, f) != 1) {
            printf("FICHIER VIDE\n");
            fclose(f);
            return;
        } else {
            do {
                t[*nb] = dep;
                (*nb)++;
            } while (fread(&dep, sizeof(char), 1, f) == 1 && *nb < TAILLE_DEPLACEMENT);
        }
    }
    fclose(f);
}
