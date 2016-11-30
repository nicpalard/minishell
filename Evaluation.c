#include "Evaluation.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int (*commandes_internes[NB_COMMANDES_INTERNES]) (char**) = {echo_interne, date_interne, cd_interne, pwd_interne, history_interne, hostname_interne, kill_interne, exit_interne, remote};
char* noms_commandes_internes[NB_COMMANDES_INTERNES] = {"echo", "date", "cd", "pwd", "history", "hostname", "kill", "exit", "remote" };

int evaluer_expr_recu(Expression* e, bool bg);

void verifier(int cond, char *s)
{
  if (!cond)
    {
      perror(s);
      exit(EXIT_FAILURE);
    }
}

int commande_interne(char* commande){
  for (int i = 0 ; i != NB_COMMANDES_INTERNES ; i++) {
    if (!strcmp(noms_commandes_internes[i], commande))
      return i;
  }
  return -1;
}

int evaluer_expr(Expression *e)
{
	evaluer_expr_recu(e, false);
}

int evaluer_expr_recu(Expression *e, bool bg) {
	int pid;
	
	switch(e->type){
	case SIMPLE:{
		int ret;
                int commande = commande_interne(e->arguments[0]);
                if (commande != -1)
                  ret = (*commandes_internes[commande])(e->arguments + 1);
                else {
			verifier((pid = fork()) != -1, "fork failed");
			if(pid == 0){ //Fils => Exec de la cmd
				execvp(e->arguments[0], e->arguments);
				perror("exec failed");
				exit(EXIT_FAILURE);
			} else {
				int s2;
				if(!bg){ //Appelé sans bg on attends que le fils se termine
					waitpid(pid, &ret, 0);
				}
			}	
		}
                int status;
		while( 0 < (pid = waitpid(-1, &status, WNOHANG) )){
			remote_remove_pid(pid); //Remove la connexion de la liste si besoin
		}
		return ret;
		break;
	}
	case BG:
		evaluer_expr_recu(e->gauche, true);
		break;
	case REDIRECTION_O:{
	    int fd = open(e->arguments[0], O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
		verifier((fd >= 0), "open failed");
		int backup = dup(STDOUT_FILENO);
		
		dup2(fd, STDOUT_FILENO);
		evaluer_expr_recu(e->gauche, bg);
		dup2(backup, STDOUT_FILENO);
		
		close(backup);
		close(fd);
		break;
	}
	case REDIRECTION_A:{
		int fd = open(e->arguments[0], O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR);
		verifier((fd >= 0), "open failed");
		int backup = dup(STDOUT_FILENO);
		
		dup2(fd, STDOUT_FILENO);
		evaluer_expr_recu(e->gauche, bg);
		dup2(backup, STDOUT_FILENO);
		
		close(backup);
		close(fd);
		break;
	}
	case REDIRECTION_EO:{
		int fd = open(e->arguments[0], O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR);
		verifier((fd >= 0), "open failed");
		int backup = dup(STDOUT_FILENO);
		int backup2 = dup(STDERR_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		evaluer_expr_recu(e->gauche, bg);
		dup2(backup, STDOUT_FILENO);
		dup2(backup2, STDERR_FILENO);
		close(backup);
		close(backup2);
		close(fd);
		break;
	}
	case REDIRECTION_E:{
		int fd = open(e->arguments[0], O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR);
		verifier((fd >= 0), "open failed");
		int backup = dup(STDERR_FILENO);
		
		dup2(fd, STDERR_FILENO);
		evaluer_expr_recu(e->gauche, bg);
		dup2(backup, STDERR_FILENO);
		
		close(backup);
		close(fd);
		break;
	}
	case REDIRECTION_I:{
		int fd = open(e->arguments[0], O_RDONLY, NULL);
		verifier((fd >= 0), "open failed");
		int backup = dup(STDIN_FILENO);
		
		dup2(fd, STDIN_FILENO);
		evaluer_expr_recu(e->gauche, bg);
		dup2(backup, STDIN_FILENO);
		
		close(backup);
		close(fd);
		break;
	}
	case PIPE:{
		int tube[2];
		verifier(pipe(tube) != -1, "pipe failed\n");
		verifier((pid = fork()) != -1, "fork failed\n");
		if(pid == 0){ // Fils donc premiere commande
			close(tube[0]);
			dup2(tube[1], STDOUT_FILENO);
			close(tube[1]);
			evaluer_expr_recu(e->gauche, false);
			exit(0);
		} else {
			int backup = dup(STDIN_FILENO);
			close(tube[1]);
			dup2(tube[0], STDIN_FILENO);
			close(tube[0]);
			evaluer_expr_recu(e->droite, bg);
			dup2(backup, STDIN_FILENO);
		}
		break;
	}
	case SEQUENCE:{
		evaluer_expr_recu(e->gauche, bg);
		evaluer_expr_recu(e->droite, bg);
		break;
	}
	case SEQUENCE_OU:{
		if(evaluer_expr_recu(e->gauche, bg)){
			evaluer_expr_recu(e->droite, bg);
		}
		break;
	}
	case SEQUENCE_ET:{
		if(!evaluer_expr_recu(e->gauche, bg)){
			evaluer_expr_recu(e->droite, bg);
		}
		break;
	}
	case SOUS_SHELL:{
		verifier((pid = fork()) !=-1, "fork failed\n");
		if(pid == 0){
			evaluer_expr_recu(e->gauche, false);
			exit(0);
		} else {
			if(!bg){ //Appelé sans bg on attends que le fils se termine
				waitpid(pid, NULL, 0);
			}
		}
		break;
	}		
	case VIDE:
		break;
	}
}
