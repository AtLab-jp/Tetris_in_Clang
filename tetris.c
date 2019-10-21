#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define BORDER 2
#define WIDTH  2000
#define HIGHT 1000
#define SIZE 30
#define SIZE_X (10+2)
#define SIZE_Y (20+2)

typedef struct _TAG_POSITION {
	int x;
	int y;
}POSITION;

typedef struct _TAG_PANEL {
	int color;
	POSITION loc;
} PANEL;

Display *dpy;
Window w;
Window root;
int screen, xx, yy;
unsigned long black, white;
GC	gc;
XEvent e;
char color_name[10][20]={"white","red","purple","blue","yellow", "green", "cyan", "orange", "gray", "black"};
unsigned long c_no[10];
Colormap cmap;
XColor c0,c1;
PANEL panel[SIZE_X+2][SIZE_Y+2];
POSITION memori[4];
int xStart = (SIZE_X/2)-2, yStart = 0;
int mBlock;
int mDown = 200;
int iScore = 0;
int correction = 0;
bool bNextBlock = true;
bool bMoved = false;
bool bRotation = false;
bool bGameOver = false;
int block[7][4][4] = {{{0, 0, 0, 0},
			           {1, 1, 1, 1},
			           {0, 0, 0, 0},
			           {0, 0, 0, 0}
			          },
			          {{0, 0, 0, 0},
			           {2, 2, 2, 0},
			           {0, 2, 0, 0},
			           {0, 0, 0, 0}
			          },
			          {{0, 0, 0, 0},
			           {0, 3, 3, 0},
			           {0, 3, 3, 0},
			           {0, 0, 0, 0}
			          },
			          {{0, 0, 0, 0},
			           {4, 4, 0, 0},
			           {0, 4, 4, 0},
			           {0, 0, 0, 0}
			          },
			          {{0, 0, 0, 0},
			           {0, 5, 5, 0},
			           {5, 5, 0, 0},
			           {0, 0, 0, 0}
			          },
			          {{0, 0, 0, 0},
			           {6, 6, 6, 0},
			           {0, 0, 6, 0},
			           {0, 0, 0, 0}
			          },
			          {{0, 0, 0, 0},
			           {7, 7, 7, 0},
			           {7, 0, 0, 0},
			           {0, 0, 0, 0}
			          }};

void setup();
void print_board();
void clear_block();
void wmEventReader();
void wmCreateBlock(int mBlock);
bool wmBlockDown();
void wmRotationBlock();
bool wmChecker();
void delete_line();
void wmGameOver();

int main(int argc, char *argv[]) {
	setup();
	srand((unsigned)time(NULL));
	int time = 0;
	while(true) {
		if(bNextBlock) {
			wmCreateBlock(mBlock = rand()%7);
		}
		if(bGameOver) {
			wmGameOver();
			print_board();
			XFlush(dpy);
			sleep(2);
			printf("\nScore : %d\n\n", iScore);
			return 0;
		}
		if(bMoved && !bNextBlock)
			clear_block();
		time++;
		if(time==mDown) {
			yStart++;
			time = 0;
		}
		bNextBlock = false;
		bMoved = false;
		xStart += correction;
		wmBlockDown();
		while(XPending(dpy))
			wmEventReader();
		print_board();
		XFlush(dpy);
		delete_line();
	}
}

void setup() {
	dpy = XOpenDisplay("");
	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);
	white = WhitePixel (dpy, screen);
	black = BlackPixel (dpy, screen);
	cmap = DefaultColormap(dpy,0);
	for(int i=0;i<10;i++){
		XAllocNamedColor(dpy,cmap,color_name[i],&c1,&c0);
		c_no[i]=c1.pixel;
	}	
	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);
	gc = XCreateGC(dpy, w, 0, NULL);
	XSelectInput(dpy, w, KeyPressMask);
	XMapWindow(dpy, w);
	for(int y=0; y<SIZE_Y; y++)
		for(int x=0; x<SIZE_X; x++) {
			panel[x][y].loc.x = 200+x*SIZE;
			panel[x][y].loc.y = 50+y*SIZE;
			panel[x][y].color = 0;
			if(y==0 || y==SIZE_Y-1 || x==0 || x==SIZE_X-1) {
				if(x<(SIZE_X/2)-2 || x>(SIZE_X/2)-2+3)
					panel[x][y].color = 8;
				else if(y!=0)
					panel[x][y].color = 8;
			}
		}
}

