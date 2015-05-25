/*
Windows time structure
typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;
*/
//CROSS PLATFORM TIMESTAMP FORMAT: DD/MM/YYYY/HH/MM/SS/mmm/
/*
D=day
M=Month
Y=Year
H=Hour
M=Minute
S=Second
m=Millisecond
*/
#ifdef _WIN32
#include<Windows.h>
int TS_invertDigits(int no);
void TS_AddCharEquivalent(int aux,int &pos,char * str);
char* GetWinTime();
char * TS_Partition(char * str,int &i);
unsigned long long TS_GetLongTime(char * str);

#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
	#include <sys/time.h>
	unsigned long GetTime();
	// Other kinds of Mac OS
    #else
        // Unsupported platform
    #endif
#elif __linux
    // linux
#endif
#include <boost/chrono.hpp>
bool CheckTimeStampFormat(char * stamp);
//unsigned long long GetTimestampDifference(char * reference, char * checked);
unsigned long long GetTimestampDifference(unsigned long long t1,unsigned long long t2);
unsigned long long GetSysTime();
