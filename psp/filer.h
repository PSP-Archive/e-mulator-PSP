#ifndef FILER_H
#define FILER_H

extern char LastPath[], FilerMsg[];

int getExtId(const char *szFilePath);
int searchFile(const char *path, const char *name);
int getFilePath(char*,char*,int*);

#endif
