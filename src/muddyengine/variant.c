#include <muddyengine/variant.h>
#include <assert.h>

void vset_char(variant_t * v, char value)
{
	assert(v != 0);
	v->type = VAR_CHAR;
	v->char_value = value;
}

void vset_short(variant_t * v, short value)
{
	assert(v != 0);
	v->type = VAR_SHORT;
	v->short_value = value;
}

void vset_int(variant_t * v, int value)
{
	assert(v != 0);
	v->type = VAR_INT;
	v->int_value = value;
}

void vset_long(variant_t * v, long value)
{
	assert(v != 0);
	v->type = VAR_LONG;
	v->long_value = value;
}

void vset_float(variant_t * v, float value)
{
	assert(v != 0);
	v->type = VAR_FLOAT;
	v->float_value = value;
}

void vset_double(variant_t * v, double value)
{
	assert(v != 0);
	v->type = VAR_DOUBLE;
	v->double_value = value;
}

void vset_ptr(variant_t * v, const void *value)
{
	assert(v != 0);
	v->type = VAR_PTR;
	v->ptr_value = value;
}

char vget_char(variant_t * v)
{
	assert(v != 0);
	return v->char_value;
}

int vget_int(variant_t * v)
{
	assert(v != 0);
	return v->int_value;
}

long vget_long(variant_t * v)
{
	assert(v != 0);
	return v->long_value;
}

float vget_float(variant_t * v)
{
	assert(v != 0);
	return v->float_value;
}

double vget_double(variant_t * v)
{
	assert(v != 0);
	return v->double_value;
}

const void *vget_ptr(variant_t * v)
{
	assert(v != 0);
	return v->ptr_value;
}

int vget_type(variant_t * v)
{
	assert(v != 0);
	return v->type;
}
