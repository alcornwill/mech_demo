
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"
#include "mech.h"
#define F3D_DEBUG
#include "loader.h"

#define TITLE "Mech Demo"
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

#define FIXED(f) (f / 1000)

unsigned int gNumMats = 0;
unsigned int gNumObjects = 0;
struct Material * gMats = NULL;
struct Object * gObjects = NULL;
struct Object * gRoot = NULL; // root object
unsigned int gVBOSize = 0;
unsigned int gIBOSize = 0;

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;

GLuint gProgramID = 0;
GLint gVertexPos3DLocation = -1;
GLint gNormalLocation = -1;
//GLint gColorLocation = -1;
GLint gMVPMatrixLocation = -1;
GLint gNormalMatrixLocation = -1;
GLint gDiffuseColorLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;

unsigned char *keys;
int quit = 0;

mat4_t proj;
mat4_t view;
mat4_t pv;

unsigned char pose = 0;


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
    gWindow = SDL_CreateWindow( TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
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
    const GLchar * vsSource = "\
#version 330\n\
\n\
//#define USE_VERTEX_COLORS\n\
//#define UNLIT\n\
\n\
const vec3 ambient = vec3(0.1f, 0.1f, 0.1f);\n\
const vec3 dlight = vec3(0.03f, 0.98f, 0.08f); // direction\n\
\n\
in vec3 position;\n\
in vec3 normal;\n\
in vec3 color;\n\
//in vec2 uv;\n\
\n\
out vec4 Color;\n\
\n\
uniform mat4 MVP;\n\
uniform mat3 NormalMatrix;\n\
uniform vec3 DiffuseColor;\n\
\n\
void directional_light(vec3 surface_normal, inout vec3 scatteredLight)\n\
{\n\
    vec3 direction = normalize(dlight);\n\
    float diffuse = max(0.0, dot(surface_normal, direction));\n\
	scatteredLight += diffuse;\n\
}\n\
\n\
void main()\n\
{\n\
    vec4 v_pos = vec4(position, 1.0f);\n\
	gl_Position = MVP * v_pos;\n\
\n\
	vec3 col = DiffuseColor;\n\
	#ifdef USE_VERTEX_COLORS\n\
	col *= color;\n\
	#endif\n\
\n\
    #ifndef UNLIT\n\
	vec3 scatteredLight = ambient;\n\
	vec3 surface_normal = normalize(NormalMatrix * normal);\n\
	directional_light(surface_normal, scatteredLight);\n\
    col *= scatteredLight;\n\
    #endif\n\
\n\
    // note this value is unsaturated, we would saturate with min(color, vec4(1.0)\n\
	Color = vec4(col, 1.0f);\n\
}";

    const GLchar * fsSource = "\
#version 330\n\
\n\
in vec4 Color;\n\
out vec4 outputColor;\n\
\n\
void main()\n\
{\n\
    vec4 outColor = Color;\n\
    outputColor = outColor;\n\
}";

    
	gProgramID = glCreateProgram();
    
    // vertex shader
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertexShader, 1, &vsSource, NULL );
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
    glShaderSource( fragmentShader, 1, &fsSource, NULL);
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
    gVertexPos3DLocation = glGetAttribLocation( gProgramID, "position" );
    if( gVertexPos3DLocation == -1 )
    {
        printf( "position is not a valid glsl program variable!\n" );
        return 0;
    }
    
    // normal
    gNormalLocation = glGetAttribLocation( gProgramID, "normal" );
    if( gNormalLocation == -1 )
    {
        printf( "normal is not a valid glsl program variable!\n" );
        return 0;
    }
    
    // // color
    // gColorLocation = glGetAttribLocation( gProgramID, "color" );
    // if( gColorLocation == -1 )
    // {
        // printf( "color is not a valid glsl program variable!\n" );
        // return 0;
    // }
    
    //Get model matrix location
    gMVPMatrixLocation = glGetUniformLocation( gProgramID, "MVP" );
    if( gMVPMatrixLocation == -1 )
    {
        printf( "mvp is not a valid glsl program variable!\n" );
        return 0;
    }
    
    //Get normal matrix location
    gNormalMatrixLocation = glGetUniformLocation( gProgramID, "NormalMatrix" );
    if( gNormalMatrixLocation == -1 )
    {
        printf( "NormalMatrix is not a valid glsl program variable!\n" );
        return 0;
    }
    
    //Get diffuse color location
    gDiffuseColorLocation = glGetUniformLocation( gProgramID, "DiffuseColor" );
    if( gDiffuseColorLocation == -1 )
    {
        printf( "DiffuseColor is not a valid glsl program variable!\n" );
        return 0;
    }
            
    //Initialize clear color
    glClearColor( 1.f, 1.f, 1.f, 1.f );
    
    return 1;
}

