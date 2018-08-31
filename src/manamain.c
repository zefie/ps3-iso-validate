#include "zefie.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ird.h"
#include "iso.h"
#include "md5.h"
#include "manamain.h"

uint32_t reverse32(uint32_t val)
{
   return ((val>>24)&0x000000FF) | ((val>>8)&0x0000FF00) | ((val<<8)&0x00FF0000) | ((val<<24)&0xFF000000);
}

uint16_t reverse16(uint16_t val)
{
   return (((val>>8)&0x00FF) | ((val<<8)&0xFF00));
}

char *GetExtention(char *path)
{
    int n = strlen(path);
    int m = n;

    while(m > 1 && path[m] != '.' && path[m] != '/') m--;

    if(strcmp(&path[m], ".0")==0 || strcmp(&path[m], ".66600")==0) { // splitted
       m--;
       while(m > 1 && path[m] != '.' && path[m] != '/') m--;
    }

    if(path[m] == '.') return &path[m];

    return &path[n];
}

u8 path_info(char *path)
{
	struct stat s;
	if(stat(path, &s) != 0) return _NOT_EXIST;
	if(S_ISDIR(s.st_mode)) return _DIRECTORY; else
	return _FILE;
}

u8 exist(char *path)
{
	if(path_info(path) == _NOT_EXIST) return NO;
	return YES;
}

u64 get_size(char *path, u8 upd_data)
{
	struct stat s;

	if(stat(path, &s) != 0) return 0; else
	if(!S_ISDIR(s.st_mode)) { //FILE
		if(upd_data==YES) {
			total_size+=s.st_size;
			nb_file+=1;
		}
		return s.st_size;
	}

	if(upd_data == YES) nb_directory+=1;

	u64 dir_size=0;
	DIR *d;
	struct dirent *dir;

	d = opendir(path);
	if(d==NULL) return 0;

	while ((dir = readdir(d))) {
		if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;

		char temp[255];
		sprintf(temp, "%s/%s", path, dir->d_name);

		dir_size += get_size(temp, upd_data);
	}
	closedir(d);


	return dir_size;
}

u8 get_SectorSize(FILE*  fd, u32 *SectorSize, u32 *jmp)
{
	char CD01[8] = {0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0x00};

	u32 all_sizes[4] = {0x800, 0x930, 0x920, 0x990};
	u32 sector_size=0;
	u32 jp=0;
	char *data = (char *) malloc(9);

	int i;
	for(i=0; i<4; i++) {
		memset(data, 0, sizeof(data));
		fseek(fd, all_sizes[i]*0x10, SEEK_SET);
		fread(data, 8, 1, fd);
		if(!memcmp((char *) data, (char *) CD01, 8)) {
			sector_size=all_sizes[i];
			jp=0;
			break;
		}
		fseek(fd, all_sizes[i]*0x10+0x18, SEEK_SET);
		fread(data, 8, 1, fd);
		if(!memcmp((char *) data, (char *) CD01, 8)) {
			sector_size=all_sizes[i];
			jp=0x18;
			break;
		}
	}
	free(data);

	if(sector_size==0) return FAILED;

	*SectorSize = sector_size;
	*jmp = jp;

	return SUCCESS;

}

