


int init();
int initGL();
void initBuffers();
void load3DFile();
void initTank();
void update(float dt);
void render();
void close();
void printProgramLog( GLuint program );
void printShaderLog( GLuint shader );

struct DrawElementInfo {
    unsigned int offset; // start of indices
    unsigned int baseVertex; // number of preceding vertices
};

struct Material {
    char * name;
    float color[3];
};

struct Geom {
    struct Material * mat;
    struct DrawElementInfo drawinfo;
    float *vertices; // note: fixed point values
    unsigned short *indices;
    unsigned short *edges;
    float *normals;
    float *colors;
    float *uvs;
    unsigned short numVertices;
    unsigned short numIndices;
    unsigned short numEdges;
    unsigned short numNormals;
    unsigned short numColors;
    unsigned short numUVs;
};

struct Mesh {
    unsigned char numGeoms;
    struct Geom * geoms;
};

struct AnimKey {
    unsigned short time;
    mat4_t transform;
};

struct Anim {
    struct AnimKey * keys;
    unsigned short numKeys;
};


struct Object {
    char * name;
    struct Object * parent;
    struct Object ** children;
    unsigned short numChildren;
    struct Mesh mesh;
    struct Anim anim;
    // vec3_t position;
    // vec3_t rotation;
    mat4_t model;
    mat4_t mvp;
    mat4_t normalMatrix;
};

void setObjectParent(struct Object * obj, struct Object * parent);
