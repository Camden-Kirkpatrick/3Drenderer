#include "light.h"

// Light from the camera
// 1 in z, means the light is pointing down the +z axis
light_t light = {.direction = {0, 0, 1}};

// Apply brightness scaling to a 32-bit ARGB color.
// The factor (0–1) scales RGB while keeping alpha unchanged.
// Multiplying masked channel values (e.g. 0x00FF0000 * 0.5) can produce
// "bleed" bits outside the intended 8-bit channel (e.g. 0x007F8000).
// To fix this, each result is masked again before recombining with OR.
uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor)
{
	if (percentage_factor < 0)
		percentage_factor = 0;
	if (percentage_factor > 1)
		percentage_factor = 1;

	uint32_t a = (original_color & 0xFF000000);
	uint32_t r = (original_color & 0x00FF0000) * percentage_factor;
	uint32_t g = (original_color & 0x0000FF00) * percentage_factor;
	uint32_t b = (original_color & 0x000000FF) * percentage_factor;

	uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
	return new_color;
}