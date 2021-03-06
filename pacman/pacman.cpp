//
// pacman.cpp: classic arcade game
//
// Structure:
// function main gets called first and mainly initializes GLUT the graphics library
// GLUT works the following way:
// 1. initializes the window
// 2. add a "display" function. This function draws objects in the window. Here: rocks, the ship and torpedos. 
//    If a new object gets introduced, for example the alien saucer that shows up every once in a while
//    this function would have to be changed
// 3. add an "idle" function. This function animates the objects. It caluluates a new position for the rocks by adding the speed to the old position
//    It checks whether rocks are colliding with each other and determines how to bounce them of each other. It checks for collision of rocks with the ship
//    It checks whether torpedos are active and need to be animated. 
//    It also animates the ship with its current speed and if it is under thrust adds a little flame
//    Further it checks whether any object is "out of bounds" and wraps it around or erases it.
// 4. add a "keyboard" function. This function reacts to keypresses. For example 'a' and 's' rotate the ship, so a new position get calculated here.
//

#include <iostream>   // to print debug messages
#include <algorithm>  // max and min
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
#include <chrono>     // to wait milliseconds, used in display
#include <thread>     // also for wait
#include <ctime>      // time based functions, here used to seed rand()


// windows size
#define SIZEX 1000
#define SIZEY 1000

// for angle radian converion - C sin() and cos() want radians, we think here in angles
#define PI 3.14

// keys that cause actions - move left, right, up and down, make frame rate faster, slower and reset to orginal state
#define LEFT 'a'
#define RIGHT 's'
#define UP 'w'
#define DOWN 'z'
#define FASTER '+'
#define SLOWER '-'
#define RESET 'r'

#define MAXDOTS 1000
#define MAXLINES 100
#define STEP 10


// define the pacman, lines and dots
struct pacman {
	int x, y;
	int radius;
	char direction;
	bool mouthopen;
};

struct dot {
	int x, y, radius;
	bool active;
};

struct line {
	int startx, starty;
	int endx, endy;
};


// variables
pacman p;
dot dots[MAXDOTS];
line lines[MAXLINES];
int nroflines;
int nrofdots;

int frames;
int score;


//// functions

void setline(int gridx1, int gridy1, int gridx2, int gridy2) {
	lines[nroflines].startx = 5 + ((gridx1 - 1) * 30);
	lines[nroflines].starty = 5 + ((gridy1 - 1) * 30);
	lines[nroflines].endx = 5 + ((gridx2 - 1) * 30);
	lines[nroflines].endy = 5 + ((gridy2 - 1) * 30);
	
	if (nroflines < MAXLINES) {
		nroflines++;
	}
}


void setdots(int gridx1, int gridy1, int gridx2, int gridy2) {
	int x, y;


	if (gridx1 == gridx2) {
		for (y = gridy1; y < gridy2; y++) {
			dots[nrofdots].x = 5 + (gridx1 - 1) * 30;
			dots[nrofdots].y = 5 + (y - 1) * 30;
			dots[nrofdots].radius = 1;
			dots[nrofdots].active = true;
			if (nrofdots < MAXDOTS) {
				nrofdots++;
			}
		}
	}
	else {
		for (x = gridx1; x < gridx2; x++) {
			dots[nrofdots].x = 5 + (x - 1) * 30;
			dots[nrofdots].y = 5 + (gridy1 - 1) * 30;
			dots[nrofdots].radius = 1;
			dots[nrofdots].active = true;
			if (nrofdots < MAXDOTS) {
				nrofdots++;
			}
		}
	}
}


void mkbox(int x1, int y1, int x2, int y2) {
	setline(x1, y1, x2, y1);
	setline(x2, y1, x2, y2);
	setline(x2, y2, x1, y2);
	setline(x1, y2, x1, y1);
	setdots(x1 - 1, y1 - 1, x2 + 2, y1 - 1);
	setdots(x1 - 1, y2 + 1, x2 + 2, y2 + 1);
	setdots(x1 - 1, y1, x1 - 1, y2 + 1);
	setdots(x2 + 1, y1, x2 + 1, y2 + 1);
}