u8 get_FileOffset(FILE* fd, char *path, u64 *FileOffset, u32 *FileSize)
{
	u32 root_table = 0;
	u32 SectSize=0;
	u32 JP=0;

	if( get_SectorSize(fd, &SectSize, &JP) == FAILED) return FAILED;

	//fseek(fd, SectSize*0x10+0xA2+JP, SEEK_SET);
	fseek(fd, SectSize*288, SEEK_SET);

	fread(&root_table, sizeof(u32), 1, fd);
//	if(root_table == 0) return FAILED;
//	fseek(fd, SectSize*root_table, SEEK_SET);

	char *sector = (char *) malloc(SectSize);

	if(sector == NULL) {
		print_load("Error : get_FileOffset : malloc");
		return FAILED;
	}

	int k=0;
	int len = strlen(path);
	char item_name[255] = "";

	for(int i=0; i <= len; i++) {
		if(i==0 && path[0] == '/') continue;

		if(path[i] == '/' || i==len) {
			strncpy(item_name, path+i-k, k);
			printf("item: %s\n",item_name);
			printf("want: %s\n",path);

			memset(sector, 0, SectSize);
			u32 offset = 0;
			fread(sector, 1, SectSize, fd);
			for(int j=0; j<SectSize; j++) {
				if(strncmpi((char *) &sector[j], (char *) item_name , k)==0) {
					printf("sect: %s\n",&sector[j]);
					if(i==len) {
						memcpy(&offset, &sector[j-0x1B], 4);
						*FileOffset = (u64)offset*(u64)SectSize+(u64)JP;
						u32 size=0;
						memcpy(&size, &sector[j-0x13], 4);
						*FileSize = size;
						free(sector);
						return SUCCESS;
					}
					memcpy(&offset, &sector[j-0x1B], 4);
					fseek(fd, SectSize*offset, SEEK_SET);

					break;
				}
			}
			if(offset == 0) {
				printf("offs: %i\n",offset);
				free(sector);
				return FAILED;
			}
			memset(item_name, 0, sizeof(item_name));
			k=0;
		}
		else k++;
	}

	free(sector);
	return FAILED;
}

u8 is_iso(char *file_name)
{
	char *Ext = GetExtention(file_name);

	if( (Ext[1] == 'I' || Ext[1] == 'i')
	&& 	(Ext[2] == 'S' || Ext[2] == 's')
	&&	(Ext[3] == 'O' || Ext[3] == 'o') )	return YES;

	if( (Ext[1] == 'B' || Ext[1] == 'b')
	&& 	(Ext[2] == 'I' || Ext[2] == 'i')
	&&	(Ext[3] == 'N' || Ext[3] == 'n') )	return YES;

	if( (Ext[1] == 'M' || Ext[1] == 'm')
	&& 	(Ext[2] == 'D' || Ext[2] == 'd')
	&&	(Ext[3] == 'F' || Ext[3] == 'f') )	return YES;

	if( (Ext[1] == 'I' || Ext[1] == 'i')
	&& 	(Ext[2] == 'M' || Ext[2] == 'm')
	&&	(Ext[3] == 'G' || Ext[3] == 'g') )	return YES;

	return NO;
}

u8 is_66600(char *file_name)
{
	int l = strlen(file_name);

	if( file_name[l-6] == '.'
	&&	file_name[l-5] == '6'
	&&	file_name[l-4] == '6'
	&&	file_name[l-3] == '6'
	&&	file_name[l-2] == '0'
	&&	file_name[l-1] == '0' ) return YES;

	return NO;
}

u8 is_666XX(char *file_name)
{
	if(is_66600(file_name)) return NO;

	int l = strlen(file_name);

	if( file_name[l-6] == '.'
	&&	file_name[l-5] == '6'
	&&	file_name[l-4] == '6'
	&&	file_name[l-3] == '6') return YES;

	return NO;
}

u8 is_splitted_iso(char *file_name)
{
	if(is_iso(file_name) == NO) return NO;

	int l = strlen(file_name);

	if( file_name[l-2] == '.'
	&&	file_name[l-1] == '0' ) return YES;

	return NO;
}

