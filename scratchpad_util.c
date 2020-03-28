#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "shell_cmd.h"
#include "scratchpad_util.h"
#include "util.h"

int size = 0;
scratchpad_t *s = NULL; 

void growArr() {
	s = (scratchpad_t *) realloc(s, (++size) * sizeof(scratchpad_t));
	s[size-1] = malloc(sizeof(struct scratchpad));
}

scratchpad_t *readData(char *fileName) 
{
	FILE *f = fopen(fileName, "r");
	if (!f) die("file not found!");

	growArr();
	while(fscanf(f, "%s %d %d\n", s[size-1]->name, &(s[size-1]->width), &(s[size-1]->height)) == 3) {
		//printf("while reading: %s: %d\t%d\n", s[size-1]->name, s[size-1]->width, s[size-1]->height);
		memset(s[size-1]->id, '\0', ID_SIZE * sizeof(char));
		growArr();
	}
	free(s[size-1]); 
	s[size-1] = NULL;

	fclose(f);
	return s;
}

void updateRules() 
{
	char class[106], rect[100];
	for(scratchpad_t *ptr = s; *ptr; ptr++) { // delete the old rules
		sprintf(class, "URxvt:%s", (*ptr)->name);
		char *cmd[] = { "bspc", "rule", "-r", class, NULL };
		process(cmd);
	}
	wait(NULL);

	int hx = DESKTOP_WIDTH / 2, hy = DESKTOP_HEIGHT / 2;
	for(scratchpad_t *ptr = s; *ptr; ptr++) { // add new rules
		int x = (*ptr)->width, y = (*ptr)->height;
		sprintf(class, "URxvt:%s", (*ptr)->name);
		sprintf(rect, "rectangle=%dx%d+%d+%d", x, y, hx - (x / 2), hy - (y / 2));
		char *cmd[] = { "bspc", "rule", "-a", class, "state=floating", "sticky=on", rect, NULL };
		process(cmd);
	}
	wait(NULL);
}

void createMissingWindow(scratchpad_t s) {
	printf("creating missing window..\n");
	char shellCmd[108];
	sprintf(shellCmd, "scratch %s", s->name);
	char *trmCmd[] = { "urxvtc", "-name", s->name, "-e", "bash", "-c", shellCmd, NULL };
	process_t p1 = process(trmCmd);
	close(p1.fd_write); close(p1.fd_read);
	char *xdoCmd[] = { "xdo", "hide", "-N", "URxvt", "-n", s->name, "-m", NULL };
	process_t p2 = process(xdoCmd);
	close(p2.fd_write); close(p2.fd_read);

	waitpid(p2.pid, NULL, 0);
}

void addMissingWindows() 
{
	char instance[100];
	for(scratchpad_t *ptr = s; *ptr; ptr++) { // add IDs to struct, if no ID added -> windows doesn't exist -> create it
		sprintf(instance, "%s", (*ptr)->name);
		char *cmd[] = { "xdo", "id", "-N", "URxvt", "-n", instance, NULL };
		process_t p = process(cmd);
		read(p.fd_write, (*ptr)->id, 10*sizeof(char));
		waitpid(p.pid, NULL, 0);
		if (strlen((*ptr)->id) == 0)
			createMissingWindow(*ptr);
	}
}


int main() {
	char *config_home = getenv("XDG_CACHE_HOME");
	char config_path[100];
	snprintf(config_path, sizeof(config_path), "%s/%s", config_home, CONFIG_FILE);
	scratchpad_t *s = readData(config_path);
	updateRules(); // remove bspwm rules relevant to scratchpads and recreate them
	addMissingWindows();
	for(scratchpad_t *t = s;*t;t++) {
		printf("%s (%s):\t %dx%d\n", (*t)->id, (*t)->name, (*t)->width, (*t)->height);
		free(*t);
	}
	free(s);
	return 0;
}