void initBuffers() {
    // VAO
    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);
    
    //Create VBO
    glGenBuffers( 1, &gVBO );
    glBindBuffer( GL_ARRAY_BUFFER, gVBO );
    glBufferData( GL_ARRAY_BUFFER, gVBOSize, NULL, GL_STATIC_DRAW );
    
    // add data
    unsigned int vboOffset = 0;
    unsigned int baseVertex = 0;
    
    // vertex data
    glEnableVertexAttribArray( gVertexPos3DLocation );
    glVertexAttribPointer( gVertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) vboOffset);
    
    for (int i = 0; i < gNumObjects; ++i) {
        struct Object *obj = &gObjects[i];
        for (int j=0; j < obj->mesh.numGeoms; ++j) {
            struct Geom * geom = &obj->mesh.geoms[j];
            size_t size = geom->numVertices * 4 * 3;
            glBufferSubData( GL_ARRAY_BUFFER, vboOffset, size, geom->vertices);
            vboOffset += size;
            geom->drawinfo.baseVertex = baseVertex;
            baseVertex += geom->numVertices;
        }
    }
    
    // normal data
    glEnableVertexAttribArray( gNormalLocation );
    glVertexAttribPointer( gNormalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)vboOffset );
    
    for (int i = 0; i < gNumObjects; ++i) {
        struct Object *obj = &gObjects[i];
        for (int j=0; j < obj->mesh.numGeoms; ++j) {
            struct Geom * geom = &obj->mesh.geoms[j];
            size_t size = geom->numNormals * 4 * 3;
            glBufferSubData( GL_ARRAY_BUFFER, vboOffset, size, geom->normals);
            vboOffset += size;
        }
    }
    
    // // color data
    // glEnableVertexAttribArray( gColorLocation );
    // glVertexAttribPointer( gColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)vboOffset );
    
    // for (int i = 0; i < gNumObjects; ++i) {
        // struct Object *obj = &gObjects[i];
        // size_t size = obj->mesh.numColors * 4 * 3;
        // glBufferSubData( GL_ARRAY_BUFFER, vboOffset, size, obj->mesh.colors);
        // vboOffset += size;
    // }
    
    // TODO UVs (if present)
    
    
    //Create IBO
    glGenBuffers( 1, &gIBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, gIBOSize, NULL, GL_STATIC_DRAW );
    
    unsigned int iboOffset = 0;
    
    for (int i = 0; i < gNumObjects; ++i) {
        struct Object *obj = &gObjects[i];
        for (int j=0; j < obj->mesh.numGeoms; ++j) {
            struct Geom * geom = &obj->mesh.geoms[j];
            size_t size = geom->numIndices * 4;
            glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, iboOffset, size, geom->indices);
            geom->drawinfo.offset = iboOffset;
            iboOffset += size;
        }
    }
    // TODO edges if present

    glBindVertexArray(0);
}

void setObjectParent(struct Object * obj, struct Object * parent) {
    obj->parent = parent;
    ++parent->numChildren;
    
    // realloc child array
    struct Object * newmem;
    newmem = realloc(parent->children, sizeof(struct Object) * parent->numChildren);
    if (!newmem) {
        printf("ERROR memory reallocation error!\n");
    }
    parent->children = newmem;
    
    // set child
    parent->children[parent->numChildren-1] = *obj;
}

