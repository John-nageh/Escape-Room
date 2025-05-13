#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <gl/tiny_obj_loader.h>
#include<iostream>
#include<string>
#include<Windows.h>
#include<GL/stb_image.h>
#include<GL/glut.h>
#include <random>
#include <chrono>
#include <cmath>

using namespace std;

mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());

GLuint textures[20];  // VERY IMPORTANT: CHANGE NUMBER BASED ON NUMBER OF TEXTURES
// Skeleton model data
vector<float> skeletonVertices;
vector<float> skeletonTexCoords;

// Skull model data
vector<float> skullVertices;
vector<float> skullTexCoords;

unsigned int texture;
int width, height, nrChannels;
unsigned char* data = NULL;
bool fullScreenMode = true, lightOn = false;
float eyey = 10, eyez = 40, eyex = 0;
float centerX, centerY, centerZ = 20;
float rat = 0;


const double PI = acos(-1);				// Start of Camera Variables
bool firstMouse = true;
float yaw = -80, pitch = -20;
float lastX = 400, lastY = 300;
float sensitivity = 0.05f;
bool skipNextMouseMovement = true;	//End of Camera Variables


float fanRotationAngle = 0.0f;
float matamb[] = { 1.0f,1.0f,1.0f,1.0f },
matdiff[] = { 1.0f,1.0f,1.0f,1.0f },
matspec[] = { 0.64f,1.0f,1.0f,1.0f },
matshin[] = { 4 },
lightamb[] = { 1,1,1,1 },
lightdiff[] = { 0,.6,.6,1 },
lightspec[] = { .2,.2,.2,1 },
lightPos[] = { 1.0, 1.0, 1.0, 1.0 };

float TABLE_TOP_WIDTH = 8, TABLE_TOP_LENGTH = 10, TABLE_TOP_THICKNESS = 1;	// Start of Table Variables
float TABLE_LEG_HEIGHT = 7, TABLE_LEG_WIDTH = 1;
float tableX = 10, tableY, tableZ;											//End of Table Variables

float gh_x, gh_z;

string pass = "";

bool isShowBox = 0;

void init_textures();
void use_texture(int);

void background();
void mydraw();
void reshape(int, int);
void fanTimer(int);
void lightTimer(int);
void ghostTimer(int);
void keyboard(unsigned char, int, int);
void specialKeyboard(int, int, int);
//void load(int imgnum);
//void check(unsigned char* data);
float toRad(float);
void mouseMovement(int, int);

void rightWall();
void leftWall();
void frontWall();
void backWall();
void floor();
void roof();
void Room();
void drawChair();
void chair();
void drawTable();
void drawFan();

void drawCube(float, float, float);
void drawTableLeg(float, float);
void drawTable2();

void drawCard();
void drawCoffin();
void drawModel(const vector<float>&,
	const vector<float>&,
	float, float, float,
	float, float, float,
	float, float,
	int, float, int);
void loadModel(const char* filename, vector<float>& vertices, vector<float>& texCoords);

void drawTriangleHand(float, float);
void drawClockFace();
void drawClock();
void drawI(float, float, float);
void drawV(float, float, float);
void drawX(float, float, float);

void drawPainring();
void drawSafeBox();
void drawLamp();

void mouseClick(int, int, int, int);
void draw2DMessageBox(const char*);

void drawFrame(int, float, float, float);

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutCreateWindow("Escape if you can... ");
	glutFullScreen();

	background();

	loadModel("skeleton.obj", skeletonVertices, skeletonTexCoords);
	loadModel("Skull_OBJ.OBJ", skullVertices, skullTexCoords);
	init_textures();
	
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glutPassiveMotionFunc(mouseMovement);
	glutDisplayFunc(mydraw);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, fanTimer, 0); // fan
	glutTimerFunc(0, lightTimer, 0); // light
	glutTimerFunc(0, ghostTimer, 0); // ghost
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouseClick);

	glutMainLoop();
    return 0;
}

void background() {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	// Lighting
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightspec);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdiff);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glMaterialfv(GL_FRONT, GL_AMBIENT, matamb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matdiff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matspec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matshin);
}

void toggleLight(int value) {
	lightOn = !lightOn;

	if (lightOn) {
		glEnable(GL_LIGHT0);  // ON
	}
	else {
		glDisable(GL_LIGHT0); // OFF
	}
	//glutPostRedisplay();
}

void mydraw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(eyex, eyey, eyez,
		centerX, centerY, centerZ,
		0, 1, 0);

	glPushMatrix();
	glDisable(GL_LIGHTING);
	glRotatef(fanRotationAngle, 0.0f, 1.0f, 0.0f);
	drawFan();
	glPopMatrix();
	glEnable(GL_LIGHTING);

	Room();
	chair();
	drawTable();
	drawTable2();
	drawCard();
	drawCoffin();
	drawClock();
	drawSafeBox();
	drawPainring();
	drawLamp();

	drawFrame(13, 19.5f, 12.0f, 12.0f);
	drawFrame(14, -19.5f, 12.0f, 12.0f);
	drawFrame(15, -19.5f, 12.0f, 4.0f);

	if (isShowBox) {
		draw2DMessageBox("Write The Password");
	}

	glutSwapBuffers();
}

void reshape(int w, int h) {
	if (h == 0) h = 1;
	rat = w / (float)h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, rat, 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void ghostTimer(int v) {
	gh_x = -20;
	gh_z = rng() % 38 - 15;
	glutPostRedisplay();
	glutTimerFunc(12000, ghostTimer, 0);
}

void finishTimer(int v) {
	glutTimerFunc(5000, lightTimer, 0);
	exit(0);
}

// Light Toggle Timer
void lightTimer(int v) {
	toggleLight(v);
	glutPostRedisplay();
	if(lightOn)
		glutTimerFunc(rng() % 10000, lightTimer, 0);
	else 
		glutTimerFunc(rng() % 1000, lightTimer, 0);
}

// fan movement timer
void fanTimer(int v) {
	fanRotationAngle += 20.0f;
	if (fanRotationAngle > 360.0f)
		fanRotationAngle -= 360.0f;

	glutPostRedisplay();
	glutTimerFunc(100, fanTimer, 0);
	gh_x += 0.8;
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		if (isShowBox)
			isShowBox = 0, pass = "";
		else
			exit(0);
	}
	if (isShowBox) {
		if (key <= '9' && key >= '0') {
			//cout << key << '\n';
			pass += key;
			if (pass == "5793") {
				pass = "Perfect, You Got The Key And Won, Congratulations\n";
				glutTimerFunc(5000, finishTimer, 0);
			}
		}
		else if (key == 8 && !pass.empty())
			pass.pop_back();
	}
}

