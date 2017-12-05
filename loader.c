
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
    
    f3dinfo->meshes = malloc(f3dinfo->numMeshes * sizeof(struct MeshInfo));
    if (!f3dinfo->meshes) {
        printf("ERROR memory allocation error\n");
    }
    
    // meshes
    for (int i = 0; i < f3dinfo->numMeshes; ++i) {
        struct MeshInfo * meshinfo = &f3dinfo->meshes[i];
        
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
        
        // num geoms
        if (!fread(&meshinfo->numGeoms, 1, 1, file))
            printf("ERROR file read error!\n");
        printf("num geoms: %i\n", meshinfo->numGeoms);
        
        meshinfo->geoms = malloc(meshinfo->numGeoms * sizeof(struct GeomInfo));
        if (!meshinfo->geoms) {
            printf("ERROR memory allocation error\n");
        }
        
        for (int n = 0; n < meshinfo->numGeoms; ++n) {     
            struct GeomInfo * geominfo = &meshinfo->geoms[n];
            
            // mat name length
            if (!fread(&geominfo->matNameLen, 1, 1, file))
                printf("ERROR file read error!\n");
            printf("mat name length: %i\n", geominfo->matNameLen);
            
            // mat name
            geominfo->matName = malloc(geominfo->matNameLen);
            if (!geominfo->matName)
                printf("ERROR memory allocation error");
            
            if (!fread(&geominfo->matName, 1, geominfo->matNameLen, file))
                printf("ERROR file read error!\n");
            printf("mat name: \"%.*s\"\n", geominfo->matNameLen, &geominfo->matName);
            
            // num vertex
            if (!fread(&geominfo->numVertices, 2, 1, file))
                printf("ERROR file read error!\n");
            printf("num vertices: %i\n", geominfo->numVertices);
            
            // vertices
            geominfo->vertices = malloc(2 * 3 * geominfo->numVertices);
            if (!geominfo->vertices)
                printf("ERROR memory allocation error\n");
            
            if (!fread(&geominfo->vertices, 2 * 3, geominfo->numVertices, file))
                printf("ERROR file read error!\n");
            printf("read vertices\n");
            
            // num index
            if (!fread(&geominfo->numIndices, 2, 1, file))
                printf("ERROR file read error!\n");
            printf("num indices: %i\n", geominfo->numIndices);
            
            // indices
            if (geominfo->numIndices) {
                geominfo->indices = malloc(2 * 3 * geominfo->numIndices);
                if (!geominfo->indices)
                    printf("ERROR memory allocation error\n");
                if (!fread(&geominfo->indices, 2 * 3, geominfo->numIndices, file))
                    printf("ERROR file read error!\n");
                printf("read indices\n");
            }
            
            // num edge
            if (!fread(&geominfo->numEdges, 2, 1, file))
                printf("ERROR file read error!\n");
            printf("num edges: %i\n", geominfo->numEdges);
            
            // edges
            if (geominfo->numEdges) {
                geominfo->edges = malloc(2 * 2 * geominfo->numEdges);
                if (!geominfo->edges)
                    printf("ERROR memory allocation error\n");
                if (!fread(&geominfo->edges, 2 * 2, geominfo->numEdges, file))
                    printf("ERROR file read error!\n");
                printf("read edges\n");
            }
            
            // num normals
            if (!fread(&geominfo->numNormals, 2, 1, file))
                printf("ERROR file read error!\n");
            printf("num normals: %i\n", geominfo->numNormals);
            
            // normals
            if (geominfo->numNormals) {
                geominfo->normals = malloc(2 * 3 * geominfo->numNormals);
                if (!geominfo->normals)
                    printf("ERROR memory allocation error\n");
                if (!fread(&geominfo->normals, 2 * 3, geominfo->numNormals, file))
                    printf("ERROR file read error!\n");
                printf("read normals\n");
            }
            
            // num colors
            if (!fread(&geominfo->numColors, 2, 1, file))
                printf("ERROR file read error!\n");
            printf("num colors: %i\n", geominfo->numColors);
            
            // colors
            if (geominfo->numColors) {
                geominfo->colors = malloc(2 * 3 * geominfo->numColors);
                if (!geominfo->colors)
                    printf("ERROR memory allocation error\n");
                if (!fread(&geominfo->colors, 2 * 3, geominfo->numColors, file))
                    printf("ERROR file read error!\n");
                printf("read colors\n");
            }
            
            // num uvs
            if (!fread(&geominfo->numUVs, 2, 1, file))
                printf("ERROR file read error!\n");
            printf("num uvs: %i\n", geominfo->numUVs);
            
            // uvs
            if (geominfo->numUVs) {
                geominfo->uvs = malloc(2 * 2 * geominfo->numUVs);
                if (!geominfo->uvs)
                    printf("ERROR memory allocation error");
                if (!fread(&geominfo->uvs, 2 * 2, geominfo->numUVs, file))
                    printf("ERROR file read error!\n");
                printf("read uvs\n");
            }
        }
    }
    
    // number of materials
    if (!fread(&f3dinfo->numMats, 2, 1, file))
        printf("ERROR file read error!\n");
    
    f3dinfo->mats = malloc(f3dinfo->numMats * sizeof(struct MaterialInfo));
    if (!f3dinfo->mats)
        printf("ERROR memory allocation error\n");
        
    for (int i = 0; i < f3dinfo->numMats; ++i) {
        struct MaterialInfo * matinfo = &f3dinfo->mats[i];
        
        // name length
        if (!fread(&matinfo->nameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // name
        matinfo->name = malloc(1 * matinfo->nameLen);
        if (!fread(&matinfo->name, 1, matinfo->nameLen, file))
            printf("ERROR file read error!\n");
        
        // diffuse color
        if (!fread(&matinfo->color, 2, 3, file))
            printf("ERROR file read error!\n");
    }
    
    // number of objects
    if (!fread(&f3dinfo->numObjects, 2, 1, file))
        printf("ERROR file read error!\n");
    
    f3dinfo->objects = malloc(f3dinfo->numObjects * sizeof(struct ObjectInfo));
    if (!f3dinfo->objects)
        printf("ERROR memory allocation error\n");
    
    for (int i = 0; i < f3dinfo->numObjects; ++i) {
        struct ObjectInfo * objinfo = &f3dinfo->objects[i];
        
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
    
    f3dinfo->anims = malloc(f3dinfo->numAnims * sizeof(struct AnimInfo));
    if (!f3dinfo->anims)
        printf("ERROR memory allocation error\n");
    
    for (int i = 0; i < f3dinfo->numAnims; ++i) {
        struct AnimInfo * animinfo = &f3dinfo->anims[i];
        
        // name length
        if (!fread(&animinfo->nameLen, 1, 1, file))
            printf("ERROR file read error!\n");
        
        // name
        animinfo->name = malloc(1 * animinfo->nameLen);
        if (!fread(&animinfo->name, 1, animinfo->nameLen, file))
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
        // for each geom
        for (int j = 0; j < meshinfo->numGeoms; ++j) {
            struct GeomInfo * geominfo = &meshinfo->geoms[j];
            free(geominfo->matName);
            free(geominfo->vertices);
            free(geominfo->indices);
            free(geominfo->edges);
            free(geominfo->normals);
            free(geominfo->colors);
            free(geominfo->uvs);
        }
        free(meshinfo->geoms);
    }
    // free meshes
    free(f3dinfo->meshes);
    
    // for each material
    for (int i = 0; i < f3dinfo->numMats; ++i) {
        struct MaterialInfo * matinfo = &f3dinfo->mats[i];
        free(matinfo->name);
    }
    free(f3dinfo->mats);
    
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
        free(animinfo->name);
        free(animinfo->keys);
    }
    // free anims
    free(f3dinfo->anims);
    
    // free f3dinfo
    free(f3dinfo);
}