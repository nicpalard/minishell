#ifndef _EVALUATION_H
#define _EVALUATION_H

#include "Shell.h"
#include <stdbool.h>
#include <sys/wait.h>
#include "Commandes_Internes.h"

extern int evaluer_expr(Expression *e);

#endif
