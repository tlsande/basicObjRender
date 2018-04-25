//
//		          Programming Assignment #1
//
//			        Victor Zordan
//
//               Lee Sanders
//
//
/***************************************************************************/

                                                   /* Include needed files */
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
//#include <iostream>


#include <GL/glut.h>   // The GL Utility Toolkit (Glut) Header

void write_pixel(int x, int y, double intensity);

#define WIDTH 1000
#define HEIGHT 1000
#define DEPTH 1000
#define maxVerts 300
#define maxPolys 150

struct Point {
   int x;
   int y;
};

void drawLine(Point p1, Point p2);





int x_last,y_last;


/***************************************************************************/
// vars created for proj 2 (would be much easier to create classes next time)
typedef struct vec {
  float x;
  float y;
  float z;
}Vec;

typedef struct edgetable {
  double yUpper;
  double xLower;
  double invSlope;
}EdgeTable;

EdgeTable edgeTable[3];
EdgeTable aet[3];


Vec vertex[maxVerts];
int numVerts;
Vec normal[maxVerts];
int numNorm;
Vec textureCoords[maxVerts];
int nTx;
Vec center;

typedef struct poly{
  int vertex[3];
  int normals[3];
}Polygon;

Polygon polys[maxPolys];
int polyCount;

typedef struct polygon{
  vec vertex[3];
}Polygons;
Polygons polygons[maxPolys];
int numPolygons;


double size = .5;

int geo = 0;
/***************************************************************************/

// Work vars I created
//struct Point t1;
//struct Point t2;

// 4 control points
struct Point Points[4];


bool ranBezier = false;
bool gotPoints = false;

int curPoint = 0;

int oldX;
int oldY;

// How many lines to draw for bezier curve
const int numPoints = 20;

double n[numPoints+1];
struct Point curvePoints[numPoints+1];

int pixelRange = 5;
int pointToUpdate = -1;
int oldCPX = 0, oldCPY = 0;
struct Point newPoint, oldPoint;

bool startUpdate = false;
int dif;
int updates = 200;
double mu = 0.0;
double inc = 1.0 / updates;
int count = 0;

/***************************************************************************/
// Codes for proj 2 here
/***************************************************************************/
// parse .obj to get relevant info
void parseObj(char* in) {
  FILE* file = fopen(in, "r");
  if(file == NULL) {
    printf("File not opened\n");
  }
  else {
    printf("File %s opened\n", in);

    char word[128];
    while(fscanf(file, " %127s", word) == 1) {
      if (strcmp(word, "v") == 0){
        fscanf(file, "%f %f %f\n", &vertex[numVerts].x, &vertex[numVerts].y, &vertex[numVerts].z );
        numVerts++;
      }
      else if (strcmp(word, "vt") == 0) {
        fscanf(file, "%f %f\n", &textureCoords[nTx].x, &textureCoords[nTx].y );
        nTx++;
      }
      else if(strcmp(word, "vn") == 0) {
        fscanf(file, "%f %f %f\n", &normal[numNorm].x, &normal[numNorm].y, &normal[numNorm].z);
        numNorm++;
      }
      else if(strcmp(word, "f") == 0) {
        fscanf(file, "%i/%*i/%i %i/%*i/%i %i/%*i/%i\n",
                      &polys[polyCount].vertex[0], &polys[polyCount].normals[0],
                      &polys[polyCount].vertex[1], &polys[polyCount].normals[1],
                      &polys[polyCount].vertex[2], &polys[polyCount].normals[2]);
        polyCount++;
      }
    }
  }
}
// print out parsed obj info
void printObj() {
  printf("---------------------Vertexes---------------------\n");
  //printf("Vertex 0 (%f, %f, %f)\n", vertex[0].x, vertex[0].y, vertex[0].z);
  for(int i = 0; i < numVerts; i++) {
    printf("Vertex %i (%f, %f, %f)\n", i, vertex[i].x, vertex[i].y, vertex[i].z);
  }


  printf("---------------------TextureC---------------------\n");
  for(int i = 0; i < nTx; i++) {
    printf("Texture coordinate %i (%f, %f)\n", i, textureCoords[i].x, textureCoords[i].y);
  }

  printf("---------------------Normals----------------------\n");
  for(int i = 0; i < numNorm; i++) {
    printf("Normal %i (%f, %f, %f)\n", i, normal[i].x, normal[i].y, normal[i].z);
  }
  printf("---------------------Normals----------------------\n");
  for(int i =0; i < polyCount; i++) {
    printf("Polygon, Vertices = {(%f, %f, %f), (%f, %f, %f), (%f, %f, %f)}\n",
          vertex[polys[i].vertex[0]-1].x, vertex[polys[i].vertex[0]-1].y, vertex[polys[i].vertex[0]-1].z,
          vertex[polys[i].vertex[1]-1].x, vertex[polys[i].vertex[1]-1].y, vertex[polys[i].vertex[1]-1].z,
          vertex[polys[i].vertex[2]-1].x, vertex[polys[i].vertex[2]-1].y, vertex[polys[i].vertex[2]-1].z);
  }
}

