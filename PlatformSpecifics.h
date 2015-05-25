#include "CoClipConstants.h"
#ifdef _WIN64
   //define something for Windows (64-bit)
	#include <windows.h>
	#include <winsock2.h>
	#include <direct.h>
	#include <stdlib.h>
	#include <iphlpapi.h>
	size_t getTotalSystemMemory()
	{
		printf("\nCalled sys mem!");
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);
		return status.ullTotalPhys;
	}
	bool GetUIDandLoadAddresses(unsigned long long & uid,char * pcname)
	{
		//UID will be made of the first 2 chars of the adapter name and of the mac address
		//IPHLP is supposed to work for this
		IP_ADAPTER_INFO AdapterInfo[MAX_NET_ADAPTERS]; // Allocate information
		// for up to 16 NICs
		DWORD dwBufLen = sizeof(AdapterInfo); // Save memory size of buffer

		DWORD dwStatus = GetAdaptersInfo( // Call GetAdapterInfo
		AdapterInfo, // [out] buffer to receive data
		&dwBufLen); // [in] size of receive data buffer
		if(dwStatus == ERROR_SUCCESS); // Verify return value is
			return false;

		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
		// current adapter info
		do
		{
			IP_ADDR_STRING * PTR = &pAdapterInfo->IpAddressList;
			//go through all the ip addresses and add them
			do
			{
				NetExits[adapterIDX].IP=htonl(INET_CHARtoH(PTR->IpAddress.String));
				NetExits[adapterIDX].NetMask = htonl(INET_CHARtoH(PTR->IpMask.String));
				NetExits[adapterIDX].Bcast = NetExits[adapterIDX].IP & NetExits[adapterIDX++].NetMask;
				PTR = PTR->Next;
			}
			while(PTR);
			//calculating UID
			uid=0;
			uid |= pAdapterInfo->AdapterName[1];
			uid<<=8;
			uid |= pAdapterInfo->AdapterName[2];
			uid<<=8;
			for(int i=0;i<6;++i)
			{
				uid |= pAdapterInfo->Address[i] ;
				uid<<=8;
			}
			pAdapterInfo = pAdapterInfo->Next; // Progress through
			// linked list
		}
		while(pAdapterInfo); // Terminate if last adapter

		char * pname = new char[150];
		gethostname(pname, sizeof(pname));
		int i=0;
		for(i=0;pname[i];++i)
			pcname[i]=pname[i];
		pcname[i]=0;
		delete[] pname;

		return true;
	}