void specialKeyboard(int key, int x, int y) {
	if (key == GLUT_KEY_F1) {
		fullScreenMode = !fullScreenMode;
		if (fullScreenMode) {
			glutFullScreen();
		}
		else {
			glutReshapeWindow(GetSystemMetrics(SM_CXSCREEN) / 2,
				GetSystemMetrics(SM_CYSCREEN) / 2);
			glutPositionWindow(GetSystemMetrics(SM_CXSCREEN) / 4,
				GetSystemMetrics(SM_CYSCREEN) / 4);
		}
	}

	float speed = 1.5;
	if (key == GLUT_KEY_RIGHT) {
		if (centerX + cos(toRad(yaw) + PI / 2) * speed >= 6 && centerZ + sin(toRad(yaw) + PI / 2) * speed <= 7)
			return;
		if (centerX + cos(toRad(yaw) + PI / 2) * speed <= -6 && centerZ + sin(toRad(yaw) + PI / 2) * speed <= 24)
			return;
		if (centerX + cos(toRad(yaw) + PI / 2) * speed >= 17.5)
			return;
		if (centerX + cos(toRad(yaw) + PI / 2) * speed <= -17.5)
			return;
		if (centerZ + sin(toRad(yaw) + PI / 2) * speed > 47)
			return;
		if (centerZ + sin(toRad(yaw) + PI / 2) * speed < -19)
			return;

		eyex += cos(toRad(yaw) + PI / 2) * speed;
		eyez += sin(toRad(yaw) + PI / 2) * speed;
		centerX += cos(toRad(yaw) + PI / 2) * speed;
		centerZ += sin(toRad(yaw) + PI / 2) * speed;
	}
	if (key == GLUT_KEY_LEFT) {
		if (centerX + cos(toRad(yaw) + PI / 2) * speed >= 6 && centerZ + sin(toRad(yaw) + PI / 2) * speed <= 7)
			return;
		if (centerX + cos(toRad(yaw) + PI / 2) * speed <= -6 && centerZ + sin(toRad(yaw) + PI / 2) * speed <= 24)
			return;
		if (centerX + cos(toRad(yaw) + PI / 2) * speed >= 17.5)
			return;
		if (centerX + cos(toRad(yaw) + PI / 2) * speed <= -17.5)
			return;
		if (centerZ + sin(toRad(yaw) + PI / 2) * speed > 47)
			return;
		if (centerZ + sin(toRad(yaw) + PI / 2) * speed < -19)
			return;

		eyex -= cos(toRad(yaw) + PI / 2) * speed;
		eyez -= sin(toRad(yaw) + PI / 2) * speed;
		centerX -= cos(toRad(yaw) + PI / 2) * speed;
		centerZ -= sin(toRad(yaw) + PI / 2) * speed;
	}
	if (key == GLUT_KEY_DOWN) {
		if (centerX + cos(toRad(yaw)) * speed >= 6 && centerZ + sin(toRad(yaw)) * speed <= 7)
			return;
		if (centerX + cos(toRad(yaw)) * speed <= -6 && centerZ + sin(toRad(yaw)) * speed <= 24)
			return;
		if (centerX + cos(toRad(yaw)) * speed >= 17.5)
			return;
		if (centerX + cos(toRad(yaw)) * speed <= -17.5)
			return;
		if (centerZ + sin(toRad(yaw)) * speed > 47)
			return;
		if (centerZ + sin(toRad(yaw)) * speed < -19)
			return;

		eyex -= cos(toRad(yaw)) * speed;
		eyez -= sin(toRad(yaw)) * speed;
		centerX -= cos(toRad(yaw)) * speed;
		centerZ -= sin(toRad(yaw)) * speed;
	}
	if (key == GLUT_KEY_UP) {
		if (centerX + cos(toRad(yaw)) * speed >= 6 && centerZ + sin(toRad(yaw)) * speed <= 7) 
			return;
		if (centerX + cos(toRad(yaw)) * speed <= -6 && centerZ + sin(toRad(yaw)) * speed <= 24)
			return;
		if (centerX + cos(toRad(yaw)) * speed >= 17.5)
			return;
		if (centerX + cos(toRad(yaw)) * speed <= -17.5)
			return;
		if (centerZ + sin(toRad(yaw)) * speed > 47)
			return;
		if (centerZ + sin(toRad(yaw)) * speed < -19)
			return;

		eyex += cos(toRad(yaw)) * speed;
		eyez += sin(toRad(yaw)) * speed;
		centerX += cos(toRad(yaw)) * speed;
		centerZ += sin(toRad(yaw)) * speed;

	}
	glutPostRedisplay();
}