// calculate and set center of model
void getCenter() {
  double x, y, z;
  for(int i = 0; i < numVerts; i++) {
    // center.x += vertex[i].x;
    // center.y += vertex[i].y;
    // center.z += vertex[i].z;

    x += vertex[i].x;
    y += vertex[i].y;
    z += vertex[i].z;
  }
  //printf("test Center = (%f, %f, %f)\n", x,y, z);
  x /= numVerts;
  y /= numVerts;
  z /= numVerts;

  center.x = x;
  center.y = y;
  center.z = z;

  //printf("test Center = (%f, %f, %f)\n", center.x, center.y, center.z);
}

void printCenter() {
  printf("Center = (%f, %f, %f)\n", center.x, center.y, center.z);
}

// scale vertices based off a scale factor and on which axis
void scale(double factor, char axis) {
  for(int i = 0; i < numVerts; i++) {
    if(axis == 'x') {
      vertex[i].x *= factor;
    }
    else if(axis == 'y') {
      vertex[i].y *= factor;
    }
    else if(axis == 'z') {
      vertex[i].z *= factor;
    }
    else {
      vertex[i].x *= factor;
      vertex[i].y *= factor;
      vertex[i].z *= factor;
    }
  }
  getCenter();
}

// Move model to fit screen
void setModel() {
  double xMin, xMax, yMin, yMax, zMin, zMax;
  double scaleUp = 1.25, scaleDown = .75;

  xMax = xMin = vertex[0].x;
  yMin = yMax = vertex[0].y;
  zMin = zMax = vertex[0].z;

  for(int i = 0; i < numVerts; i++) {
    if(xMax < vertex[i].x) {
      xMax = vertex[i].x;
    }
    if(xMin > vertex[i].x) {
      xMin = vertex[i].x;
    }
    if(yMax < vertex[i].y) {
      yMax = vertex[i].y;
    }
    if(yMin > vertex[i].y) {
      yMin = vertex[i].y;
    }
    if(zMax < vertex[i].z) {
      zMax = vertex[i].z;
    }
    if(zMin > vertex[i].z) {
      zMin = vertex[i].z;
    }
  }

  double width = sqrt((xMax - xMin)*(xMax - xMin));
  double height = sqrt((yMax - yMin)*(yMax - yMin));
  double depth = sqrt((zMax - zMin)*(zMax - zMin));

  if(width < WIDTH*size || height < HEIGHT*size || depth < DEPTH*size) {
    while(width < WIDTH*size || height < HEIGHT*size || depth < DEPTH*size) {
      scale(scaleUp, 'n');
      xMax *= scaleUp;
      xMin *= scaleUp;
      yMax *= scaleUp;
      yMin *= scaleUp;
      zMax *= scaleUp;
      zMin *= scaleUp;
      width = sqrt((xMax - xMin) * (xMax - xMin));
      height = sqrt((yMax - yMin) * (yMax - yMin));
      depth = sqrt((zMax - zMin) * (zMax - zMin));
    }
  }

  if(width > WIDTH*size || height > HEIGHT*size || depth > DEPTH*size || xMax > WIDTH || yMax > HEIGHT || zMax > DEPTH) {
    while(width > WIDTH*size || height > HEIGHT*size || depth > DEPTH*size || xMax > WIDTH || yMax > HEIGHT || zMax > DEPTH) {
      scale(scaleDown, 'n');
      xMax *= scaleDown;
      xMin *= scaleDown;
      yMax *= scaleDown;
      yMin *= scaleDown;
      zMax *= scaleDown;
      zMin *= scaleDown;
      width = sqrt((xMax - xMin) * (xMax - xMin));
      height = sqrt((yMax - yMin) * (yMax - yMin));
      depth = sqrt((zMax - zMin) * (zMax - zMin));    xMax = xMax * scaleDown;
    }
  }

  getCenter();

  for(int i = 0; i < numVerts; i++) {
      vertex[i].x = (vertex[i].x - center.x) + WIDTH/2;
      vertex[i].y = (vertex[i].y - center.y) + HEIGHT/2;
      vertex[i].z = (vertex[i].z - center.z) - DEPTH/2;
  }
  getCenter();
}

