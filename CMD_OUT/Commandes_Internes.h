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

int echo_interne (char* message, int out);
void date_interne (int out);
void cd_interne(char* chemin);
int pwd_interne(int out);
void history_interne(int combien, int out);
int hostname_interne(int out);
void kill_interne(int pid, int sig);
void exit_interne();

#endif
