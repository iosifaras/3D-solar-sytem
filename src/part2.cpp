/*
Particle system based on Particle Engine Using Triangle Strips by Nehe Productions
http://nehe.gamedev.net/tutorial/particle_engine_using_triangle_strips/21001/
*/

#include <iostream>
#include <stdlib.h>
#include <map>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLSLProgram.h"
#include "GLSLSourceFileReader.h"
#include "OBJFileReader.h"
#include "Trackball.h"
//#include "AntTweakBar.h"
//#include <AntTweakBar\AntTweakBar.h>
#include "lodepng.h"
// Represents an indexed triangle mesh.
struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;
};

// Represents an 8-bit bitmap image.
struct Image_t {
  int width;
  int height;
  std::vector<unsigned char> data;
};

// Represents a vertex array object (VAO) created from a mesh. Used
// for rendering.
struct MeshVAO {
    GLuint vao;
    GLuint vertexVBO;
    GLuint normalVBO;
    GLuint indexVBO;
    int numIndices;
};

// Struct for global resources.
struct Globals {
    int width;
    int height;
    int windowID;
    cgtk::GLSLProgram program;
    cgtk::Trackball trackball;
    Mesh mesh;
    MeshVAO meshVAO;
};

Globals globals;

bool toggleAmbient = true;
bool toggleDiffuse = true;
bool toggleSpecular = true;
bool gammaCorrection = true;
bool inverse = true;
float zoomFactor = -20.0f;
float width;
float height;


GLUquadric *sun;

//CONSTANTS
const float PI = 3.14;

//CAMERA

float angle = 0.0;			 // Angle of rotation for the camera direction
float lx = 0.0f, lz = -1.0f; // Direction of the camera.
float x = 0.0f, z = 5.0f;	 // X and Z position for the camera.

//TEXTURES
GLuint textures[11];          //The size of the array corresponds to the number of textures.

//ORBITS
float orbitAroundSun = 0.0f;

//MERCURY
float mercury_x = 0.0f;
float mercury_y = 0.0f;
float mercuryOriginX = 0.0f;
float mercuryOrbitAroundSun = 0.0f;
float mercuryRotation = 0.0f;

//VENUS
float venus_x = 0.0f;
float venus_y = 0.0f;
float venusOriginX = 0.0f;
float venusOrbitAroundSun = 0.0f;
float venusRotation = 0;

//EARTH
float earth_x = 0.0f;
float earth_y = 0.0f;
float earthOriginX = 0.0f;
float earthOrbitAroundSun = 0.0f;
float earthRotation = 0;

//MARS
float mars_x = 0.0f;
float mars_y = 0.0f;
float marsOriginX = 0.0f;
float marsOrbitAroundSun = 0.0f;
float marsRotation = 0.0f;

//JUPITER
float jupiter_x = 0.0f;
float jupiter_y = 0.0f;
float jupiterOriginX = 0.0f;
float jupiterOrbitAroundSun = 0.0f;
float jupiterRotation = 0.0f;

//SATURN
float saturn_x = 0.0f;
float saturn_y = 0.0f;
float saturnOriginX = 0.0f;
float saturnOrbitAroundSun = 0.0f;
float saturnRotation = 0.0f;

//URANUS
float uranus_x = 0.0f;
float uranus_y = 0.0f;
float uranusOriginX = 0.0f;
float uranusOrbitAroundSun = 0.0f;
float uranusRotation = 0.0f;

//NEPTUNE
float neptune_x = 0.0f;
float neptune_y = 0.0f;
float neptuneOriginX = 0.0f;
float neptuneOrbitAroundSun = 0.0f;
float neptuneRotation = 0.0f;

//PARTICLES
const int MAX_PARTICLES = 10; //Since the particles are based on spheres, rendering many of them will reduce performance.

float slowdown = 2.0f; //Slow down particles
float particleXSpeed;
float particleYSpeed;

//Struct for the particles.
typedef struct
{
	bool active;
	float life;
	float fade;
	float x;
	float y;
	float z;
	float xD;
	float yD;
	float zD;
	float horizontalMovement;
}
particles;

particles particle[MAX_PARTICLES];

// Returns the value of the environment variable whose name is
// specified by the argument.
std::string getEnvVar(const std::string &name)
{
    char *value = getenv(name.c_str());
    if (value == NULL) {
        return std::string();
    }
    else {
        return std::string(value);
    }
}

