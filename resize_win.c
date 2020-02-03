#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "util.h"
#include "shell_cmd.h"

#define ID_SIZE (11)
#define UP 	(1)
#define DOWN 	(2)
#define LEFT 	(3)
#define RIGHT 	(4)
#define HORIZONTAL 	(0)
#define VERTICAL 	(1)

static char *directions[4] = { "top", "bottom", "left", "right" };
static int verbose = 0;

struct state {
	int selectedChild;
	int child1;
	int child2;
	int splitType;
};

int validInput(char *direction) 
{
	if (strcmp(direction, "up") == 0) 	return UP;
	if (strcmp(direction, "down") == 0) 	return DOWN;
	if (strcmp(direction, "left") == 0) 	return LEFT;
	if (strcmp(direction, "right") == 0) 	return RIGHT;
	else return 0;
}

int queryJson(struct state *s, process_t p1, process_t p2) {
	char ch, tmp[20]; FILE *f;

	if (!(f = fdopen(p1.fd_write, "r"))) return 0;
	if ((ch = fgetc(f)) == EOF) return 0;
	while (ch != EOF) {
		dprintf(p2.fd_read, "%c", ch); // send json output to jq
		ch = fgetc(f);
	} fclose(f); close(p2.fd_read); // done writing to jq

	if (!(f = fdopen(p2.fd_write, "r"))) return 0;
	fscanf(f, "%s\n%8d\n%8d", tmp, &(s->child1), &(s->child2));
	fclose(f);
	s->splitType = strcmp(tmp, "horizontal") == HORIZONTAL ? HORIZONTAL : VERTICAL;
	return 1;
}

struct state *getState(int dir)
{
	FILE *f = NULL;
	char *jq[] = { "jq", "-r", ".splitType,.firstChild.id,.secondChild.id", NULL};
	struct state *s = malloc(sizeof(struct state));

	int depth = 1; // how many parents up we have to go 
	char *bspc_path = NULL;
	int bspc_path_size = 0;

	while (1) {
		bspc_path_size += 9;
		bspc_path = realloc(bspc_path, bspc_path_size * sizeof(char));
		if (depth > 1) 
			strncat(bspc_path, "#@parent", 9);
		else {
			memset(bspc_path, '\0', bspc_path_size);
			strncat(bspc_path, "@parent", 8);
		}
		char *jsonParent[] = { "bspc", "query", "-T", "-n", bspc_path, NULL};
		process_t p1 = process(jsonParent);
		process_t p2 = process(jq);
		if (!queryJson(s, p1, p2)) goto free_before_exit;

		switch (s->splitType) { // splitType of PARENT, so 2 childs have the opposite splitType
			case VERTICAL: 
				if (dir == UP || dir == DOWN) { // in this case we need the parent node of this parent
					depth++;
					continue;
				} else break;
				
			case HORIZONTAL:
				if (dir == LEFT || dir == RIGHT) { // same case
					depth++;
					continue;
				} else break;
		}

		char *focusedChild[] = { "bspc", "query", "-N", "-n", NULL};
		process_t p3 = process(focusedChild);
		close(p3.fd_read); // we're not writing anything to this process
		if (!(f = fdopen(p3.fd_write, "r"))) goto free_before_exit;
		unsigned int t = 0; fscanf(f, "%x\n", &t);
		fclose(f);
		close(p3.fd_write); close(p1.fd_read); close(p1.fd_write); close(p2.fd_write);
		s->selectedChild = t;
		break;
	}
	free(bspc_path);
	return s;

free_before_exit:
	free(bspc_path);
	free(s);
	exit(1);
}

void runCmd(char *args[]) {
	for (int i = 0; args[i] != NULL; i++) {
		printf("%s ", args[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[]) {
	int dir = 0, x = 0, y = 0;
	if (argc != 2) die("Usage: resize_win [direction]\n\t- direction: up, down, left, right");
	if (!(dir = validInput(argv[1]))) die("Invalid direction!");
	struct state *s = getState(dir);
	if (verbose) printf("split: %d\tselected: %d\tchild 1: %d,\tchild2: %d\n", s->splitType, s->selectedChild, s->child1, s->child2);
	switch (s->splitType) {
		case HORIZONTAL:
			x = 0, y = dir == UP ? -20 : 20;
			dir = s->child1 == s->selectedChild ? DOWN : UP;
			break;
		case VERTICAL:
			y = 0, x = dir == LEFT ? -20 : 20;
			dir = s->child1 == s->selectedChild ? RIGHT : LEFT;
			break;
	}
	char xTxt[10], yTxt[10];
	sprintf(xTxt, "%d", x); sprintf(yTxt, "%d", y);
	char *bspc[] = { "bspc", "node", "-z", directions[dir-1], xTxt, yTxt, NULL };
	if (verbose) runCmd(bspc);
	process_t p = process(bspc);
	close(p.fd_read); close(p.fd_write);
	wait(NULL);
	free(s);
	return 0;
}
