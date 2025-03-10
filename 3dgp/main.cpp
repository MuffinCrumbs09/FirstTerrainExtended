#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// GLSL Program
C3dglProgram program;

// 3D Models
C3dglSkyBox skybox;
C3dglTerrain terrain, road;
C3dglModel streetLamp, stone, ufo, man;

// Textures
GLuint idTexSand, idTexRoad;

// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

// Cycle
bool dayTime = true;

// Animations
float ufoPos = 0;
bool ufoPath;

bool init()
{
	// shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use()) return false;

	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// load your 3D models here!
	if (!terrain.load("models\\heightmap.png", 10)) return false;
	if (!road.load("models\\roadmap.png", 10)) return false;
	if (!streetLamp.load("models\\streetlamp.obj")) return false;
	if (!stone.load("models\\stone\\stone.obj")) return false;
	if (!ufo.load("models\\ufo\\ufo.obj")) return false;
	if (!man.load("models\\man.fbx")) return false;

	// load Sky Box
	if (!skybox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;

	// load textures
	stone.loadMaterials("models\\stone");

	man.loadAnimations();

	C3dglBitmap bm;

	// sand
	bm.load("models/sand.png", GL_RGBA);
	if (!bm.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());

	// road
	bm.load("models/road.jpg", GL_RGBA);
	if (!bm.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexRoad);
	glBindTexture(GL_TEXTURE_2D, idTexRoad);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());

	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));
	//glutSetVertexAttribTexCoord2(program.getAttribLocation("aTexCoord"));

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(4.0, 1.5, 30.0),
		vec3(4.0, 1.5, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;
	float terrainY, x, z;

	// LIGHTING

	if (dayTime)
	{
		// Day Time
		//program.sendUniform("lightDir.direction", vec3(1.0, 1, 1.0));
		//program.sendUniform("lightDir.diffuse", vec3(1, 1, 1));
		//program.sendUniform("materialDiffuse", vec3(.8, .8, .8));
		//program.sendUniform("materialAmbient", vec3(.1, .1, .1));

		//program.sendUniform("lightAmbient.color", vec3(0, 0, 0));

		// prepare ambient light for the skybox
		program.sendUniform("lightAmbient.color", vec3(1.0, 1.0, 1.0));
		program.sendUniform("materialAmbient", vec3(1.0, 1.0, 1.0));
		program.sendUniform("materialDiffuse", vec3(0.0, 0.0, 0.0));

		// render the skybox
		m = matrixView;
		skybox.render(m);

		// revert normal light after skybox
		program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));

		program.sendUniform("globalIntensity", 0);
	}
	else
	{
		// Night Time
		program.sendUniform("lightDir.direction", vec3(.5, -.5, .5));
		program.sendUniform("lightDir.diffuse", vec3(.1, .1, .3));
		program.sendUniform("materialDiffuse", vec3(.2, .2, .3));
		program.sendUniform("materialAmbient", vec3(0, 0, .1));

		program.sendUniform("lightAmbient.color", vec3(1, 1, 1));

		program.sendUniform("globalIntensity", 10);
	}

	// TEXTURES
	program.sendUniform("texture0", 0);

	// Translate - Rotate - Scale

	// render terrain
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	m = translate(matrixView, vec3(0, 0, 0));
	terrain.render(m);

	// render road
	glBindTexture(GL_TEXTURE_2D, idTexRoad);
	m = translate(matrixView, vec3(0, 0, 0));
	road.render(m);

	// render lamp

	x = 4; z = 20;
	// Light 1
	m = matrixView;
	m = translate(m, vec3(x, 3.5, z));
	m = scale(m, vec3(.1, .1, .1));
	program.sendUniform("matrixModelView", m);
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
	//program.sendUniform("lightAmbient1.color", vec3(1.0, 1.0, 1.0)); // Set emissive light
	glutSolidSphere(6, 32, 32);

	// Lamp 1
	m = matrixView;	
	terrainY = terrain.getInterpolatedHeight(x, z);
	m = translate(m, vec3(x, terrainY, z));
	m = scale(m, vec3(.05f, .025, .05f));

	program.sendUniform("materialAmbient", vec3(.2, .2, .2));
	program.sendUniform("materialDiffuse", vec3(.8, .7, .5));
	program.sendUniform("materialSpecular", vec3(.3, .3, .3));
	program.sendUniform("materialShininess", 32);

	program.sendUniform("lightPoint.position", vec3(x, 3.5, z));
	program.sendUniform("lightPoint.diffuse", vec3(.8, .7, .5));
	program.sendUniform("lightPoint.specular", vec3(.3, .3, .2));
	streetLamp.render(0, m);

	x = -4; z = 5;
	// Light 2
	m = matrixView;
	m = translate(m, vec3(x, 4.25, z));
	m = scale(m, vec3(.1, .1, .1));
	program.sendUniform("matrixModelView", m);
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
	//program.sendUniform("lightAmbient2.color", vec3(1.0, 1.0, 1.0)); // Set emissive light
	glutSolidSphere(6, 32, 32);

	// Lamp 2
	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(x, z);
	m = translate(m, vec3(x, terrainY, z));
	m = scale(m, vec3(.05f, .025f, .05f));

	program.sendUniform("materialAmbient", vec3(.2, .2, .2));
	program.sendUniform("materialDiffuse", vec3(.8, .7, .5));
	program.sendUniform("materialSpecular", vec3(.3, .3, .3));
	program.sendUniform("materialShininess", 32);

	program.sendUniform("lightPoint1.position", vec3(x, 3.5, z));
	program.sendUniform("lightPoint1.diffuse", vec3(.8, .7, .5));
	program.sendUniform("lightPoint1.specular", vec3(.3, .3, .2));
	streetLamp.render(0, m);

	x = 4; z = -10;
	// Light 3
	m = matrixView;
	m = translate(m, vec3(x, 3, z));
	m = scale(m, vec3(.1, .1, .1));
	program.sendUniform("matrixModelView", m);
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
	//program.sendUniform("lightAmbient3.color", vec3(1.0, 1.0, 1.0)); // Set emissive light
	glutSolidSphere(6, 32, 32);

	// Lamp 3
	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(x, z);
	m = translate(m, vec3(x, terrainY, z));
	m = scale(m, vec3(.05f, .025f, .05f));

	program.sendUniform("materialAmbient", vec3(.2, .2, .2));
	program.sendUniform("materialDiffuse", vec3(.8, .7, .5));
	program.sendUniform("materialSpecular", vec3(.3, .3, .3));
	program.sendUniform("materialShininess", 32);

	program.sendUniform("lightPoint2.position", vec3(x, 3.5, z));
	program.sendUniform("lightPoint2.diffuse", vec3(.8, .7, .5));
	program.sendUniform("lightPoint2.specular", vec3(.3, .3, .2));

	streetLamp.render(0, m);

	x = -4; z = -25;
	// Light 4
	m = matrixView;
	m = translate(m, vec3(x, 4.25, z));
	m = scale(m, vec3(.1, .1, .1));
	program.sendUniform("matrixModelView", m);
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(0.0f, 0.0f, 0.0f));
	//program.sendUniform("lightAmbient4.color", vec3(1.0, 1.0, 1.0)); // Set emissive light
	glutSolidSphere(6, 32, 32);

	// Lamp 4
	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(x, z);
	m = translate(m, vec3(x, terrainY, z));
	m = scale(m, vec3(.05f, .025f, .05f));

	program.sendUniform("materialAmbient", vec3(.2, .2, .2));
	program.sendUniform("materialDiffuse", vec3(.8, .7, .5));
	program.sendUniform("materialSpecular", vec3(.3, .3, .3));
	program.sendUniform("materialShininess", 32);

	program.sendUniform("lightPoint3.position", vec3(-4, 3.5, -25));
	program.sendUniform("lightPoint3.diffuse", vec3(.8, .7, .5));
	program.sendUniform("lightPoint3.specular", vec3(.3, .3, .2));

	streetLamp.render(0, m);

	// Stones
	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(18, -30);
	m = translate(m, vec3(18, terrainY, -30));
	m = rotate(m, radians(90.f), vec3(0, 1, 0));
	m = scale(m, vec3(.25, .25, .25));
	m = scale(m, vec3(.25, .5, .25));
	stone.render(0, m);

	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(20, -12);
	m = translate(m, vec3(20, terrainY, -12));
	m = rotate(m, radians(180.f), vec3(0, 1, 0));
	m = scale(m, vec3(.25, .25, .25));
	m = scale(m, vec3(.25, .25, .25));
	m = scale(m, vec3(1, .7, 1));
	stone.render(0, m);

	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(-10, -14);
	m = translate(m, vec3(-10, terrainY, -14));
	m = scale(m, vec3(.25, .25, .25));
	m = scale(m, vec3(.25, .25, .25));
	m = scale(m, vec3(.4, .4, .4));
	stone.render(0, m);

	m = matrixView;
	terrainY = terrain.getInterpolatedHeight(-14, 3);
	m = translate(m, vec3(-14, terrainY, 3));
	m = rotate(m, radians(270.f), vec3(0, 1, 0));
	m = scale(m, vec3(.25, .25, .25));
	m = scale(m, vec3(.25, .25, .25));
	stone.render(0, m);

	// UFO
	m = matrixView;
	m = translate(m, vec3(0, 25, ufoPos));
	m = rotate(m, radians(90.f), vec3(0, 1, 0));
	m = rotate(m, radians(90.f), vec3(1, 0, 0));
	m = scale(m, vec3(.05, .05, .05));
	ufo.render(0, m);

	// Spotlight
	mat4 lightMatrix = translate(matrixView, vec3(0, 25, ufoPos));

	program.sendUniform("matrixModelView", m);
	program.sendUniform("spotlight.matrix", lightMatrix);
	program.sendUniform("spotlight.position", vec3(0, 23, ufoPos));
	program.sendUniform("spotlight.direction", vec3(0, -1, 0));
	program.sendUniform("spotlight.diffuse", vec3(0.0, 1.0, 0.0));
	program.sendUniform("spotlight.specular", vec3(0.0, 1.0, 0.0));
	program.sendUniform("spotlight.cutOff", cos(radians(10.0f)));
	program.sendUniform("spotlight.attenuation", 5);
	program.sendUniform("spotlight.intensity", 2);

	// UFO movement
	if (ufoPath)
		ufoPos += 0.01f;
	else
		ufoPos -= 0.01f;

	if (ufoPos > 30 || ufoPos < -30)
		ufoPath = !ufoPath;

	// Man
	std::vector<mat4> transforms;
	man.getAnimData(0, time, transforms);
	program.sendUniform("bones", &transforms[0], transforms.size());

	m = matrixView;
	m = translate(m, vec3(-8, terrain.getInterpolatedHeight(-8, -25), -25));
	m = scale(m, vec3(0.01, 0.01, 0.01));
	man.render(m);
}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	program.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));;

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	case 'n': dayTime = !dayTime; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: First Terrain");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