char *LoadFileFromISO(u8 prog, char *path, char *filename, int *size)
{
	FILE* f;
	f = fopen(path, "rb");
	if(f==NULL) return NULL;

	u64 file_offset=0;
	u8 ret=0;
	int file_size=0;

	ret = get_FileOffset(f, filename, &file_offset, (u32 *) &file_size);
	//print_load("Error : %s %llX", path, file_offset);
	if(file_offset==0 || file_size==0 || ret == FAILED) {fclose(f); return NULL;}

	u8 split666 = is_66600(path);

	if(is_splitted_iso(path)==YES || split666) {

		char iso_path[128];
		char temp[128];
		u64 fsize=0;
		int i;
		int l = strlen(path);

		strcpy(iso_path, path);
		iso_path[l-1]=0;
		if(split666) iso_path[l-2]=0;
		strcpy(temp, iso_path);

		for(i=0; i<100; i++) {
			if(split666) sprintf(iso_path, "%s%02d", temp, i);
			else sprintf(iso_path, "%s%d", temp, i);

			fsize = get_size(iso_path, NO);

			if(file_offset<fsize) {
				if(i!=0) {
					fclose(f);
					f = fopen(iso_path, "rb");
					if(f==NULL) return NULL;
				}
				break;
			}
			file_offset -= fsize;
		}
	}

	u64 val64;
	fseek(f, file_offset-0x14, SEEK_SET);
	fread(&val64, sizeof(u64), 1, f);

	u8 is_bin=NO;
	if(val64==0xFFFFFFFFFFFFFF00) is_bin=YES;

	fseek(f, file_offset, SEEK_SET);

	char *mem = malloc(file_size);
	if(mem == NULL) {fclose(f); return NULL;}

	if(prog) prog_bar1_value=0;
	u64 read = 0;
	while(read < file_size) {
		u32 wrlen = 2048;
		if(read+wrlen > file_size) wrlen = (u32)file_size-read;
		fread(mem+read, sizeof(u8), wrlen, f);
		if(is_bin) fseek(f, 0x130, SEEK_CUR);
		read += wrlen;
		if(prog) prog_bar1_value = (read*100)/file_size;
	}
	fclose(f);

	if(prog) prog_bar1_value=-1;

	*size= file_size;
	return mem;
}

u8 ISOtype(char *isoPath)
{
	FILE* f;
	f = fopen(isoPath, "rb");
	if(f==NULL) {
		//print_load("Error : failed to open %s", isoPath);
		return NO;
	}

	u32 SectSize=0;
	u32 JP=0;

	if( get_SectorSize(f, &SectSize, &JP) == FAILED) { 
		fclose(f);
		return _ISO;
	}

	char *mem =  (char *) malloc(0x40);
	if(mem==NULL) {
		fclose(f);
		//print_load("Error : malloc failed");
		return NO;
	}
	memset(mem, 0, sizeof(mem));
	fseek(f, SectSize*0x10+JP, SEEK_SET);

	fread(mem, 1, 0x40, f);

	if(!memcmp((char *) &mem[0x28], (char *) "PS3VOLUME", 0x9)) {
		free(mem);
		fclose(f);
		return _ISO_PS3;
	}
	if(!memcmp((char *) &mem[0x8], (char *) "PSP GAME", 0x8)) {
		free(mem);
		fclose(f);
		return _ISO_PSP;
	}
/* bad idea : bin/cue PS2 exist too..
	if(!memcmp((char *) &mem[0x8], (char *) "PLAYSTATION", 0xB)) {
		free(mem);
		fclose(f);
		if(JP==0) return _ISO_PS2; 
		else	  return _ISO_PS1;
	}
*/
	free(mem);
	fclose(f);

	int file_size;

	mem = LoadFileFromISO(NO, isoPath, "SYSTEM.CNF", &file_size);
	if( mem != NULL ) {
		if(strstr(mem, "BOOT2") != NULL) {
			free(mem);
			return _ISO_PS2;
		} else
		if(strstr(mem, "BOOT") != NULL) {
			free(mem);
			return _ISO_PS1;
		} else {
			free(mem);
			return _ISO;
		}
	}

	mem = LoadFileFromISO(NO, isoPath, "/PS3_GAME/PARAM.SFO", &file_size);
	if( mem != NULL ) {
		free(mem);
		return _ISO_PS3;
	}
	mem = LoadFileFromISO(NO, isoPath, "/PSP_GAME/PARAM.SFO", &file_size);
	if( mem != NULL ) {
		free(mem);
		return _ISO_PSP;
	}

	return _ISO;

}

