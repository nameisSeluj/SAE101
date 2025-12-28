/**
* @file code.c
* @brief Sokoban
* @author Jules Delapilliere
* @version version 1.0
* @date 09/11/2025
*
* Code du jeu Sokoban en C, en mode non graphique, dans le cadre de la SAE 1.01
*
*/

/* Fichiers inclus */
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

/* Définition de constante*/
#define TAILLE 12
#define DEP 1000
#define VIDE ' '
#define SOKOBAN '@'
#define CIBLE '.'
#define SOKOBAN_CIBLE '+'
#define CAISSE '$'
#define CAISSE_CIBLE '*'
#define MUR '#'
#define HAUT 'z'
#define BAS 's'
#define DROITE 'd'
#define GAUCHE 'q'
#define ABANDON 'x'
#define RECOMMENCE 'r'
#define UNDO 'u'
#define ZOOM '+'
#define DE_ZOOM '-'
#define SOKO_GAUCHE 'g'
#define SOKO_DROITE 'd'
#define SOKO_HAUT 'h'
#define SOKO_BAS 'b'
#define CAISSE_GAUCHE 'G'
#define CAISSE_DROITE 'D'
#define CAISSE_HAUT 'H'
#define CAISSE_BAS 'B'
#define ZOOM_MAX 3
#define ZOOM_MIN 1

/* Définition de type*/
typedef char t_plateau[TAILLE][TAILLE];
typedef int t_tab_deplacement[DEP];

/* Définition de fonction*/
void affichage_fin(bool win, bool surrend, t_tab_deplacement depl, 
    int count, t_plateau plato);
void def_zoom(int *zoom, int coef);
void enregistrer_plateau(t_plateau plat);
void enregistrer_deplacement(t_tab_deplacement depl, int count);
void afficher_entete(char nom[20],int count);
void afficher_plateau(t_plateau plat, int zoom);
void cherche_joueur(t_plateau plat, int *x, int *y);
bool depl_case(t_plateau plat, int nextI, int nextJ, int coefX, int coefY);
void mettre_a_jour_plateau(t_plateau plat, int i, int j, int nextI,
     int nextJ, char destAvMv);
bool deter_direct_code(char direct, int *coefX, int *coefY, char *codeD);
void deplacer(t_plateau plat,t_tab_deplacement depl, char direct, 
    int *count, int *x, int *y);
void undo(t_plateau plat, t_tab_deplacement depl, int *count, int *x, int *y);
bool gagne(t_plateau plat);
bool verif_recommencer();
bool verif_abandonner();
void charger_partie(t_plateau plat, char fichier[]);
void enregistrer_partie(t_plateau plat, char fichier[]);
int kbhit();
void enregistrerDeplacements(t_tab_deplacement t, int nb, char fic[]);


/**
* @brief Entrée du programme
* @return 0 : arrêt normal du programme
* Charge une partie depuis un fichier en .sok et laisse 
* le joueur jouer jusqu'à la fin du jeu, dans une limite
* de 1000 déplacements enregistrable.
*
*/
int main() {
	t_plateau plato;
	t_tab_deplacement depl;
	char nom[20] = " ";
	char touche;
	int x = 0, y = 0;
	int count = 0;
	int zoom = 1;
	bool win = false;
    bool surrend = false;
	printf("Tappez un nom de fichier \n");
	scanf("%s", nom);
	charger_partie(plato, nom);
	cherche_joueur(plato, &x, &y);
	while ( (win != true) && (surrend != true) ) {
		afficher_entete(nom, count);
		afficher_plateau(plato, zoom);
		while (kbhit()==0)
		{}
		touche = getchar();
		switch (touche) {
			case ABANDON:
				if ( verif_abandonner() ) {
                    surrend = true;
                }
				break;
			case HAUT:
				deplacer(plato, depl, HAUT, &count, &x, &y);
				break;
			case GAUCHE:
				deplacer(plato, depl, GAUCHE, &count, &x, &y);
				break;
			case BAS:
				deplacer(plato, depl, BAS, &count, &x, &y);
				break;
			case DROITE:
				deplacer(plato, depl, DROITE, &count, &x, &y);
				break;
			case RECOMMENCE:
                if ( verif_recommencer() ){
                    count = 0;
				    charger_partie(plato, nom);
				    cherche_joueur(plato, &x, &y);
                }
				break;
			case UNDO:
				undo(plato, depl, &count, &x, &y);
				break;
			case ZOOM:
				def_zoom(&zoom, 1);
				break;
			case DE_ZOOM:
				def_zoom(&zoom, -1);
				break;
		}
		system("clear");
		win = gagne(plato);
	}
	affichage_fin(win, surrend, depl, count, plato);
	return 0;
}


