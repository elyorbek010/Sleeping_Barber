#ifndef BARBERSHOP_H
#define BARBERSHOP_H

#include "vector.h"

#define HAIRCUT_TIME_US 1000

typedef enum
{
    BARBERSHOP_SUCCESS,
    BARBERSHOP_FAILURE,
    BARBERSHOP_FULL
} barbershop_ret_t;

typedef struct barbershop_ctx barbershop_t;

barbershop_ret_t barbershop_open(size_t chairs_n);

barbershop_ret_t barbershop_close(void);

barbershop_ret_t have_a_haircut(size_t pthread_id);

#endif // BARBERSHOP_H