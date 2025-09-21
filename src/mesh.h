#pragma once

#include "vector.h"
#include "triangle.h"
#include "upng.h"

#define N_CUBE_VERTICES 8
extern vec3_t cube_vertices[N_CUBE_VERTICES];

#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 triangles per face
extern face_t cube_faces[N_CUBE_FACES];

// This is a struct for dynamic size meshes
typedef struct {
   vec3_t* vertices;   // dynamic array of vertices
   face_t* faces;      // dynamic array of faces
   upng_t* texture;    // mesh PNG texture pointer
   vec3_t rotation;    // rotation with x, y, and z values
   vec3_t scale;       // scale with x, y, and z
   vec3_t translation; // translate with x, y, and z values
} mesh_t;

extern mesh_t m;

void load_cube_mesh_data(void);
void load_mesh_obj_data(mesh_t *mesh, const char *obj_file, uint32_t obj_color);
void load_mesh(char *obj_file, char *png_file, vec3_t scale, vec3_t translation, vec3_t rotation);
void load_mesh_png_data(mesh_t *mesh, const char* png_file);

mesh_t* get_mesh(int mesh_index);
int get_num_meshes(void);

void free_meshes(void);