void init_textures() {
	const char* filenames[20] = {
		"", // dummy for index 0
		"floor.jpg",		//1
		"roof.jpg",			//2
		"chair-wood.jpg",	//3
		"table_texture.jpg",//4
		"fan_txt.PNG",		//5
		"wood.jpg",			//6
		"card.jpg",			//7
		"darkwood.jpg",		//8
		"sk.jpg",			//9
		"safeBox.jpg",		//10
		"clock.jpg",		//11
		"pic.jpg",          //12
		"frame1.jpg",
		"frame2.jpg",
		"frame3.jpg"
	};

	for (int i = 1; i < 16; ++i) {
		int width, height, nrChannels;
		unsigned char* data = stbi_load(filenames[i], &width, &height, &nrChannels, 0);
		if (data) {
			glGenTextures(1, &textures[i]);
			glBindTexture(GL_TEXTURE_2D, textures[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else {
			cout << "Failed to load texture: " << filenames[i] << endl;
		}
	}
}

void use_texture(int imgnum) {
	glBindTexture(GL_TEXTURE_2D, textures[imgnum]);
}
//void load(int imgnum) {
//	if (imgnum == 1) {
//
//		data = stbi_load("floor.jpg", &width, &height, &nrChannels, 0);
//		check(data);
//	}
//	else if (imgnum == 2) {
//
//		data = stbi_load("roof.jpg", &width, &height, &nrChannels, 0);
//		check(data);
//	}
//	else if (imgnum == 3) {
//		data = stbi_load("chair-wood.jpg", &width, &height, &nrChannels, 0);
//		check(data);
//	}
//	else if (imgnum == 4) {
//
//		data = stbi_load("table_texture.jpg", &width, &height, &nrChannels, 0);
//		check(data);
//	}
//	else if (imgnum == 5) {
//
//		data = stbi_load("fan_txt.PNG", &width, &height, &nrChannels, 0);
//		check(data);
//	}
//	else if (imgnum == 6) {
//
//		data = stbi_load("wood.jpg", &width, &height, &nrChannels, 0);
//		check(data);
//	}
//}
//
//void check(unsigned char* data) {
//	if (data)
//	{
//		glGenTextures(1, &texture);
//		glBindTexture(GL_TEXTURE_2D, texture);
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	}
//	else
//	{
//		cout << "Failed to load texture" << endl;
//	}
//	stbi_image_free(data);
//}

float toRad(float deg) {
	return (deg * PI) / 180;
}

//void mouseMovement(int horizontalPos, int verticalPos) {
//	if (firstMouse)
//	{
//		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
//		lastX = horizontalPos;
//		lastY = verticalPos;
//		firstMouse = false;
//	}
//	if (skipNextMouseMovement) {
//		skipNextMouseMovement = false;
//		//lastX = glutGet(GLUT_WINDOW_WIDTH) / 2;
//		//lastY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
//		return;
//	}
//	float xoffset = horizontalPos - lastX;
//	float yoffset = lastY - verticalPos;
//	lastX = horizontalPos;
//	lastY = verticalPos;
//
//	xoffset *= sensitivity;
//	yoffset *= sensitivity;
//
//	yaw += xoffset;
//	if (yaw > 360.0f) yaw -= 360.0f;
//	if (yaw < 0.0f) yaw += 360.0f;
//
//	pitch += yoffset;
//	if (pitch > 89)
//		pitch = 89;
//	if (pitch < -89)
//		pitch = -89;
//
//	centerX = eyex + cos(toRad(yaw)) * cos(toRad(pitch));
//	centerY = eyey + sin(toRad(pitch));
//	centerZ = eyez + sin(toRad(yaw)) * cos(toRad(pitch));
//
//	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
//	skipNextMouseMovement = true;
//	glutPostRedisplay();
//}

void mouseMovement(int horizontalPos, int verticalPos) {
	int centerScreenX = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int centerScreenY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (horizontalPos == centerScreenX && verticalPos == centerScreenY) {
		return;
	}

	float xoffset = horizontalPos - centerScreenX;
	float yoffset = centerScreenY - verticalPos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	if (yaw > 360.0f) yaw -= 360.0f;
	if (yaw < 0.0f) yaw += 360.0f;

	pitch += yoffset;
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	centerX = eyex + cos(toRad(yaw)) * cos(toRad(pitch));
	centerY = eyey + sin(toRad(pitch));
	centerZ = eyez + sin(toRad(yaw)) * cos(toRad(pitch));

	glutWarpPointer(centerScreenX, centerScreenY);
	glutPostRedisplay();
}

void floor() {
	use_texture(1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(20, -2, 50);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(-20, -2, 50);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(-20, -2, -20);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(20, -2, -20);
	glEnd();
}

void roof() {
	use_texture(2);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(20, 25, -20);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(20, 25, 50);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(-20, 25, 50);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(-20, 25, -20);
	glEnd();
}

void rightWall() {
	use_texture(2);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(20, -2, -20);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(20, -2, 50);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(20, 25, 50);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(20, 25, -20);
	glEnd();
}

void leftWall() {
	use_texture(2);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-20, -2, -20);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(-20, -2, 50);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(-20, 25, 50);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(-20, 25, -20);
	glEnd();
}

void frontWall() {
	use_texture(2);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-20, -2, -20);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(20, -2, -20);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(20, 25, -20);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(-20, 25, -20);
	glEnd();
}

void backWall() {
	use_texture(2);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-20, -2, 50);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(20, -2, 50);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(20, 25, 50);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(-20, 25, 50);

	glEnd();
}

void Room()
{
	floor();
	roof();
	frontWall();
	backWall();
	rightWall();
	leftWall();
	glDisable(GL_TEXTURE_2D);
}

