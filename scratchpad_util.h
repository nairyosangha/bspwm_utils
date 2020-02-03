#ifndef SCRATCHPAD_UTIL_H
#define SCRATCHPAD_UTIL_H

#define DESKTOP_WIDTH 1920
#define DESKTOP_HEIGHT 1080
#define CONFIG_FILE "scratch.conf"
#define ID_SIZE 11

typedef struct scratchpad {
	char name[100];
	char id[ID_SIZE];
	int width;
	int height;
} *scratchpad_t;

void growArr();
scratchpad_t *readData(char *fileName);
void updateRules();
void createMissingWindow(scratchpad_t s);
void addMissingWindows();

#endif
