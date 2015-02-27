#include "variant.h"
#include <assert.h>

void vset_char(variant_t *v, char value)
{
    assert(v != 0);
    v->type = VAR_CHAR;
    v->value.vchar = value;
}

void vset_short(variant_t *v, short value)
{
    assert(v != 0);
    v->type = VAR_SHORT;
    v->value.vshort = value;
}

void vset_int(variant_t *v, int value)
{
    assert(v != 0);
    v->type = VAR_INT;
    v->value.vint = value;
}

void vset_long(variant_t *v, long value)
{
    assert(v != 0);
    v->type = VAR_LONG;
    v->value.vlong = value;
}

void vset_float(variant_t *v, float value)
{
    assert(v != 0);
    v->type = VAR_FLOAT;
    v->value.vfloat = value;
}

void vset_double(variant_t *v, double value)
{
    assert(v != 0);
    v->type = VAR_DOUBLE;
    v->value.vdouble = value;
}

void vset_ptr(variant_t *v, const void *value)
{
    assert(v != 0);
    v->type = VAR_PTR;
    v->value.ptr = value;
}

char vget_char(variant_t *v)
{
    assert(v != 0);
    return v->value.vchar;
}

int vget_int(variant_t *v)
{
    assert(v != 0);
    return v->value.vint;
}

long vget_long(variant_t *v)
{
    assert(v != 0);
    return v->value.vlong;
}

float vget_float(variant_t *v)
{
    assert(v != 0);
    return v->value.vfloat;
}

double vget_double(variant_t *v)
{
    assert(v != 0);
    return v->value.vdouble;
}

const void *vget_ptr(variant_t *v)
{
    assert(v != 0);
    return v->value.ptr;
}

int vget_type(variant_t *v)
{
    assert(v != 0);
    return v->type;
}
