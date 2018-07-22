#include <iostream>
#include <GL/glut.h>
#include  <math.h>
#include <stdio.h>

using namespace std ;

void initialize();
void initialize_game();
void display_handler();
void reshape_handler(int w, int h);
void keyboard_handler(unsigned char key, int x, int y);


struct Vector3 {
	double x, y, z;
	Vector3() { x = y = z = 0.0; }
	Vector3(double x1, double y1, double z1) : x(x1), y(y1), z(z1) { }
};

struct Disc {
	
	Vector3 position; //location
	Vector3 normal;   //orientation
};

// Rods and Discs Globals - Can be changed for different levels
const int NUM_DISCS = 3;
const double ROD_HEIGHT = 5.0;

struct Rod {
	Vector3 positions[NUM_DISCS];
	int occupancy_val[NUM_DISCS];
};

struct GameBoard {
	double x_min, y_min, x_max, y_max; //Base in XY-Plane
	double rod_base_rad;               //Rod's base radius
	Rod rods[3];	
};

int intro = 0 ;

//Game Globals
Disc discs[NUM_DISCS];
GameBoard t_board;



//Globals for window, time, FPS, moves
int window_width = 900, window_height = 400;

const double PI = 3.141592653589793;









int main2()
{

    // Initialize GLUT Window
	//glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Towers of Hanoi");
	glutDestroyWindow(1);

	initialize();
	cout << "Towers of Hanoi" << endl;

	//Callbacks
	glutDisplayFunc(display_handler);
	glutReshapeFunc(reshape_handler);
	glutKeyboardFunc(keyboard_handler);


	glutMainLoop();
	return 0;
}

void initialize()
{
	glClearColor(1,1,1,1); //Setting the clear color
	
	glEnable(GL_DEPTH_TEST);  //Enabling Depth Test

	//Setting Light0 parameters
	
	GLfloat light0_pos[] = { 1.0f, 1.0f, 0.0f, 1.0f }; // A positional light
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

	glEnable(GL_LIGHTING);			  //Enabling Lighting
	glEnable(GL_LIGHT0);		      //Enabling Light0	


	initialize_game();

}

