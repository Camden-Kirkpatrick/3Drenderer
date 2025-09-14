#include <math.h>
#include "clipping.h"

#define NUM_PLANES 6
plane_t frustum_planes[NUM_PLANES];

///////////////////////////////////////////////////////////////////////////////
// Frustum planes are defined by a point and a normal vector
///////////////////////////////////////////////////////////////////////////////
// Near plane   :  P=(0, 0, znear), N=(0, 0,  1)
// Far plane    :  P=(0, 0, zfar),  N=(0, 0, -1)
// Top plane    :  P=(0, 0, 0),     N=(0, -cos(fov/2), sin(fov/2))
// Bottom plane :  P=(0, 0, 0),     N=(0, cos(fov/2), sin(fov/2))
// Left plane   :  P=(0, 0, 0),     N=(cos(fov/2), 0, sin(fov/2))
// Right plane  :  P=(0, 0, 0),     N=(-cos(fov/2), 0, sin(fov/2))
///////////////////////////////////////////////////////////////////////////////
//
//           /|\
//         /  | | 
//       /\   | |
//     /      | |
//  P*|-->  <-|*|   ----> +z-axis
//     \      | |
//       \/   | |
//         \  | | 
//           \|/
//
///////////////////////////////////////////////////////////////////////////////
void init_frustum_planes(float fovx, float fovy, float z_near, float z_far)
{
	float cos_half_fovx = cos(fovx / 2);
	float sin_half_fovx = sin(fovx / 2);
	float cos_half_fovy = cos(fovy / 2);
	float sin_half_fovy = sin(fovy / 2);

	frustum_planes[LEFT_FRUSTUM_PLANE].point = (vec3_t){0, 0, 0};
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fovx;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fovx;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = (vec3_t){0, 0, 0};
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fovx;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fovx;

	frustum_planes[TOP_FRUSTUM_PLANE].point = (vec3_t){0, 0, 0};
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fovy;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fovy;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = (vec3_t){0, 0, 0};
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fovy;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fovy;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = (vec3_t){0, 0, z_near};
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

	frustum_planes[FAR_FRUSTUM_PLANE].point = (vec3_t){0, 0, z_far};
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2)
{
	return (polygon_t){
		.vertices = {v0, v1, v2},
		.num_vertices = 3
	};
}

void triangles_from_polygon(polygon_t *polygon, triangle_t triangles[], int *num_triangles)
{
	for (int i = 0; i < polygon->num_vertices - 2; i++)
	{
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;

		triangles[i].points[0] = vec4_from_vec3(polygon->vertices[index0]);
		triangles[i].points[1] = vec4_from_vec3(polygon->vertices[index1]);
		triangles[i].points[2] = vec4_from_vec3(polygon->vertices[index2]);
	}
	*num_triangles = polygon->num_vertices - 2;
}

void clip_polygon_against_plane(polygon_t *polygon, int plane)
{
	vec3_t plane_point = frustum_planes[plane].point;
	vec3_t plane_normal = frustum_planes[plane].normal;

	// Store the vertices of the final clipped polygon
	vec3_t inside_vertices[MAX_NUM_POLYGON_VERTICES];
	int num_inside_vertices = 0;

	// Keep track of the current and previous vertex of the polygon
	vec3_t *current_vertex = &polygon->vertices[0]; // first vertex
	vec3_t *previous_vertex = &polygon->vertices[polygon->num_vertices - 1]; // last vertex

	// Find the dot product of the current and previous vertex, so we know if they are inside or outside the plane
	// dot = 0 -> vertex is exactly on the plane
	// dot > 0 -> vertex is inside the plane
	// dot < 0 -> vertex is outside the plane (these will be clipped)
	float current_dot = 0.0f;
	float previous_dot= vec3_dot(vec3_sub(*previous_vertex, plane_point), plane_normal);

	// Loop through all the polygon vertices
	while(current_vertex != &polygon->vertices[polygon->num_vertices])
	{
		// Calculate the dot product for each vertex to determine if it's inside or outside the plane
		current_dot = vec3_dot(vec3_sub(*current_vertex, plane_point), plane_normal);

		// If we changed from inside to outside the plane, or vice-versa
		if (current_dot * previous_dot < 0)
		{
			// Calculate the linear interpolation factor 't', t = dotQ1 / (dotQ1 - dotQ2)
			float t = previous_dot / (previous_dot - current_dot);

			// Calculate the intersection point, I = Q1 + t(Q2 - Q1)
			vec3_t intersection_point = vec3_clone(current_vertex); 			 // I = current vertex
			intersection_point = vec3_sub(intersection_point, *previous_vertex); // I = Q2 - Q1
			intersection_point = vec3_mul(intersection_point, t); 				 // I = t(Q2 - Q1)
			intersection_point = vec3_add(*previous_vertex, intersection_point); // I = Q1 + t(Q2 - Q1)

			inside_vertices[num_inside_vertices++] = vec3_clone(&intersection_point);
		}

		// If the current vertex is inside the plane
		if (current_dot > 0)
			inside_vertices[num_inside_vertices++] = vec3_clone(current_vertex);

		previous_dot = current_dot;
		previous_vertex = current_vertex;
		current_vertex++; // increment the pointer to the next element
	}
	// Copy all the inside vertices into the original polygon 
	for (int i = 0; i < num_inside_vertices; i++)
	{
		polygon->vertices[i] = inside_vertices[i];
	}
	polygon->num_vertices = num_inside_vertices;
}

void clip_polygon(polygon_t *polygon)
{
	clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE); 
	clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE); 
	clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE); 
	clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE); 
	clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE); 
	clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE); 
}