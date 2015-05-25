#include "Timestamping.h"
int TS_invertDigits(int no)
{
	int aux=0;
	while (no)
	{
		aux=aux*10+(no%10);
		no/=10;
	}
	return aux;
}
#ifdef _WIN32
unsigned long long GetTimeDate()//Windows Time Date
{
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);
	unsigned long long toRet = 0;
	
	int aux;
	//adding the year
	aux = int(SystemTime.wYear);
	toRet   = aux;
	//adding the month
	aux     = int(SystemTime.wMonth);
	toRet <<= 4; //max value is 12 so 4 bits for it
	toRet  |= aux;
	//adding the day
	aux = int(SystemTime.wDay);
	toRet <<= 8;
	toRet  |= aux;
	//adding the hour
	aux = int(SystemTime.wHour);
	toRet <<= 8;
	toRet  |= aux;
	//adding the minute
	aux = int(SystemTime.wMinute);
	toRet <<= 8;
	toRet  |= aux;
	//adding the second
	aux = int(SystemTime.wSecond);
	toRet <<= 8;
	toRet  |= aux;
	//adding the millisecond
	aux = int(SystemTime.wMilliseconds);
	toRet <<=12;
	toRet  |= aux;
	
	return toRet;
}
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE    
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
	unsigned long long GetTimeDate()
	{
		timeval * tv;
		timezone * tz;
		gettimeofday(struct timeval *tv, struct timezone *tz);

		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFTimeInterval minsFromGMT = CFTimeZoneGetSecondsFromGMT(tz, CFAbsoluteTimeGetCurrent()) / 60.0;
		CFRelease(tz);
	}
	#else
        // Unsupported platform
    #endif
#elif __linux
    // linux
#endif
unsigned long long GetTimestampDifference(unsigned long long  t1,unsigned long long  t2)
{
	return (t1<t2)?t2-t1:t1-t2;
}
unsigned long long GetSysTime()
{
	//return GetTimeDate(); //just get the tick count for now
	return GetTickCount();
}