void initialize_game()
{

	// Initializing GameBoard
	t_board.rod_base_rad = 1;
	t_board.x_min = 0.0;
	t_board.x_max = 10 * t_board.rod_base_rad;
	t_board.y_min = 0.0;
	t_board.y_max = 5 * t_board.rod_base_rad;

	double x_center = (t_board.x_max - t_board.x_min) / 2.0;
	double y_center = (t_board.y_max - t_board.y_min) / 2.0;

	double dx = (t_board.x_max - t_board.x_min) / 3.0; //Since 3 rods
	double r = t_board.rod_base_rad;

    //Initializing Rods Occupancy value		
	for (size_t i = 0; i < 3; i++)
	{
	//	printf("%d ",i);
		for (size_t h = 0; h < NUM_DISCS; h++)
		{
			if (i == 0) 
			{
				t_board.rods[i].occupancy_val[h] = NUM_DISCS - 1 - h;
			}
			else t_board.rods[i].occupancy_val[h] = -1;
			//printf("%d ",t_board.rods[i].occupancy_val[h]);
		}
		//printf("\n");
	}
	
	//Initializing Rod positions
	for (size_t i = 0; i < 3; i ++)
	{
		for (size_t h = 0; h < NUM_DISCS; h++)
		{
			double x = x_center + ((int)i - 1) * dx;
			double y = y_center;
			double z = (h + 1) * 0.5;
			Vector3& pos_to_set = t_board.rods[i].positions[h];
			pos_to_set.x = x;
			pos_to_set.y = y;
			pos_to_set.z = z;	
			//printf("%f %f %f \n",x,y,z);
		}
	}

	//2) Initializing Discs
	for (size_t i = 0; i < NUM_DISCS; i++)
	{
		discs[i].position = t_board.rods[0].positions[NUM_DISCS - i - 1];
		/*printf(" %f ",discs[i].position.x);
		printf("%f ",discs[i].position.y);
		printf("%f \n ",discs[i].position.z);
		//Normals are initialized whie creating a Disc object - ie in constructor of Disc
		*/
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
	float yellow[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glColor3f(0,0,0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex2f(board.x_min, board.y_min);
		glVertex2f(board.x_min, board.y_max);
		glVertex2f(board.x_max, board.y_max);
		glVertex2f(board.x_max, board.y_min);
	glEnd();
	
	//Drawing Rods and Pedestals
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);

	double r = board.rod_base_rad;
	for (size_t i = 0; i < 3; i++)
	{
	
		Vector3 const& p = board.rods[i].positions[0];
		draw_solid_cylinder(p.x, p.y, r * 0.1, ROD_HEIGHT - 0.1);
		draw_solid_cylinder(p.x, p.y, r, 0.1);
	}

	glPopMatrix();
}

void display_handler()
{
	


	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		double x_center = (t_board.x_max - t_board.x_min) / 2.0;
		double y_center = (t_board.y_max - t_board.y_min) / 2.0;
		double r = t_board.rod_base_rad;
	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(x_center, y_center - 10.0, 3.0 * r,	x_center, y_center, 3.0,0.0, 0.0, 1.0);
					
		glPushMatrix();
		draw_board_and_rods(t_board);
		glPopMatrix();
		
		glFlush();
		glutSwapBuffers();
	
}

void reshape_handler(int w, int h)
{
	window_width = w;
	window_height = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (GLfloat)w / (GLfloat)h, 0.1, 20.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
}

void keyboard_handler(unsigned char key, int x, int y)
{
    //Console Outputs
	switch (key) 
	{
		case 'q' :
		case 'Q' :
		case 27 : exit(0);
		
		case 'n':
		case 'N':
			intro = 1 ;
				glutDisplayFunc(display_handler);
	glutReshapeFunc(reshape_handler);
	glutKeyboardFunc(keyboard_handler);
	glutIdleFunc(display_handler);
			main2();
		
			break;
			
			
		default: break;
	}
}


void drawBitmapText(char *string,float x,float y,float z) 
{  
	char *c;
	glRasterPos3f(x, y,z);

	for (c=string; *c != '\0'; c++) 
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
	}
}

void drawStrokeText(char*string,int x,int y,int z)
{
	  char *c;
	  glPushMatrix();
	  glTranslatef(x, y+8,z);
	glScalef(0.49f,-0.508f,z);
  
	  for (c=string; *c != '\0'; c++)
	  {
    		glutStrokeCharacter(GLUT_STROKE_ROMAN , *c);
	  }
	  glPopMatrix();
}

void init()
{
	glClearColor(1.0,1.0,1.0,1.0); 
}


void reshape(int w,int h) 
{ 
 
    glViewport(0,0,w,h); 
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity(); 
    gluOrtho2D(0,w,h,0); 
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity(); 

}


void render(void)
{ 
	glClear(GL_COLOR_BUFFER_BIT); 
	glLoadIdentity();
 
	glColor3f(1,1,0);
	drawStrokeText("Towers of Hanoi Simulation",200,200,0);
	glColor3f(1,1,1);
	drawStrokeText("A Project by Nandan A.S. ",400,600,0);

	glutSwapBuffers(); 
} 



int main(int argc, char* argv[])
{
		// initialize glut 
        glutInit(&argc, argv); 
 
        // specify the display mode to be RGB and single buffering  
        // we use single buffering since this will be non animated 
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE); 
 
        // define the size 
        glutInitWindowSize(1400,1400); 
 
        // the position where the window will appear 
        glutInitWindowPosition(100,100); 
        glutCreateWindow("TOH"); 
 		glutKeyboardFunc(keyboard_handler);
        glutDisplayFunc(render);
		
        glutReshapeFunc(reshape); 

        // enter the main loop 
        glutMainLoop(); 
	return 0;
}

