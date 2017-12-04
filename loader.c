
#include "loader.h"
#include <stdio.h>
#include <stdlib.h>


struct File3DInfo * loadFile3D(char path[]) 
{
    // load binary file at path
    // parse bytes...
    
    printf("reading 3d file \"%s\"\n", path);
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("ERROR failed to open file!\n");
        return NULL;
    }
    
    struct File3DInfo *f3dinfo = malloc(sizeof(struct File3DInfo));
    if (!f3dinfo)
        printf("ERROR memory allocation error");
    
    //fseek(file, 0, SEEK_SET);
    
    //printf("current file pointer position: %i\n", ftell(file));
    
    // number of meshes
    if (!fread(&f3dinfo->numMeshes, 2, 1, file))
        printf("ERROR file read error!\n");
    printf("num meshes: %i\n", f3dinfo->numMeshes);
    
    // meshes
    for (int i = 0; i < f3dinfo->numMeshes; ++i) {
        struct MeshInfo * meshinfo = malloc(sizeof(struct MeshInfo));
        if (!meshinfo)
            printf("ERROR memory allocation error\n");
        
        // name length
        if (!fread(&meshinfo->nameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        printf("name length: %i\n", meshinfo->nameLen);
        
        // name
        meshinfo->name = malloc(meshinfo->nameLen);
        if (!meshinfo->name)
            printf("ERROR memory allocation error");
        
        if (!fread(&meshinfo->name, 1, meshinfo->nameLen, file))
            printf("ERROR file read error!\n");
        printf("mesh name: \"%.*s\"\n", meshinfo->nameLen, &meshinfo->name);
        
        // num vertex
        if (!fread(&meshinfo->numVertices, 2, 1, file))
            printf("ERROR file read error!\n");
        printf("num vertices: %i\n", meshinfo->numVertices);
        
        // vertices
        meshinfo->vertices = malloc(2 * 3 * meshinfo->numVertices);
        if (!meshinfo->vertices)
            printf("ERROR memory allocation error\n");
        
        if (!fread(&meshinfo->vertices, 2 * 3, meshinfo->numVertices, file))
            printf("ERROR file read error!\n");
        printf("read vertices\n");
        
        // num index
        if (!fread(&meshinfo->numIndices, 2, 1, file))
            printf("ERROR file read error!\n");
        printf("num indices: %i\n", meshinfo->numIndices);
        
        // indices
        if (meshinfo->numIndices) {
            meshinfo->indices = malloc(2 * 3 * meshinfo->numIndices);
            if (!meshinfo->indices)
                printf("ERROR memory allocation error\n");
            if (!fread(&meshinfo->indices, 2 * 3, meshinfo->numIndices, file))
                printf("ERROR file read error!\n");
            printf("read indices\n");
        }
        
        // num edge
        if (!fread(&meshinfo->numEdges, 2, 1, file))
            printf("ERROR file read error!\n");
        printf("num edges: %i\n", meshinfo->numEdges);
        
        // edges
        if (meshinfo->numEdges) {
            meshinfo->edges = malloc(2 * 2 * meshinfo->numEdges);
            if (!meshinfo->edges)
                printf("ERROR memory allocation error\n");
            if (!fread(&meshinfo->edges, 2 * 2, meshinfo->numEdges, file))
                printf("ERROR file read error!\n");
            printf("read edges\n");
        }
        
        // num normals
        if (!fread(&meshinfo->numNormals, 2, 1, file))
            printf("ERROR file read error!\n");
        printf("num normals: %i\n", meshinfo->numNormals);
        
        // normals
        if (meshinfo->numNormals) {
            meshinfo->normals = malloc(2 * 3 * meshinfo->numNormals);
            if (!meshinfo->normals)
                printf("ERROR memory allocation error\n");
            if (!fread(&meshinfo->normals, 2 * 3, meshinfo->numNormals, file))
                printf("ERROR file read error!\n");
            printf("read normals\n");
        }
        
        // num colors
        if (!fread(&meshinfo->numColors, 2, 1, file))
            printf("ERROR file read error!\n");
        printf("num colors: %i\n", meshinfo->numColors);
        
        // colors
        if (meshinfo->numColors) {
            meshinfo->colors = malloc(2 * 3 * meshinfo->numColors);
            if (!meshinfo->colors)
                printf("ERROR memory allocation error\n");
            if (!fread(&meshinfo->colors, 2 * 3, meshinfo->numColors, file))
                printf("ERROR file read error!\n");
            printf("read colors\n");
        }
        
        // num uvs
        if (!fread(&meshinfo->numUVs, 2, 1, file))
            printf("ERROR file read error!\n");
        printf("num uvs: %i\n", meshinfo->numUVs);
        
        // uvs
        if (meshinfo->numUVs) {
            meshinfo->uvs = malloc(2 * 2 * meshinfo->numUVs);
            if (!meshinfo->uvs)
                printf("ERROR memory allocation error");
            if (!fread(&meshinfo->uvs, 2 * 2, meshinfo->numUVs, file))
                printf("ERROR file read error!\n");
            printf("read uvs\n");
        }
    }
    
    // number of objects
    if (!fread(&f3dinfo->numObjects, 2, 1, file))
        printf("ERROR file read error!\n");
    
    for (int i = 0; i < f3dinfo->numObjects; ++i) {
        struct ObjectInfo * objinfo = malloc(sizeof(struct ObjectInfo));
        
        // name length
        if (!fread(&objinfo->nameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // name
        objinfo->name = malloc(1 * objinfo->nameLen);
        if (!fread(&objinfo->name, 1, objinfo->nameLen, file))
            printf("ERROR file read error!\n");
        
        // parent name length
        if (!fread(&objinfo->parentNameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // parent name
        if (objinfo->parentNameLen) {
            objinfo->parentName = malloc(1 * objinfo->parentNameLen);
            if (!fread(&objinfo->parentName, 1, objinfo->parentNameLen, file))
                printf("ERROR file read error!\n");
        } else {
            objinfo->parentName = NULL;
        }
        
        // mesh name length
        if (!fread(&objinfo->meshNameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // mesh name
        if (objinfo->meshNameLen) {
            objinfo->meshName = malloc(1 * objinfo->meshNameLen);
            if (!fread(&objinfo->meshName, 1, objinfo->meshNameLen, file))
                printf("ERROR file read error!\n");
        } else {
            objinfo->meshName = NULL;
        }
        
        // anim name length
        if (!fread(&objinfo->animNameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // anim name
        if (objinfo->animNameLen) {
            objinfo->animName = malloc(1 * objinfo->animNameLen);
            if (!fread(&objinfo->animName, 1, objinfo->animNameLen, file))
                printf("ERROR file read error!\n");
        } else {
            objinfo->animName = NULL;
        }
        
        // transform
        if (!fread(&objinfo->transform, 2, 16, file))
            printf("ERROR file read error!\n");
    }
    
    // number of animations
    if (!fread(&f3dinfo->numAnims, 2, 1, file))
        printf("ERROR file read error!\n");
    
    for (int i = 0; i < f3dinfo->numAnims; ++i) {
        struct AnimInfo * animinfo = malloc(sizeof(struct AnimInfo));
        
        // note: no anim name...
        
        // object name length
        if (!fread(&animinfo->objectNameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // object name
        animinfo->objectName = malloc(1 * animinfo->objectNameLen);
        if (!fread(&animinfo->objectName, 1, animinfo->objectNameLen, file))
            printf("ERROR file read error!\n");
        
        // num keys
        if (!fread(&animinfo->numKeys, 2, 1, file))
            printf("ERROR file read error!\n");
        
        // animation keys
        for (int j = 0; j < animinfo->numKeys; ++j) {
            struct AnimKeyInfo * keyinfo = malloc(sizeof(struct AnimKeyInfo));
            
            // time
            if (!fread(&keyinfo->time, 2, 1, file))
                printf("ERROR file read error!\n");
            
            // transform
            if (!fread(&keyinfo->transform, 2, 16, file))
                printf("ERROR file read error!\n");
        }
    }
    
    
    fclose(file);
    
    return f3dinfo;
}

void f3dFree(struct File3DInfo * f3dinfo) {
    // for each mesh
    for (int i = 0; i < f3dinfo->numMeshes; ++i) {
        struct MeshInfo * meshinfo = &f3dinfo->meshes[i];
        free(meshinfo->name);
        free(meshinfo->vertices);
        free(meshinfo->indices);
        free(meshinfo->edges);
        free(meshinfo->normals);
        free(meshinfo->colors);
        free(meshinfo->uvs);
    }
    // free meshes
    free(f3dinfo->meshes);
    
    // for each object
    for (int i = 0; i < f3dinfo->numObjects; ++i) {
        struct ObjectInfo * objinfo = &f3dinfo->objects[i];
        free(objinfo->name);
        free(objinfo->meshName);
        free(objinfo->animName);
    }
    // free objects
    free(f3dinfo->objects);
    
    // for each anim
    for (int i = 0; i < f3dinfo->numAnims; ++i) {
        struct AnimInfo * animinfo = &f3dinfo->anims[i];
        free(animinfo->objectName);
        free(animinfo->keys);
    }
    // free anims
    free(f3dinfo->anims);
    
    // free f3dinfo
    free(f3dinfo);
}