/**
* @brief Gère l'affichage de la défaite/victoire de l'utilisateur
* @param win de type bool : true si l'utilisateur a gagné
* @param surrend de type bool : true si l'utilisateur a abandonné
* @param depl de type t_tab_deplacement : le tableau des déplacements
* @param count de type int : le nombre de déplacements
*/
void affichage_fin(bool win, bool surrend, t_tab_deplacement depl, 
    int count, t_plateau plato) {
	if ( win ) {
		printf("Félicitation, vous avez gagnez ! \n");
        enregistrer_deplacement(depl, count);
	} else if ( surrend ) {
        enregistrer_plateau(plato);
		enregistrer_deplacement(depl, count);
		printf("Dommage, vous ferez mieux la prochaine fois !\n");
	}
}


/**
* @brief Vérifie si le changement et possible puis l'effectue
* @param zoom de type int : le niveau de zoom actuel
* @param coef de type int : le changement de zoom
*/
void def_zoom(int *zoom, int coef) {
    if ( ( *zoom < ZOOM_MAX ) && ( coef == 1 ) ) {
        *zoom += coef;
    } else if ( ( *zoom > ZOOM_MIN ) && ( coef == -1 ) ) {
        *zoom += coef;
    }
}


/**
* @brief Propose et effectue si nécéssaire l'enregistrement de la partie
* @param plat de type t_plateau : le plateau de jeu
*/
void enregistrer_plateau(t_plateau plat) {
	char nom[20] = " ";
	char action;
	printf("Voulez vous enregistrez la partie ? (y/n) \n");
	scanf(" %c", &action);
	if (action == 'y') {
		printf("Veuillez nommez votre fichier \n");
		scanf("%s", nom);
		enregistrer_partie(plat, nom);
	}
}


/**
* @brief Propose et effectue si nécéssaire l'enregistrement des déplacements
* @param depl de type t_tab_deplacement : la liste des déplacements
* @param count de type int : le nombre de coups joués
*/
void enregistrer_deplacement(t_tab_deplacement depl, int count) {
	char nom[20] = " ";
	char action;
	printf("Voulez vous enregistrez vos déplacements ? (y/n) \n");
	scanf(" %c", &action);
	if (action == 'y') {
		printf("Veuillez nommez votre fichier \n");
		scanf("%s", nom);
		enregistrerDeplacements(depl, count, nom);
	}
}


/**
* @brief Affiche l'entête de jeu
* @param nom de type char : le nom du fichier
* @param count de type int : le nombre de coups joués
*/
void afficher_entete(char nom[20],int count) {
	printf("===== ENTETE =====\n");
	printf("Partie : %s \n", nom);
	printf("zqsd : déplacements\n");
	printf("x : abandon \n");
	printf("r : recommencer\n");
	printf("u : annuler le déplacement\n");
	printf("+ : Zoom le jeu\n");
	printf("- : Dé-zoom le jeu\n");
	printf("----- %d déplacements effectués ----- \n", count);
	printf("==================\n");
}


/**
* @brief Affiche le plateau de jeu avec un niveau de zoom
* @param plat de type t_plateau : le plateau de jeu
* @param niveauZoom de type int : le niveau de zoom (1 à 3)
*/
void afficher_plateau(t_plateau plat, int niveauZoom) {
    char caractereAAfficher;

    // Boucle sur les lignes du plateau (i)
    for (int i = 0; i < TAILLE; i++) {

        // Boucle pour le zoom vertical (z_i)
        for (int z_i = 0; z_i < niveauZoom; z_i++) {

            // Boucle sur les colonnes du plateau (j)
            for (int j = 0; j < TAILLE; j++) {
                
                // Déterminer le caractère à afficher
                if (plat[i][j] == CAISSE_CIBLE) {
                    caractereAAfficher = CAISSE;
                } else if (plat[i][j] == SOKOBAN_CIBLE) {
                    caractereAAfficher = SOKOBAN;
                } else {
                    caractereAAfficher = plat[i][j];
                }

                // Boucle pour le zoom horizontal (z_j)
                for (int z_j = 0; z_j < niveauZoom; z_j++) {
                    printf("%c", caractereAAfficher);
                }
            }
            // Nouvelle ligne pour le zoom vertical
            printf("\n");
        }
    }
}


/**
* @brief Recherche et initialise les coordonnées du joueur
* @param plat de type t_plateau : le plateau de jeu
* @param x de type *int : la ligne du joueur (sera modifié)
* @param y de type *int : la colonne du joueur (sera modifié)
*/
void cherche_joueur(t_plateau plat, int *x, int *y) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if ( (plat[i][j] == SOKOBAN) || (plat[i][j] == SOKOBAN_CIBLE) ) {
                *x = i;
                *y = j;
            }
        }
    }
}


