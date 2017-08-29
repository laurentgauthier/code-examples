/* The following define is required to enable access to some
 * of the profiling information.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <benchmark/benchmark.h>

extern void bitonic_merge_setup();
extern int bitonic_merge_check();
extern void bitonic_merge_test1();
extern void bitonic_merge_test2();
extern void bitonic_merge_test3();
extern void bitonic_merge_test4();
extern void bitonic_merge_copy();
extern void bitonic_empty_function();

void bitonic_merge_benchmark()
{
    struct sched_param o_param;
    struct sched_param n_param;
    int policy;
    struct profile_info_t timeOut, timeStamp;
    struct profile_info_t accumulation, average;
    int i;
    int count;

    memset(&o_param, 0, sizeof(o_param));
    memset(&n_param, 0, sizeof(n_param));

    pthread_t thread_info;
    thread_info = pthread_self();

    /* Save the current settings */
    pthread_getschedparam(thread_info, &policy, &o_param);

    /* Set policy to RR and priority to the highest one */
    n_param.sched_priority = sched_get_priority_max(SCHED_RR);
    if (pthread_setschedparam(thread_info, SCHED_RR, &n_param) != 0) {
        /* FIXME Check for the return code of this function */
        fprintf(stderr, "Failed to raise the thread priority\n");
    }

    count = 0;
    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        bitonic_merge_test1();
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
        count += bitonic_merge_check();
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("bitonic_merge_test1", &average);
    printf("check result: %d\n", count/1024);

    count = 0;
    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        bitonic_merge_test2();
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
        count += bitonic_merge_check();
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("bitonic_merge_test2", &average);
    printf("check result: %d\n", count/1024);

    count = 0;
    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        bitonic_merge_test3();
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
        count += bitonic_merge_check();
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("bitonic_merge_test3", &average);
    printf("check result: %d\n", count/1024);

    count = 0;
    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        bitonic_merge_test4();
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
        count += bitonic_merge_check();
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("bitonic_merge_test4", &average);
    printf("check result: %d\n", count/1024);

    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        bitonic_merge_copy();
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("bitonic_merge_copy", &average);

    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        bitonic_empty_function();
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("empty_function", &average);

    profile_clear_elapsed_time(&accumulation);
    for (i=0; i<1024; i++) {
        bitonic_merge_setup();
        profile_mark_time(&timeStamp);
        /* Nothing. */
        profile_measure_elapsed_time(&timeStamp);
        profile_accumulate_elapsed_time(&accumulation, &timeStamp);
    }
    profile_average_elapsed_time(&accumulation, 1024, &average);
    profile_print("nothing", &average);

    /* Restore the previous settings */
    pthread_setschedparam(thread_info, policy, &o_param);

}

int main(int argc, char **argv)
{
    bitonic_merge_benchmark();
}

