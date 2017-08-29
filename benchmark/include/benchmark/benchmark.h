#ifndef BENCHMARK_BENCHMARK_H
#define BENCHMARK_BENCHMARK_H

/* The following define is required to enable access to some
 * of the profiling information.
 */
#define _GNU_SOURCE

#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <inttypes.h>

struct profile_info_t {
	uint64_t wall_time;		/* wall time */
	uint64_t ru_stime;		/* user CPU time */
	uint64_t ru_utime;		/* system CPU time */
	long     ru_nvcsw;		/* voluntary context switches */
	long     ru_nivcsw;		/* involuntary context switches */
};

static inline void profile_mark_time(struct profile_info_t * t)
{
	struct timespec ts;
	struct rusage ru;

    //(void)getrusage(RUSAGE_THREAD, &ru);
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);

	t->wall_time = (((uint64_t) ts.tv_sec * 1000000000ULL)
		+ (uint64_t) ts.tv_nsec);
	t->ru_stime = (((uint64_t) ru.ru_stime.tv_sec * 1000000000ULL)
		+ (uint64_t) ru.ru_stime.tv_usec * 1000ULL);
	t->ru_utime = (((uint64_t) ru.ru_utime.tv_sec * 1000000000ULL)
		+ (uint64_t) ru.ru_utime.tv_usec * 1000ULL);
	t->ru_nvcsw = ru.ru_nvcsw;
	t->ru_nivcsw = ru.ru_nivcsw;
}

static inline void profile_measure_elapsed_time(struct profile_info_t * t)
{
	struct timespec ts;
	struct rusage ru;

    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    //(void)getrusage(RUSAGE_THREAD, &ru);

	t->wall_time = (((uint64_t) ts.tv_sec * 1000000000ULL)
		+ (uint64_t) ts.tv_nsec) - t->wall_time;
	t->ru_stime = (((uint64_t) ru.ru_stime.tv_sec * 1000000000ULL)
		+ (uint64_t) ru.ru_stime.tv_usec * 1000ULL) - t->ru_stime;
	t->ru_utime = (((uint64_t) ru.ru_utime.tv_sec * 1000000000ULL)
		+ (uint64_t) ru.ru_utime.tv_usec * 1000ULL) - t->ru_utime;
	t->ru_nvcsw = ru.ru_nvcsw - t->ru_nvcsw;
	t->ru_nivcsw = ru.ru_nivcsw - t->ru_nivcsw;
}

static inline void profile_accumulate_elapsed_time(struct profile_info_t *acc, struct profile_info_t *t)
{
    acc->wall_time += t->wall_time;
    acc->ru_stime += t->ru_stime;
    acc->ru_utime += t->ru_utime;
    acc->ru_nvcsw += t->ru_nvcsw;
    acc->ru_nivcsw += t->ru_nivcsw;
}

static inline void profile_average_elapsed_time(struct profile_info_t *acc, int sample_count, struct profile_info_t *average)
{
    average->wall_time = acc->wall_time / sample_count;
    average->ru_stime = acc->ru_stime / sample_count;
    average->ru_utime = acc->ru_utime / sample_count;
    average->ru_nvcsw = acc->ru_nvcsw / sample_count;
    average->ru_nivcsw = acc->ru_nivcsw / sample_count;
}

static inline void profile_clear_elapsed_time(struct profile_info_t *t)
{
    t->wall_time = 0;
    t->ru_stime = 0;
    t->ru_utime = 0;
    t->ru_nvcsw = 0;
    t->ru_nivcsw = 0;
}

static inline void profile_compute_elapsed_time(struct profile_info_t * t1, struct profile_info_t * t2, struct profile_info_t *t)
{
    t->wall_time = t2->wall_time - t1->wall_time;
    t->ru_stime = t2->ru_stime - t1->ru_stime;
    t->ru_utime = t2->ru_utime - t1->ru_utime;
    t->ru_nvcsw = t2->ru_nvcsw - t1->ru_nvcsw;
    t->ru_nivcsw = t2->ru_nivcsw - t1->ru_nivcsw;
}

static inline void profile_print(char *text, struct profile_info_t * t)
{
    printf("%s:\n", text);
    printf(" wall_time = %lu ns\n", t->wall_time);
    printf(" ru_stime = %lu ns\n", t->ru_stime);
    printf(" ru_utime = %lu ns\n", t->ru_utime);
    printf(" ru_nvcsw = %lu ns\n", t->ru_nvcsw);
    printf(" ru_nivcsw = %lu ns\n", t->ru_nivcsw);
}

#endif