/**
* @brief Tente de déplacer une caisse si possible
* @param plat de type t_plateau : le plateau de jeu
* @param nextI de type int : la ligne où se trouve la caisse (future position Sokoban)
* @param nextJ de type int : la colonne où se trouve la caisse (future position Sokoban)
* @param coefX de type int : le coefficient de déplacement sur X
* @param coefY de type int : le coefficient de déplacement sur Y
* @return true si la caisse a été déplacée, false sinon (mur, hors limite, ou autre caisse)
*/
bool depl_case(t_plateau plat, int nextI, int nextJ, int coefX, int coefY) {
    int boxI = nextI + coefX;
    int boxJ = nextJ + coefY;

    if ( boxI < 0 || boxI >= TAILLE || boxJ < 0 || 
        boxJ >= TAILLE || plat[boxI][boxJ] == MUR ) {
        return false; 
    }

    char destinationCaisse = plat[boxI][boxJ];
    
    if (destinationCaisse == VIDE || destinationCaisse == CIBLE) {
        if (destinationCaisse == VIDE) {
            plat[boxI][boxJ] = CAISSE;
        } else {
            plat[boxI][boxJ] = CAISSE_CIBLE;
        }
        return true; 
    }

    return false; 
}


/**
* @brief Met à jour le plateau de jeu après un déplacement réussi du Sokoban
* @param plat de type t_plateau : le plateau de jeu
* @param i de type int : l'ancienne ligne du Sokoban
* @param j de type int : l'ancienne colonne du Sokoban
* @param nextI de type int : la nouvelle ligne du Sokoban
* @param nextJ de type int : la nouvelle colonne du Sokoban
* @param destAvMv de type char : le contenu de la case (nextI, nextJ) avant le déplacement
*/
void mettre_a_jour_plateau(t_plateau plat, int i, int j, int nextI,
     int nextJ, char destAvMv) {
    if (plat[i][j] == SOKOBAN_CIBLE) {
        plat[i][j] = CIBLE;
    } else {
        plat[i][j] = VIDE;
    }

    if (destAvMv == CIBLE || destAvMv == CAISSE_CIBLE) {
        plat[nextI][nextJ] = SOKOBAN_CIBLE;
    } else { 
        plat[nextI][nextJ] = SOKOBAN;
    }
}


/**
* @brief Détermine les coefficients et le code pour la direction donnée
* @param direct de type char : direction
* @param coefX de type *int : coefficient de déplacement ligne 
* @param coefY de type *int : coefficient de déplacement colonne 
* @param codeD de type *char : code du déplacement Sokoban simple
* @return true si la direction est reconnue, false sinon
*/
bool deter_direct_code(char direct, int *coefX, int *coefY, char *codeD) {
    bool var = false;
    switch (direct) {
        case HAUT:
            *coefX = -1; 
            *codeD = SOKO_HAUT;
            var = true;
            break;
        case GAUCHE:
            *coefY = -1;
            *codeD = SOKO_GAUCHE;
            var = true;
            break;
        case BAS:
            *coefX = 1;
            *codeD = SOKO_BAS;
            var = true;
            break;
        case DROITE:
            *coefY = 1;
            *codeD = SOKO_DROITE;
            var = true;
            break;
    }
    return var;
}


/**
* @brief Gère la globalité du déplacement du Sokoban
* @param plat de type t_plateau : le plateau de jeu
* @param depl de type t_tab_deplacement : la liste des déplacements
* @param direct de type char : la direction du déplacement
* @param count de type *int : le compteur de coups
* @param x de type *int : la ligne du joueur
* @param y de type *int : la colonne du joueur
*/
void deplacer(t_plateau plat, t_tab_deplacement depl, char direct, 
              int *count, int *x, int *y) {
    int coefX = 0;
    int coefY = 0;
    char codeD = ' '; 
    bool mouvementValide;
    bool pousseeReussie = true;
    int i = *x; 
    int j = *y;
    int nextI;
    int nextJ;
    char caseSuivante;

    mouvementValide = deter_direct_code(direct, &coefX, &coefY, &codeD); 

    if (mouvementValide) {

        nextI = i + coefX;
        nextJ = j + coefY;

        if ( (nextI < 0) || (nextI >= TAILLE) || (nextJ < 0) || 
        (nextJ >= TAILLE) || (plat[nextI][nextJ] == MUR) ) { 
            mouvementValide = false; 
        } else {
            caseSuivante = plat[nextI][nextJ];

            if ( (caseSuivante == CAISSE) || (caseSuivante == CAISSE_CIBLE) ) {
                pousseeReussie = depl_case(plat, nextI, nextJ, coefX, coefY);
                
                if (!pousseeReussie) {
                    mouvementValide = false;

                } else {
                    if (codeD == SOKO_HAUT) { codeD = CAISSE_HAUT; }
                    else if (codeD == SOKO_BAS) { codeD = CAISSE_BAS; }
                    else if (codeD == SOKO_GAUCHE) { codeD = CAISSE_GAUCHE; }
                    else { codeD = CAISSE_DROITE; }
                }
            }

            if (mouvementValide) {
                mettre_a_jour_plateau(plat, i, j, nextI, nextJ, caseSuivante);
                depl[*count] = codeD;
                (*count)++;
                *x = nextI;
                *y = nextJ;
            }
        }
    }
}


