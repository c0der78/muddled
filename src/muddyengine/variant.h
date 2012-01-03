#ifndef __VARIANT_H_

#define VAR_CHAR	0
#define VAR_SHORT 	1
#define VAR_INT 	2
#define VAR_LONG	3
#define VAR_FLOAT	4
#define VAR_DOUBLE	5
#define VAR_PTR 	6

typedef struct variant_t {
    int type;
    union {
        char char_value;
        short short_value;
        int int_value;
        long long_value;
        float float_value;
        double double_value;
        const void* ptr_value;
    };
} variant_t;

void vset_char(variant_t *, char);
void vset_short(variant_t *, short);
void vset_int(variant_t *, int);
void vset_long(variant_t *, long);
void vset_float(variant_t *, float);
void vset_double(variant_t *, double);
void vset_ptr(variant_t *, const void*);

#endif
