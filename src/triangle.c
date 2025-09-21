#include "triangle.h"
#include "display.h"
#include "swap.h"
#include "upng.h"
#include "mathdefs.h"

vec3_t get_triangle_normal(vec4_t vertices[3])
{
    // These are the vectors that make up the triangle
    vec3_t vec_a = vec3_from_vec4(vertices[0]);
	vec3_t vec_b = vec3_from_vec4(vertices[1]);
	vec3_t vec_c = vec3_from_vec4(vertices[2]);

	// Find the 2 edge vectors to compute the normal with
	vec3_t vec_ab = vec3_sub(vec_b, vec_a);
	vec3_t vec_ac = vec3_sub(vec_c, vec_a);
    vec3_normalize(&vec_ab);
    vec3_normalize(&vec_ac);

    // Compute the normal
	vec3_t normal = vec3_cross(vec_ab, vec_ac);
	vec3_normalize(&normal);

    return normal;
}

void draw_triangle(Window *w, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	draw_line(w, x0, y0, x1, y1, color);
	draw_line(w, x1, y1, x2, y2, color);
	draw_line(w, x2, y2, x0, y0, color);
}

// Use the same process as draw_texel
void draw_triangle_pixel(
        Window *w,
        int x, int y, uint32_t color,
        vec4_t point_a, vec4_t point_b, vec4_t point_c
    )
{
    if (x < 0 || x >= w->width || y < 0 || y >= w->height) return;

    vec2_t p = {x, y};
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    float interpolated_reciprocal_w;

    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    interpolated_reciprocal_w = 1.0f - interpolated_reciprocal_w;

    if (interpolated_reciprocal_w < w->z_buffer[(w->width * y) + x])
    {
        draw_pixel(w, x, y, color);
        w->z_buffer[(w->width * y) + x] = interpolated_reciprocal_w;
    }
}

// Use the same process as draw_textured_triangle
void draw_filled_triangle(
        Window *w,
        int x0, int y0, float z0, float w0,
        int x1, int y1, float z1, float w1,
        int x2, int y2, float z2, float w2,
        uint32_t color
    )
{
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }
    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);

        if (y0 > y1)
        {
            int_swap(&y0, &y1);
            int_swap(&x0, &x1);
            float_swap(&z0, &z1);
            float_swap(&w0, &w1);
        }
    }
    
    vec4_t point_a = {x0, y0, z0, w0};
    vec4_t point_b = {x1, y1, z1, w1};
    vec4_t point_c = {x2, y2, z2, w2};

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////

    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Only draw the top (flat-bottom) half if it has vertical height.
    // If y1 == y0, the triangle is actually flat-top and there is no top half to render.
    if (y0 != y1)
    {
        // Render the flat-bottom triangle
        for (int y = y0; y <= y1; y++)
        {   
            float x_start = x1 + (y - y1) * inv_slope_1;
            float x_end = x0 + (y - y0) * inv_slope_2;

            // Swap if x_start is to the right of x_end
            if (x_end < x_start) float_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++)
            {
                draw_triangle_pixel(w, x, y, color, point_a, point_b, point_c);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////

    inv_slope_1 = (float)(x2 - x1) / (y2 - y1);
    inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Only draw the bottom (flat-yop) half if it has vertical height.
    // If y1 == y2, the triangle is actually flat-bottom and there is no bottom half to render.
    if (y1  != y2)
    {
        // Render the flat-top triangle
        for (int y = y1; y <= y2; y++)
        {
            // Find the new x_start and x_end for the scanline
            float x_start = x1 + (y - y1) * inv_slope_1;
            float x_end = x0 + (y - y0) * inv_slope_2;

            // Swap if x_start is to the right of x_end
            // This can occur if the object is rotated
            if (x_end < x_start) float_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++)
            {
                draw_triangle_pixel(w, x, y, color, point_a, point_b, point_c);
            }
        }
    }
}

// Given 3 triangle vertices and a point inside the triangle, return alpha, beta, and gamma
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p)
{
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);
    vec2_t ap = vec2_sub(p, a);

    // We use the Cross Product to find the area of the parallelogram
    // However, the Cross Product isnt defined for a vec2, so we are just
    // using the formula for the z component of the Cross Product (see vector.c)
    float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x);

    // Find the area for each sub-triangle
    float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;
    float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;
    float gamma = 1.0 - alpha - beta;

    return (vec3_t){alpha, beta, gamma};
}

