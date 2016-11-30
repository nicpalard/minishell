#include "Commandes_Internes.h"

#define TAILLE 256
#define ANNEE 1900

const char* jours[] = {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"};
const char* mois[] = {"janvier", "fevrier", "mars", "avril", "mai", "juin", "juillet", "aout", "septembre", "octobre", "novembre", "decembre"};

int echo_interne(char* message, int out){
	char c;
	int i = 0;
	while(i < strlen(message)){
		c = message[i];
		write(out, &c, sizeof(char));
		i++;
		}
	return EXIT_SUCCESS;
} // Fonctionne

void date_interne(int out){
	//On récupere la date actuelle
	time_t horodatage;
	horodatage = time(NULL);
	struct tm *temps;
	temps = localtime(&horodatage);
	
	char buffer[TAILLE];
	sprintf(buffer, "%s %d %s %d, %d:%d:%d (UTC)", jours[temps->tm_wday], temps->tm_mday, mois[temps->tm_mon], ANNEE + temps->tm_year, temps->tm_hour, temps->tm_min, temps->tm_sec);
	
	write(out, &buffer, sizeof(char) * strlen(buffer));
} //Fonctionne

void cd_interne(char* chemin){
	char* dir = get_current_dir_name(); //Chemin absolue du répertoire courant
  
	char* chemin_entier = malloc(strlen(dir) + strlen(chemin) +1);
	strcpy(chemin_entier, dir); //Je met le chemin absolue dans fullpath
	strcat(chemin_entier, chemin); //J'y ajoute le path

	chdir(chemin_entier); //Je m'y place
	free(chemin_entier); //Je libere la variable non utilisée
}
  

int pwd_interne(int out){
	char buffer[TAILLE];
	strcpy(buffer, get_current_dir_name());
	if(write(out, &buffer, strlen(get_current_dir_name())))
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
} //Fonctionne

void history_interne(int combien, int out){
	HIST_ENTRY ** historique; //Un tableau de HIST_ENTRY : Une structure de données d'un historique
	historique = history_list();
	int start = 0;
	if(combien > 0 && history_length > combien)
		start = history_length- combien;
	if(historique){ // => historique != NULL      
		for(int i = start; historique[i] ; i++){
			char buffer[TAILLE];
			sprintf(buffer, " %d  %s\n", i + history_base, historique[i]->line);
			write(out, &buffer, sizeof(char) * strlen(buffer));
		}
	}
}//Fonctionne

int hostname_interne(int out){
	char nom[TAILLE];
	gethostname(nom, TAILLE);
	if(write(out, &nom, strlen(nom)))
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
} //Fonctionne

void kill_interne(int pid, int sig){
	kill(pid, sig);
} //Fonctionne

void exit_interne(){
	//removeRemote();
	exit(0);
}//Fonctionne
