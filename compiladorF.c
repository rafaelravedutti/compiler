
/* -------------------------------------------------------------------
 *            Aquivo: compilador.c
 * -------------------------------------------------------------------
 *              Autor: Bruno Muller Junior
 *               Data: 08/2007
 *      Atualizado em: [15/03/2012, 08h:22m]
 *
 * -------------------------------------------------------------------
 *
 * Funções auxiliares ao compilador
 *
 * ------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compilador.h"


/* -------------------------------------------------------------------
 *  variáveis globais
 * ------------------------------------------------------------------- */

FILE *fp = NULL;

void geraCodigo(char *rot, char *comando) {
  if(fp == NULL) {
    fp = fopen ("MEPA", "w");
  }

  if(rot == NULL) {
    fprintf(fp, "     %s\n", comando);
  } else {
    fprintf(fp, "%s: %s \n", rot, comando);
  }

  fflush(fp);
}

int imprimeErro(char* erro) {
  fprintf(stderr, "Erro na linha %d - %s\n", line_number, erro);
  exit(-1);
}