void draw_texel(
        Window *w,
        int x, int y, upng_t *texture,
        vec4_t point_a, vec4_t point_b, vec4_t point_c,
        tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
     )
{
    if (x < 0 || x >= w->width || y < 0 || y >= w->height) return;

    vec2_t p = {x, y};
    // We don't need to pass in z and w to caluclate the Barycentric coordinates
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    // Get alpha, beta, and gamma
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Now we need to ensure a perspective-correct texture mapping
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // Interpolate U and V in screen space using barycentric weights, 
    // but scaled by 1/w so that they vary linearly after the perspective divide
    interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
    interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

    // Interpolate 1/w itself (this is also linear in screen space)
    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    // Recover the true U and V by dividing out the interpolated 1/w.
    // This cancels the earlier divide by w and gives perspective-correct coordinates.
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // Since U,V have an origin in the bottom left, and the texture has an origin in the top left,
    // we have to flip V, by doing 1.0 - V
    interpolated_v = 1.0f - interpolated_v;

    // Get the texture's width and height
    int tex_width = upng_get_width(texture);
    int tex_height = upng_get_height(texture);

    // Scale normalized UVs (0–1 range) up to texture pixel coordinates
    int tex_x = (int)(interpolated_u * (tex_width  - 1));
    int tex_y = (int)(interpolated_v * (tex_height - 1));

    // Ensure tex_x and tex_y are in the range [0, texture_width/height - 1]
    if (tex_x < 0) tex_x = 0;
    else if (tex_x >= tex_width) tex_x = tex_width - 1;

    if (tex_y < 0) tex_y = 0;
    else if (tex_y >= tex_height) tex_y = tex_height - 1;


    // Adjust 1/w, so that pixels that are closer to the camera have a smaller value
    interpolated_reciprocal_w = 1.0f - interpolated_reciprocal_w;

    // Only draw the pixel if the depth value is les then the one previously stored in the z-buffer
    if (interpolated_reciprocal_w < w->z_buffer[(w->width * y) + x])
    {
        // Get the buffer of colros from the texture
        uint32_t *tex_buffer = (uint32_t*)upng_get_buffer(texture);

        // Draw the correct color from the texture
        draw_pixel(w, x, y, tex_buffer[(tex_width * tex_y) + tex_x]);
        // Update the z-buffer with the 1/w of the current pixel
        w->z_buffer[(w->width * y) + x] = interpolated_reciprocal_w;
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle based on a texture array of colors.
// We split the original triangle in two, half flat-bottom and half flat-top.
// The z and w components are important for ensuring a perspective correct texture.
///////////////////////////////////////////////////////////////////////////////////
//        v0
//        /\
//       /  \
//      /    \
//     /      \
//   v1--------\
//     \_       \
//        \_     \
//           \_   \
//              \_ \
//                 \\
//                   \
//                    v2
/////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(
        Window *w,
        upng_t *texture,
        int x0, int y0, float z0, float w0, float u0, float v0,
        int x1, int y1, float z1, float w1, float u1, float v1,
        int x2, int y2, float z2, float w2, float u2, float v2
    )
{
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);

        if (y0 > y1)
        {
            int_swap(&y0, &y1);
            int_swap(&x0, &x1);
            float_swap(&z0, &z1);
            float_swap(&w0, &w1);
            float_swap(&u0, &u1);
            float_swap(&v0, &v1);
        }
    }
    
    vec4_t point_a = {x0, y0, z0, w0};
    vec4_t point_b = {x1, y1, z1, w1};
    vec4_t point_c = {x2, y2, z2, w2};

    tex2_t a_uv = {u0, v0};
    tex2_t b_uv = {u1, v1};
    tex2_t c_uv = {u2, v2};

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////

    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Only draw the top (flat-bottom) half if it has vertical height.
    // If y1 == y0, the triangle is actually flat-top and there is no top half to render.
    if (y0 != y1)
    {
        // Render the flat-bottom triangle
        for (int y = y0; y <= y1; y++)
        {   
            float x_start = x1 + (y - y1) * inv_slope_1;
            float x_end = x0 + (y - y0) * inv_slope_2;

            // Swap if x_start is to the right of x_end
            if (x_end < x_start) float_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++)
            {
                // Draw the pixel that comes from the texture
                draw_texel(w, x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////

    inv_slope_1 = (float)(x2 - x1) / (y2 - y1);
    inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Only draw the bottom (flat-yop) half if it has vertical height.
    // If y1 == y2, the triangle is actually flat-bottom and there is no bottom half to render.
    if (y1  != y2)
    {
        // Render the flat-top triangle
        for (int y = y1; y <= y2; y++)
        {
            // Find the new x_start and x_end for the scanline
            float x_start = x1 + (y - y1) * inv_slope_1;
            float x_end = x0 + (y - y0) * inv_slope_2;

            // Swap if x_start is to the right of x_end
            // This can occur if the object is rotated
            if (x_end < x_start) float_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++)
            {
                // Draw the pixel that comes from the texture
                draw_texel(w, x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }
}