void initvars() {
	int i, j;


	srand(time(NULL));
	p.x = SIZEX / 2 - 5;
	p.y = SIZEY / 2 + 15;
	p.radius = 24;

	for (i = 0; i < MAXDOTS; i++) {
		dots[i].active = false;
	}

	nroflines = 0;
	nrofdots = 0;
	// outerbox
	setline(1, 1, 34, 1);
	setline(34, 1, 34, 17);
	setline(34, 19, 34, 32);
	setline(34, 32, 1, 32);
	setline(1, 32, 1, 19);
	setline(1, 17, 1, 1);

	// horizontal corridor with dots - corridors are odd, dots are even
	setline(1, 17, 15, 17);
	setline(1, 19, 15, 19);
	setdots(1, 18, 15, 18);
	setline(19, 17, 34, 17);
	setline(19, 19, 34, 19);
	setdots(19, 18, 34, 18);

	// right upper
	mkbox(23, 25, 32, 30);
	mkbox(19, 21, 21, 30);
	mkbox(23, 21, 27, 23);
	mkbox(29, 21, 32, 23);

	//left upper
	mkbox(3, 21, 7, 30);
	mkbox(9, 25, 17, 30);
	mkbox(9, 21, 17, 23);

	// right lower rectangle
	mkbox(15, 3, 25, 7);
	mkbox(27, 3, 32, 9);
	mkbox(27, 11, 32, 15);
	mkbox(21, 9, 25, 11);
	mkbox(21, 13, 25, 15);

	// left lower rectangle
	mkbox(3, 3, 13, 7);
	mkbox(1, 9, 7, 11);
	mkbox(1, 13, 7, 15); 
	mkbox(9, 9, 11, 15);
	mkbox(13, 9, 19, 15);

	// optimize dots
	for (i = 0; i < MAXDOTS; i++) {
		for (j = i+1; j < MAXDOTS; j++) {
			if ((dots[i].x == dots[j].x) && (dots[i].x == dots[j].y)) {
				dots[j].active = false;
			}
		}
	}
	p.mouthopen = true;
	frames = 80;
	score = 0;
}


void printScore(int s) {
	char vstr[80];
	int i;

	sprintf_s(vstr, "%07d", s);
	glRasterPos2f(500, 950); // location to start printing text
	glColor3f(1.0f, 1.0f, 1.0f);
	for (i = 0; i < strlen(vstr); i++) // loop until i is greater then length
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, vstr[i]); // Print a character on the screen
	}
}


bool collisiondetected(pacman r1, dot r2) {
	int dist, rad;

	dist = (r1.x - r2.x) * (r1.x - r2.x) + (r1.y - r2.y) * (r1.y - r2.y);
	rad = (r1.radius + r2.radius) * (r1.radius + r2.radius);
	if ((dist <= rad) && ( r1.y == r2.y )) {
		return true;
	}
	else {
		return false;
	}
}