// Draws the wireframe of model
void drawWireFrame() {
  struct Point temp1, temp2, temp3;
  for(int i = 0; i < polyCount; i++) {
    temp1.x = vertex[polys[i].vertex[0] - 1].x;
    temp1.y = vertex[polys[i].vertex[0] - 1].y;
    temp2.x = vertex[polys[i].vertex[1] - 1].x;
    temp2.y = vertex[polys[i].vertex[1] - 1].y;
    temp3.x = vertex[polys[i].vertex[2] - 1].x;
    temp3.y = vertex[polys[i].vertex[2] - 1].y;

    drawLine(temp1, temp2);
    drawLine(temp2, temp3);
    drawLine(temp3, temp1);
  }
}

// Move vertices into polygons array
void setPolygons() {
  for(int i = 0; i < polyCount; i++) {
    // face 1
    polygons[i].vertex[0].x = vertex[polys[i].vertex[0] - 1].x;
    polygons[i].vertex[0].y = vertex[polys[i].vertex[0] - 1].y;
    polygons[i].vertex[0].z = vertex[polys[i].vertex[0] - 1].z;
    // face 2
    polygons[i].vertex[1].x = vertex[polys[i].vertex[1] - 1].x;
    polygons[i].vertex[1].y = vertex[polys[i].vertex[1] - 1].y;
    polygons[i].vertex[1].z = vertex[polys[i].vertex[1] - 1].z;
    // face 3
    polygons[i].vertex[2].x = vertex[polys[i].vertex[2] - 1].x;
    polygons[i].vertex[2].y = vertex[polys[i].vertex[2] - 1].y;
    polygons[i].vertex[2].z = vertex[polys[i].vertex[2] - 1].z;
  }
}

// translate model
void translate(double x, double y, double z) {
  for(int i = 0; i < numVerts; i++) {
    vertex[i].x += x;
    vertex[i].y += y;
    vertex[i].z += z;
  }
  getCenter();
}

// rotate model along an axis by a certain degree
void rotate(char axis, double degree) {
  double x = center.x, y = center.y, z = center.z;
  translate(-x, -y, -z);
  double t = degree * 3.14 / 180;
  double newX, newY, newZ;

  for(int i = 0; i < numVerts; i++) {
    if(axis == 'x') {
      newY = vertex[i].y * cos(t) + vertex[i].z * -sin(t);
      newZ = vertex[i].y * sin(t) + vertex[i].z * cos(t);
      vertex[i].y = newY;
      vertex[i].z = newZ;
    }
    else if(axis == 'y') {
      newX = vertex[i].x * cos(t) + vertex[i].z * sin(t);
      newZ = vertex[i].x * -sin(t) + vertex[i].z * cos(t);
      vertex[i].x = newX;
      vertex[i].z = newZ;
    }
  }
  getCenter();
  translate(-center.x + x, -center.y + y, -center.z + z);
}

// scale entire model
void scaleModel(double factor, char axis) {
  //double x = center.x, y = center.y, z = center.z;
  scale(factor, axis);

  //translate(-center.x + x, -center.y + y, -center.z + z);
}

/***************************************************************************/

// Draws a line from p1 to p2
void drawLine(Point p1, Point p2) {
   //printf("p1 = (%i, %i), p2 = (%i, %i)", p1.x, p1.y, p2.x, p2.y);
   int dx, dy;
   int inc;
   float xInc, yInc;
   float x = p1.x, y = p1.y;

   dx = p2.x - p1.x;
   dy = p2.y - p1.y;

   if(abs(dx) > abs(dy)) {
      inc = abs(dx);
   }
   else {
      inc = abs(dy);
   }

   xInc = dx / (float) inc;
   yInc = dy / (float) inc;

   for(int i = 0; i < inc; i++) {
      write_pixel(x, y, 1.0);
      x += xInc;
      y += yInc;
   }
}


