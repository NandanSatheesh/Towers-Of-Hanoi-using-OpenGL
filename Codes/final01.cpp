#include <iostream>
#include <GL/glut.h>
#include <cmath>


#define NUM_DISCS 3
#define ROD_HEIGHT 3
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define PI 22/7.0
#define DISC_SPACING 0.6 
#define BOARD_X 10
#define BOARD_Y 5
//Keep DISC_SPACING between 0.3 to 0.6 for best results

using namespace std;

struct Vector3 {
	double x, y, z;
	Vector3() { x = y = z = 0.0; }
	Vector3(double x, double y, double z) : x(x), y(y), z(z) { }
	
};

struct Disc {
	Disc() { normal = Vector3(0.0, 0.0, 1.0); }
	
	Vector3 position; //location
	Vector3 normal;   //orientation
};



struct Rod {
	Vector3 positions[NUM_DISCS];
	int occupancy_val[NUM_DISCS];
};

struct GameBoard {
	double x_min, y_min, x_max, y_max; //Base in XY-Plane
	double rod_base_rad;               //Rod's base radius
	Rod rods[3];	
};



//Game Globals
Disc discs[NUM_DISCS];
GameBoard t_board;


bool to_solve = false;


size_t moves = 0;

size_t window_width = WINDOW_WIDTH, window_height = WINDOW_HEIGHT;




void initialize();
void initialize_game();
void display_handler();
void reshape_handler(int w, int h);
void keyboard_handler(unsigned char key, int x, int y);




int main(int argc, char** argv)
{

    // Initialize GLUT Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Towers of Hanoi");

	initialize();
	cout << "Towers of Hanoi" << endl;
	cout << "Press H for Help" << endl;

	//Callbacks
	glutDisplayFunc(display_handler);
	glutReshapeFunc(reshape_handler);
	glutKeyboardFunc(keyboard_handler);
	
	

	glutMainLoop();
	return 0;
}

void initialize()
{
	glClearColor(0,0,0,0); //Setting the clear color
	

	glEnable(GL_DEPTH_TEST); //Enabling Depth Test

	//Setting Light0 parameters
	GLfloat light0_pos[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // A positional light
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

	glEnable(GL_LIGHTING);  //Enabling Lighting
	glEnable(GL_LIGHT0);   //Enabling Light0	

	//Initializing Game State 
	initialize_game();

}

void initialize_game()
{
	//Initializing 1)GameBoard t_board 2) Discs discs  3) ActiveDisc active_disc
   // State
	
	//1) Initializing GameBoard
	t_board.rod_base_rad = 1.0;
	t_board.x_min = 0.0;
	t_board.x_max = BOARD_X * t_board.rod_base_rad;
	t_board.y_min = 0.0;
	t_board.y_max = BOARD_Y * t_board.rod_base_rad;

	double x_center = (t_board.x_max - t_board.x_min) / 2.0;
	double y_center = (t_board.y_max - t_board.y_min) / 2.0;

	double dx = (t_board.x_max - t_board.x_min) / 3.0; //Since 3 rods
	double r = t_board.rod_base_rad;

    //Initializing Rods Occupancy value		
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t h = 0; h < NUM_DISCS; h++)
		{
			if (i == 0) 
			{
				t_board.rods[i].occupancy_val[h] = NUM_DISCS - 1 - h;
			}
			else
				t_board.rods[i].occupancy_val[h] = -1;
		}
	}

	
	//Initializing Rod positions
	for (size_t i = 0; i < 3; i ++)
	{
		for (size_t h = 0; h < NUM_DISCS; h++)
		{
			double x = x_center + ((int)i - 1) * dx;
			double y = y_center;
			double z = (h + 1) * DISC_SPACING;
			Vector3& pos_to_set = t_board.rods[i].positions[h];
			pos_to_set.x = x;
			pos_to_set.y = y;
			pos_to_set.z = z;	
		}
	}


	//2) Initializing Discs
	for (size_t i = 0; i < NUM_DISCS; i++)
	{
		discs[i].position = t_board.rods[0].positions[NUM_DISCS - i - 1];
		//Normals are initialized whie creating a Disc object - ie in constructor of Disc
	}



}

