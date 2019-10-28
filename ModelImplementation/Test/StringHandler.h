#ifndef __STRINGHANDLER_H__
#define __STRINGHANDLER_H__

#include <iostream>
#include <list>

std::list<std::string> getFileNames(char* strDirectory);
char* getDirectory(char** argv, int value);
char* composeFilePath(char* path, char* file, char* ext);
std::string getListFileName(std::list<std::string> FileNames, int index);

#endif