#elif _WIN32
   //define something for Windows (32-bit)
	#include <windows.h>
	#include <winbase.h>
	#include <winsock2.h>
	#include <direct.h>
	#include <stdlib.h>
	#include <iphlpapi.h>

    //system memory
	size_t getTotalSystemMemory()
	{
		MEMORYSTATUS status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatus(&status);
		if(status.dwMemoryLoad<100)
			return (status.dwTotalPhys/(status.dwMemoryLoad+1));
		return 0;
	}
	bool GetUIDandLoadAddresses(unsigned long long & uid,char * pcname)
	{
		pcname[0]=0;
		//UID will be made of the first 2 chars of the adapter name and of the mac address
		//IPHLP is supposed to work for this
		IP_ADAPTER_INFO AdapterInfo[MAX_NET_ADAPTERS]; // Allocate information
		// for up to 16 NICs
		DWORD dwBufLen = sizeof(AdapterInfo); // Save memory size of buffer

		DWORD dwStatus = GetAdaptersInfo( // Call GetAdapterInfo
		AdapterInfo, // [out] buffer to receive data
		&dwBufLen); // [in] size of receive data buffer
		//assert(dwStatus == ERROR_SUCCESS); // Verify return value is
		unsigned int bip;
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
		// current adapter info
		do
		{
			IP_ADDR_STRING * PTR = &pAdapterInfo->IpAddressList;
			int lastIDX = adapterIDX;
			//go through all the ip addresses and add them
			do
			{
				bip=INET_CHARtoH(PTR->IpAddress.String);
				if(bip)
				{
					NetExits[adapterIDX].IP=htonl(bip);
					NetExits[adapterIDX].NetMask = htonl(INET_CHARtoH(PTR->IpMask.String));
					NetExits[adapterIDX].Bcast = (NetExits[adapterIDX].IP | (~NetExits[adapterIDX].NetMask));
					adapterIDX++;
				}
				PTR = PTR->Next;
			}
			while(PTR);
			//get gateways
			int lcpy = lastIDX;
			PTR = &pAdapterInfo->GatewayList;
			do
			{
				bip = htonl(INET_CHARtoH(PTR->IpAddress.String));
				//let's check if the gateway is in the same network with any other previous networks
				lastIDX=lcpy;
				for(;lastIDX<adapterIDX;++lastIDX)
					if((bip&NetExits[lastIDX].NetMask) == (NetExits[lastIDX].IP&NetExits[lastIDX].NetMask))
					{//if they are in the same network it means the gateway is relevant to this record
						NetExits[lastIDX].Gateway = bip;
						printf("\nAvailable Gateway:%s",PTR->IpAddress.String);
					}
				PTR = PTR->Next;
			}
			while(PTR);
			//calculating UID
			uid=0;
			uid |= pAdapterInfo->AdapterName[1];
			uid<<=8;
			uid |= pAdapterInfo->AdapterName[2];
			uid<<=8;
			for(int i=0;i<6;++i)
			{
				uid |= pAdapterInfo->Address[i] ;
				uid<<=8;
			}
			pAdapterInfo = pAdapterInfo->Next; // Progress through
			// linked list
		}
		while(pAdapterInfo); // Terminate if last adapter

		char pme[150];
		gethostname(pme, sizeof(pme));
		printf("\npname:%s",pme);

		int i=0;
		for(i=0;pme[i];++i)
			pcname[i]=pme[i];
		pcname[i]=0;

		return true;
	}
	//locking
	//critical section implementation
	class LockOBJ
	{
		public:
		bool IsInvalid;
		CRITICAL_SECTION CriticalSection;
		bool InitLockSystem()
		{
			// Initialize the critical section one time only.
			if(! InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400) )
			{
				printf("\nLast Error Code: %d",GetLastError());
				return false;
			}
			return true;
		}
		LockOBJ()
		{
			IsInvalid = false;
			if(!InitLockSystem())
				IsInvalid = true;
		}
		bool tryLOCK()
		{
			return TryEnterCriticalSection(&CriticalSection);
		}
		void LOCK()
		{
			try
			{
				EnterCriticalSection(&CriticalSection);
			}
			catch(...)
			{
				printf("\nCRITICAL SECTION TIMEOUT (DEAD LOCK EXITED...)");
			}
		}
		void unLOCK()
		{
			LeaveCriticalSection(&CriticalSection);
		}
	};
	unsigned long long FileSize(char * name)
	{
		unsigned long long size=0;
		/*//LET'S GET THE FILE HANDLE ( JUST OPEN IT FIRST);
		HANDLE file = CreateFile(name,
								 GENERIC_READ,
								 0,
								 NULL,
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL,
								 NULL);
		//NOW LET'S GET THE FILE SIZE
		GetFileInformationByHandleEx(file,FILE_BASIC_INFO,&size,sizeof(unsigned long long));

		//RELEASE THE FILE
		CloseHandle(file);
		*/
		return size;
	}
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
	//Mac OS
	static double ParseMemValue(const char * b)
	{
	   while((*b)&&(isdigit(*b) == false)) b++;
	   return isdigit(*b) ? atof(b) : -1.0;
	}

	// Returns a number between 0.0f and 1.0f, with 0.0f meaning all RAM is available, and 1.0f meaning all RAM is currently in use
	float GetSystemMemoryUsagePercentage()
	{
	    FILE * fpIn = popen("/usr/bin/vm_stat", "r");
	    if (fpIn)
		{
			double pagesUsed = 0.0, totalPages = 0.0;
			char buf[512];
			while(fgets(buf, sizeof(buf), fpIn) != NULL)
			{
				if (strncmp(buf, "Pages", 5) == 0)
				{
					double val = ParseMemValue(buf);
					if(val >= 0.0)
					{
						if ((strncmp(buf, "Pages wired", 11) == 0)||(strncmp(buf, "Pages active", 12) == 0))
							pagesUsed += val;
					    totalPages += val;
					}
				}
				else if (strncmp(buf, "Mach Virtual Memory Statistics", 30) != 0) break;  // Stop at "Translation Faults", we don't care about anything at or below that
			}
			pclose(fpIn);

			if (totalPages > 0.0)
				return (float) (pagesUsed/totalPages);
	    }
		return -1.0f;  // indicate failure
	}
    #else
        // Unsupported platform
    #endif