/**
* @brief Effectue l'annulation du dernier déplacement
* @param plat de type t_plateau : le plateau de jeu
* @param depl de type t_tab_deplacement : la liste des anciens déplacements
* @param count de type *int : le compteur de coups
* @param x de type *int : coordonnee x actuelle du joueur
* @param y de type *int : coordonnee y actuelle du joueur
*/
void undo(t_plateau plat, t_tab_deplacement depl, int *count, int *x, int *y) {
    if (*count <= 0) {
        return; 
    }
    char lastDep = depl[*count - 1];
    int coefX = 0;
    int coefY = 0;
    switch (lastDep) {
        case SOKO_HAUT: case CAISSE_HAUT:
            coefX = 1; 
            break;
        case SOKO_BAS: case CAISSE_BAS:
            coefX = -1; 
            break;
        case SOKO_GAUCHE: case CAISSE_GAUCHE:
            coefY = 1; 
            break;
        case SOKO_DROITE: case CAISSE_DROITE:
            coefY = -1; 
            break;
    }
    int nextX = *x + coefX;
    int nextY = *y + coefY;
    if ( (lastDep == CAISSE_HAUT) || (lastDep == CAISSE_BAS) || 
         (lastDep == CAISSE_GAUCHE) || (lastDep == CAISSE_DROITE) ) {
        int beforeX = *x - coefX;
        int beforeY = *y - coefY;
        if (plat[beforeX][beforeY] == CAISSE_CIBLE) {
            plat[beforeX][beforeY] = CIBLE;
        } else {
            plat[beforeX][beforeY] = VIDE;
        }
        if (plat[*x][*y] == SOKOBAN_CIBLE) {
            plat[*x][*y] = CAISSE_CIBLE;
        } else {
            plat[*x][*y] = CAISSE;
        }
    } else {
        if (plat[*x][*y] == SOKOBAN_CIBLE) {
            plat[*x][*y] = CIBLE;
        } else {
            plat[*x][*y] = VIDE;
        }
    }
    if (plat[nextX][nextY] == CIBLE) {
        plat[nextX][nextY] = SOKOBAN_CIBLE;
    } else {
        plat[nextX][nextY] = SOKOBAN;
    }
    (*count)--;
    *x = nextX;
    *y = nextY;
}


/**
* @brief Vérifie si le joueur a gagné ou non
* @param plat de type t_plateau : le plateau de jeu
* @return un booléen : true si gagné et false sinon
*/
bool gagne(t_plateau plat) {
	bool result = true;
	for(int i=0; i < TAILLE; i++){
		for(int j=0; j < TAILLE; j++){
			if((plat[i][j] == CIBLE)||(plat[i][j] == SOKOBAN_CIBLE)) {
				result = false;
			}
		}
	}
	return result;
}


/**
* @brief Demande au joueur s'il veut vraiment recommencer la partie
* @return un booléen : true si l'utilisateur valide, false sinon
*/
bool verif_recommencer() {
    bool res = false;
    char action = ' ';
    printf("Êtes vous sûr de vouloir recommencer ? (y/n) \n");
    scanf(" %c", &action);
    if ( action == 'y' ) {
        res = true;
    }
    return res;
}


/**
* @brief Demande au joueur s'il veut vraiment abandonner la partie
* @return un booléen : true si l'utilisateur valide, false sinon
*/
bool verif_abandonner() {
    bool res = false;
    char action = ' ';
    printf("Êtes vous sûr de vouloir abandonner ? (y/n) \n");
    scanf(" %c", &action);
    if ( action == 'y' ) {
        res = true;
    }
    return res;
}


// Code fourni


void charger_partie(t_plateau plateau, char fichier[]){
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


void enregistrer_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne='\n';

    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}


int kbhit(){
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}


void enregistrerDeplacements(t_tab_deplacement t, int nb, char fic[]){
    FILE * f;

    f = fopen(fic, "w");
    fwrite(t,sizeof(char), nb, f);
    fclose(f);
}