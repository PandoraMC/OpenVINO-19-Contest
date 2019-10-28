#include <iostream>
#include <list>
#include <dirent.h>
#include <string.h>
#include "StringHandler.h"

using namespace std;

list<string> getFileNames(char* strDirectory){
	list<string> lstFiles;
	DIR *dirFiles;
	struct dirent *element;
	dirFiles = opendir(strDirectory);
	if(dirFiles){
        	while ((element = readdir(dirFiles)) != NULL)
			lstFiles.push_front(element->d_name);
        	closedir(dirFiles);
	}
	return lstFiles;
}

char* getDirectory(char** argv, int value){
	int intPathLen = strlen(*(argv + value));
	char *chrPath = (char*)malloc(sizeof(char)*(intPathLen + 2));
	strcpy(chrPath, *(argv + value));
	*(chrPath + intPathLen) = '/';
	*(chrPath + intPathLen + 1) = '\0';
	return chrPath;
}

char* composeFilePath(char* path, char* file, char* ext){
	int intPathLen = strlen(path);
	int intFileLen = strlen(file);
	int intExtLen = strlen(ext);
	int fullPathLen = intPathLen + intFileLen + intExtLen;
	char *ptrFullPath = (char*)malloc(sizeof(char)*(fullPathLen + 1));
	strcpy(ptrFullPath, path);
	strcpy(ptrFullPath + intPathLen, file);
	strcpy(ptrFullPath + intPathLen + intFileLen, ext);
	*(ptrFullPath + fullPathLen) = '\0';
	return ptrFullPath;
}

string getListFileName(list<string> FileNames, int index){
	list<string>::iterator FileName = FileNames.begin();
	advance(FileName, index);
	return *FileName;
}