void drawChair()
{
	glEnable(GL_TEXTURE_2D);
	use_texture(3);
	// Surface
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.5f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 1.5f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.5f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.5f, -1.0f);
	glEnd();

	float legWidth = 0.2f;
	float legHeight = 1.5f;

	// Front Right Leg
	glBegin(GL_QUADS);
	// Front
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.9f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.7f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.7f, legHeight, 0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.9f, legHeight, 0.9f);
	// Back
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.9f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.7f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.7f, legHeight, 0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.9f, legHeight, 0.7f);
	// Right
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.9f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.9f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.9f, legHeight, 0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.9f, legHeight, 0.9f);
	// Left
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.7f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.7f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.7f, legHeight, 0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.7f, legHeight, 0.9f);
	glEnd();

	// Front Left Leg
	glBegin(GL_QUADS);
	// Front
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.9f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.9f, legHeight, 0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f, legHeight, 0.9f);
	// Back
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.9f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.9f, legHeight, 0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f, legHeight, 0.7f);
	// Right
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.9f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.9f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.9f, legHeight, 0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.9f, legHeight, 0.9f);
	// Left
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, 0.0f, 0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.7f, 0.0f, 0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.7f, legHeight, 0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f, legHeight, 0.9f);
	glEnd();

	// Back Right Leg
	glBegin(GL_QUADS);
	// Front
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.9f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.7f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.7f, legHeight, -0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.9f, legHeight, -0.7f);
	// Back
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.9f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.7f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.7f, legHeight, -0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.9f, legHeight, -0.9f);
	// Right
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.9f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.9f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.9f, legHeight, -0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.9f, legHeight, -0.7f);
	// Left
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.7f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.7f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.7f, legHeight, -0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.7f, legHeight, -0.7f);
	glEnd();

	// Back Left Leg
	glBegin(GL_QUADS);
	// Front
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.9f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.9f, legHeight, -0.7f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f, legHeight, -0.7f);
	// Back
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.9f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.9f, legHeight, -0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f, legHeight, -0.9f);
	// Right
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.9f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.9f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.9f, legHeight, -0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.9f, legHeight, -0.7f);
	// Left
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, 0.0f, -0.7f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.7f, 0.0f, -0.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.7f, legHeight, -0.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f, legHeight, -0.7f);
	glEnd();

	// Back support
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 3.5f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 3.5f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.5f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.5f, -1.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void chair()
{
	glPushMatrix();
	glTranslatef(12, -2, -16);
	glScalef(2.0f, 2.0f, 2.0f);
	drawChair();
	glPopMatrix();
}

void drawTable() {
	glEnable(GL_TEXTURE_2D);
	use_texture(4);  // Load table texture (Capture.PNG)

	glPushMatrix();
	glTranslatef(-6, 0, 15); 
	glRotatef(90, 0, 1, 0);

	// Draw Tabletop
	glPushMatrix();
	glTranslatef(0, 0, -8); // Move the table deeper into the room (away from front)

	glBegin(GL_QUADS);

	// Top Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 0.5f, -3.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, 0.5f, -3.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 0.5f, 3.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 0.5f, 3.0f);

	// Bottom Face (optional, depending on your preference for texturing the bottom side)
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, -0.5f, -3.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, -0.5f, -3.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, -0.5f, 3.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, -0.5f, 3.0f);

	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, -0.5f, 3.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, -0.5f, 3.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 0.5f, 3.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 0.5f, 3.0f);

	// Back Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-5, -0.5, -3);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(5, -0.5, -3);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(5, 0.5, -3);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-5, 0.5, -3);

	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, -0.5f, -3.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-5.0f, -0.5f, 3.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-5.0f, 0.5f, 3.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 0.5f, -3.0f);

	// Right Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(5.0f, -0.5f, -3.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, -0.5f, 3.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 0.5f, 3.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(5.0f, 0.5f, -3.0f);

	glEnd();
	glPopMatrix();

	// Disable texture if legs don't need textures
	glDisable(GL_TEXTURE_2D);

	// Draw Table Legs
	float legHeight = 2.0f;
	float legSize = 0.3f;
	float offsetX = 4.0f;
	float offsetZ = 2.0f;
	float tableZ = -8.0f; // same as table

	// Enable texture for the legs
	glEnable(GL_TEXTURE_2D);
	use_texture(4);  // Load texture for legs if it's different

	for (int dx = -1; dx <= 1; dx += 2) {
		for (int dz = -1; dz <= 1; dz += 2) {
			glPushMatrix();
			glTranslatef(dx * offsetX, -1.6f, dz * offsetZ + tableZ);
			glScalef(legSize, legHeight, legSize);

			// Draw each leg as a cube with texture coordinates
			glBegin(GL_QUADS);

			// Front Face of Leg
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);

			// Back Face of Leg
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);

			// Left Face of Leg
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);

			// Right Face of Leg
			glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);

			// Top Face of Leg
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);

			// Bottom Face of Leg
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);

			glEnd();
			glPopMatrix();
		}
	}

	// Disable texture after drawing legs
	glDisable(GL_TEXTURE_2D);

	drawModel(skullVertices, skullTexCoords, 2, 1.7, -8, 0.09f, 0.09f, 0.09f, 0, 0, 9, 1, 0);       // Skull
	glPopMatrix();
}