// Returns the absolute path to the shader directory.
// std::string shaderDir(void)
// {

//     return "shaders/";
// }

std::string modelDir(void)
{
    std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
        exit(EXIT_FAILURE);
    }
    return rootDir + "/3d_models/";
}

// Returns the absolute path to the directory that contains the cube
// map sets.
std::string textureDir(void)
{
    std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
        exit(EXIT_FAILURE);
    }
    return rootDir + "/Textures/";
}

Image_t loadPNG(std::string const& filename)
{
    std::cout << "Loading image from " << filename << " ..." << std::endl;  

    std::vector<unsigned char> data;
    unsigned width, height;
    unsigned error = lodepng::decode(data, width, height, filename);
    if (error != 0) {
        std::cout << "Error: " << lodepng_error_text(error) << std::endl;
        exit(EXIT_FAILURE);
    }
  
    Image_t image;
    image.width = width;
    image.height = height;
    image.data = data;
  
    std::cout << "Done!" << std::endl;
    std::cout << "Image width: " << image.width << std::endl;
    std::cout << "Image height: " << image.height << std::endl;
    std::cout << "Number of elements: " << image.data.size() << std::endl;
  
    return image;
}

void LoadTextures(std::string const& dirname)
{
	glGenTextures(11, textures); //Create the space for the textures.
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	Image_t sun = loadPNG(std::string(dirname + "/sun.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(sun.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	Image_t mercury = loadPNG(std::string(dirname + "/mercury.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(mercury.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[2]);
	Image_t venus = loadPNG(std::string(dirname + "/venus.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(venus.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[3]);
	Image_t earth = loadPNG(std::string(dirname + "/earth.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(earth.data[0])); 

	glBindTexture(GL_TEXTURE_2D, textures[4]);
	Image_t mars = loadPNG(std::string(dirname + "/mars.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(mars.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[5]);
	Image_t jupiter = loadPNG(std::string(dirname + "/jupiter.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(jupiter.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[6]);
	Image_t saturn = loadPNG(std::string(dirname + "/saturn.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(saturn.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[7]);
	Image_t uranus = loadPNG(std::string(dirname + "/uranus.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(uranus.data[0]));  

	glBindTexture(GL_TEXTURE_2D, textures[8]);
	Image_t neptune = loadPNG(std::string(dirname + "/neptune.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(neptune.data[0]));

	glBindTexture(GL_TEXTURE_2D, textures[9]);
	Image_t MW = loadPNG(std::string(dirname + "/MW.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1280, 640, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(MW.data[0]));

	glBindTexture(GL_TEXTURE_2D, textures[10]);
	Image_t Particle = loadPNG(std::string(dirname + "/Particle.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(Particle.data[0]));
}

void initGLEW(void)
{
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        std::cerr << "Error: " << glewGetErrorString(status) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void displayOpenGLVersion(void)
{
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
}

void createShaderProgram(const std::string &vertexShaderFilename,
                         const std::string &fragmentShaderFilename,
                         cgtk::GLSLProgram *program)
{
    cgtk::GLSLSourceFileReader glslReader;
    glslReader.read(vertexShaderFilename.c_str());
    std::string vertexShaderSource = glslReader.getSourceString();
    glslReader.read(fragmentShaderFilename.c_str());
    std::string fragmentShaderSource = glslReader.getSourceString();

    program->create(vertexShaderSource, fragmentShaderSource);
    if (!program->isProgram() || !program->isValid()) {
        std::cerr << "Error: Could not create program." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void loadMesh(const std::string &filename, Mesh *mesh)
{
    cgtk::OBJFileReader reader;
    reader.load(filename.c_str());
    mesh->vertices = reader.getVertices();
    mesh->normals = reader.getNormals();
    mesh->indices = reader.getIndices();
}

void createMeshVAO(const Mesh &mesh, cgtk::GLSLProgram &program, MeshVAO *meshVAO)
{
    // Create the actual vertex array object (VAO) for the mesh
    glGenVertexArrays(1, &(meshVAO->vao));
    glBindVertexArray(meshVAO->vao);

    // Generate and populate a VBO for the vertices
    glGenBuffers(1, &(meshVAO->vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mesh.vertices[0]),
                 mesh.vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(program.getAttribLocation("a_position"));
    glVertexAttribPointer(program.getAttribLocation("a_position"),
                          3, GL_FLOAT, GL_FALSE, 0, NULL);

    // Generate and populate a VBO for the vertex normals
    glGenBuffers(1, &(meshVAO->normalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(mesh.normals[0]),
                 mesh.normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(program.getAttribLocation("a_normal"));
    glVertexAttribPointer(program.getAttribLocation("a_normal"),
                          3, GL_FLOAT, GL_FALSE, 0, NULL);

    // Generate and populate a VBO for the element indices
    glGenBuffers(1, &(meshVAO->indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(mesh.indices[0]),
                 mesh.indices.data(), GL_STATIC_DRAW);

    meshVAO->numIndices = mesh.indices.size();

    glBindVertexArray(0); // unbind the VAO
}



void drawMesh(cgtk::GLSLProgram &program, const MeshVAO &meshVAO)
{
    program.enable();
    
    // Define the model, view, and projection matrices here
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    view = glm::lookAt(glm::vec3(0.0f, 0.0f, zoomFactor), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //Z of first parameter is zoom.
    projection = glm::perspective(90.0f,(float)globals.width/globals.height, 0.1f, 100.0f);

    // Construct the ModelViewProjection, ModelView, and normal
    // matrices here and pass them as uniform variables to the shader
    // program
    model = globals.trackball.getRotationMatrix() * model;
    glm::mat4 mvp = projection * view * model;
	globals.program.setUniformMatrix4f("mvp",mvp);
    globals.program.setUniformMatrix4f("u_mv", (view * model));
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
	//globals.program.setUniform1f("toggleAmbient", 0.05f);
	if(toggleAmbient == true)
		globals.program.setUniform1f("toggleAmbient", 0.05f);
	else
		globals.program.setUniform1f("toggleAmbient", 0.00f);

	if(toggleDiffuse == true)
		globals.program.setUniform1f("toggleDiffuse", 1.00f);
	else
		globals.program.setUniform1f("toggleDiffuse", 0.00f);

	if(toggleSpecular == true)
		globals.program.setUniform1f("toggleSpecular", 1.00f);
	else
		globals.program.setUniform1f("toggleSpecular", 0.00f);
	if(gammaCorrection == true)
		globals.program.setUniform1f("gammaCorrection", 1.00f);
	else
		globals.program.setUniform1f("gammaCorrection", 2.2f);
	if(inverse == true)
		globals.program.setUniform1f("inverse", 0.0f);
	else
		globals.program.setUniform1f("inverse", 1.0f);
    // Set up the light source and material properties and pass them
    // as uniform variables to the shader program, along with the
    // flags (uniform int variables) used for toggling on/off
    // different parts of the rendering
    //glm::mat3 light_position = glm::mat3(1.0f);
    //glm::mat3 light_color = glm::mat3(1.0f);
    //globals.program.setUniformMatrix3f("u_light_position", light_position);
    //globals.program.setUniformMatrix3f("u_light_color", light_color);


    glBindVertexArray(meshVAO.vao);
    glDrawElements(GL_TRIANGLES, meshVAO.numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    program.disable();
}

//Draw each one of the astronomical objects.
void drawSun(void)
{
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(0,0,0);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 3, 45, 45);
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawMercury(void)
{
	mercury_x = 3.9f * cos(mercuryOrbitAroundSun * 0.50f / 180.0 * PI);
	mercury_y = 3.9f * sin(mercuryOrbitAroundSun * 0.50f / 180.0 * PI);

	if(mercuryOrbitAroundSun == 0)
		mercuryOriginX = mercury_x;
	if(mercury_x >= mercuryOriginX)
		mercuryOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(mercury_x, 0, mercury_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(mercuryRotation,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.2, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawVenus(void)
{
	venus_x = 5.0f * cos(venusOrbitAroundSun * 0.50f / 180.0 * PI);
	venus_y = 5.0f * sin(venusOrbitAroundSun * 0.50f / 180.0 * PI);

	if(venusOrbitAroundSun == 0)
		venusOriginX = venus_x;
	if(venus_x >= venusOriginX)
		venusOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(venus_x, 0, venus_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(-venusRotation,0.0f,0.0f,1.0f); //Venus rotation is retrograded.
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.25, 45, 45); 
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawEarth(void)
{
	//Earth rotation around the Sun.
	 earth_x = 8.0f * cos(earthOrbitAroundSun * 0.30f / 180.0 * PI);
	 earth_y = 8.0f * sin(earthOrbitAroundSun * 0.30f / 180.0 * PI);

	 if(earthOrbitAroundSun == 0)
		 earthOriginX = earth_x;
	 if(earth_x >= earthOriginX)
		 earthOrbitAroundSun = 0.0f;
	//GLUquadricObj* q = gluNewQuadric();
   //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
	//gluQuadricNormals(q, GLU_SMOOTH);		
	//gluQuadricTexture(q, GL_TRUE);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix(); //Enter Earth's frame of reference.
	glTranslatef(earth_x,0,earth_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(earthRotation ,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.3, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix(); //Exist Earth's frame of reference.
    glDisable(GL_TEXTURE_2D);
}

void drawMars(void)
{
	mars_x = 11.0f * cos(marsOrbitAroundSun * 0.30f / 180.0 * PI);
	mars_y = 11.0f * sin(marsOrbitAroundSun * 0.30f / 180.0 * PI);

	if(marsOrbitAroundSun == 0)
		marsOriginX = mars_x;
	if(mars_x >= marsOriginX)
		marsOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(mars_x, 0, mars_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(marsRotation,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.27, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawJupiter(void)
{
	jupiter_x = 15.0f * cos(jupiterOrbitAroundSun * 0.30f / 180.0 * PI);
	jupiter_y = 15.0f * sin(jupiterOrbitAroundSun * 0.30f / 180.0 * PI);

	if(jupiterOrbitAroundSun == 0)
		jupiterOriginX = jupiter_x;
	if(jupiter_x >= jupiterOriginX)
		jupiterOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(jupiter_x, 0, jupiter_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(jupiterRotation,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.7, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawSaturn(void)
{
	saturn_x = 17.0f * cos(saturnOrbitAroundSun * 0.30f / 180.0 * PI);
	saturn_y = 17.0f * sin(saturnOrbitAroundSun * 0.30f / 180.0 * PI);

	if(saturnOrbitAroundSun == 0)
		saturnOriginX = saturn_x;
	if(saturn_x >= saturnOriginX)
		saturnOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(saturn_x, 0, saturn_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(saturnRotation,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.35, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawUranus(void)
{
	uranus_x = 19.0f * cos(uranusOrbitAroundSun * 0.30f / 180.0 * PI);
	uranus_y = 19.0f * sin(uranusOrbitAroundSun * 0.30f / 180.0 * PI);

	if(uranusOrbitAroundSun == 0)
		uranusOriginX = uranus_x;
	if(uranus_x >= uranusOriginX)
		uranusOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(uranus_x, 0, uranus_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(uranusRotation,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.2, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawNeptune(void)
{
	neptune_x = 21.0f * cos(neptuneOrbitAroundSun * 0.30f / 180.0 * PI);
	neptune_y = 21.0f * sin(neptuneOrbitAroundSun * 0.30f / 180.0 * PI);

	if(neptuneOrbitAroundSun == 0)
		neptuneOriginX = neptune_x;
	if(neptune_x >= neptuneOriginX)
		neptuneOrbitAroundSun = 0.0f;
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(neptune_x, 0, neptune_y);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(neptuneRotation,0.0f,0.0f,1.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 0.4, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawMilkyWay(void) //Draw the skysphere of the Milky Way. 
{
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	glTranslatef(0, 0, 0);
	glRotatef(7,1.0f,0.0f,0.0f);
	glRotatef(0.0f,1.0f,0.0f,0.0f);
	gluQuadricTexture(sun, 1);
	gluSphere(sun, 40, 45, 45); //Parameters -> (qobj, radius, slices, stacks)
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawParticles(void)
{
	glActiveTexture(GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, textures[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPushMatrix();
	
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if(particle[i].active)
		{
			float x = particle[i].x;
			float y = particle[i].y;
			float z = particle[i].z;


			glTranslatef(x, z, y);

			gluQuadricTexture(sun, 1);
			gluSphere(sun, 0.07, 45, 45); //Parameters -> (qobj, radius, slices, stacks)

			//glBegin(GL_TRIANGLE_STRIP);						// Build Quad From A Triangle Strip
			//    glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z); // Top Right
			//	glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z); // Top Left
			//	glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z); // Bottom Right
			//	glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z); // Bottom Left
			//glEnd();										// Done Building Triangle Strip

			particle[i].x += 0.006;// Move On The X Axis By X Speed
			//particle[i].y+=particle[i].yD/(slowdown*1000);// Move On The Y Axis By Y Speed
			//particle[i].z+=particle[i].zD/(slowdown*1000);// Move On The Z Axis By Z Speed

			particle[i].xD = particle[i].horizontalMovement;			// Take Pull On X Axis Into Account		// Take Pull On Z Axis Into Account
			particle[i].life-=particle[i].fade;

			if (particle[i].life<0.0f && particle[i].x > 40)					// If Particle Is Burned Out
			{
				particle[i].life=360.0f;					// Give It New Life
				particle[i].fade= 0.053;              // Random Fade Value
				particle[i].x = -20.0f;				// Center On X Axis
				particle[i].y=  rand() % 61 - 30;		//Random number between -5 and 5
				particle[i].z= rand() % 11 - 5;						// Center On Z Axis
			}
		}
	}
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

//End of drawing of astronomical objects.


//Draw the whole model of the Solar System.
void DisplayModel()
{

	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LIGHT_MODEL_AMBIENT);	
	//glEnable(GL_LIGHTING);
	 // Define the model, view, and projection matrices here
 //   glm::mat4 model = glm::mat4(1.0f);
 //  glm::mat4 view = glm::mat4(1.0f);
 //glm::mat4 projection = glm::mat4(1.0f);
 // view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //Z of first parameter is zoom.
 //  projection = glm::perspective(90.0f,(float)globals.width/globals.height, 0.1f, 100.0f);

 //   // Construct the ModelViewProjection, ModelView, and normal
 //   // matrices here and pass them as uniform variables to the shader
 //   // program
 //model = globals.trackball.getRotationMatrix() * model;
 //   glm::mat4 mvp = projection * view * model;
	////globals.program.setUniformMatrix4fv("mvp",mvp);
 //   //globals.program.setUniformMatrix4fv("u_mv", (view * model));
 //   glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
	drawSun();
	drawMercury();
	drawVenus();
	drawEarth();
	drawMars();
	//drawJupiter();
	drawSaturn();
	drawUranus();
	drawNeptune();

	//glDisable(GL_LIGHTING);
	//glutSwapBuffers();
	//glFlush();
}

void initializeTrackball(void)
{
    double radius = double(std::min(globals.width, globals.height)) / 2.0;
    globals.trackball.setRadius(radius);
    glm::vec2 center = glm::vec2(globals.width, globals.height) / 2.0f;
    globals.trackball.setCenter(center);
}

void init(void)
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_NORMALIZE);
	//glEnable(GL_COLOR_MATERIAL);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective (90.0, 1000/(float)1000, 0.5, 100.0);
	//glMatrixMode(GL_MODELVIEW);

    glClearColor(0.0, 0.0, 0.0, 1.0);
	sun = gluNewQuadric();
	LoadTextures(textureDir());

	//Initialize the particles.
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		particle[i].active = true;
		particle[i].life=360.0f;					// Give It New Life
		particle[i].fade= 0.053;              // Random Fade Value
		particle[i].x = -20.0f;				// Center On X Axis
		particle[i].y=  rand() % 61 - 30;		//Random number between -5 and 5
		particle[i].z= rand() % 11 - 5;	    // Random Speed On Z Axis

		particle[i].horizontalMovement = 0.5f;

	}
	
    //gluQuadricTexture(sun, GL_TRUE);
    loadMesh((modelDir() + "bunny.obj"), &globals.mesh);

    //std::string vshaderFilename = shaderDir() + "mesh.vert";
    //std::string fshaderFilename = shaderDir() + "mesh.frag";
    //createShaderProgram(vshaderFilename, fshaderFilename, &globals.program);
	

    //createMeshVAO(globals.mesh, globals.program, &globals.meshVAO);

    initializeTrackball();
	
}

void display(void)
{

	//glViewport(0, 0, 1000, 1000);
	//Works
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, 1000/(float)1000, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glm::mat4 model = glm::mat4(1.0f);
	model = globals.trackball.getRotationMatrix();
    gluLookAt(0, zoomFactor, 1, 0, 0, 0, 0.0, 1.0, 0);
	
	//gluLookAt(	x, 1.0f, z,
	//	x+lx, 1.0f,  z+lz,
	//	0.0f, 1.0f,  0.0f);


	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective (90.0, 1000/(float)1000, 0.5, 100.0);
	//glMatrixMode(GL_MODELVIEW);
 //   gluLookAt(0,0,-3,0,0,0,0,1,0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // ensures that polygons overlap correctly
	drawSun();
	drawMercury();
	drawVenus();
	drawEarth();
	drawMars();
	drawJupiter();
	drawSaturn();
	drawUranus();
	drawNeptune();
	drawMilkyWay();
	drawParticles();
	//DisplayModel();
    //drawMesh(globals.program, globals.meshVAO);
	//TwDraw();
	orbitAroundSun += 0.15f;


	mercuryOrbitAroundSun += 0.06f;
	venusOrbitAroundSun += 0.05f;
	earthOrbitAroundSun += 0.07f;
	marsOrbitAroundSun += 0.09f;
	jupiterOrbitAroundSun += 0.10;
	saturnOrbitAroundSun += 0.13;
	uranusOrbitAroundSun += 0.15;
	neptuneOrbitAroundSun += 0.17;

	//Update the planets rotation.
	//This is not accurate. The rotations are not correctly calculated in relation to Earth rotation time.
	mercuryRotation += 0.07f;
	venusRotation += 0.03f; //Venus' rotation is the slowest one from the solar system (243 Earth days).
	earthRotation += 0.20f;
	marsRotation += 0.22f;  //24.6 Earth hours.
	jupiterRotation += 35.0f; //10 Earth hours. Fastest in the solar system.
	saturnRotation += 14.0f;
	uranusRotation += 10.0f;
	neptuneRotation += 11.0f;

	if(mercuryRotation > 360)
		mercuryRotation = -360;
	if(venusRotation > 360)
		venusRotation = -360;
	if(earthRotation > 360)
		earthRotation = -360;
	if(marsRotation > 360)
		marsRotation = -360;
	if(jupiterRotation > 360)
		jupiterRotation = -360;
	if(saturnRotation > 360)
		saturnRotation = -360;
	if(uranusRotation > 360)
		uranusRotation = -360;
	if(neptuneRotation > 360)
		neptuneRotation = -360;
	//if(orbitAroundSun > 360)
	//	orbitAroundSun = 0;
	//glDisable(GL_LIGHTING);
    glutSwapBuffers();
	//glfwSwapBuffers();
}

void reshape(int width, int height)
{
    globals.width = width;
    globals.height = height;
    globals.trackball.setRadius(double(std::min(width, height)) / 2.0);
    globals.trackball.setCenter(glm::vec2(width, height) / 2.0f);
    glViewport(0, 0, globals.width, globals.height);
}

void keyboard(unsigned char key, int x, int y)
{
	float multiplier = 0.01f;
	printf("User pressed the %c key\n", key); 
	glutPostRedisplay();
	switch(key)
	{
	case 'a':
		{
			angle -= 0.01f;
			lx = sin(angle);
			lz = -cos(angle);
		}	break;
		
	case 'd':
		{
			angle += 0.01f;
			lx = sin(angle);
			lz = -cos(angle);
			break;
		}
	case 's':
		{
			x -= lx * multiplier;
			z -= lz * multiplier;
			break;
		}
	case 'w':
		{
			x += lx * multiplier;
			z += lz * multiplier;
			break;
		}
	case 'i':
		if(inverse == true)
			inverse = false;
		else
			inverse  = true;
		break;

	}
}

void mouseButtonPressed(int button, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        globals.trackball.setCenter(glm::vec2(x, y));
        globals.trackball.startTracking(glm::vec2(x, y));
    }
	if (button == 3 && zoomFactor != -38) //Clamp the value of zoomFactor between -38 and 38.
		zoomFactor -= 1.00;
	if (button == 4 && zoomFactor != 38)
		zoomFactor += 1.00;
}

void mouseButtonReleased(int button, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        globals.trackball.stopTracking();
    }
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        mouseButtonPressed(button, x, y);
    }
    else {
        mouseButtonReleased(button, x, y);
    }
}

void moveTrackball(int x, int y)
{
    if (globals.trackball.tracking()) {
        globals.trackball.move(glm::vec2(x, y));
    }
}

void motion(int x, int y)
{
    moveTrackball(x, y);
}

void idle(void)
{
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    globals.width = 1000;
    globals.height = 1000;
    glutInitWindowSize(globals.width, globals.height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Model viewer");
    initGLEW();
    displayOpenGLVersion();
    init();
    glutReshapeFunc(&reshape);
    glutDisplayFunc(&display);
	glutKeyboardFunc(&keyboard);
    glutMouseFunc(&mouse);
    glutMotionFunc(&motion);
    glutIdleFunc(&idle);
    glutMainLoop();

    return EXIT_SUCCESS;
}