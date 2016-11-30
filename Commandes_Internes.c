#include "Commandes_Internes.h"

#define TAILLE 256
#define ANNEE 1900
#define MAX_CONNEXIONS 100

#define TAILLE_MAX_COMMANDE 50

const char* jours[] = {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"};
const char* mois[] = {"janvier", "fevrier", "mars", "avril", "mai", "juin", "juillet", "aout", "septembre", "octobre", "novembre", "decembre"};

typedef struct Connexion {
	int pid;
	char* nom;
	int entree_tube;
	//int sortie_tube;
} Connexion;

static int cpt_connexions = 0;
static Connexion* connexions[MAX_CONNEXIONS];

int r_remove();

void verif(int cond, char *s)
{
  if (!cond)
    {
      perror(s);
      exit(EXIT_FAILURE);
    }
}


/*---------------------------------------------------
-----------------------------------------------------
----------------- COMMANDES INTERNES ----------------
-----------------------------------------------------
---------------------------------------------------*/

int echo_interne(char** message){
	for(int i = 0 ; message[i] ; i++){
		printf("%s ", message[i]);
	}
	printf("\n");
	return EXIT_SUCCESS;
}

int date_interne(char** arguments){
	//On récupere la date actuelle
	time_t horodatage;
	horodatage = time(NULL);
	struct tm *temps;
	temps = localtime(&horodatage);
	printf("%s %d %s %d, %d:%d:%d\n", jours[temps->tm_wday], temps->tm_mday, mois[temps->tm_mon], ANNEE + temps->tm_year, temps->tm_hour, temps->tm_min, temps->tm_sec);
	return EXIT_SUCCESS;
}

int cd_interne(char** arguments){ 
  char* chemin;
  if (arguments[0] == NULL) {
    chemin = malloc(sizeof(char) * 3);
    strcpy(chemin, "~/");
  } else
    chemin = arguments[0];

  char* chemin_entier;
  if (chemin[0] == '/') 
    chemin_entier = chemin;
  else {
    const char* dos;
    if (chemin[0] == '~'){
      dos = getpwuid(getuid())->pw_dir; //Chemin absolu du répertoire courant
	  chemin = chemin+1;
	}
    else
      dos = get_current_dir_name();
  
    chemin_entier = malloc(sizeof(char) * (strlen(dos) + strlen(chemin) + 1));
    strcpy(chemin_entier, dos); //Je met le chemin absolu dans fullpath
    strcat(chemin_entier, "/"); // Crade mais assume
    strcat(chemin_entier, chemin); //J'y ajoute le path
  }
  chdir(chemin_entier); //Je m'y place
  free(chemin_entier);
  if (arguments[0] == NULL)
    free(chemin);
  return EXIT_SUCCESS;
}
  

int pwd_interne(char** arguments){
	printf("%s\n", get_current_dir_name());
	return EXIT_SUCCESS;
}

int history_interne(char** arguments){
        int combien;
        if (arguments[0] == NULL)
          combien = 0;
        else
          combien = atoi(arguments[0]);
	HIST_ENTRY ** historique; //Un tableau de HIST_ENTRY : Une structure de données d'un historique
	historique = history_list();
	int start = 0;
	if(combien > 0 && history_length > combien)
		start = history_length- combien;
	if(historique){ // => historique != NULL      
		for(int i = start; historique[i] ; i++){
			printf(" %d  %s\n", i + history_base, historique[i]->line);
		}
	}
	return EXIT_SUCCESS;
}

int hostname_interne(char** arguments){
	char nom[TAILLE];
	gethostname(nom, TAILLE);
	printf("%s\n", nom);
	return EXIT_SUCCESS;
}

int kill_interne(char** arguments){
        int pid, sig;
        if (arguments[0] == NULL || arguments[1] == NULL)
          return EXIT_FAILURE;
        else {
          pid = atoi(arguments[0]);
          sig = atoi(arguments[1]);
        }
	kill(pid, sig);
	return EXIT_SUCCESS;
} 

int exit_interne(char** arguments){
	r_remove();
	exit(0);
	return EXIT_SUCCESS;
}

/*---------------------------------------------------
-----------------------------------------------------
------------------ REMOTE COMMANDES -----------------
-----------------------------------------------------
---------------------------------------------------*/

int r_add(char ** arguments){
	int pid;
	for(int i = 0 ; arguments[i] ; i++){
		int tube[2];
		verif(pipe(tube) != -1, "pipe failed\n");
		verif((pid = fork()) != -1, "fork failed\n");
		if(pid == 0){ //Fils donc connexion ssh
			close(tube[1]);
			dup2(tube[0], STDIN_FILENO);
			close(tube[0]);
			execlp("ssh", "ssh", arguments[i] , "bash", NULL);
			perror("exec failed\n");
			exit(EXIT_FAILURE);
		}
		close(tube[0]);

		Connexion* c = malloc(sizeof(Connexion));
		c->nom = malloc(sizeof(char)*strlen(arguments[i])+1);
		strcpy(c->nom, arguments[i]);
		c->entree_tube = tube[1];
                c->pid = pid;


                // On parcourt le tableau de connexions jusqu'à trouver une case vide au millieu. Si on n'en trouve pas, c'est à dire qu'on sort avec cpt==0, on ajoute à la fin.
                int cpt = cpt_connexions;
                for (int i = 0 ; cpt != 0; i++) {
                  if (connexions[i] == NULL) {
		    connexions[i] = c;
                    break;
                  }
                  else
                    cpt--;
                }
                if (cpt == 0)
                  connexions[cpt_connexions] = c;

		cpt_connexions++;
	}					
	return EXIT_SUCCESS;
}

int r_remove_indice(int i){
	close(connexions[i]->entree_tube);
	free(connexions[i]->nom);
	free(connexions[i]);
	connexions[i] = NULL;
	cpt_connexions--;
	return EXIT_SUCCESS;
}

int r_remove(){
	int cpt = cpt_connexions;
	for(int i = 0 ; cpt != 0 ; i++){
		if(connexions[i] != NULL){
			r_remove_indice(i);
			cpt--;
		}
	}
	return EXIT_SUCCESS;
}

int r_list(){
	
	if(cpt_connexions == 0){
		printf("No connexions\n");
	}
	
	int cpt = cpt_connexions;
	for(int i = 0 ; cpt != 0 ; i++){
		if(connexions[i] != NULL){
			printf("------- %s ------\n", connexions[i]->nom);
			cpt--;
		}
	}
	return EXIT_SUCCESS;
}

int execute_command(char ** arguments, Connexion* co){
  char commande[TAILLE_MAX_COMMANDE];
  strcpy(commande, arguments[0]);
  for (int i = 1; arguments[i] != NULL ; i++) {
    strcat(commande, " ");
    strcat(commande, arguments[i]);
  }
  strcat(commande, "\n");
  
  write(co->entree_tube, commande, sizeof(char) * strlen(commande));
}

int r_cmd(char ** arguments){
  if(arguments[1] == NULL){
    printf("bad number of arguments : remote machine-name command\n");
    return EXIT_FAILURE;
  }
  int cpt = cpt_connexions;
  for(int i = 0 ; cpt != 0; i++){
    if(connexions[i] != NULL){
      if(!strcmp(arguments[0], connexions[i]->nom)){
	execute_command(arguments + 1, connexions[i]);
      }
      cpt--;
    }
  }
  return EXIT_SUCCESS;
}

int r_all(){
  return EXIT_SUCCESS;
}

int r_help(){
	printf("\n-----Here is a list of the aviable commands:-----\n\n  ** remote add list-of-machines : [create a Shell on each machine of the list.]\n  ** remote remove : [shutdown all the remote shells.]\n  ** remote list : [show the list of connected shells.]\n  ** remote machine-name cmd : [execute the command cmd on the already connected machine-name's Shell.]\n  ** remote all cmd : [execute the command cmd on all of the connected machines's Shell.]\n\nCarefulf : cmd can only be a simple command.\n");
	return EXIT_SUCCESS;
}
	

int remote(char ** arguments){

	if(arguments[0] == NULL){
		printf("please enter a valid number of arguments or type \"remote help\" to get some help.\n");
		return EXIT_FAILURE;
	}

	char* func = arguments[0];
		

	if(!strcmp(func, "add")){
		return r_add(arguments+1);
	} else if (!strcmp(func,"remove")){
		return r_remove();
	} else if (!strcmp(func,"list")){
		return r_list();
	} else if (!strcmp(func, "help")){
		return r_help();
	} else if (!strcmp(func, "all")){
		return r_all(arguments+1);
	} else {
		return r_cmd(arguments);
	}
	return EXIT_SUCCESS;
}

int remote_remove_pid(int pid){
	for(int i = 0 ; i < cpt_connexions ; i++){
		if(connexions[i]->pid == pid){
			r_remove_indice(i);
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}
			
