
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"
#define F3D_DEBUG
#include "loader.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define ASPECT_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)
#define NEAR 0.1f
#define FAR 100.0f
#define FOV 60
#define FPS 30
#define TICKS_PER_SECOND ((float)1000 / (float)FPS)

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define CLAMP(a, b, c) (MAX(b, MIN(a, c)))


int init();
int initGL();
void initTank();
void update(float dt);
void render();
void close();
void printProgramLog( GLuint program );
void printShaderLog( GLuint shader );


struct DrawElementInfo {
    unsigned int count;
    unsigned int offset; // start of indices
    unsigned int basevertex; // number of preceding vertices
};

struct Object {
    struct Object * parent;
    struct Object * children;
    struct MeshInfo * mesh;
    struct AnimationInfo * anim;
    mat4_t transform;
    struct DrawElementInfo draw;
};



SDL_Window* gWindow = NULL;
SDL_GLContext gContext;

GLuint gProgramID = 0;
GLint gVertexPos3DLocation = -1;
GLint gMVPMatrixLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;

unsigned char *keys;
int quit = 0;

struct File3DInfo * f3dinfo;

mat4_t proj;
mat4_t view;
mat4_t pv;

mat4_t proj_ortho;
mat4_t view_ortho;
mat4_t pv_ortho;

float gTankRotSpeed = 2 * M_PI; // one revolution per second
float gTankSpeed = 5.0f; // 5 units per second

float gPlayerInputY = 0;
float gPlayerInputRot = 0;

vec3_t gTankPosition;
float gTankRotZ;
mat4_t gTankMVPMat;
mat4_t gTankModelMat;

mat4_t gLandscapeMVPMat;
mat4_t gLandscapeModelMat;


int init()
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		return 0;
	}
    
    SDL_AudioInit("directsound");
	
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    
    //Create window
    gWindow = SDL_CreateWindow( "Tank Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    if( gWindow == NULL )
    {
        printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }

    //Create context
    gContext = SDL_GL_CreateContext( gWindow );
    if( gContext == NULL )
    {
        printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }
    
    // Initialize GLEW
    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
    }

    //Initialize OpenGL
    if( !initGL() )
    {
        printf( "Unable to initialize OpenGL!\n" );
        return 0;
    }

	return 1;
}

int initGL()
{
	gProgramID = glCreateProgram();
    
    // vertex shader
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	const GLchar* vertexShaderSource[] =
	{
		"#version 140\nin vec3 LVertexPos3D;\nuniform mat4 mvp;\n void main() { vec4 pos = vec4(LVertexPos3D, 1);\n pos = mvp * pos;\n gl_Position = pos; }"
	};
	glShaderSource( vertexShader, 1, vertexShaderSource, NULL );
	glCompileShader( vertexShader );

	// check for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vShaderCompiled );
	if( vShaderCompiled != GL_TRUE )
	{
		printf( "Unable to compile vertex shader %d!\n", vertexShader );
		printShaderLog( vertexShader );
        return 0;
	}
    
    glAttachShader( gProgramID, vertexShader );

    // fragment shader
    GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    const GLchar* fragmentShaderSource[] =
    {
        "#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
    };
    glShaderSource( fragmentShader, 1, fragmentShaderSource, NULL );
    glCompileShader( fragmentShader );

    // check for errors
    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled );
    if( fShaderCompiled != GL_TRUE )
    {
        printf( "Unable to compile fragment shader %d!\n", fragmentShader );
        printShaderLog( fragmentShader );
        return 0;
    }
    
    glAttachShader( gProgramID, fragmentShader );

    //Link program
    glLinkProgram( gProgramID );

    //Check for errors
    GLint programSuccess = GL_TRUE;
    glGetProgramiv( gProgramID, GL_LINK_STATUS, &programSuccess );
    if( programSuccess != GL_TRUE )
    {
        printf( "Error linking program %d!\n", gProgramID );
        printProgramLog( gProgramID );
        return 0;
    }
        
    //Get vertex attribute location
    gVertexPos3DLocation = glGetAttribLocation( gProgramID, "LVertexPos3D" );
    if( gVertexPos3DLocation == -1 )
    {
        printf( "LVertexPos3D is not a valid glsl program variable!\n" );
        return 0;
    }
    
    //Get model matrix location
    gMVPMatrixLocation = glGetUniformLocation( gProgramID, "mvp" );
    if( gMVPMatrixLocation == -1 )
    {
        printf( "mvp is not a valid glsl program variable!\n" );
        return 0;
    }
            
    //Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f );
    
    f3dinfo = loadFile3D("../mech.3d");

    // VAO
    // glGenVertexArrays(1, &gVAO);
    // glBindVertexArray(gVAO);
    
    // //Create VBO
    // glGenBuffers( 1, &gVBO );
    // glBindBuffer( GL_ARRAY_BUFFER, gVBO );
    // glBufferData( GL_ARRAY_BUFFER, TANK_VERTEX_DATA_SIZE + LANDSCAPE_VERTEX_DATA_SIZE, NULL, GL_STATIC_DRAW );
    // glBufferSubData( GL_ARRAY_BUFFER, 0, TANK_VERTEX_DATA_SIZE, tankVertexData);
    // glBufferSubData( GL_ARRAY_BUFFER, TANK_VERTEX_DATA_SIZE, LANDSCAPE_VERTEX_DATA_SIZE, landscapeVertexData);
    
    // // shader input
    // glEnableVertexAttribArray( gVertexPos3DLocation );
    // glVertexAttribPointer( gVertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL );
    
    // //Create IBO
    // glGenBuffers( 1, &gIBO );
    // glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
    // glBufferData( GL_ELEMENT_ARRAY_BUFFER, TANK_EDGE_DATA_SIZE + LANDSCAPE_EDGE_DATA_SIZE, NULL, GL_STATIC_DRAW );
    // glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, TANK_EDGE_DATA_SIZE, tankEdgeData);
    // glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, TANK_EDGE_DATA_SIZE, LANDSCAPE_EDGE_DATA_SIZE, landscapeEdgeData);

    // glBindVertexArray(0);
    
	return 1;
}

