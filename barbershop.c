#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "barbershop.h"
#include "vector.h"
#include "debug.h"

static vector_t *waiting_room = NULL;
static pthread_t barber_id;

static void *barber_routine(void *arg)
{
    pthread_t id;

    while (true)
    {
        if (vector_pop(waiting_room, (void **)&id) == VECTOR_END)
            return NULL;

        usleep(HAIRCUT_TIME_US);

        pthread_kill(id, SIGUSR1);
    }
}

barbershop_ret_t have_a_haircut(size_t pthread_id)
{
    size_t id = pthread_id;

    vector_ret_t ret = vector_push(waiting_room, (void *)id);

    if (ret == VECTOR_FAILURE)
        return BARBERSHOP_FAILURE;
    else if (ret == VECTOR_OVERFLOW)
        return BARBERSHOP_FULL;
    else
    {
        int sig = 0;
        sigset_t sig_set;
        sigemptyset(&sig_set);
        sigaddset(&sig_set, SIGUSR1);

        sigwait(&sig_set, &sig);
    }

    return BARBERSHOP_SUCCESS;
}

static void sig_handler(int sig)
{
    return;
}

static void sig_init(void)
{
    struct sigaction sigusr1;

    sigusr1.sa_flags = 0;
    sigusr1.sa_handler = sig_handler;
    sigemptyset(&sigusr1.sa_mask);
    sigaddset(&sigusr1.sa_mask, SIGUSR1);

    sigaction(SIGUSR1, &sigusr1, NULL);
}

barbershop_ret_t barbershop_open(size_t chairs_n)
{
    sig_init();

    waiting_room = vector_create(chairs_n);

    if (waiting_room == NULL)
        return BARBERSHOP_FAILURE;

    if (pthread_create(&barber_id, NULL, barber_routine, NULL) != 0)
        return BARBERSHOP_FAILURE;

    return BARBERSHOP_SUCCESS;
}

barbershop_ret_t barbershop_close(void)
{
    if (vector_shut_down(waiting_room) != VECTOR_SUCCESS)
        return BARBERSHOP_FAILURE;

    if (pthread_join(barber_id, NULL) != 0)
    {
        vector_destroy(waiting_room);
        return BARBERSHOP_FAILURE;
    }

    if (vector_destroy(waiting_room) != 0)
        return BARBERSHOP_FAILURE;

    return BARBERSHOP_SUCCESS;
}