bool hitwall(pacman p, char direction) {
	int i;

	// check if pacman will be colliding with any of the walls, common case is a line perpendicular to movement, but there is also head on
	for (i = 0; i < nroflines; i++) {
		switch (direction) {
		case LEFT:
			// if line is perpendicular, i.e. vertical
			if (lines[i].startx == lines[i].endx) {
				// does the line exist on the y coordinate of the pacman, i.e. it could end above or below
				if (((p.y + p.radius) >= std::min(lines[i].starty, lines[i].endy)) && 
					((p.y+p.radius) <= std::max(lines[i].starty, lines[i].endy))) {
					std::cout << i << " " << p.x << " " << lines[i].startx << "\n";
					// is the pacman to the right of it, if we are to left and we want to geo left that line does not matter 
					if (p.x - p.radius > lines[i].startx) {
						// will the next move to the left cross the line 
						std::cout << i << " " << p.x << " " << lines[i].startx << " " << p.y << " " << lines[i].endy << "\n";
						if ((p.x - p.radius - STEP) <= lines[i].startx) {
							std::cout << "LEFT WALL\n";
							return true;
						}
					}
				}
			}
			else {
				// head on case - circle overlaps the horizontal line, we are to the right of it and the next step would collide
				if (((p.y + p.radius) >= lines[i].starty) &&
					((p.y - p.radius) <= lines[i].starty) &&
					((p.x - p.radius) > std::max(lines[i].startx, lines[i].endx)) &&
					((p.x - p.radius - STEP) < std::max(lines[i].startx, lines[i].endx))) {
					return true;
				}
			}
			break;
		case RIGHT:
			// if line is vertical
			if (lines[i].startx == lines[i].endx) {
				// does the line exist on the y coordinate of the pacman
				if (((p.y + p.radius) >= std::min(lines[i].starty, lines[i].endy)) &&
					((p.y + p.radius) <= std::max(lines[i].starty, lines[i].endy))) {
					std::cout << i << " " << p.x << " " << lines[i].startx << "\n";
					// is the pacman to the left of it 
					if (p.x + p.radius < lines[i].startx) {
						// will the next move to the RIGHT cross the line
						std::cout << i << " " << p.x << " " << lines[i].startx << "\n";
						if ((p.x + p.radius + STEP) >= lines[i].startx) {
							std::cout << "RIGHT WALL\n";
							return true;
						}
					}
				}
			}
			else {
				// head on case - circle overlaps the horizontal lines, we are to the left it and the next step would collide
				if (((p.y + p.radius) >= lines[i].starty) &&
					((p.y - p.radius) <= lines[i].starty) &&
					((p.x + p.radius) < std::min(lines[i].startx, lines[i].endx)) &&
					((p.x + p.radius + STEP) > std::min(lines[i].startx, lines[i].endx))) {
					return true;
				}
			}
			break;
		case UP:
			// if line is horizontal
			if (lines[i].starty == lines[i].endy) {
				// does the line exist on the x coordinate of the pacman
				if (((p.x + p.radius) >= std::min(lines[i].startx, lines[i].endx)) &&
					((p.x + p.radius) <= std::max(lines[i].startx, lines[i].endx))) {
					std::cout << i << " " << p.y << " " << lines[i].starty << "\n";
					// is the pacman to the bottom of it 
					if (p.y + p.radius < lines[i].starty) {
						// will the next move to the left cross the line
						std::cout << i << " " << p.y << " " << lines[i].starty << " " << p.x << " " << lines[i].endx << "\n";
						if ((p.y + p.radius + STEP) >= lines[i].starty) {
							std::cout << "UP WALL\n";
							return true;
						}
					}
				}
			}
			else {
				// head on case - circle overlads the vertical lines, we are below it and the next step would collide
				if (((p.x + p.radius) >= lines[i].startx) &&
					((p.x - p.radius) <= lines[i].startx) &&
					((p.y + p.radius) < std::min(lines[i].starty, lines[i].endy)) &&
					((p.y + p.radius + STEP) > std::min(lines[i].starty, lines[i].endy))) { 
					return true;
				}
			}
			break;
		case DOWN:
			// if line is horizontal
			if (lines[i].starty == lines[i].endy) {
				// does the line exist on the x coordinate of the pacman
				if (((p.x + p.radius) >= std::min(lines[i].startx, lines[i].endx)) &&
					((p.x + p.radius) <= std::max(lines[i].startx, lines[i].endx))) {
					std::cout << i << " " << p.y << " " << lines[i].starty << "\n";
					// is the pacman to the top of it 
					if (p.y - p.radius > lines[i].starty) {
						// will the next move to the left cross the line
						std::cout << i << " " << p.y << " " << lines[i].starty << " " << p.x << " " << lines[i].endx << "\n";
						if ((p.y - p.radius - STEP) <= lines[i].starty) {
							std::cout << "BOTTOM WALL\n";
							return true;
						}
					}
				}
			}
			else {
				// head on case - circle overlaps the vertical lines, we are above it and the next step would collide
				if (((p.x + p.radius) >= lines[i].startx) &&
					((p.x - p.radius) <= lines[i].startx) &&
					((p.y - p.radius) > std::max(lines[i].starty, lines[i].endy)) &&
					((p.y - p.radius - STEP) < std::max(lines[i].starty, lines[i].endy))) {
					return true;
				}
			}
			break;
		}
	}
	return false;
}