#elif __linux
    // linux
	#include <unistd.h>
	size_t getTotalSystemMemory()
	{
		long pages = sysconf(_SC_PHYS_PAGES);
		long page_size = sysconf(_SC_PAGE_SIZE);
		return pages * page_size;
	}
#endif
unsigned long long GetAvailableMem()
{
#ifdef _WIN64
//define something for Windows (64-bit)
 printf("\nOS is Win (program running in 64x) with mem:%lld",(unsigned long long)getTotalSystemMemory());
 return (unsigned long long)getTotalSystemMemory();
#elif _WIN32
 printf("\nOS is Win (program running in x86) with mem:%lld",(unsigned long long)getTotalSystemMemory());
 return (unsigned long long)getTotalSystemMemory();
//define something for Windows (32-bit)
 //printf("\nOs is Win34 with mem:%lld",(unsigned long long)getTotalSystemMemory());
 //return (unsigned long long)getTotalSystemMemory();
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
        // Unsupported platform
    #endif
#elif __linux
    // linux
	printf("\nOS is Unix with mem:%lld",(unsigned long long)getTotalSystemMemory());
	return (unsigned long long)getTotalSystemMemory();
#endif
	return 0;
}
void MakeDir(char * a)
{
#ifdef _WIN64
	_mkdir(a);
#elif _WIN32
	_mkdir(a);
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
        // Unsupported platform
    #endif
#elif __linux

#endif

}
char * GetSystemSeparators()
{
	#ifdef _WIN64
		return "\\/";
	#elif _WIN32
		return "\\/";
	#elif __APPLE__
		#include "TargetConditionals.h"
		#if TARGET_OS_IPHONE
         // iOS device
		#elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
		#elif TARGET_OS_MAC
        // Other kinds of Mac OS
		#else
        // Unsupported platform
		#endif
	#elif __linux
		return "/";
	#endif
		return "\\/";
}
bool IsIn(char a, char * sep)
{
	for(int i=0;sep[i];++i)
		if(a==sep[i])
			return 1;
	return 0;
}
bool BuildPathToFile(char * path)
{
	char where=0;//not word
	char * separators = GetSystemSeparators();
	char pth[MAX_FILESYS_PATH];
	int k;
	int i;
	for(i=0;path[i];++i)
	{
		if(IsIn(path[i],separators))
		{
			if(where==1)
			{
				for(k=0;k<i;++k)
					pth[k]=path[k];
				pth[k]=0;

				printf("\nMaking:%s",pth);
				MakeDir(pth);
			}
			where=0;
		}
		else
			where=1;
	}
	delete separators;
	return true;
}
/////////////////////THE SOCKET CLASS///////////////////////
#ifdef _WIN64
    #include<windows.h>
    #include<winsock2.h> //WS2_32.lib and IpHlpApi.lib
    bool StartNetworking()
    {
        WSADATA info;
        return !WSAStartup(MAKEWORD(2,0), &info);
    }
    void ShutDownNetworking()
    {
        WSACleanup();
    }
#elif _WIN32
    #include<windows.h>
    #include<winsock2.h> //WS2_32.lib and IpHlpApi.lib
    bool StartNetworking()
    {
        WSADATA info;
        return !WSAStartup(MAKEWORD(2,0), &info);
    }
    void ShutDownNetworking()
    {
        WSACleanup();
    }
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
        //mac os includes
    #else
        // Unsupported platform
    #endif
#elif __linux
    //linux includes
#endif
////////////////////////////////////////////////////////////
