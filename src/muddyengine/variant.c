#include <muddyengine/variant.h>

void vset_char(variant_t *v, char value)
{
	v->type = VAR_CHAR;
	v->char_value = value;
}

void vset_short(variant_t *v, short value)
{
	v->type = VAR_SHORT;
	v->short_value = value;
}

void vset_int(variant_t *v, int value)
{
	v->type = VAR_INT;
	v->int_value = value;
}

void vset_long(variant_t *v, long value)
{
	v->type = VAR_LONG;
	v->long_value = value;	
}

void vset_float(variant_t *v, float value)
{
	v->type = VAR_FLOAT;
	v->float_value = value;
}

void vset_double(variant_t *v, double value)
{
	v->type = VAR_DOUBLE;
	v->double_value = value;
}

void vset_ptr(variant_t *v, const void * value)
{
	v->type = VAR_PTR;
	v->ptr_value = value;
}