void print_board() {
	for(int y=0; y<SIZE_Y; y++)
		for(int x=0; x<SIZE_X; x++) {
			XDrawRectangle(dpy, w, gc, panel[x][y].loc.x, panel[x][y].loc.y, SIZE, SIZE);
			XSetForeground(dpy, gc, c_no[panel[x][y].color]);
			XFillRectangle(dpy, w, gc, panel[x][y].loc.x+1, panel[x][y].loc.y+1, SIZE-1, SIZE-1);
			XSetForeground(dpy, gc, c_no[9]);
		}
}

void clear_block() {
	for(int x=0; x<4; x++)
		panel[memori[x].x][memori[x].y].color = 0;
}

void wmEventReader() {
	XNextEvent(dpy,&e);
	int key = e.xkey.keycode;
	switch(key) {
		case 111 :
			bRotation = true;
			wmRotationBlock();
			break;
		case 116 :
			yStart++;
			break;
		case 114 :
			xStart++;
			break;
		case 113 :
			xStart--;
			break;
		case 65 :
			while(!bNextBlock) {
				yStart++;
				clear_block();
				if(!wmBlockDown()) {
					yStart -= 1;
					wmBlockDown();
					bNextBlock = true;
				}
			}
			break;
		case 24 :
			bGameOver = true;
			break;
	}
}

void wmCreateBlock(int mBlock) {
	xStart = (SIZE_X/2)-2;
	yStart = 0;
	int num = 0;
	if(!wmChecker()) {
		bGameOver = true;
		return;
	}
	for(int y=0; y<4; y++)
		for(int x=0; x<4; x++) {
			if(panel[x+xStart][y+yStart].color==0 && block[mBlock][x][y]!=0) {
				panel[x+xStart][y+yStart].color = block[mBlock][x][y];
				memori[num].x = x+xStart;
				memori[num].y = y+yStart;
				num++;
			}
		}
	bNextBlock = false;
	bMoved = true;
}

bool wmBlockDown() {
	int num = 0;
	correction = 0;
	if(!wmChecker()) {
		if(bRotation) {
			for(int i=0; i<3; i++)
				wmRotationBlock();
			bRotation = false;
		}
		else {
			if(xStart>(SIZE_X/2)-2)
				correction = -1;
			else
				correction = 1;
		}
		bMoved = false;
		return false;
	}
	for(int y=0; y<4; y++)
		for(int x=0; x<4; x++) {
			if(panel[x+xStart][y+yStart].color==0 && block[mBlock][x][y]!=0) {
				panel[x+xStart][y+yStart].color = block[mBlock][x][y];
				memori[num].x = x+xStart;
				memori[num].y = y+yStart;
				num++;
			}
		}
	bRotation = false;
	bMoved = true;
	return true;
}

void wmRotationBlock() {
	int temp[4][4];
	for(int y=0; y<4; y++)
		for(int x=0; x<4; x++)
			temp[x][y] = block[mBlock][x][y];
	for(int y=0; y<4; y++)
		for(int x=0; x<4; x++)
			block[mBlock][x][y] = temp[y][3-x];
}

bool wmChecker() {
	for(int y=0; y<4; y++)
		for(int x=0; x<4; x++)
			if(block[mBlock][x][y]!=0 && panel[x+xStart][y+yStart].color!=0)
				return false;
	return true;
}

void delete_line() {
	bool bLine = true;
	int line[4] = {0, 0, 0, 0};
	int num = 0;
	for(int y=0; y<SIZE_Y-1; y++) {
		for(int x=0; x<SIZE_X-1; x++)
			if(panel[x][y].color==0)
				bLine = false;
		if(bLine) {
			line[num] = y;
			num++;
		}
		bLine = true;
	}
	if(line[0]==0)
		return;
	num = 0;
	while(line[num]!=0) {
		for(int y=line[num]; y>1; y--)
			for(int x=1; x<SIZE_X-1; x++)
				panel[x][y].color = panel[x][y-1].color;
		num++;
		iScore++;
		if(mDown!=0)
			mDown--;
	}
	sleep(1);
}

void wmGameOver() {
	for(int y=0; y<SIZE_Y; y++)
		for(int x=0; x<SIZE_X; x++)
			if(panel[x][y].color!=0 && panel[x][y].color!=8)
				panel[x][y].color = 1;
}