void load3DFile() {
        
    struct File3DInfo * f3dinfo = loadFile3D("../mech.3d");
    
    // first process all materials
    gMats = malloc(sizeof(struct Material) * f3dinfo->numMats);
    
    // for each material
    for (int i=0; i < f3dinfo->numMats; ++i) {
        struct MaterialInfo * matinfo = &f3dinfo->mats[i];
        
        // create new material
        struct Material * mat = &gMats[i];
        
        mat->name = malloc(matinfo->nameLen);
        if (!mat->name) {
            printf("ERROR memory allocation error");
        }
        
        mat->color[0] = FIXED(matinfo->color[0]);
        mat->color[1] = FIXED(matinfo->color[1]);
        mat->color[2] = FIXED(matinfo->color[2]);
    }
    
    
    gObjects = malloc(sizeof(struct Object) * f3dinfo->numObjects);
    
    // for each object in f3dinfo
    for (int i=0; i < f3dinfo->numObjects; ++i) {
        struct ObjectInfo * objinfo = &f3dinfo->objects[i];
        
        // create new Object
        struct Object * obj = &gObjects[i];
        obj->parent = NULL;
        obj->children = NULL;
        // copy name
        obj->name = malloc(objinfo->nameLen);
        memcpy(obj->name, objinfo->name, objinfo->nameLen);
        
        // copy the transform
        //short * m = &objinfo.transform;
        // lol
        // todo decompose transform matrix to position and rotation
        // key.transform = mat4(
            // FIXED(m[0]), FIXED(m[1]), FIXED(m[2]), FIXED(m[3]), 
            // FIXED(m[4]), FIXED(m[5]), FIXED(m[6]), FIXED(m[7]),
            // FIXED(m[8]), FIXED(m[9]), FIXED(m[10]), FIXED(m[12]),
            // FIXED(m[13]), FIXED(m[14]), FIXED(m[15]), FIXED(m[16])
        // );
        
        // find mesh with name objinfo->meshName
        for (int j=0; j < f3dinfo->numMeshes; ++j) {
            struct MeshInfo * meshinfo = &f3dinfo->meshes[j];
            obj->mesh.numGeoms = meshinfo->numGeoms;
            if (!obj->mesh.numGeoms) {
                printf("ERROR no geoms... wierd\n");
            }
            obj->mesh.geoms = malloc(obj->mesh.numGeoms * sizeof(struct Geom));
            if (!obj->mesh.geoms) {
                printf("ERROR memory allocation error\n");
            }
            
            if (strcmp(meshinfo->name, objinfo->meshName)) {
                // copy data
            
                for (int n=0; n < obj->mesh.numGeoms; ++n) {
                    struct GeomInfo * geominfo = &meshinfo->geoms[n];
                    struct Geom * geom = &obj->mesh.geoms[n];
                    
                    // find material with name geom->matName
                    for (int m=0; m < gNumMats; ++m) {
                        struct Material * mat = &gMats[m];
                        
                        if (strcmp(geominfo->matName, mat->name)) {
                            geom->mat = mat;
                            break;
                        }
                    }
                
                    geom->numVertices = geominfo->numVertices;
                    geom->numIndices = geominfo->numIndices;
                    geom->numEdges = geominfo->numEdges;
                    geom->numNormals = geominfo->numNormals;
                    geom->numColors = geominfo->numColors;
                    geom->numUVs = geominfo->numUVs;
                    
                    // we can just copy the index and edge data
                    if (geom->numIndices) {
                        size_t size = 2 * 3 * geom->numIndices;
                        geom->indices = malloc(size);
                        memcpy(geom->indices, geominfo->indices, size);
                    } else {
                        geom->indices = NULL;
                    }
                    
                    // copy edges
                    if (geom->numEdges) {
                        size_t size = 2 * 2 * geom->numEdges;
                        geom->edges = malloc(size);
                        memcpy(geom->edges, geominfo->edges, size);
                    } else {
                        geom->edges = NULL;
                    }
                    
                    // NOW
                    // we have to divide every single float
                    // by 1000
                    // because I decided to store as fixed-point...
                    
                    // copy vertices
                    geom->vertices = malloc(4 * 3 * geom->numVertices);
                    for (int k = 0; k < 3 * geom->numVertices; ++k) {
                        geom->vertices[k] = FIXED(geominfo->vertices[k]);
                    }
                    
                    // copy normals
                    if (geom->numNormals) {
                        geom->normals = malloc(4 * 3 * geom->numNormals);
                        for (int k = 0; k < 3 * geom->numNormals; ++k) {
                            geom->normals[k] = FIXED(geominfo->normals[k]);
                        }
                    } else {
                        geom->normals = NULL;
                    }
                    
                    // copy colors
                    if (geom->numColors) {
                        geom->colors = malloc(4 * 3 * geom->numColors);
                        for (int k = 0; k < 3 * geom->numColors; ++k) {
                            geom->colors[k] = FIXED(geominfo->colors[k]);
                        }
                    } else {
                        geom->colors = NULL;
                    }
                    
                    // copy uvs
                    if (geom->numUVs) {
                        geom->uvs = malloc(4 * 3 * geom->numUVs);
                        for (int k = 0; k < 2 * geom->numUVs; ++k) {
                            geom->uvs[k] = FIXED(geominfo->uvs[k]);
                        }
                    } else {
                        geom->uvs = NULL;
                    }
                    
                    // TODO we will need useEdges somewhere?
                    // storing the attributes used is useful anyway
                    
                    break;
                }
            }
        }
        
        // find anim with name objinfo->animName
        for (int j=0; j < f3dinfo->numAnims; ++j) {
            struct AnimInfo * animinfo = &f3dinfo->anims[j];
            if (strcmp(animinfo->name, objinfo->animName)) {
                // copy keys
                obj->anim.numKeys = animinfo->numKeys;
                obj->anim.keys = malloc(sizeof(struct AnimKey) * obj->anim.numKeys);
                
                for (int k = 0; k < obj->anim.numKeys; ++k) {
                    struct AnimKey * key = &obj->anim.keys[k];
                    struct AnimKeyInfo * keyinfo = &animinfo->keys[k];
                    
                    key->time = keyinfo->time;
                    
                    short * t = &keyinfo->transform;
                    key->transform = mat4(
                        FIXED(t[0]), FIXED(t[1]), FIXED(t[2]), FIXED(t[3]), 
                        FIXED(t[4]), FIXED(t[5]), FIXED(t[6]), FIXED(t[7]),
                        FIXED(t[8]), FIXED(t[9]), FIXED(t[10]), FIXED(t[12]),
                        FIXED(t[13]), FIXED(t[14]), FIXED(t[15]), FIXED(t[16])
                    );
                }
            }
        }
    }
    
    // iterate over objects again
    // set the parent
    for (int i = 0; i < gNumObjects; ++i) {
        struct ObjectInfo * objinfo = &f3dinfo->objects[i];
        struct Object * obj = &gObjects[i];
        
        // find parent
        if (objinfo->parentName) {
            for (int j = 0; j < gNumObjects; ++j) {
                struct Object * parent = &gObjects[j];
                if (strcmp(parent->name, objinfo->parentName)) {
                    setObjectParent(obj, parent);
                    break;
                }
            }   
        }
    }
    
    // calculate gVBOSize and gIBOSize
    for (int i = 0; i < gNumObjects; ++i) {
        struct Mesh * mesh = &gObjects[i].mesh;
        
        for (int i = 0; i < mesh->numGeoms; ++i) {
            struct Geom * geom = &mesh->geoms[i];
            
            gVBOSize += 4 * 3 * geom->numVertices;
            gVBOSize += 4 * 3 * geom->numNormals;
            gVBOSize += 4 * 3 * geom->numColors;
            gVBOSize += 4 * 2 * geom->numUVs;
            
            gIBOSize += 3 * geom->numIndices;
            // gIBOSize += 2 * geom.numEdges;
        }
    }
    
    // find root
    for (int i = 0; i < gNumObjects; ++i) {
        struct Object * obj = &gObjects[i];
        if (!obj->parent) {
            gRoot = obj;
            break;
        }
    }
    
    
    f3dFree(f3dinfo);
}