void display() {
	float fi;
	int circlex, circley;
	int i;


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

	// draw the lines
	for (i = 0; i < nroflines; i++) {
		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2i(lines[i].startx, lines[i].starty);
		glVertex2i(lines[i].endx, lines[i].endy);
		glEnd();
	}

	// draw the dots
	for (i = 0; i < MAXDOTS; i++) {
		if (dots[i].active) {
			glBegin(GL_POLYGON);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex2i(dots[i].x + 1, dots[i].y + 1);
			glVertex2i(dots[i].x - 1, dots[i].y + 1);
			glVertex2i(dots[i].x - 1, dots[i].y - 1);
			glVertex2i(dots[i].x + 1, dots[i].y - 1);
			glEnd();
		}
	}

	// draw the pacman
	glBegin(GL_POLYGON);
	glColor3f(0.9f, 0.7f, 0.4f);
	for (fi = 0; fi < 2 * PI; fi += PI / 12) {
		circlex = p.x + cos(fi) * p.radius;
		circley = p.y + sin(fi) * p.radius;
		glVertex2i(circlex, circley);
	}
	glEnd();
	if (p.mouthopen) {
		// draw a triangle on top of the circle in the direction of movement.
		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex2i(p.x, p.y);
		switch (p.direction) {
		case LEFT:
			glVertex2i(p.x - p.radius, p.y + p.radius);
			glVertex2i(p.x - p.radius, p.y - p.radius);
			break;
		case UP:
			glVertex2i(p.x - p.radius, p.y + p.radius);
			glVertex2i(p.x + p.radius, p.y + p.radius);
			break;
		case RIGHT:
			glVertex2i(p.x + p.radius, p.y + p.radius);
			glVertex2i(p.x + p.radius, p.y - p.radius);
			break;
		case DOWN:
			glVertex2i(p.x - p.radius, p.y - p.radius);
			glVertex2i(p.x + p.radius, p.y - p.radius);
			break;
		}
		glEnd();
		p.mouthopen = !p.mouthopen;
	}

	printScore(score);
	glFlush();  // Render now
}


void processNormalKeys(unsigned char key, int a, int b) {
	int i;


	std::cout << "in Keys\n";
	// go left or rotate right, up or down
	if (key == LEFT) {
		p.mouthopen = !p.mouthopen;
		p.direction = LEFT;
		if (!hitwall(p, LEFT)) {
			p.x -= STEP;
		}
	}
	if (key == UP) {
		p.mouthopen = !p.mouthopen;
		p.direction = UP;
		if (!hitwall(p, UP)) {
			p.y += STEP;
		}
	}
	if (key == RIGHT) {
		p.mouthopen = !p.mouthopen;
		p.direction = RIGHT;
		if (!hitwall(p, RIGHT)) {
			p.x += STEP;
		}
	}
	if (key == DOWN) {
		p.mouthopen = !p.mouthopen;
		p.direction = DOWN;
		if (!hitwall(p, DOWN)) {
			p.y -= STEP;
		}
	}
	if (key == RESET) {
		initvars();
	}

	// check if a dot was eaten
	for (i = 0; i < MAXDOTS; i++) {
		if ( dots[i].active ) {
			if (collisiondetected(p, dots[i])) {
				dots[i].active = false;
				score += 100;
			}
		}
	}

	// check if wrap occurrs
	if (p.x < 0) {
		p.x = SIZEX;
	}
	if (p.x > SIZEX) {
		p.x = 0;
	}
	if (p.y < 0) {
		p.y = SIZEY;
	}
	if (p.y > SIZEY) {
		p.y = 0;
	}

	display();
}


void idleFunc() {

	std::this_thread::sleep_for(std::chrono::milliseconds(frames));
	display();
}


/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {

	initvars();

	glutInit(&argc, argv);                           // Initialize GLUT

	glutCreateWindow("OpenGL Pacman v1");            // Create a window with the given title
	glutInitWindowSize(SIZEX, SIZEY);                // Set the window's initial width & height
	glutInitWindowPosition(0, 0);                    // Position the window's initial top-left corner
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, SIZEX, 0, SIZEY);

	glutKeyboardFunc(processNormalKeys);             // register the callback for keypress
	glutIdleFunc(idleFunc);

	glutDisplayFunc(display); // Register display callback handler for window re-paint
	glutMainLoop();           // Enter the infinitely event-processing loop
	return 0;
}