u8 get_ext(char *file)
{
	char file_name[128];

	u8 is_path=NO;

	if(strstr(file, "/")) is_path=YES;

	if(is_path==YES) {
		if(path_info(file) == _DIRECTORY)  {
			char temp[255];
			sprintf(temp, "%s/PS3_GAME/PARAM.SFO", file);
			if(path_info(temp) == _FILE) return _JB_PS3;
			sprintf(temp, "%s/PSP_GAME/PARAM.SFO", file);
			if(path_info(temp) == _FILE) return _JB_PSP;
			sprintf(temp, "%s/SYSTEM.CNF", file);
			if(path_info(temp) == _FILE) {
				FILE* f;
				f=fopen(temp, "rb");
				if(f!=NULL) {
					fgets(temp, 128, f);
					strtok(temp, " =");
					fclose(f);
					if(!strcmp(temp, "BOOT2")) return _JB_PS2; else
					if(!strcmp(temp, "BOOT")) return _JB_PS1;
				}
				return _FILE;
			}
			return _DIRECTORY;
		}
		strcpy(file_name, &strrchr(file, '/')[1]);
	} else strcpy(file_name, file);

	char *ext = GetExtention(file_name);
	if(ext==NULL) return _FILE;

	if (!strcmp(file_name, "xRegistry.sys")) {
		return _XREG;
	} else
	if (!strcmp(ext, ".66600")) {
		return _66600;
	} else
	if (!strcmp(ext, ".TTF") || !strcmp(ext, ".ttf")) {
		return _TTF;
	} else
	if (!strcmp(ext, ".JPG") || !strcmp(ext, ".jpg")) {
		return _JPG;
	} else
	if (!strcmp(ext, ".PNG") || !strcmp(ext, ".png")) {
		return _PNG;
	} else
	if (!strcmp(ext, ".P3T") || !strcmp(ext, ".p3t")) {
		return _P3T;
	} else
	if (!strcmp(ext, ".THM") || !strcmp(ext, ".thm")) {
		return _THM;
	} else
	if (!strcmp(ext, ".RAF") || !strcmp(ext, ".raf")) {
		return _RAF;
	} else
	if (!strcmp(ext, ".JSX") || !strcmp(ext, ".jsx")) {
		return _JSX;
	} else
	if (!strcmp(ext, ".JS") || !strcmp(ext, ".js")) {
		return _JS;
	} else
	if (!strcmp(ext, ".MD5") || !strcmp(ext, ".md5")) {
		return _MD5;
	} else
	if (!strcmp(ext, ".SHA1") || !strcmp(ext, ".sha1")) {
		return _SHA1;
	} else
	if (!strcmp(ext, ".NFO") || !strcmp(ext, ".nfo")) {
		return _NFO;
	} else
	if (!strcmp(ext, ".LOG") || !strcmp(ext, ".log")) {
		return _LOG;
	} else
	if (!strcmp(ext, ".INI") || !strcmp(ext, ".ini")) {
		return _INI;
	} else
	if (!strcmp(ext, ".VAG") || !strcmp(ext, ".vag")) {
		return _VAG;
	} else
	if (!strcmp(ext, ".QRC") || !strcmp(ext, ".qrc")) {
		return _QRC;
	} else
	if (!strcmp(file_name, "EBOOT.ELF") || !strcmp(file_name, "EBOOT.elf")) {
		return _EBOOT_ELF;
	} else
	if (!strcmp(file_name, "EBOOT.BIN")) {
		return _EBOOT_BIN;
	} else
	if (!strcmp(ext, ".SELF") || !strcmp(ext, ".self")) {
		return _SELF;
	} else
	if (!strcmp(ext, ".ELF") || !strcmp(ext, ".elf")) {
		return _ELF;
	} else
	if (!strcmp(ext, ".TXT") || !strcmp(ext, ".txt")) {
		return _TXT;
	} else
	if (!strcmp(ext, ".XML") || !strcmp(ext, ".xml")) {
		return _XML;
	} else
	if (!strcmp(ext, ".SPRX") || !strcmp(ext, ".sprx")) {
		return _SPRX;
	} else
	if (!strcmp(ext, ".PRX") || !strcmp(ext, ".prx")) {
		return _PRX;
	} else
	if (!strcmp(ext, ".PKG") || !strcmp(ext, ".pkg")) {
		return _PKG;
	} else
	if (!strcmp(ext, ".TRP") || !strcmp(ext, ".trp")) {
		return _TRP;
	} else
	if (!strcmp(ext, ".SFO") || !strcmp(ext, ".sfo")) {
		return _SFO;
	} else
	if (!strcmp(ext, ".RCO") || !strcmp(ext, ".rco")) {
		return _RCO;
	} else
	if (!strcmp(ext, ".CSO") || !strcmp(ext, ".cso")) {
		return _CSO;
	} else
	if (is_iso(file)==YES) {
		if(is_path == YES) return ISOtype(file);
		return _ISO;
	} else
	if (!strcmp(ext, ".GTF") || !strcmp(ext, ".gtf")) {
		return _GTF;
	} else
	if (!strcmp(ext, ".DDS") || !strcmp(ext, ".dds")) {
		return _DDS;
	}  else
	if (!strcmp(ext, ".ZIP") || !strcmp(ext, ".zip")) {
		return _ZIP;
	}

	return _FILE;
}

