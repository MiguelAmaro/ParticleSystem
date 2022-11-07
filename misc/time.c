#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "src\types.h"
#include <stdio.h>
#include<stdlib.h>
#define Microseconds 1000000
int main(void)
{
  printf("hello time world\n");
  
  u64 TickFrequency = 0;
  u64 WorkEndTick = 0;
  u64 WorkBeginTick = 0;
  f64 TotalMicrosElapsed = 0.0;
  f64 TotalSecondsElapsed = 0.0;
  f64 AvgMicrosElapsed = 0.0;
  u64 FrameCounter = 0;
  QueryPerformanceFrequency((LARGE_INTEGER *)&TickFrequency);
  printf("freq: %llu ticks per second\n", TickFrequency);
  printf("freq: %llu ticks per us\n", TickFrequency/Microseconds);
  printf("freq: %lf us per tick\n", Microseconds/(f64)TickFrequency);
  int Running = 1;
  while(Running)
  {
    QueryPerformanceCounter((LARGE_INTEGER *)&WorkBeginTick);
    //work
    
    
    //end work
    QueryPerformanceCounter((LARGE_INTEGER *)&WorkEndTick);
    u64 TickDelta = WorkEndTick-WorkBeginTick;
    f64 MicrosElapsed   = TickDelta*TickFrequency/(u64)Microseconds;
    TotalMicrosElapsed += MicrosElapsed;
    TotalSecondsElapsed = TotalMicrosElapsed/(f64)Microseconds;
    AvgMicrosElapsed    = TotalMicrosElapsed/(f64)FrameCounter;
    
    printf(" total: %lfus"
           " total sec: %lfs"
           " avg: %lfus"
           " frameid: %llu"
           " tick delta: %llu"
           " time: %lfus\r",
           TotalMicrosElapsed,
           TotalSecondsElapsed,
           AvgMicrosElapsed,
           FrameCounter,
           TickDelta,
           MicrosElapsed);
    FrameCounter++;
  }
  
  return;
}