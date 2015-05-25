#include "CoClipConstants.h"
#include<stdio.h>
void ExecuteCommand(char * cmd,unsigned long long barg[],char sarg[][MAX_FIELD_LEN+1]);
void ProcessCommand(char * cmd,unsigned int len);
void AppendToString(char * dest,char * srs);
#ifdef _WIN32
	#define BUFFERSIZE 512
   //define something for Windows (32-bit)
	#include <windows.h>
	HANDLE PipeWait=0;
	HANDLE PipeListen=0;
	HANDLE hPipe;
	HANDLE iPipe;
	DWORD WINAPI WaitForPipe(LPVOID lpParam);
	bool IPCconnected = false;
	bool InitIPC()
	{
		int NoParam2;
		PipeWait = CreateThread(NULL, 0, WaitForPipe, &NoParam2, THREAD_SUSPEND_RESUME, NULL);
		if(!PipeWait)
		{
			printf("\n**##$$##**FATAL ERROR! COULD NOT START IPC THREAD! This program is useless right now. Restart!");
			///////////
			return false;
		}
		return true;
	}//this will start a new thread that will wait keep attepting to link with the pipe
	bool WriteToInterface(char * data)
	{
		if(IPCconnected)
		{
			LPTSTR lpvMessage=TEXT(data);
			DWORD cbToWrite, cbWritten;
			cbToWrite = (lstrlen(lpvMessage)+1)*sizeof(TCHAR);
	    	//printf("\nIPC::Sending %d byte message: %s", cbToWrite, lpvMessage);
			//FlushFileBuffers(hPipe);//flush the pipe to be able to send
			BOOL fSuccess = WriteFile(
					hPipe,                  // pipe handle
					lpvMessage,             // message
					cbToWrite,              // message length
					&cbWritten,             // bytes written
					NULL);                  // not overlapped
			//printf(" ***And it's away!");
			if ( ! fSuccess)
			{
				printf("\nIPC::WriteFile to pipe failed. LAST ERROR=%d", GetLastError());
				if (GetLastError() == ERROR_BROKEN_PIPE)
					printf("\n Warning!::Server Closed Pipe! This may mean that the application crashed!");
				return false;
			}
			return true;
		}
		return false;
	}
	DWORD WINAPI InterfaceListener(LPVOID lpParam)
	{
		DWORD cbRead;
		BOOL fSuccess;
		char chBuf[BUFFERSIZE];
		do
		{
			printf("\nWaitting for new PIPE message!***");

			do
			{
				//i shall try to flush it here

				fSuccess = ReadFile(
				iPipe,    // pipe handle
				chBuf,    // buffer to receive reply
				BUFFERSIZE*sizeof(char),  // size of buffer
				&cbRead,  // number of bytes read
				NULL);    // not overlapped

				if ( ! fSuccess && GetLastError() != ERROR_MORE_DATA )
					break;

				printf("\nIPC_READER:pipe content:%s\n", chBuf );
				ProcessCommand(chBuf,cbRead);
			}
			while ( ! fSuccess);  // repeat loop if ERROR_MORE_DATA

			if ( ! fSuccess)
			{

				printf( TEXT("\nIPC_LISTENER::ReadFile from pipe failed. Last Error=%d\n"), GetLastError());
				//goidn back to waiting for the pipe
				IPCconnected = false;
				return 0;
			}
		}
		while(1);
		return 0;
	}
	DWORD WINAPI WaitForPipe(LPVOID lpParam)
	{
		BOOL   fSuccess = FALSE;
		DWORD  dwMode;
		LPTSTR outPipename = TEXT("\\\\.\\pipe\\CoClipNetworkingToInterface");
		LPTSTR inPipename = TEXT("\\\\.\\pipe\\CoClipInterfaceToNetworking");
		// Try to open a named pipe; wait for it, if necessary.
		while(1)
		{
            while (1)
            {
                hPipe = CreateFile(
                outPipename,   // pipe name
                GENERIC_WRITE,
                0,              // no sharing
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe
                NULL,//attributes
                NULL);          // no template file

                Sleep(20);

                iPipe = CreateFile(
                inPipename,   // pipe name
                GENERIC_READ,
                0,              // no sharing
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe
                NULL,//attributes
                NULL);          // no template file

                // Break if the pipe handle is valid.
                if (hPipe != INVALID_HANDLE_VALUE and iPipe != INVALID_HANDLE_VALUE)
                    break;
                // Exit if an error other than ERROR_PIPE_BUSY occurs.
                if (GetLastError() != ERROR_PIPE_BUSY)
                {
                    //printf("\nIPC::FATAL:: Could not open pipe. Last Error=%d\n", GetLastError() );
                    //return 0;
                }
                // All pipe instances are busy, so wait for 20 seconds.
                if ( ! WaitNamedPipe(outPipename, 1000) )
                {
                    //printf("\nIPC::Could not open pipe: 1 second wait timed out.");
                    //printf("\nIPC::Retrying...");
                    Sleep(1000);
                }
                else
                    break;
        }
        // The pipe connected; change to message-read mode.
        printf("\nIPC:Pipe opened! Setting comm mode!");
        dwMode = PIPE_READMODE_BYTE;
        fSuccess = SetNamedPipeHandleState(
        hPipe,    // pipe handle
        &dwMode,  // new pipe mode
        NULL,     // don't set maximum bytes
        NULL);    // don't set maximum time

        if ( ! fSuccess)
        {
            printf("\nIPC::ERROR:: SetNamedPipeHandleState failed. GLE=%d\n", GetLastError() );
            //return 0;
        }

        printf("\nIPC::DONE::Starting Pipe listener!");
        PipeListen = 0;
        IPCconnected = true;
        InterfaceListener(0);
        printf("\nInPipe closed! Entering wait state...");
        DisconnectNamedPipe(iPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(iPipe);
        CloseHandle(hPipe);
	}
	return 0;
}
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
         // iOS device
    #elif TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_MAC
	#else
        // Unsupported platform
    #endif
#elif __linux

#endif
void AppendToString(char * dest,char * srs)
{
	int i=0;
	int idx=0;
	for(idx=0;dest[idx];++idx);
	for(i=0;srs[i];++i)
		dest[idx++]=srs[i];
	dest[idx]=0;
}
void ProcessCommand(char * cmd,unsigned int len)
{
	//command structure
	//#b:dinary-data#s:string-data#c:command_name -s is for string -b is for number / is used for escaping characters
	//identify commands
	printf("\nIPC_COMMAND length:%d",len);
	char cmdName[MAX_FIELD_LEN+1];
	char sarg[MAX_ARGS][MAX_FIELD_LEN+1];
	unsigned long long barg[MAX_ARGS];
	int nIndex=0;
	int sIndex=0;
	int bIndex=0;
	char type;
	int i=0;
	while(i<len)
    {
        if(cmd[i]!='#')
        {
            printf("\nIPC sintax error at:%d>%s",i,cmd);
            return;
        }
        type = cmd[i+1];
        nIndex = 0;
        for( i=i+2 ; i<len && i<MAX_FIELD_LEN && cmd[i]!='#'; ++i )
        {
            cmdName[nIndex++]=cmd[i];
            if(nIndex>MAX_FIELD_LEN-1)
                nIndex = MAX_FIELD_LEN-1;
        }
        cmdName[nIndex]=0;
        if(type=='s')
        {
            int j;
            for(j=0;j<nIndex;++j)
                sarg[sIndex][j]=cmdName[j];
            sarg[sIndex++][j]=0;
            if(sIndex>MAX_ARGS-1)
                sIndex = MAX_ARGS-1;
        }
        if(type=='b')
        {
            barg[bIndex++] = UnSerialize(cmdName);
            if(bIndex > MAX_ARGS-1)
                bIndex = MAX_ARGS-1;
        }
        if(type=='c')
        {
            ExecuteCommand(cmdName,barg,sarg);
            sIndex=0;
            bIndex=0;
        }
        if( i >= MAX_FIELD_LEN && cmd[i]!='#' )
            i++;
    }
}
void ExecuteCommand(char *cmd,unsigned long long barg[],char sarg[][MAX_FIELD_LEN+1])
{
    printf("\nIPC::Received Command:%s",cmd);
	if( !strcmp(cmd, "exit") )
		TimeToExit = true;//causing program to exit

	if( !strcmp(cmd, "coff") )
	{
	    //filename, towho
		OfferFile(sarg[0],barg[0]);
	}
	if( !strcmp(cmd, "foff") )
	{
	    printf("\nLoading offer form:%s",sarg[0]);
        OfferFilesToUsers(sarg[0]);
	}
	if( !strcmp(cmd,"faccept"))
	{
	    //faccept: from_who tid
		CoClipAcceptOrDenyTask(barg[0],barg[1],true,sarg[0]);
	}
	if( !strcmp(cmd,"fdeny"))
	{
	    //faccept: from_who tid
		CoClipAcceptOrDenyTask(barg[0],barg[1],false,"");
	}
	if( !strcmp(cmd,"scancel"))
	{
		CoClipCancelSend(barg[0]);
	}
	if( !strcmp(cmd,"rcancel"))
	{
		//from-who, tid
		CoClipCancelRecv(barg[0],barg[1]);
	}
    if( !strcmp(cmd,"avatar"))
	{
		//request avatar
		RequestAvatar(barg[0]);
	}
}
