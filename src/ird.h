#ifndef _IRD_H
#define _IRD_H

#include "zefie.h"

u8 IRD_match(char *titleID, char *IRD_PATH);
int IRDMD5(char *IRD_PATH, char *GAME_PATH);

#endif
