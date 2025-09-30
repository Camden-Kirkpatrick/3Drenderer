#pragma once
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
 
#define DEG2RAD(d) ((d) * (float)(M_PI / 180.0f))
#define RAD2DEG(r) ((r) * (float)(180.0f / M_PI))