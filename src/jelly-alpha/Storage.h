#ifndef STORAGE_H
#define STORAGE_H

#include <FS.h>
#include <SD.h>

bool logging = false;

void initSDCard();
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, String message);
void removeFile(fs::FS &fs, const char *path);

#endif
