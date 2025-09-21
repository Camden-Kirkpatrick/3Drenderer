#include "texture.h"
#include <stdio.h>

tex2_t tex2_clone(tex2_t *t)
{
    return (tex2_t){t->u, t->v};
}