u8 md5_filefromISO(char *path, char *filename, unsigned char output[16])
{
	FILE* f;
	f = fopen(path, "rb");
	if(f==NULL) {
		memset(output, 0, sizeof(output));
		return FAILED;
	}

	u64 file_offset=0;
	u8 ret=0;
	u32 file_size=0;

	ret = get_FileOffset(f, filename, &file_offset, (u32 *) &file_size);
	//print_load("Warning : %s offset %llX, size %llX, ret %d", filename, file_offset, file_size, ret);
	if(file_offset==0 || file_size==0 || ret == FAILED) {fclose(f);return FAILED;}

	u8 split666 = is_66600(path);
	if(is_splitted_iso(path)==YES || split666) {

		char iso_path[128];
		char temp[128];
		u64 fsize=0;
		int i;
		int l = strlen(path);

		strcpy(iso_path, path);
		iso_path[l-1]=0;
		if(split666) iso_path[l-2]=0;
		strcpy(temp, iso_path);

		for(i=0; i<32; i++) {
			if(split666) sprintf(iso_path, "%s%02d", temp, i);
			else sprintf(iso_path, "%s%d", temp, i);

			fsize = get_size(iso_path, NO);

			if(file_offset<fsize) {
				if(i!=0) {
					fclose(f);
					f = fopen(iso_path, "rb");
					if(f==NULL) return FAILED;
				}
				break;
			}
			file_offset -= fsize;
		}
	}

	u64 val64;
	fseek(f, file_offset-0x14, SEEK_SET);
	fread(&val64, sizeof(u64), 1, f);

	u8 is_bin=NO;
	if(val64==0xFFFFFFFFFFFFFF00) is_bin=YES;

	fseek(f, file_offset, SEEK_SET);

	md5_context ctx;
	u32 wrlen = 2048;
	unsigned char buf[wrlen];
	u64 read = 0;

	prog_bar1_value=0;

	md5_starts( &ctx );

	while(read < file_size) {
		if(read+wrlen > file_size) wrlen = (u32)file_size-read;
		fread(buf, sizeof(u8), wrlen, f);
		if(is_bin) fseek(f, 0x130, SEEK_CUR);
		read += wrlen;
		prog_bar1_value = (read*100)/file_size;
		md5_update(&ctx, buf, wrlen);
	}

	fclose(f);

	prog_bar1_value=-1;

	md5_finish(&ctx, output);

	memset(&ctx, 0, sizeof(md5_context));

	return SUCCESS;
}

