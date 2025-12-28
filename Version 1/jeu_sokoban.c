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
#define VIDE ' '
#define SOKOBAN '@'
#define CIBLE '.'
#define SOKOBAN_CIBLE '+'
#define CAISSE '$'
#define CAISSE_CIBLE '*'
#define HAUT 'z'
#define BAS 's'
#define DROITE 'd'
#define GAUCHE 'q'
#define ABANDON 'x'
#define RECOMMENCE 'r'

/* Définition de type*/
typedef int t_plateau[TAILLE][TAILLE];

/* Définition de fonction*/
void afficher_entete(char nom[20],int count);
void afficher_plateau(t_plateau plateau);
void cherche_joueur(t_plateau plateau, int *x, int *y);
void moov_normal_vide(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_normal_boite(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_normal_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_normal_boite_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_cible_vide(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_cible_boite(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_cible_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void moov_cible_boite_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y);
void deplacer(t_plateau plateau, char direction, int *count);
bool gagne(t_plateau plateau);
void charger_partie(t_plateau plateau, char fichier[]);
void enregistrer_partie(t_plateau plateau, char fichier[]);
int kbhit();


/**
* @brief Entrée du programme
* @return 0 : arrêt normal du programme
* Charge une partie depuis un fichier en .sok et laisse 
* le joueur jouer jusqu'à la fin du jeu
*
*/
int main() {
	t_plateau plato;
	char nom[20] = " ", touche, action;
	int count = 0;
	bool win = false, surend = false;
	printf("Tappez un nom de fichier \n");
	scanf("%s", nom);
	charger_partie(plato, nom);
	while ((win != true) && (surend != true)) {
		afficher_entete(nom, count);
		afficher_plateau(plato);
		while (kbhit()==0) 
		{}
		touche = getchar();
		switch (touche) {
			case 'x':
				//Demander à l'utilisateur s'il veut enregistrer
				printf("Voulez vous enregistrez ? (y/n) \n");
				scanf("%c", &action);

				//Enregistrement si Oui
				if (action == 'y') {
					printf("Veuillez nommez votre fichier \n");
					scanf("%s", nom);
					enregistrer_partie(plato, nom);
				}

				surend = true;
				break;
			case HAUT:
				//déplacement vers le haut
				deplacer(plato, HAUT, &count);
				break;
			case GAUCHE:
				//déplacement vers la gauche
				deplacer(plato, GAUCHE, &count);
				break;
			case BAS:
				//déplacement vers le bas
				deplacer(plato, BAS, &count);
				break;
			case DROITE:
				//déplacement vers la droite
				deplacer(plato, DROITE, &count);
				break;
			case RECOMMENCE:
				//recommence le niveau
				count = 0;
				charger_partie(plato, nom);
				break;
		}
		system("clear");
		win = gagne(plato);
	}

	if (win == true) {
		printf("Félicitation, vous avez gagnez !");
	} else {
		printf("Dommage, vous ferez mieux la prochaine fois");
	}

	return 0;
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
	printf("----- %d déplacements effectués ----- \n", count);
	printf("==================\n");
}


/**
* @brief Affiche le plateau de jeu
* @param plateau de type t_plateau : le plateau de jeu
*/
void afficher_plateau(t_plateau plateau) {
	for(int i=0; i < TAILLE; i++){
		for(int j=0; j < TAILLE; j++){
			if (plateau[i][j] == SOKOBAN_CIBLE) {
				printf("%c", SOKOBAN);
			} else if (plateau[i][j] == CAISSE_CIBLE) {
				printf("%c", CAISSE);
			} else {
				printf("%c", plateau[i][j]);
			}
		}
		printf("\n");
	}
}


/**
* @brief Cherche le joueur
* @param plateau de type t_plateau : le plateau de jeu
* @param x de type *int : la première coordonnée du joueur
* @param y de type *int : la deuxième coordonnée du joueur
*/
void cherche_joueur(t_plateau plateau, int *x, int *y) {
	for(int i=0; i < TAILLE; i++){
		for(int j=0; j < TAILLE; j++){
			if((plateau[i][j] == SOKOBAN)||(plateau[i][j] == SOKOBAN_CIBLE)) {
				*x = i;
				*y = j;
			}
		}
	}
}


/**
* @brief Effectue le déplacement du Sokoban normal vers une case vide
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_normal_vide(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	plateau[i + coef_x][j + coef_y] = SOKOBAN;
	plateau[i][j] = VIDE;
}


/**
* @brief Effectue le déplacement du Sokoban normal poussant une boite
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_normal_boite(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	if(plateau[i + (coef_x*2)][j + (coef_y*2)] == VIDE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE;
		moov_normal_vide(plateau, i, j, coef_x, coef_y);
	} else if(plateau[i + (coef_x*2)][j + (coef_y*2)] == CIBLE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE_CIBLE;
		moov_normal_vide(plateau, i, j, coef_x, coef_y);
	}
}


/**
* @brief Effectue le déplacement du Sokoban normal vers une cible
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_normal_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	plateau[i + coef_x][j + coef_y] = SOKOBAN_CIBLE;
	plateau[i][j] = VIDE;
}


/**
* @brief Effectue le déplacement du Sokoban normal poussant une boite en dehors d'une cible
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_normal_boite_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	if(plateau[i + (coef_x*2)][j + (coef_y*2)] == VIDE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE;
		plateau[i + coef_x][j + coef_y] = SOKOBAN_CIBLE;
		plateau[i][j] = VIDE;
	} else if(plateau[i + (coef_x*2)][j + (coef_y*2)] == CIBLE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE_CIBLE;
		plateau[i + coef_x][j + coef_y] = SOKOBAN_CIBLE;
		plateau[i][j] = VIDE;
	}
}


/**
* @brief Effectue le déplacement du Sokoban sur cible vers une case vide
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_cible_vide(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	plateau[i + coef_x][j + coef_y] = SOKOBAN;
	plateau[i][j] = CIBLE;
}


/**
* @brief Effectue le déplacement du Sokoban sur cible qui pousse une caisse
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_cible_boite(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	if(plateau[i + (coef_x*2)][j + (coef_y*2)] == VIDE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE;
		moov_cible_vide(plateau, i, j, coef_x, coef_y);
	} else if(plateau[i + (coef_x*2)][j + (coef_y*2)] == CIBLE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE_CIBLE;
		moov_cible_vide(plateau, i, j, coef_x, coef_y);
	}
}


/**
* @brief Effectue le déplacement du Sokoban sur cible vers une autre cible
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_cible_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	plateau[i + coef_x][j + coef_y] = SOKOBAN_CIBLE;
	plateau[i][j] = CIBLE;
}


/**
* @brief Effectue le déplacement du Sokoban sur cible qui pousse une caisse étant sur une cible
* @param plateau de type t_plateau : le plateau de jeu
* @param i de type int : la première coordonnée du joueur
* @param j de type int : la deuxième coordonnée du joueur
* @param coef_x de type int : le coefficient de déplacement sur x
* @param coef_y de type int : le coefficient de déplacement sur y
*/
void moov_cible_boite_cible(t_plateau plateau, int i, int j, int coef_x, int coef_y) {
	if(plateau[i + (coef_x*2)][j + (coef_y*2)] == VIDE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE;
		moov_cible_cible(plateau, i, j, coef_x, coef_y);
	} else if(plateau[i + (coef_x*2)][j + (coef_y*2)] == CIBLE) {
		plateau[i + (coef_x*2)][j + (coef_y*2)] = CAISSE_CIBLE;
		moov_cible_cible(plateau, i, j, coef_x, coef_y);
	}
}


/**
* @brief Gère la globalité du déplacement et de la direction
* @param plateau de type t_plateau : le plateau de jeu
* @param direction de type char : la direction du déplacement
*/
void deplacer(t_plateau plateau, char direction, int *count) {
	int coef_x = 0, coef_y = 0;

	//On récupère la position du joueur
	int i , j;
	cherche_joueur(plateau, &i, &j);
	

	//On défini la direction
	switch (direction) {
		case 'z':
			coef_x = -1;
			coef_y = 0;
			break;
		case 'q':
			coef_x = 0;
			coef_y = -1;
			break;
		case 's':
			coef_x = 1;
			coef_y = 0;
			break;
		case 'd':
			coef_x = 0;
			coef_y = 1;
			break;
	}

	if (plateau[i][j] == SOKOBAN) {
		switch(plateau[i + coef_x][j + coef_y]) {
			case VIDE:
				moov_normal_vide(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
			case CAISSE:
				moov_normal_boite(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
			case CIBLE:
				moov_normal_cible(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
			case CAISSE_CIBLE:
				moov_normal_boite_cible(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
		}
	}else {
		switch(plateau[i + coef_x][j + coef_y]) {
			case CAISSE:
				moov_cible_boite(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
			case VIDE:
				moov_cible_vide(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
			case CIBLE:
				moov_cible_cible(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
			case CAISSE_CIBLE:
				moov_cible_boite_cible(plateau, i, j, coef_x, coef_y);
				(*count)++;
				break;
		}
	}
}


/**
* @brief Vérifie si le joueur a gagné ou non
* @param plateau de type t_plateau : le plateau de jeu
* @return un booléen : true si gagné et false sinon
*/
bool gagne(t_plateau plateau) {
	bool result = true;
	for(int i=0; i < TAILLE; i++){
		for(int j=0; j < TAILLE; j++){
			if((plateau[i][j] == CIBLE)||(plateau[i][j] == SOKOBAN_CIBLE)) {
				result = false;
			}
		}
	}
	return result;
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