void drawFan() {
	glEnable(GL_TEXTURE_2D);

	use_texture(5);

	float poleHeight = 3.0f;
	float poleSize = 0.3f;

	glPushMatrix();
	glTranslatef(0, 22, 0); // Position at ceiling level

	// ==== Draw Pole ====
	glPushMatrix();
	glScalef(poleSize, poleHeight, poleSize);

	glBegin(GL_QUADS);
	// Pole faces
	glTexCoord2f(0, 0); glVertex3f(-0.5f, 0, 0.5f);
	glTexCoord2f(1, 0); glVertex3f(0.5f, 0, 0.5f);
	glTexCoord2f(1, 1); glVertex3f(0.5f, 1, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 1, 0.5f);

	glTexCoord2f(0, 0); glVertex3f(-0.5f, 0, -0.5f);
	glTexCoord2f(1, 0); glVertex3f(0.5f, 0, -0.5f);
	glTexCoord2f(1, 1); glVertex3f(0.5f, 1, -0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 1, -0.5f);

	glTexCoord2f(0, 0); glVertex3f(-0.5f, 0, -0.5f);
	glTexCoord2f(1, 0); glVertex3f(-0.5f, 0, 0.5f);
	glTexCoord2f(1, 1); glVertex3f(-0.5f, 1, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 1, -0.5f);

	glTexCoord2f(0, 0); glVertex3f(0.5f, 0, -0.5f);
	glTexCoord2f(1, 0); glVertex3f(0.5f, 0, 0.5f);
	glTexCoord2f(1, 1); glVertex3f(0.5f, 1, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(0.5f, 1, -0.5f);
	glEnd();

	glPopMatrix(); // Done drawing pole

	// ==== Move down slightly ====
	glTranslatef(0.0f, 0.0f, 0.0f);

	// ==== Draw Fan Center ====
	float baseRadius = 1.2f;
	int numSegments = 50;

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5f, 0.5f);
	glVertex3f(0, 0, 0);

	for (int i = 0; i <= numSegments; i++) {
		float angle = i * 2.0f * 3.14159f / numSegments;
		float x = cos(angle) * baseRadius;
		float z = sin(angle) * baseRadius;
		glTexCoord2f(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle));
		glVertex3f(x, 0, z);
	}
	glEnd();

	// ==== Rotate Blades ====
	glRotatef(fanRotationAngle, 0, 1, 0);

	// ==== Draw Blades ====
	float bladeLength = 7.5f;
	float bladeWidth = 0.7f;
	float bladeHeight = 0.1f;

	for (int i = 0; i < 4; i++) {
		glPushMatrix();
		glRotatef(i * 90.0f, 0, 1, 0);
		glTranslatef(bladeLength / 2, 0, 0);

		glScalef(bladeLength, bladeHeight, bladeWidth);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f(-0.5f, 0, 0.5f);
		glTexCoord2f(1, 0); glVertex3f(0.5f, 0, 0.5f);
		glTexCoord2f(1, 1); glVertex3f(0.5f, 0, -0.5f);
		glTexCoord2f(0, 1); glVertex3f(-0.5f, 0, -0.5f);
		glEnd();

		glPopMatrix();
	}

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void drawCube(float width, float height, float depth) {
	float w = width / 2, h = height / 2, d = depth / 2;
	glBegin(GL_QUADS);

	// Front
	glTexCoord2f(0, 0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, d);

	// Back
	glTexCoord2f(0, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(w, h, -d);

	// Left
	glTexCoord2f(0, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, -d);

	// Right
	glTexCoord2f(0, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(w, h, d);

	// Top
	glTexCoord2f(0, 0); glVertex3f(-w, h, d);
	glTexCoord2f(1, 0); glVertex3f(w, h, d);
	glTexCoord2f(1, 1); glVertex3f(w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, -d);

	// Bottom
	glTexCoord2f(0, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, -h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, -h, d);

	glEnd();
}

// Draw table leg
void drawTableLeg(float x, float z) {
	glPushMatrix();
	glTranslatef(x, TABLE_TOP_THICKNESS, z);
	drawCube(TABLE_LEG_WIDTH, TABLE_LEG_HEIGHT, TABLE_LEG_WIDTH);
	glPopMatrix();
}

// Draw the table
void drawTable2() {
	glPushMatrix();
	glTranslatef(tableX, tableY, tableZ);

	// Draw table top
	glEnable(GL_TEXTURE_2D);
	use_texture(6);
	glPushMatrix();
	glTranslatef(0.0f, TABLE_LEG_HEIGHT - 2, 0.0f);
	drawCube(TABLE_TOP_WIDTH, TABLE_TOP_THICKNESS, TABLE_TOP_LENGTH);
	glPopMatrix();

	// Draw four legs
	float legOffsetX = TABLE_TOP_WIDTH / 2 - TABLE_LEG_WIDTH / 2;
	float legOffsetZ = TABLE_TOP_LENGTH / 2 - TABLE_LEG_WIDTH / 2;

	drawTableLeg(-legOffsetX, -legOffsetZ);  // Back-left
	drawTableLeg(-legOffsetX, legOffsetZ);   // Front-left
	drawTableLeg(legOffsetX, -legOffsetZ);  // Back-right
	drawTableLeg(legOffsetX, legOffsetZ);   // Front-right
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	drawModel(skullVertices, skullTexCoords,
		10.0f, TABLE_LEG_HEIGHT - 1.6f + TABLE_TOP_THICKNESS / 1.0f, -0.9f, // tx, ty, tz
		0.09f, 0.09f, 0.09f,                                            // scale to fit
		45.0f, 1.0f,                                        // rotate if needed
		9, 1, 0);
	drawModel(skullVertices, skullTexCoords,
		12.0f, TABLE_LEG_HEIGHT - 1.6f + TABLE_TOP_THICKNESS / 1.0f, -0.9f, // tx, ty, tz
		0.07f, 0.07f, 0.07f,                                            // scale to fit
		-60.0f, 1.0f,                                        // rotate if needed
		9, 1, 0);
}

void drawCard() {
	float w = 0.64f, h = 0.9f, t = 0.05f;

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	use_texture(7);
	glTranslatef(9.0f, 5.48f, 0.0f);
	glRotatef(90, 1, 0, 0);
	glRotatef(45, 0, 0, 1);
	// Front face (textured)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-w, -h, t);
	glTexCoord2f(1.0, 0.0); glVertex3f(w, -h, t);
	glTexCoord2f(1.0, 1.0); glVertex3f(w, h, t);
	glTexCoord2f(0.0, 1.0); glVertex3f(-w, h, t);
	glEnd();

	// Back face (textured or plain)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-w, -h, -t);
	glTexCoord2f(1.0, 0.0); glVertex3f(w, -h, -t);
	glTexCoord2f(1.0, 1.0); glVertex3f(w, h, -t);
	glTexCoord2f(0.0, 1.0); glVertex3f(-w, h, -t);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 0.0f, 0.0f);
	glDisable(GL_LIGHTING);
	// Sides
	glBegin(GL_QUADS);
	// Top

	glVertex3f(-w, h, t); glVertex3f(w, h, t);
	glVertex3f(w, h, -t); glVertex3f(-w, h, -t);
	// Bottom
	glVertex3f(-w, -h, t); glVertex3f(-w, -h, -t);
	glVertex3f(w, -h, -t); glVertex3f(w, -h, t);
	// Right
	glVertex3f(w, -h, t); glVertex3f(w, -h, -t);
	glVertex3f(w, h, -t); glVertex3f(w, h, t);
	// Left
	glVertex3f(-w, -h, t); glVertex3f(-w, h, t);
	glVertex3f(-w, h, -t); glVertex3f(-w, -h, -t);
	glEnd();

	glPopMatrix();
}

void drawCoffin() {
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);

	float th = 0.06, sg = 0.02, sg2 = 0.01;
	float vx[] = { 0, 2.05, 2.05, 0, -2.05, -2.05 };
	float vz[] = { 1.02, 0.52, -0.52, -1.02, -0.52, 0.52 };

	// Bottom (untextured)
	glTranslatef(-13.5, -1.9, -6);
	glScalef(6, 6, 6);
	glRotatef(270, 0, 1, 0);
	glColor3f(0.3, 0.15f, 0.05);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 6; i++)
		glVertex3f(vx[i], th, vz[i]);
	glEnd();

	glColor3f(0.3, 0.15f, 0.05);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 6; i++)
		glVertex3f(vx[i], 0, vz[i]);
	glEnd();

	for (int i = 0; i < 6; ++i) {
		int next = (i + 1) % 6;

		glColor3f(0.3, 0.15f, 0.05);
		glBegin(GL_QUADS);
		glVertex3f(vx[i], 0, vz[i]);
		glVertex3f(vx[next], 0, vz[next]);
		glVertex3f(vx[next], th, vz[next]);
		glVertex3f(vx[i], th, vz[i]);
		glEnd();
	}

	float vx2[] = { 0, 2, 2, 0, -2, -2 };
	float vz2[] = { 1, 0.5, -0.5, -1, -0.5, 0.5 };

	// Sides with texture
	use_texture(8);
	for (int i = 0; i < 6; ++i) {
		int next = (i + 1) % 6;

		glColor3f(1, 1, 1); // Texture color
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(vx2[i], th, vz2[i]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(vx2[next], th, vz2[next]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(vx2[next], 0.75, vz2[next]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(vx2[i], 0.75, vz2[i]);
		glEnd();

		// Optional black border lines
		glDisable(GL_TEXTURE_2D);
		glColor3f(0, 0, 0);
		glLineWidth(5);
		glBegin(GL_LINES);
		glVertex3f(vx2[i], th, vz2[i]);
		glVertex3f(vx2[i], 0.75, vz2[i]);
		glVertex3f(vx2[i], 0.75, vz2[i]);
		glVertex3f(vx2[next], 0.75, vz2[next]);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}

	glDisable(GL_TEXTURE_2D);

	drawModel(skeletonVertices, skeletonTexCoords,
		1.5f, 0.4f, 0.0f,       // tx, ty, tz (centered)
		1.4f, 1.4f, 1.4f,       // scale down to fit
		90.0f, 1.0f, // rotate model to lie flat
		9, 1, 0);
	glPopMatrix();

	drawModel(skeletonVertices, skeletonTexCoords,
		gh_x, 4, gh_z,       // tx, ty, tz (centered)
		6, 6, 6,       // scale down to fit
		0, 0, // rotate model to lie flat
		9, 0.1, 1);
}


// Load the OBJ model
// Load OBJ model and extract vertices and texture coordinates
void loadModel(const char* filename, vector<float>& vertices, vector<float>& texCoords) {
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn, err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);
	if (!ret) {
		cerr << "Error loading model: " << err << endl;
		return;
	}

	if (!warn.empty()) {
		cerr << "Warning: " << warn << endl;
	}

	vertices.clear();
	texCoords.clear();

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			// Vertex
			vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
			vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
			vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

			// Texture
			if (index.texcoord_index >= 0 && !attrib.texcoords.empty()) {
				texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
				texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
			}
			else {
				texCoords.push_back(0.0f);
				texCoords.push_back(0.0f);
			}
		}
	}

	cout << "Loaded " << vertices.size() / 3 << " vertices from " << filename << endl;
}