char *LoadFile(char *path, int *file_size)
{
	*file_size = 0;

//	sysLv2FsChmod(path, FS_S_IFMT | 0777);

	struct stat s;
	if(stat(path, &s) != 0) return NULL;
	if(S_ISDIR(s.st_mode)) return NULL;

	*file_size = s.st_size;

	char *mem = malloc(*file_size);
	if(mem==NULL) return NULL;

	int f1 = open(path, O_RDONLY, 0766);
	if(f1<0) return NULL;

	u64 uread = read(f1, mem, *file_size);

	close(f1);

	if(uread != *file_size) {
		free(mem);
		*file_size=0;
		return NULL;
	}

	return mem;
}

u8 md5_file(char *path, unsigned char output[16])
{
	FILE *f;
	size_t n;
	md5_context ctx;
	unsigned char buf[1024];
	uint64_t uread=0;
	uint64_t file_size;

	f = fopen( path, "rb");
	if( f == NULL ) {
		print_load("Error : md5_file, failed to open file");
		return FAILED;
	}

	md5_starts( &ctx );

	print_load("Calculating MD5...");
	prog_bar1_value=0;

	fseek (f , 0 , SEEK_END);
	file_size = ftell (f);
	fseek(f, 0, SEEK_SET);

	while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 ) {
		uread+=n;
		prog_bar1_value=(uread*100)/file_size;
		md5_update( &ctx, buf, n );
	}

	md5_finish( &ctx, output );

	memset( &ctx, 0, sizeof( md5_context ) );

	fclose(f);

	return SUCCESS;
}

u8 *LoadMEMfromISO(char *iso_file, u32 sector, u32 offset, u32 size)
{
	FILE *f;
	u32 SectSize=0;
	u32 JP=0;

	print_load("Open %s", iso_file);
	f = fopen(iso_file, "rb");
	if(f==NULL) {
		print_load("Error : LoadMEMfromISO, failed to fopen");
		return NULL;
	}
	if( get_SectorSize(f, &SectSize, &JP) == FAILED) {
		fclose(f);
		print_load("Error : LoadMEMfromISO, failed to get_SectorSize");
		return NULL;
	}
	u8 *mem = (u8*) malloc(size+1);
	if(mem==NULL) {
		print_load("Error : LoadMEMfromISO, failed to malloc");
		fclose(f);
		return NULL;
	}

	u64 iso_offset = (u64)SectSize*(u64)sector+(u64)offset+(u64)JP;
	u8 split666 = is_66600(iso_file);
	if(is_splitted_iso(iso_file)==YES || split666) {
		char iso_path[128];
		char temp[128];
		u64 fsize=0;
		int i;
		int l = strlen(iso_file);

		strcpy(iso_path, iso_file);
		iso_path[l-1]=0;
		if(split666) iso_path[l-2]=0;
		strcpy(temp, iso_path);

		for(i=0; i<32; i++) {
			if(split666) sprintf(iso_path, "%s%02d", temp, i);
			else sprintf(iso_path, "%s%d", temp, i);

			fsize = get_size(iso_path, NO);

			if(iso_offset<fsize) {
				if(i!=0) {
					fclose(f);
					f = fopen(iso_path, "rb");
					if(f==NULL) return NULL;
				}
				break;
			}
			iso_offset -= fsize;
		}
	}

	fseek(f, iso_offset, SEEK_SET);

	//print_load("ISO offset : %016llX", iso_offset);
	if( fread(mem, size, 1, f) != size) {
		print_load("Error : LoadMEMfromISO, failed to fread");
		free(mem);
		fclose(f);
		return NULL;
	}
	fclose(f);

	return mem;
}