/***************************************************************************/

void init_window()
                 /* Clear the image area, and set up the coordinate system */
{

        					       /* Clear the window */
        glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_SMOOTH);
        glOrtho(0,WIDTH,0,HEIGHT,-1.0,1.0);
}

/***************************************************************************/

void write_pixel(int x, int y, double intensity)
                                         /* Turn on the pixel found at x,y */
{

        glColor3f (intensity, intensity, intensity);
        glBegin(GL_POINTS);
           glVertex3i( x, y, 0);
        glEnd();
}

//***************************************************************************/

void display ( void )   // Create The Display Function
{

  //gluLookAt(0, 0, -1, 0, 0, 0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	      // Clear Screen


// Test stuff
    // struct Point temp1, temp2;
    // temp1.x = center.x;
    // temp1.y = center.y;
    // temp2.x = 0;
    // temp2.y = 0;
    // drawLine(temp1, temp2);
    drawWireFrame();

  glutSwapBuffers();                                      // Draw Frame Buffer
}

/***************************************************************************/
void mouse(int button, int state, int x, int y)
{
/* This function I finessed a bit, the value of the printed x,y should
   match the screen, also it remembers where the old value was to avoid multiple
   readings from the same mouse click.  This can cause problems when trying to
   start a line or curve where the last one ended */
        static int oldx = 0;
        static int oldy = 0;
	int mag;

	y *= -1;  //align y with mouse
	y += 500; //ignore
	mag = (oldx - x)*(oldx - x) + (oldy - y)*(oldy - y);
	if (mag > 20) {
		printf(" x,y is (%d,%d)\n", x,y);
	}
	oldx = x;
	oldy = y;
	x_last = x;
	y_last = y;
}

/***************************************************************************/
void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{

	switch ( key ) {
		case 27:              // When Escape Is Pressed...
			exit ( 0 );   // Exit The Program
			break;
	        case '1':             // stub for new screen
		        printf("New screen\n");
            break;
          case 't':
            geo = 1;
            printf("Translation\n");
            break;
          case 'r':
            geo = 2;
            printf("Rotating\n");
            break;
          case 'e':
            geo = 3;
            printf("Scaling\n");
            break;
		default:
			break;
	}
  // transformation
  if(geo == 1) {
    if(key == 'a') {
      translate(-1.5, 0, 0);
    }
    else if(key == 'w') {
      translate(0, 1.5, 0);
    }
    else if(key == 's') {
      translate(0, -1.5, 0);
    }
    else if(key == 'd') {
      translate(1.5, 0, 0);
    }
  }
  // rotation
  if(geo == 2) {
    if(key == 'd') {
      rotate('y', -1);
    }
    else if(key == 'a') {
      rotate('y', 1);
    }
    else if(key == 'w') {
      rotate('x', 1);
    }
    else if(key == 's') {
      rotate('x', -1);
    }
  }
  // scale
  if(geo == 3) {
    if(key =='a') {
      scaleModel(.90, 'x');
    }
    else if(key == 'w') {
      scaleModel(1.1, 'y');
    }
    else if(key == 's') {
      scaleModel(.9, 'y');
    }
    else if(key == 'd') {
      scaleModel(1.1, 'x');
    }
  }
}
/***************************************************************************/

int main (int argc, char *argv[])
{
  if(argc == 2) {
    printf("Reading obj file\n");
    printf("Name of file: %s\n", argv[1]);
    parseObj(argv[1]);
    printObj();

    getCenter();

    printCenter();

    setModel();
    printCenter();

    //setPolygons();
  }

/* This main function sets up the main loop of the program and continues the
   loop until the end of the data is reached.  Then the window can be closed
   using the escape key.						  */

	glutInit            ( &argc, argv );
       	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize  ( 1000,1000 );
	glutCreateWindow    ( "Computer Graphics" );
	glutDisplayFunc     ( display );
	glutIdleFunc	    ( display );
	glutMouseFunc       ( mouse );
	glutKeyboardFunc    ( keyboard );

        init_window();				             //create_window

	glutMainLoop        ( );                 // Initialize The Main Loop
}