//Draw function for drawing a cylinder given position and radius and height
void draw_solid_cylinder(double x, double y, double r, double h)
{
	GLUquadric* q = gluNewQuadric();
	GLint slices = 50;
	GLint stacks = 10;
	
	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	gluCylinder(q, r, r, h, slices, stacks);
	glTranslatef(0, 0, h);
	gluDisk(q, 0, r, slices, stacks);
	glPopMatrix();
	gluDeleteQuadric(q);
}

//Draw function for drawing rods on a given game board i.e. base
void draw_board_and_rods(GameBoard const& board)
{
	//Materials, 
	GLfloat mat_white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_yellow[] = { 0.9654f, 0.9870f, 0.3210f, 1.0f };

	glPushMatrix();
	//Drawing the Base Rectangle [where the rods are placed]
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex2f(board.x_min, board.y_min);
		glVertex2f(board.x_min, board.y_max);
		glVertex2f(board.x_max, board.y_max);
		glVertex2f(board.x_max, board.y_min);
	glEnd();

	//Drawing Rods and Pedestals
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_yellow);

	double r = board.rod_base_rad;
	for (size_t i = 0; i < 3; i++)
	{
		Vector3 const& p = board.rods[i].positions[0];
		draw_solid_cylinder(p.x, p.y, r * 0.1, ROD_HEIGHT - 0.1);
		draw_solid_cylinder(p.x, p.y, r, 0.1);
	}

	glPopMatrix();
}


// Draw function for drawing discs
void draw_discs()
{
	int slices = 100;
	int stacks = 10;

	double rad;

	GLfloat r, g, b;
	GLfloat emission[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat material[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	for (size_t i = 0; i < NUM_DISCS; i++)
	{
		switch (i)
		{
		case 0: r = 0; g = 0; b = 1;
			break;
		case 1: r = 0; g = 1; b = 0;
			break;
		case 2: r = 0, g = 1; b = 1;
			break;
		case 3 : r = 1; g = 0; b =0 ;
			break ;
		case 4 : r = 1; g = 0; b = 1;
			break; 
		case 5 : r = 1; g = 1; b = 0;
			break; 
		default: r = g = b = 1;
			break;
		};

		material[0] = r;
		material[1] = g;
		material[2] = b;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);

		GLfloat u = 0.0f;
		
		

		GLfloat factor = 1.0f;
		switch (i) {
			case 0: factor = 0.2;
				break;
			case 1: factor = 0.4;
				break;
			case 2: factor = 0.6;
				break;
			case 3: factor = 0.8;
				break;
			default: break;
		};
		rad = factor * t_board.rod_base_rad;
		

		glPushMatrix();
		glTranslatef(discs[i].position.x, discs[i].position.y, discs[i].position.z);
		double theta = acos(discs[i].normal.z);
		theta *= 180.0f / PI;
		
		glutSolidTorus(0.2 * t_board.rod_base_rad, rad, stacks, slices);
		glPopMatrix();

		glMaterialfv(GL_FRONT, GL_EMISSION, no_emission); 
	}
}

void display_handler()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double x_center = (t_board.x_max - t_board.x_min) / 2.0;
	double y_center = (t_board.y_max - t_board.y_min) / 2.0;
	double r = t_board.rod_base_rad;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	x_center, y_center - 10.0, 3.0 * r,
				x_center, y_center, 3.0,
				0.0, 0.0, 1.0  );

	glPushMatrix();
		draw_board_and_rods(t_board);
		draw_discs();	
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

void reshape_handler(int w, int h)
{
	window_width= w;
	window_height = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 20.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}




void keyboard_handler(unsigned char key, int x, int y)
{

    //Console Outputs
	switch (key) 
	{
	case 27: cout << "Steps Taken: " << moves << endl;
		exit(0);
		break;

	case 'h':
	case 'H': 
		cout << "ESC: Quit" << endl;
		cout << "S: Solve from Initial State" << endl;
		cout << "T: shows the current number of steps used" << endl;
		cout << "H: Help" << endl;
		break;


	case 't':
	case 'T':
		cout << "Steps = " << moves << endl;
		break;

	default:
		break;
	};
}
