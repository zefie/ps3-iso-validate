#ifndef _MANAMAIN_H
#define _MANAMAIN_H

#define OK 		1
#define NOK 		0
#define YES		1
#define NO 		0
#define TRUE 		1
#define FALSE 		0
#define SUCCESS 	1
#define FAILED	 	0
#define ON		1
#define OFF		0
#define FOUND		1
#define NOT_FOUND	0
#define _EXIST		1
#define _NOT_EXIST	0
#define _FILE		1
#define _DIRECTORY	2
#define _PNG		3
#define _JPG		4
#define _SELF		5
#define _ELF		6
#define _EBOOT_BIN	7
#define _EBOOT_ELF	8
#define _SPRX		9
#define _PRX		10
#define _XML		11
#define _PKG		12
#define _IRD		13
#define _P3T		14
#define _THM		15
#define	_RIF		16
#define _RAP		17
#define _EDAT		18
#define _PFD		19
#define _MP4		20
#define _SFO		21
#define _TXT		22
#define _TRP		23
#define _RCO		24
#define _CSO		25
#define _ISO		26
#define _ISO_PS3	27
#define _ISO_PS2	28
#define _ISO_PS1	29
#define _ISO_PSP	30
#define _JB_PS3		31
#define _JB_PS2		32
#define _JB_PS1		33
#define _JB_PSP		34
#define _GTF		35
#define _DDS		36
#define _RAF		37
#define _JSX		38
#define _QRC		39
#define _VAG		40
#define _JS		41
#define _LOG		42
#define _INI		43
#define _NFO		44
#define _MD5		45
#define _SHA1		46
#define _XREG		47
#define _ZIP		48
#define _66600		49
#define _TTF		50

u8 GetParamSFO(const char *name, char *value, int pos, char *path);
u8 Get_ID(char *gpath, u8 platform, char *game_ID);

#endif