int search_IRD(char *titleID, char *dir_path, char *IRD_path)
{
	char temp[128];

	sprintf(temp, "%s/%s.ird", dir_path, titleID);

	if(path_info(temp) == _FILE) {
		if ( IRD_match(titleID, temp) == YES ) {
			strcpy(IRD_path, temp);
			return FOUND;
		}
	}


	DIR *d;
	struct dirent *dir;

	d = opendir(dir_path);
	if(d==NULL) return NOT_FOUND;

	while ((dir = readdir(d))) {
		if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;
		if(dir->d_type & DT_DIR) continue;

		sprintf(temp, "%s/%s", dir_path, dir->d_name);

		if(strstr(dir->d_name, ".ird")) {
			if ( IRD_match(titleID, temp) == YES ) {
				strcpy(IRD_path, temp);
				closedir(d);
				return FOUND;
			}
		}
	}
	closedir(d);

	return NOT_FOUND;
}

FILE* openSFO(char *path, u32 *start_offset, u32 *size)
{
	FILE* sfo=NULL;

	u8 type = get_ext(path);

	if(type != _ISO_PS3 && type != _ISO_PSP && type != _JB_PS3 && type != _JB_PSP && type != _SFO) return NULL;

	if(type == _SFO) {
		sfo = fopen(path, "rb+");
		if(sfo==NULL) return NULL;

		fseek(sfo , 0 , SEEK_END);
		*size = ftell (sfo);
		fseek(sfo, 0, SEEK_SET);

		*start_offset=0;

		return sfo;
	} else
	if(type == _ISO_PS3) {

		sfo = fopen(path, "rb+");
		if(sfo==NULL) return NULL;
		u64 file_offset=0;
		u8 ret=0;
		int file_size=0;

		ret = get_FileOffset(sfo, "/PS3_GAME/PARAM.SFO", &file_offset,  (u32 *) &file_size);

		if(file_offset==0 || file_size==0 || ret == FAILED) {fclose(sfo); return NULL;}

		*start_offset=file_offset;
		*size=file_size;

		return sfo;
	} else
	if(type == _ISO_PSP) {
		sfo = fopen(path, "rb+");
		if(sfo==NULL) return NULL;
		u64 file_offset=0;
		u8 ret=0;
		int file_size=0;

		ret = get_FileOffset(sfo, "/PSP_GAME/PARAM.SFO", &file_offset,  (u32 *) &file_size);

		if(file_offset==0 || file_size==0 || ret == FAILED) {fclose(sfo); return NULL;}

		*start_offset=file_offset;
		*size=file_size;

		return sfo;
	} else
	if(type== _JB_PS3) {
		char SFO_path[255];
		sprintf(SFO_path, "%s/PS3_GAME/PKGDIR/PARAM.SFO", path);
		if(path_info(SFO_path) == _NOT_EXIST) sprintf(SFO_path, "%s/PS3_GAME/PARAM.SFO", path);
		if(path_info(SFO_path) == _NOT_EXIST) return NULL;

		sfo = fopen(SFO_path, "rb+");
		if(sfo==NULL) return NULL;

		fseek(sfo , 0 , SEEK_END);
		*size = ftell (sfo);
		fseek(sfo, 0, SEEK_SET);

		*start_offset=0;

		return sfo;
	} else
	if(type == _JB_PSP) {
		char SFO_path[255];

		sprintf(SFO_path, "%s/PSP_GAME/PARAM.SFO", path);

		sfo = fopen(SFO_path, "rb+");
		if(sfo==NULL) return NULL;

		fseek(sfo , 0 , SEEK_END);
		*size = ftell (sfo);
		fseek(sfo, 0, SEEK_SET);

		*start_offset=0;

		return sfo;
	}

	return NULL;
}

