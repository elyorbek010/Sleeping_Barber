#define _GNU_SOURCE
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "vector.h"

#define CHECK_AND_RETURN_IF_NOT_EXIST(pointer_object) \
    do                                                \
    {                                                 \
        if (pointer_object == NULL)                   \
        {                                             \
            return VECTOR_FAILURE;                    \
        }                                             \
    } while (0)

struct vector_t
{
    size_t capacity;
    size_t begin; // begin index is inclusive
    size_t end;   // end index is exclusive

    bool shut_down_flag;

    pthread_mutex_t guard;
    pthread_cond_t avail;

    void *element[]; // flexible array
};

/*
 * FUNCTION DECLARATIONS
 */

vector_t *vector_create(const size_t capacity);

vector_ret_t vector_shut_down(vector_t *vector);

vector_ret_t vector_destroy(vector_t *vector);

vector_ret_t vector_push(vector_t *vector, void *element);

vector_ret_t vector_pop(vector_t *vector, void **element);

static vector_ret_t vector_expand(vector_t *vector);

static inline size_t vector_next_index(const size_t index, const size_t capacity);

/*
 * FUNCTION DEFINITIONS
 */

vector_t *vector_create(size_t capacity)
{
    vector_t *vector = malloc(sizeof(*vector) + (capacity + 1) * sizeof(vector->element[0]));

    if (vector == NULL)
        return NULL;

    vector->capacity = capacity;
    vector->begin = vector->end = 0;
    vector->shut_down_flag = false;

    if (pthread_mutex_init(&vector->guard, NULL) != 0)
    {
        free(vector);

        return NULL;
    }

    if (pthread_cond_init(&vector->avail, NULL) != 0)
    {
        pthread_mutex_destroy(&vector->guard);
        free(vector);
        return NULL;
    }

    return vector;
}

vector_ret_t vector_shut_down(vector_t *vector)
{
    CHECK_AND_RETURN_IF_NOT_EXIST(vector);

    if(pthread_mutex_lock(&vector->guard) != 0)
        return VECTOR_FAILURE;

    vector->shut_down_flag = true;

    if(pthread_mutex_unlock(&vector->guard) != 0)
        return VECTOR_FAILURE;

    if(pthread_cond_signal(&vector->avail) != 0)
        return VECTOR_FAILURE;
    
    return VECTOR_SUCCESS;
}

vector_ret_t vector_destroy(vector_t *vector)
{
    CHECK_AND_RETURN_IF_NOT_EXIST(vector);

    pthread_mutex_destroy(&vector->guard);
    pthread_cond_destroy(&vector->avail);

    free(vector);

    return VECTOR_SUCCESS;
}

vector_ret_t vector_push(vector_t *vector, void *element)
{
    CHECK_AND_RETURN_IF_NOT_EXIST(vector);

    if (pthread_mutex_lock(&vector->guard) != 0) // mutex lock
        return VECTOR_FAILURE;

    // check if vector is FULL
    if (vector_next_index(vector->end, vector->capacity) == vector->begin)
    {
        pthread_mutex_unlock(&vector->guard);
        return VECTOR_OVERFLOW;
    }

    // insert element
    vector->element[vector->end] = element;
    vector->end = vector_next_index(vector->end, vector->capacity);

    if (pthread_mutex_unlock(&vector->guard) != 0) // mutex unlock
        return VECTOR_FAILURE;

    if(pthread_cond_signal(&vector->avail) != 0)
        return VECTOR_FAILURE;

    return VECTOR_SUCCESS;
}

vector_ret_t vector_pop(vector_t *vector, void **p_element)
{
    CHECK_AND_RETURN_IF_NOT_EXIST(vector);
    CHECK_AND_RETURN_IF_NOT_EXIST(p_element);

    if (pthread_mutex_lock(&vector->guard) != 0) // mutex lock
        return VECTOR_FAILURE;

    // if empty, wait until element is available
    while (vector->begin == vector->end)
    {
        if (vector->shut_down_flag == true)
        {
            pthread_mutex_unlock(&vector->guard);
            return VECTOR_END;
        }

        pthread_cond_wait(&vector->avail, &vector->guard);
    }

    // withdraw element
    *p_element = vector->element[vector->begin];
    vector->begin = vector_next_index(vector->begin, vector->capacity);

    if (pthread_mutex_unlock(&vector->guard) != 0) // mutex unlock
        return VECTOR_FAILURE;

    return VECTOR_SUCCESS;
}

static inline size_t vector_next_index(const size_t index, const size_t capacity)
{
    // note: actual allocated capacity is 'capacity + 1'
    return (index + 1) % (capacity + 1);
}