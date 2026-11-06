#pragma once

int CreateChild(char * path, char* args[], int waitUntil);
int ChildWait(pid_t pid);