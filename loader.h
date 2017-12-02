
// File3D - F3D


struct MeshInfo {
    unsigned char nameLen;
    char * name;
    short *vertices; // note: fixed point values
    unsigned short *indices;
    unsigned short *edges;
    short *normals;
    short *colors;
    short *uvs;
    unsigned short numVertices;
    unsigned short numIndices;
    unsigned short numEdges;
    unsigned short numNormals;
    unsigned short numColors;
    unsigned short numUVs;
};

struct ObjectInfo {
    unsigned char nameLen;
    char * name;
    unsigned char parentNameLen;
    char * parentName;
    unsigned char meshNameLen;
    char * meshName;
    unsigned char animNameLen;
    char * animName;
    short transform[16];
};

struct AnimKeyInfo {
    unsigned short time;
    short transform[16];
};

struct AnimInfo {
    unsigned char objectNameLen;
    char * objectName;
    struct AnimKeyInfo * keys;
    unsigned short numKeys;
};

struct File3DInfo {
    unsigned short numMeshes;
    struct MeshInfo * meshes;
    unsigned short numObjects;
    struct ObjectInfo * objects;
    unsigned short numAnims;
    struct AnimInfo * anims;
};


struct File3DInfo * loadFile3D(char path[]);
void f3dFree(struct File3DInfo * f3dinfo);