// Draw the 3D textured model with passed model data, positioned to the right of the table
void drawModel(const vector<float>& vertices,
	const vector<float>& texCoords,
	float tx, float ty, float tz,
	float sx, float sy, float sz,
	float angle, float r,
	int textureId, 
	float transperancy,
	int fl) {
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	use_texture(textureId);

	glTranslatef(tx, ty, tz);
	glRotatef(angle, 0, 0, r);
	glRotatef(angle, 0, r, 0);
	glRotatef(90, 0, fl, 0);
	glScalef(sx, sy, sz);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, transperancy);
	glBegin(GL_TRIANGLES);
	for (size_t i = 0, j = 0; i < vertices.size(); i += 3, j += 2) {
		glTexCoord2f(texCoords[j], texCoords[j + 1]);
		glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
	}
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}


void drawTriangleHand(float length, float baseWidth) {
	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, length, 0.01f);                   // Tip pointing outward
	glVertex3f(-baseWidth / 2.0f, 0.0f, 0.01f);        // Left base
	glVertex3f(baseWidth / 2.0f, 0.0f, 0.01f);         // Right base
	glEnd();
}

void drawClockFace() {
	const int segments = 100;
	float radius = 1.0f;

	glColor3f(1, 1, 1);
	
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5f, 0.5f);
	glVertex3f(0, 0, 0);
	for (int i = 0; i <= segments; ++i) {
		float angle = 2.0f * PI * i / segments;
		glTexCoord2f(0.5f + 0.5f * cos(angle) * radius, 0.5f + 0.5f * sin(angle) * radius);
		glVertex3f(cos(angle) * radius, sin(angle) * radius, 0);
	}

	glEnd();
}
void drawClock() {
	glPushMatrix();
	glTranslatef(0, 20, -19.9);
	glScalef(2, 2, 2);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	use_texture(11);
	drawClockFace();
	glDisable(GL_LIGHTING);

	glPushMatrix();		// Number 12
	glScalef(0.3, 0.3, 0.3);
	drawX(-0.3, 2.5, 0);
	drawI(0.2, 2.5, 0);
	drawI(0.5, 2.5, 0);
	glPopMatrix();

	glPushMatrix();		// Number 3
	glScalef(0.3, 0.3, 0.3);
	drawI(2.6, 0, 0);
	drawI(2.8, 0, 0);
	drawI(3.0, 0, 0);
	glPopMatrix();

	glPushMatrix();		// Number 6
	glScalef(0.3, 0.3, 0.3);
	drawV(-0.2, -2.5, 0);
	drawI(0.4, -2.5, 0);
	glPopMatrix();

	glPushMatrix();		// Number 9
	glScalef(0.3, 0.3, 0.3);
	drawI(-3.0, 0, 0);
	drawX(-2.4, 0, 0);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);           
	glRotatef(-90.0f, 0, 0, 1);  // 3:00
	drawTriangleHand(0.6f, 0.16f);      // Short, wide triangle
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);           
	glRotatef(180.0f, 0, 0, 1);   // 6:00
	drawTriangleHand(0.85f, 0.10f);      // Long, thin triangle
	glPopMatrix();
	glPopMatrix();

}
void drawI(float x, float y, float z) {
	glPushMatrix();
	glTranslatef(x, y, z);  // Position the "I"

	glColor3f(0.0f, 0.0f, 0.0f);

	// Draw a thin, tall cube for "I"
	glPushMatrix();
	glScalef(0.1f, 1.0f, 0.1f);  // Scale cube into a rectangle
	glutSolidCube(1.0f);
	glPopMatrix();

	glPopMatrix();
}
void drawV(float x, float y, float z) {
	glPushMatrix();
	glTranslatef(x, y, z);  // Position the "V"
	glColor3f(0.0f, 0.0f, 0.0f);

	// Left stroke: angled toward center
	glPushMatrix();
	glTranslatef(-0.2f, 0.0f, 0.0f);
	glRotatef(20.0f, 0, 0, 1);  // Tilt in
	glScalef(0.09f, 1.1f, 0.03f); // Thin, long
	glutSolidCube(1.0f);
	glPopMatrix();

	// Right stroke: angled toward center
	glPushMatrix();
	glTranslatef(0.2f, 0.0f, 0.0f);
	glRotatef(-20.0f, 0, 0, 1); // Tilt in
	glScalef(0.09f, 1.1f, 0.03f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPopMatrix();
}
void drawX(float x, float y, float z) {
	glPushMatrix();
	glTranslatef(x, y, z);  // Position the "X"

	glColor3f(0.0f, 0.0f, 0.0f);

	// First stroke: bottom-left to top-right
	glPushMatrix();
	glRotatef(35.0f, 0, 0, 1);
	glScalef(0.07f, 1.2f, 0.07f);     // Thinner width and depth
	glutSolidCube(1.0f);
	glPopMatrix();

	// Second stroke: top-left to bottom-right
	glPushMatrix();
	glRotatef(-35.0f, 0, 0, 1);
	glScalef(0.07f, 1.2f, 0.07f);     // Thinner width and depth
	glutSolidCube(1.0f);
	glPopMatrix();

	glPopMatrix();
}


void drawSafeBox() {
	float s = 0.5f;
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	use_texture(10);
	glTranslatef(0, 10, -21.6);


	glScalef(4.0f, 4.0f, 4.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glBegin(GL_QUADS);

	// Front
	glTexCoord2f(0, 0); glVertex3f(-s, -s, s);
	glTexCoord2f(1, 0); glVertex3f(s, -s, s);
	glTexCoord2f(1, 1); glVertex3f(s, s, s);
	glTexCoord2f(0, 1); glVertex3f(-s, s, s);

	// Back
	glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
	glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
	glTexCoord2f(1, 1); glVertex3f(s, s, -s);
	glTexCoord2f(0, 1); glVertex3f(-s, s, -s);

	// Left
	glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
	glTexCoord2f(1, 0); glVertex3f(-s, -s, s);
	glTexCoord2f(1, 1); glVertex3f(-s, s, s);
	glTexCoord2f(0, 1); glVertex3f(-s, s, -s);

	// Right
	glTexCoord2f(0, 0); glVertex3f(s, -s, -s);
	glTexCoord2f(1, 0); glVertex3f(s, -s, s);
	glTexCoord2f(1, 1); glVertex3f(s, s, s);
	glTexCoord2f(0, 1); glVertex3f(s, s, -s);

	// Top
	glTexCoord2f(0, 0); glVertex3f(-s, s, -s);
	glTexCoord2f(1, 0); glVertex3f(s, s, -s);
	glTexCoord2f(1, 1); glVertex3f(s, s, s);
	glTexCoord2f(0, 1); glVertex3f(-s, s, s);

	// Bottom
	glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
	glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
	glTexCoord2f(1, 1); glVertex3f(s, -s, s);
	glTexCoord2f(0, 1); glVertex3f(-s, -s, s);

	glEnd();

	glPopMatrix();
}

float painting_rot_z = 0;
bool painting_moved = 0;
void drawPainring() {
	float x = 0.964 / 2, y = 0.6, z = 0.5;
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	use_texture(12);
	glTranslatef(0, 10, -19.5);
	glRotatef(180, 0, 0, 1);
	glScalef(10, 10, 0.1);

	glTranslatef(painting_rot_z, 0, 0);
	glRotatef(30, 0, 0, painting_rot_z);
	//glTranslatef(4.75, -6, 0);

	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glBegin(GL_QUADS);

	// Front
	glTexCoord2f(0, 0); glVertex3f(-x, -y, z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, z);
	glTexCoord2f(1, 1); glVertex3f(x, y, z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, z);

	// Back
	glTexCoord2f(0, 0); glVertex3f(-x, -y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, -z);
	glTexCoord2f(1, 1); glVertex3f(x, y, -z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, -z);

	// Left
	glTexCoord2f(0, 0); glVertex3f(-x, -y, -z);
	glTexCoord2f(1, 0); glVertex3f(-x, -y, z);
	glTexCoord2f(1, 1); glVertex3f(-x, y, z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, -z);

	// Right
	glTexCoord2f(0, 0); glVertex3f(x, -y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, z);
	glTexCoord2f(1, 1); glVertex3f(x, y, z);
	glTexCoord2f(0, 1); glVertex3f(x, y, -z);

	// Top
	glTexCoord2f(0, 0); glVertex3f(-x, y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, y, -z);
	glTexCoord2f(1, 1); glVertex3f(x, y, z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, z);

	// Bottom
	glTexCoord2f(0, 0); glVertex3f(-x, -y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, -z);
	glTexCoord2f(1, 1); glVertex3f(x, -y, z);
	glTexCoord2f(0, 1); glVertex3f(-x, -y, z);

	glEnd();

	glPopMatrix();
}

bool isClick(int mouseX, int mouseY) {
	GLdouble modelview[16], projection[16];
	GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble x1, y1, z1, x2, y2, z2;

	gluUnProject(mouseX, viewport[3] - mouseY, 0.0, modelview, projection, viewport, &x1, &y1, &z1); // near
	gluUnProject(mouseX, viewport[3] - mouseY, 1.0, modelview, projection, viewport, &x2, &y2, &z2); // far


	// Ray: from (x1, y1, z1) to (x2, y2, z2)
	// Perform intersection test with your objects

	float Z0 = -19.6, xmin = -2, xmax = 2, ymin = 8, ymax = 12;
	float dz = z2 - z1;
	if (dz == 0.0f) return false; // Ray is parallel to the plane

	float t = (Z0 - z1) / dz;

	if (t < 0.0f || t > 1.0f) return false; // Intersection is outside ray segment

	float x = x1 + t * (x2 - x1);
	float y = y1 + t * (y2 - y1);
	cout << centerX << ' ' << centerZ << '\n';

	return (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
}
void mouseClick(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (!painting_moved){
			if (isClick(x, y)) {
				painting_rot_z = 1;
				painting_moved = 1;
				cout << "Pic was clicked!\n";
			}
		}
		else {
			if (isClick(x, y)) {
				cout << "Box was clicked!\n";
				isShowBox = 1;
			}
		}
	}
}

void draw2DMessageBox(const char* title) {
	// Save current matrix modes
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1920, 0, 1080);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the box
	float x = 480, y = 360, width = 960, height = 360; // center box
	glColor4f(0.6f, 0.6f, 0.6f, 0.9f);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();

	glColor4f(1, 1, 1, 0.6);
	glBegin(GL_QUADS);
	glVertex2f(x + 30, y + height - 30);
	glVertex2f(x + width - 30, y + height - 30);
	glVertex2f(x + width - 30, y + height - 100);
	glVertex2f(x + 30, y + height - 100);
	glEnd();

	// Draw border
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(3);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();

	// Draw the title and message
	glPushMatrix();
	glColor3f(1, 0.2f, 0.2f);
	glRasterPos2f(x + 360, y + height - 160);
	for (const char* c = title; *c; c++) 
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	glPopMatrix();

	glColor3f(0, 0, 0);
	glRasterPos2f(x + 100, y + height - 70);
	for (auto c : pass) 
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);

	// Restore state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW); // reset mode to modelview
}

