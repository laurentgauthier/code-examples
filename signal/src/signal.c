
#define _POSIX_C_SOURCE 200809L /* Used to ensure that the sigwait POSIX APIs is enabled. */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/* This function runs as a thread and does all the signal handling for the process.
   In a multi-threaded application this prevents random threads from being interrupted.
   This handler thread is also not subject to the usual limitations imposed on signal
   handlers. */
void *signal_handler(void *arg)
{
    int done = 0;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

    while (!done)
    {
        int sig;
        int status = sigwait(&set, &sig);
        if (status != 0)
        {
            perror("Sigwait error");
            pthread_exit((void *)2);
        }
        switch (sig)
        {
        case SIGHUP:
        case SIGUSR1:
        case SIGUSR2:
            fprintf(stderr, "INFO: Ignore signal: %d\n", sig);
            /* Custom behaviour could be implemented here. */
            break;
        case SIGINT:
        case SIGQUIT:
            fprintf(stderr, "INFO: Abiding by signal: %d\n", sig);
            /* TODO Trigger the shutdown of the application from here. */
            done = 1;
            break;

        default:
            fprintf(stderr, "ERROR: sigwait error\n");
            break;
        }
    }

    pthread_exit((void *)0);
}

int main(int argc, char **argv)
{
    int status;
    pthread_t handler_tid;
    int handler_status;
    sigset_t set;

    /* Block all signals at the process level, as they will be handled by a dedicated thread. */
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigprocmask(SIG_BLOCK, &set, NULL);

    /* Start the thread that will handle all these signals. */
    status = pthread_create(&handler_tid, NULL,
                            signal_handler,
                            (void *)NULL);
    if (status < 0)
    {
        perror("ERROR: failed to create signal handler thread");
        exit(1);
    }

    /* TODO Start the application from here, possibly in a separate thread. */

    /* Wait for the signal handler thread to complete. */
    status = pthread_join(handler_tid, (void *)&handler_status);
    if (status < 0)
        perror("ERROR: failed to join signal handler thread");

    exit(0);
}