void initMech()
{    
    // perspective projection and view matrix
    proj = m4_perspective(FOV, ASPECT_RATIO, NEAR, FAR);
    
    view = m4_translation(vec3(0, -1, -5));
    view = m4_mul(view, m4_rotation_x(-M_PI / 2));
    
    pv = m4_mul(proj, view);
}

void updateObject(struct Object * obj) {
    printf("updating object \"%s\"", obj->name);
    // (recursive)
    
    // TODO linear interpolation
    // and sound effect
    
    struct AnimKey * key = &obj->anim.keys[pose];
    if (obj->parent) {
        obj->model = m4_mul(obj->parent->model, key->transform);
    } else {
        obj->model = key->transform;
    }
    
    obj->mvp = m4_mul(pv, obj->model);
    obj->normalMatrix = m4_invert_affine(m4_transpose(obj->model));
    
    for (int i=0; i < obj->numChildren; ++i) {
        struct Object * child = &obj->children[i];
        updateObject(child);
    }
}

void update(float dt)
{
    // parse player input
    // https://wiki.libsdl.org/SDL_Scancode
    
    if (keys[SDL_SCANCODE_1])
        pose = 0;
    if (keys[SDL_SCANCODE_2])
        pose = 1;
    if (keys[SDL_SCANCODE_3])
        pose = 2;
    if (keys[SDL_SCANCODE_4])
        pose = 3;
    if (keys[SDL_SCANCODE_5])
        pose = 4;
    
    int x = 0, y = 0;
    SDL_GetMouseState( &x, &y );

    updateObject(gRoot);
}

void render()
{
	glClear( GL_COLOR_BUFFER_BIT );
    
    glUseProgram( gProgramID );
    glBindVertexArray(gVAO);
    
    for (int i = 0; i < gNumObjects; ++i) {
        struct Object * obj = &gObjects[i];
        glUniformMatrix4fv(gMVPMatrixLocation, 1, GL_FALSE, &obj->mvp);
        glUniformMatrix4fv(gNormalMatrixLocation, 1, GL_FALSE, &obj->normalMatrix);
        for (int j = 0; j < obj->mesh.numGeoms; ++j){
            struct Geom * geom = &obj->mesh.geoms[j];
            glUniform3f(gDiffuseColorLocation, geom->mat->color[0], geom->mat->color[1], geom->mat->color[2]);
            // NOTE this is not optimal anymore because not sorted by material! (lol)
            glDrawElementsBaseVertex( GL_LINES, geom->numIndices, GL_UNSIGNED_INT, (void*)geom->drawinfo.offset, geom->drawinfo.baseVertex );
        }
    }

    glUseProgram( 0 );

    SDL_GL_SwapWindow( gWindow );
}

void close()
{
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
    load3DFile();
    initBuffers();
    initMech();
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