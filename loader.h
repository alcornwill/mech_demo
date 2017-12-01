
#include <stdio.h>
#include <stdlib.h>

// File3D - F3D


struct MeshInfo {
    char * name;
    GLuint *indices;
    GLuint *edges;
    GLfloat *vertices;
    GLfloat *normals;
    GLfloat *colors;
    GLfloat *uvs;
    unsigned int numIndices;
    unsigned int numEdges;
    unsigned int numVertices;
    unsigned int numNormals;
    unsigned int numColors;
    unsigned int numUVs;
};

struct ObjectInfo {
    char * name;
    char * meshName;
    char * parentName;
    mat4_t transform;
};

struct AnimationKeyInfo {
    unsigned int time;
    mat4_t transform;
};

struct AnimationInfo {
    char * objectName;
    struct AnimationKeyInfo * keys;
    unsigned int numKeys;
};

struct File3DInfo {
    unsigned char numMeshes;
    struct MeshInfo * meshes;
    unsigned char numObjects;
    struct ObjectInfo * objects;
    unsigned char numAnimation;
    struct AnimationInfo * animations;
};


struct File3DInfo * loadFile3D(char path[]) 
{
    // load binary file at path
    // parse bytes...
    
    FILE* file = fopen(path, "rb");
    if (!file)
        printf("ERROR failed to open file!\n");
    
    struct File3DInfo *f3dinfo = malloc(sizeof(struct File3DInfo));
    
    fseek(file, 0, SEEK_SET);
    
    //printf("current file pointer position: %i\n", ftell(file));
    
    // number of meshes
    if (!fread(&f3dinfo->numMeshes, 1, 1, file))
        printf("ERROR file read error!\n");
    #ifdef F3D_DEBUG
    printf("num meshes: %u\n", f3dinfo->numMeshes);
    #endif
    
    // meshes
    
    
    fclose(file);
    
    return f3dinfo;
}
