#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "ird.h"
#include "manamain.h"

int usage(int v, char *app) {
	printf("Usage: %s ird_directory iso_file_or_directory \n", app);
	return v;
}

bool processFile(char *file) {
	char titleID[32];
	char* result;
	char* ird_path;
	bool retbool = false;
	int i;

	printf("Searching for an IRD for %s...\n",file);
	if(Get_ID(file, _ISO_PS3, titleID) == FAILED) {
		result = "Could not get TITLE_ID";
	} else {
		printf("IRD: %s", titleID);
		return 0;
		printf("Checking %s...\n", file);
		if (IRDMD5(ird_path, file)) {
			result = "Valid";
			retbool = true;
		} else {
			result = "INVALID";
		}
	}
	printf("%s: %s\n", file, result);
	return retbool;
}

int main(int argc, char *argv[], char **envp) {
	char* ps3iso_path;
	char* ird_path;
	DIR *dir;
	if (argc > 2) {
		ird_path = argv[1];
		ps3iso_path = argv[2];

		dir = opendir(ird_path);
		if (dir) {
			/* Directory exists. */
			closedir(dir);
		}
		else {
			return usage(1, argv[0]);
		}

		dir = opendir(ps3iso_path);
		if (dir) {
			printf("PS3 ISO Directory (not yet supported): %s\n", ps3iso_path);
			closedir(dir);
			return 0;
		}
		else if (errno == ENOTDIR) {
			// try file
			if (access(ps3iso_path, R_OK) != -1) {
				return processFile(ps3iso_path);
			}
			else {
				printf("Could not access file: %s\n", ps3iso_path);
				return usage(1, argv[0]);
			}
		}
		else {
			printf("Could not open %s as file nor directory. Err: %i\n", ps3iso_path, ENOENT);
			return usage(1, argv[0]);
		}
	}
	else {
		return usage(1, argv[0]);
	}

	return usage(0, argv[0]);
}
