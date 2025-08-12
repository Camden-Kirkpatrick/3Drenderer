#include <stdio.h>
#include <stdlib.h>
#include "mesh.h"
#include "array.h"
#include "display.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0},
    .scale = {1.0, 1.0, 1.0},
    .translation = {0, 0, 0}};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, // 1
    {.x = -1, .y = 1, .z = -1},  // 2
    {.x = 1, .y = 1, .z = -1},   // 3
    {.x = 1, .y = -1, .z = -1},  // 4
    {.x = 1, .y = 1, .z = 1},    // 5
    {.x = 1, .y = -1, .z = 1},   // 6
    {.x = -1, .y = 1, .z = 1},   // 7
    {.x = -1, .y = -1, .z = 1}   // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    {.a = 1, .b = 2, .c = 3, .color = RED},
    {.a = 1, .b = 3, .c = 4, .color = RED},
    // right
    {.a = 4, .b = 3, .c = 5, .color = GREEN},
    {.a = 4, .b = 5, .c = 6, .color = GREEN},
    // back
    {.a = 6, .b = 5, .c = 7, .color = BLUE},
    {.a = 6, .b = 7, .c = 8, .color = BLUE},
    // left
    {.a = 8, .b = 7, .c = 2, .color = ORANGE},
    {.a = 8, .b = 2, .c = 1, .color = ORANGE},
    // top
    {.a = 2, .b = 7, .c = 5, .color = YELLOW},
    {.a = 2, .b = 5, .c = 3, .color = YELLOW},
    // bottom
    {.a = 6, .b = 8, .c = 1, .color = PINK},
    {.a = 6, .b = 1, .c = 4, .color = PINK}};

void load_cube_mesh_data(void)
{
    for (int i = 0; i < N_CUBE_VERTICES; i++)
    {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex);
    }

    for (int i = 0; i < N_CUBE_FACES; i++)
    {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

void load_obj_file_data(char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror(path);
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof line, f))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        if (line[0] == 'v' && (line[1] == ' '))
        {
            vec3_t v;

            if (sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z) == 3)
            {
                array_push(mesh.vertices, v);
            }
        }

        if (line[0] == 'f')
        {
            face_t face;

            if (sscanf(line + 2, "%d/%*d/%*d  %d/%*d/%*d  %d/%*d/%*d",
                       &face.a, &face.b, &face.c) == 3)
                ;
            {
                array_push(mesh.faces, face);
            }
        }
    }
    fclose(f);
}