void drawLamp() {
	const float shiftX = -2.0f;  // Shift lamp 2 units to the left

	glPushMatrix();
	glTranslatef(-6, 0, 15);
	glRotatef(90, 0, 1, 0);

	// === Lamp Base ===
	glPushMatrix();
	glTranslatef(shiftX, 1.0f, -8.0f);
	glScalef(1.0f, 0.2f, 1.0f);
	glColor3f(0.5f, 0.3f, 0.2f);
	glDisable(GL_TEXTURE_2D);
	glutSolidCube(1.0f);
	glPopMatrix();

	// === Lamp Stand ===
	glPushMatrix();
	glTranslatef(shiftX, 1.9f, -8.0f);
	glScalef(0.5f, 1.8f, 0.1f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glDisable(GL_TEXTURE_2D);
	glutSolidCube(1.0f);
	glPopMatrix();

	// === Lamp Shade (Textured) ===
	glPushMatrix();
	glTranslatef(shiftX, 3.2f, -8.0f);
	glScalef(1.5f, 1.2f, 1.5f);
	glEnable(GL_TEXTURE_2D);
	use_texture(3); // Use your texture ID
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	// Bottom face
	glTexCoord2f(0, 0); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 0); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 1); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0, 1); glVertex3f(-1.0f, -1.0f, -1.0f);

	// Side 1
	glTexCoord2f(0, 0); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 0); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 1); glVertex3f(0.5f, 1.0f, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 1.0f, 0.5f);

	// Side 2
	glTexCoord2f(0, 0); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 0); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1, 1); glVertex3f(0.5f, 1.0f, -0.5f);
	glTexCoord2f(0, 1); glVertex3f(0.5f, 1.0f, 0.5f);

	// Side 3
	glTexCoord2f(0, 0); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1, 0); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1, 1); glVertex3f(-0.5f, 1.0f, -0.5f);
	glTexCoord2f(0, 1); glVertex3f(0.5f, 1.0f, -0.5f);

	// Side 4
	glTexCoord2f(0, 0); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1, 0); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 1); glVertex3f(-0.5f, 1.0f, 0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 1.0f, -0.5f);

	// Top face
	glTexCoord2f(0, 0); glVertex3f(-0.5f, 1.0f, 0.5f);
	glTexCoord2f(1, 0); glVertex3f(0.5f, 1.0f, 0.5f);
	glTexCoord2f(1, 1); glVertex3f(0.5f, 1.0f, -0.5f);
	glTexCoord2f(0, 1); glVertex3f(-0.5f, 1.0f, -0.5f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glPopMatrix();
}

void drawFrame(int textureID, float x, float y, float z) {
	glEnable(GL_TEXTURE_2D);
	use_texture(textureID);

	glPushMatrix();

	glTranslatef(x, y, z);
	glRotatef(90, 0, 1, 0);

	float width = 5.0f;
	float height = 4.0f;

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex3f(-width / 2, -height / 2, 0);
	glTexCoord2f(1, 1); glVertex3f(width / 2, -height / 2, 0);
	glTexCoord2f(1, 0); glVertex3f(width / 2, height / 2, 0);
	glTexCoord2f(0, 0); glVertex3f(-width / 2, height / 2, 0);
	glEnd();

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}