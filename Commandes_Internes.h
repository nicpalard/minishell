#ifndef COMINTERN_H
#define COMINTERN_H

#define _GNU_SOURCE
#define _BSD_SOURCE || _XOPEN_SOURCE >=500

#include "Shell.h"
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <readline/history.h>
#include <pwd.h>

#define NB_COMMANDES_INTERNES 9

int echo_interne   (char** message);
int date_interne   (char** arguments);
int cd_interne     (char** arguments);
int pwd_interne    (char** arguments);
int history_interne(char** arguments);
int hostname_interne(char** arguments);
int kill_interne(char** arguments);
int exit_interne(char** arguments);
int remote(char ** arguments);
int remote_remove_pid(int pid);


#endif