void initTank()
{    
    // perspective projection and view matrix
    proj = m4_perspective(FOV, ASPECT_RATIO, NEAR, FAR);
    
    view = m4_translation(vec3(0, -1, -5));
    view = m4_mul(view, m4_rotation_x(-M_PI / 2));
    
    pv = m4_mul(proj, view);
    
    // orthographic projection and view matrix
    float ortho_scale = 0.5f;
    // everything is all inverted...
    proj_ortho = m4_ortho(ortho_scale, -ortho_scale, -ortho_scale, ortho_scale, FAR, NEAR);
    
    view_ortho = m4_translation(vec3(0, 0, 5));
    view_ortho = m4_mul(view_ortho, m4_rotation_x(-M_PI / 2));
    
    pv_ortho = m4_mul(proj_ortho, view_ortho);
    
    
    // object positions
    gTankPosition = vec3(0,0,0);
    gTankRotZ = 0;
    gTankModelMat = m4_identity();
    
    gLandscapeModelMat = m4_identity();
}

void update(float dt)
{
    // parse player input
    // https://wiki.libsdl.org/SDL_Scancode
    
    if (keys[SDL_SCANCODE_W])
        gPlayerInputY = 1;
    else if (keys[SDL_SCANCODE_S])
        gPlayerInputY = -1;
    else
        gPlayerInputY = 0;
    
    if (keys[SDL_SCANCODE_A])
        gPlayerInputRot = 1;
    else if (keys[SDL_SCANCODE_D])
        gPlayerInputRot = -1;
    else
        gPlayerInputRot = 0;
    
    int x = 0, y = 0;
    SDL_GetMouseState( &x, &y );
    
    
    // move tank with player input
    // rotation
    gTankRotZ += gPlayerInputRot * gTankRotSpeed * dt;
    gTankRotZ = fmod(gTankRotZ, 2.0f * M_PI);
    
    // get forward vector
    vec3_t velocity = vec3(0.0f, gPlayerInputY * gTankSpeed * dt, 0.0f);
    mat4_t rotation = m4_rotation_z(gTankRotZ);
    velocity = m4_mul_dir(rotation, velocity);
    
    // apply velocity
    gTankPosition = v3_add(gTankPosition, velocity);
    
    // tank matrix
    gTankModelMat = m4_translation(gTankPosition);
    gTankModelMat = m4_mul(gTankModelMat, m4_rotation_z(gTankRotZ));
    gTankMVPMat = m4_mul(pv, gTankModelMat);
    
    // landscape matrix
    gLandscapeMVPMat = m4_mul(pv_ortho, gLandscapeModelMat);
}

void render()
{
	glClear( GL_COLOR_BUFFER_BIT );
    
    // glUseProgram( gProgramID );
    // glBindVertexArray(gVAO);
    
    // // tank
    // glUniformMatrix4fv(gMVPMatrixLocation, 1, GL_FALSE, &gTankMVPMat);
    // glDrawElementsBaseVertex( GL_LINES, TANK_NUM_EDGE, GL_UNSIGNED_INT, NULL, 0 );
    
    // // landscape
    // glUniformMatrix4fv(gMVPMatrixLocation, 1, GL_FALSE, &gLandscapeMVPMat);
    // glDrawElementsBaseVertex( GL_LINES, LANDSCAPE_NUM_EDGE, GL_UNSIGNED_INT, (void*)TANK_EDGE_DATA_SIZE, TANK_NUM_VERTEX / 3);
    
    // glUseProgram( 0 );

    SDL_GL_SwapWindow( gWindow );
}

void close()
{
    free(f3dinfo); // could do this after load?
    
	glDeleteProgram( gProgramID );
    
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	SDL_Quit();
}

void printProgramLog( GLuint program )
{
	if( glIsProgram( program ) )
	{
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
		
		char* infoLog = malloc(sizeof(char) * maxLength);
		
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
			printf( "%s\n", infoLog );
		
		free(infoLog);
	}
	else
		printf( "Name %d is not a program\n", program );
}

void printShaderLog( GLuint shader )
{
	if( glIsShader( shader ) )
	{
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
		
		char* infoLog = malloc(sizeof(char) * maxLength);
		
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
			printf( "%s\n", infoLog );

		free(infoLog);
	}
	else
		printf( "Name %d is not a shader\n", shader );
}

int main(int argc, char *argv[])
{    
	if( !init() )
    {
		printf( "Failed to initialize!\n" );
        close();
        return 0;
    }
    
    initTank();
    keys = SDL_GetKeyboardState(NULL);
    
    SDL_Event e;
    unsigned int frameTicks = 0;
    float dt = 0;
    
    while( !quit )
    {
        int ticks = SDL_GetTicks();
        dt = ticks - frameTicks;
        frameTicks = ticks;
        
        // print fps
        // printf("%i\n", (int)(1000 / dt));
        
        while(SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                quit = 1;
        }
        
        update(dt / 1000.0f);
        render();
        
        // sleep
        int end = SDL_GetTicks();
        float delay = TICKS_PER_SECOND - (end - ticks);
        if (delay > 0)
            SDL_Delay( delay );
    }

	close();

	return 0;
}