u8 GetParamSFO(const char *name, char *value, int pos, char *path)
{
	FILE* sfo=NULL;
	u32 sfo_start=0;
	u32 sfo_size=0;

	sfo = openSFO(path, &sfo_start, &sfo_size);
	if(sfo==NULL) return FAILED;

	uint32_t key_start;
	uint32_t data_start;
	uint32_t key_name = 0;
	uint32_t data_name = 0;
	uint32_t temp32 = 0;
	uint16_t temp16 = 0;
	int i, c;

	fseek(sfo, 0x8 + sfo_start, SEEK_SET);
	fread(&key_start, sizeof(uint32_t), 1, sfo);
	fread(&data_start, sizeof(uint32_t), 1, sfo);
	key_start=reverse32(key_start);
	data_start=reverse32(data_start);
	fseek(sfo, key_start + sfo_start, SEEK_SET);

	do {
		c=fgetc(sfo);
		for(i=0; i <=strlen(name)-1 ; i++) {
			if(c == name[i]) {
				if (i==strlen(name)-1) {
					key_name = ftell(sfo) - strlen(name) - sfo_start;
					goto out1;
				}
				c=fgetc(sfo);
			} else break;
		}
	} while (ftell(sfo) - sfo_start < sfo_size);
	{fclose(sfo); return FAILED;}
	out1:
	if(key_name==0) {fclose(sfo); return FAILED;}
	key_name -= key_start;
	fseek(sfo, 0x14 + sfo_start, SEEK_SET);

	while(temp16 < key_name) {
		fread(&temp16, sizeof(uint16_t), 1, sfo);
		temp16=reverse16(temp16);
		if(key_name == temp16) break;
		fseek(sfo, 0xE, SEEK_CUR);
	}

	if(temp16 > key_name)  {fclose(sfo);return FAILED;}
	fseek(sfo, 0xA, SEEK_CUR);
	fread(&temp32, sizeof(uint32_t), 1, sfo);
	temp32 = reverse32(temp32);
	data_name = data_start + temp32;
	fseek(sfo, data_name + sfo_start, SEEK_SET);
	fgets(value, 128, sfo);
	if(strstr(value, "\n") != NULL ) strtok(value, "\n");
	if(strstr(value, "\r") != NULL ) strtok(value, "\r");
	fclose(sfo);

	return SUCCESS;
}

u8 Get_ID(char *gpath, u8 platform, char *game_ID)
{
	if(platform == _ISO_PS3 || platform == _JB_PS3) {
		return GetParamSFO("TITLE_ID", game_ID, -1, gpath);
	} else
	if(platform == _ISO_PSP || platform == _JB_PSP) {
		return GetParamSFO("DISC_ID", game_ID, -1, gpath);
	} else
	if(platform == _ISO_PS2 || platform == _ISO_PS1) {
		char *mem = NULL;
		int size;
		mem = LoadFileFromISO(NO, gpath, "SYSTEM.CNF", &size);
		if(mem==NULL) return FAILED;
		if( strstr(mem, ";") != NULL) strtok(mem, ";");
		if( strstr(mem, "\\") != NULL) strcpy(game_ID, &strrchr(mem, '\\')[1]); else
		if( strstr(mem, ":") != NULL) strcpy(game_ID, &strrchr(mem, ':')[1]);
		free(mem);
	} else
	if(platform == _JB_PS2 || platform == _JB_PS1) {
		char temp[255];
		sprintf(temp, "%s/SYSTEM.CNF", gpath);
		char *mem = NULL;
		int size;
		mem = LoadFile(temp, &size);
		if(mem==NULL) return FAILED;
		if( strstr(mem, ";") != NULL) strtok(mem, ";");
		if( strstr(mem, "\\") != NULL) strcpy(game_ID, &strrchr(mem, '\\')[1]); else
		if( strstr(mem, ":") != NULL) strcpy(game_ID, &strrchr(mem, ':')[1]);
		free(mem);
	}
	else return FAILED;

	return SUCCESS;

}
