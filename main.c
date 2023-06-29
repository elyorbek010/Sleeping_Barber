#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "barbershop.h"
#include "debug.h"

#define CLIENTS_N 15
#define CHAIRS_N 5 // number of chairs in waiting room

static void *client_routine(void *arg);

int main(void)
{
    if (barbershop_open(CHAIRS_N) != BARBERSHOP_SUCCESS)
        exit(EXIT_FAILURE);

    pthread_t thread[CLIENTS_N] = {0};
    for (size_t i = 0; i < CLIENTS_N; i++)
    {
        if (pthread_create(&thread[i], NULL, client_routine, (void *)i) != 0)
            exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < CLIENTS_N; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
            exit(EXIT_FAILURE);
    }

    if (barbershop_close() != 0)
        exit(EXIT_FAILURE);

    return 0;
}

static void *client_routine(void *arg)
{
    size_t id = (size_t)pthread_self();

    debug_print("id: %zu entering barbershop\n", id);

    barbershop_ret_t ret = have_a_haircut(id);

    if (ret == BARBERSHOP_FAILURE)
    {
        debug_print("barbershop error\n");
    }
    else if (ret == BARBERSHOP_FULL)
    {
        debug_print("id: %zu did not fit\n", id);
    }
    else
    {
        debug_print("id: %zu finished\n", id);
    }

    return NULL;
}