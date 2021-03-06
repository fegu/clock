#include "clock.h"

#ifdef _WIN32

// ***********************
// ******** WIN32 ********
// ***********************

#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define U64(x) x##Ui64
#else
  #define U64(x) x##ULL
#endif

#define DELTA_EPOCH_IN_100NS  U64(116444736000000000)

long ticks_to_nanos(LONGLONG subsecond_time, LONGLONG frequency)
{
    return (long)((1e9 * subsecond_time) / frequency);
}

ULONGLONG to_quad_100ns(FILETIME ft)
{
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return li.QuadPart;
}

void to_timespec_from_100ns(ULONGLONG t_100ns, long *t)
{
    t[0] = (long)(t_100ns / 10000000UL);
    t[1] = 100*(long)(t_100ns % 10000000UL);
}

void clock_readtime_monotonic(long* t)
{
   LARGE_INTEGER time;
   LARGE_INTEGER frequency;
   QueryPerformanceCounter(&time);
   QueryPerformanceFrequency(&frequency);
   // seconds
   t[0] = time.QuadPart / frequency.QuadPart;
   // nanos = 
   t[1] = ticks_to_nanos(time.QuadPart % frequency.QuadPart, frequency.QuadPart);
}

void clock_readtime_realtime(long* t)
{
    FILETIME ft;
    ULONGLONG tmp;

    GetSystemTimeAsFileTime(&ft);
 
    tmp = to_quad_100ns(ft);
    tmp -= DELTA_EPOCH_IN_100NS; 

    to_timespec_from_100ns(tmp, t);
}

void clock_readtime_processtime(long* t)
{
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULONGLONG time;

    GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time);
    // Both kernel and user, acc. to http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap03.html#tag_03_117

    time = to_quad_100ns(user_time) + to_quad_100ns(kernel_time);
    to_timespec_from_100ns(time, t);
}

void clock_readtime_threadtime(long* t)
{
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULONGLONG time;

    GetThreadTimes(GetCurrentThread(), &creation_time, &exit_time, &kernel_time, &user_time);
    // Both kernel and user, acc. to http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap03.html#tag_03_117

    time = to_quad_100ns(user_time) + to_quad_100ns(kernel_time);
    to_timespec_from_100ns(time, t);
}

void clock_readres_monotonic(long* t)
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    ULONGLONG resolution = U64(1000000000)/frequency.QuadPart;
    t[0] = resolution / U64(1000000000);
    t[1] = resolution % U64(1000000000);
}

void clock_readres_realtime(long* t)
{
    t[0] = 0;
    t[1] = 100;
}

void clock_readres_processtime(long* t)
{
    t[0] = 0;
    t[1] = 100;
}

void clock_readres_threadtime(long* t)
{
    t[0] = 0;
    t[1] = 100;
}

#else // Not _WIN32

// ***********************
// ******** POSIX ********
// ***********************

#include <time.h>

// due to missing define in FreeBSD 9.0 and 9.1 (http://lists.freebsd.org/pipermail/freebsd-stable/2013-September/075095.html)
#ifndef CLOCK_PROCESS_CPUTIME_ID
  #define CLOCK_PROCESS_CPUTIME_ID 15
#endif

void time_(clockid_t clock, long* t)
{

  clock_gettime(clock, (struct timespec*)t);

/*struct timespec a;

  clock_gettime(clock, &a);

  t[0] = a.tv_sec;
  t[1] = a.tv_nsec;*/

}

void clock_readtime_monotonic(long* t)
{
  time_(CLOCK_MONOTONIC, t);
}

void clock_readtime_realtime(long* t)
{
  time_(CLOCK_REALTIME, t);
}

void clock_readtime_processtime(long* t)
{
  time_(CLOCK_PROCESS_CPUTIME_ID , t);
}

void clock_readtime_threadtime(long* t)
{
  time_(CLOCK_THREAD_CPUTIME_ID , t);
}


void res_(clockid_t clock, long* t)
{

  clock_getres(clock, (struct timespec*)t);

/*struct timespec a;

  clock_getres(clock, &a);

  t[0] = a.tv_sec;
  t[1] = a.tv_nsec;*/

}

void clock_readres_monotonic(long* t)
{
  res_(CLOCK_MONOTONIC, t);
}

void clock_readres_realtime(long* t)
{
  res_(CLOCK_REALTIME, t);
}

void clock_readres_processtime(long* t)
{
  res_(CLOCK_PROCESS_CPUTIME_ID , t);
}

void clock_readres_threadtime(long* t)
{
  res_(CLOCK_THREAD_CPUTIME_ID , t);
}

#endif
