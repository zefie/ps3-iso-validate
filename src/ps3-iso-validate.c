#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "ird.h"
#include "manamain.h"

int usage(int v, char *app) {
	printf("Usage: %s ird_file iso_file \n", app);
	return v;
}


int processFile(char *file, char *ird_path) {
	char titleID[16];
	char title[128];
	char fwver[8];
	char* result;
	int retcode = 1;
	int i;

	printf("ISO File: %s\n",file);
	printf("IRD File: %s\n",ird_path);
	printf("Starting IRD Validation of %s...\n", file);
	if (IRDMD5(ird_path, file) == SUCCESS) {
		result = "Valid";
		retcode = 0;
	} else {
		result = "INVALID";
	}
	printf("%s: %s\n", file, result);
	return retcode;
}

int main(int argc, char *argv[], char **envp) {
	char* ps3iso_path;
	char* ird_path;
	FILE *ird;
	if (argc > 2) {
		ird_path = argv[1];
		ps3iso_path = argv[2];

		if (access(ird_path, R_OK) == -1) {
			printf("Could not access file: %s\n", ird_path);
			return usage(1, argv[0]);
		}

		if (access(ps3iso_path, R_OK) != -1) {
			return processFile(ps3iso_path, ird_path);
		}
		else {
			printf("Could not access file: %s\n", ps3iso_path);
			return usage(1, argv[0]);
		}
	}
	else {
		return usage(1, argv[0]);
	}

	return usage(0, argv[0]);
}
