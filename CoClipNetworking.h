//stdlib
#include <stdio.h>
#include<conio.h>
#include<map>
//OS-dependent LIB
#ifdef _WIN64
    #include<windows.h>
    #include<winsock2.h> //WS2_32.lib and IpHlpApi.lib

#elif _WIN32
    #include<windows.h>
    #include<winsock2.h> //WS2_32.lib and IpHlpApi.lib

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
//include whatever necessary to access the SOCKET structures
#include "CoClipConstants.h"
//BOOST crossplatform power
#include <boost/thread/thread.hpp>
//CoClip custom
#include "CoClipConstants.h"
//FLAGS
int TimeToExit = 0;
//STRUCTURES
struct my_scmp
{
    bool operator() (char * const a, char * const b)
    {
        return strcmp(a,b)<0;
    }
};
struct AdapterAddresses
{
unsigned int IP;
unsigned int Bcast;
unsigned int Gateway;//will have a special role!
unsigned int NetMask;
};
int adapterIDX;
AdapterAddresses NetExits[MAX_NET_ADAPTERS];

struct PKT
{
	sockaddr_in From;
	char source;//0 UDP/ 1 TCP/ 2 WWW TCP/ 3... for future use
	char packet[MAX_MTU];
};

class CyclicDeque
{
private:
int usefulLength;
int Front;
int Back;
int Length;
PKT * Que;
public:
	CyclicDeque(int len)
	{
		Que=new PKT[len];
		Front=0;
		Back=0;
		Length=len;
		usefulLength=0;
	}
	~CyclicDeque()
	{
		delete[] Que;
	}
	int length()
	{
		return usefulLength;
	}
	void add(PKT &item,char source)
	{
		for( int i=0 ; i<UDP_PACKET_SIZE ; ++i )
			Que[Back].packet[i]=item.packet[i];
		Que[Back].From = item.From;
		Que[Back].source = source;
		//if(item.FromConn)
		//	Que[Back].FromConn = item.FromConn;

		usefulLength++;
		Back++;
		if(Back==Length)
			Back=0;

		if(usefulLength>Length)
		{
			//printf("\n#@%#$%@$ PACKET Q EXCEEDED LIMMITS! PACKET LOSS!");
			pop();
		}
	}
	void pop()
	{
		if(usefulLength)
		{
			usefulLength--;
			Front++;
			if(Front==Length)
				Front=0;
		}
	}

	PKT GetFirst()
	{
		if(usefulLength)
			return Que[Front];
		return Que[0];
	}
};
CyclicDeque PacketQ(10000);
CyclicDeque SpeedQ(50000);
//utility functions
unsigned int INET_CHARtoH(char * ipv4str);
char * BinToStr(unsigned long long no,int nrb);
unsigned long long StrToBin(char * no,int from,int to);
//CORE LOCKS
boost::mutex UserLock;
boost::mutex SendLock;
boost::mutex RecvLock;
boost::mutex  SIDLock; //put before the include CoClipDataStructures in case locks will be implemented in CoClipDataStructures
//for performance issues
//CORE DATA STRUCTURES
#include "PlatformSpecifics.h"
#include "CoClipDataStructures.h" // Data structures
///////////CORE DATA STRUCTURES
UserStack  Users;
SendStack ToSend;
RecvStack ToRecv;
//use Global Scope Variables to add stuff to the Recv Que
char RecvKey[RECV_KEY_LENGTH];
RecvStruct RecvData;
//CoClip ACCOUNT INFO
UserStruct MyData;
unsigned long long MyUID;
unsigned int CurrentStreamID = 1; //stream id
char UserName[100]="Unknown";
char NewPacket = false;
//SOCKETS
//UDP socket
SOCKET UDPsoc = socket(AF_INET,SOCK_DGRAM,0);
//DATA
#include "Timestamping.h"
unsigned long long lastOfferFN = 0;
unsigned long long lastOutFN   = 0;
//unsigned int SOC_DATA_SIZE = sizeof(struct sockaddr_in);
//CORE FUNCTIONS
char * Serialize(unsigned long long val);
unsigned long long UnSerialize(char * val);
char CoClipAcceptOrDenyTask(unsigned long long uid,unsigned int tid,bool accept,char * clcp);
char CoClipCancelSend(unsigned int tid);
char CoClipCancelRecv(unsigned long long fromwho,unsigned int tid);
char OfferFilesToUsers(char * files);

map<char *,unsigned int,my_scmp> FiletoStreamID;
map<unsigned long long,UserStruct> ToEstablish;
map<unsigned long long,UserStruct> ToPunch;
map<unsigned long long,sockaddr_in> ToMaintain;
map<unsigned long long,unsigned long long>MaintainTimeStamps;

void FreeStreamIDName(char * name);
char OfferFile(char * FileName , unsigned long long ToWho);
void OfferAvatar(unsigned long long ToWho);
char RequestAvatar(unsigned long long FromWho);
//Packet Queue Structure
void SetupMyData();
char * GetNewOfferFileName();
char * GetNewOutFileName();
bool UDP_Send(char * data,sockaddr_in & To);
bool GeneralSend(unsigned long long To,char * data,int datalen,bool Reliable,unsigned short pktype,unsigned int sid);
void DataPacketHandler(PACKET & recvd);
void  UDPhandler(int lpParam);
void  SpeedHandler(int lpParam);//this may be useless now
void  UDPServer(int lpParam);
void  GeneralSender(int lpParam);
void  EstablishChannel(int lpParam);
void  Commander(int lpParam);
void GatherInfo();
void PushToQ(PKT &item, char source);
//#include "LanMulticastDiscovery.h"
//#include "WWW_TCP.h" //WWW TCP
//#include "P2P_TCP.h"  //peer to peer tcp

//to delete
unsigned long long rbenchmark;
unsigned long long sbenchmark;
void MarkStart(unsigned long long & mark){mark=GetSysTime();}
void ShowTimeElapsed(unsigned long long st)
{
	st = GetSysTime()-st;
	printf("\n Transmission Took: %lld.H %lld.M %lld.S %lld.ms TotalMS:%lld",st/1000/60/60,st/1000/60%60,st/1000%60,st%1000,st);
}
void worker(int pr)
{
    while(1)
    {
        printf("Worker here...");
    }
}
#include "WWW_TCP.h" //WWW TCP
#include "P2P_TCP.h"  //peer to peer tcp
#include "IPC_Signaling.h"
void CoClipNetworking()
{
    if(!StartNetworking())
    {
        printf("\nCould Not Start OS NETWORKING...");
        return;
    }

    printf("\nSuccesfully sterted Networking");//this is that scary part where it asks you for an id because
    printf("\nID:");
    scanf("%lld",&MyUID);
    if(!InitIPC())
    {
        printf("\nFATAL ERROR:: Could not start the IPC");
        return;
    }
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(UDP_PORT);
    UDPsoc = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
    char * OptVal = new char;
    *OptVal=1;
    setsockopt(UDPsoc,SOL_SOCKET,SO_BROADCAST,OptVal,sizeof(OptVal));
    *OptVal=15000;
    setsockopt(UDPsoc,SOL_SOCKET,SO_RCVBUF,OptVal,sizeof(OptVal)); //enlarge the receiver buffer up to 15000 bytes
    delete OptVal;

    if (UDPsoc == INVALID_SOCKET)
    {
        printf("\nInvalid SOCKET!");
        return;
    }

    if (bind(UDPsoc, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        closesocket(UDPsoc);
        printf("\nBIND_ERROR");
        return;
    }
    //gather info about pc
    printf("\nSocket running!");
    printf("\nStargint TCP connection with server...");

    SetupMyData();
    GatherInfo();
    //initialising WWW TCP connection and P2P TCP Server
    //WWW_Tcp_INIT();
    P2P_Tcp_Start();

    //WWW_Tcp_AddServer("86.135.206.208");
    //WWW_Tcp_AddServer("192.168.1.67");
    //WWW_Tcp_AddServer("192.168.1.2");
    //WWW_Tcp_AddServer("192.168.1.66");
    //WWW_Tcp_Start();

    //Testbed
    //RunCoClipTests();

    //initialising Inter Process Comm
    //InitMulticastDiscovery();
    boost::thread UDPsrvr(UDPServer,0);
    boost::thread UDPhndl(UDPhandler,0);
    boost::thread GenSndr(GeneralSender,0);
    boost::thread Cmndr(Commander,0);
    //boost::thread deep_thought_2(EstablishChannel,0);

    unsigned long long now;
    unsigned long long pfin;
    unsigned long long rnow;
    PACKET netHail;
    PACKET StillOn;
    sockaddr_in Bcast;
    Bcast.sin_family=AF_INET;/////DAMN YOU MICROSOFT DAMN YOU!
    Bcast.sin_port = htons(UDP_PORT);

    netHail.PackType = PACKET_WHO_ONLINE;
    netHail.SendID=0;
    netHail.DestID=0;
    netHail.StreamID=0;
    netHail.PayloadSize=2;
    netHail.NewData("? ",2);
    netHail.pack();

    sockaddr_in sender_id;
    unsigned long long TD;
    int i;
    map<unsigned long long,UserStruct>::iterator iter;
    while(1)
    {
        now = GetSysTime();

        MyData.LastActive = GetSysTime();

        for(i=0;i<adapterIDX;++i)//network hailing
            if(NetExits[i].IP)
            {
                Bcast.sin_addr.s_addr = NetExits[i].Bcast;
                UDP_Send(netHail.data,Bcast);
            }
        //WWW_Tcp_Send(netHail.data);
        //USER AGING
        //check if users have timed out and hail or delete them
        //UserLock.LOCK();
        /*for(iter = Users.UserQ.begin();iter!=Users.UserQ.end();iter++)
        {
            if(iter->second.UProto != PROTO_WWW_TCP && iter->second.UProto != PROTO_ESTABLISH )
            {
                rnow = GetSysTime();
                TD = GetTimestampDifference(rnow,iter->second.LastActive);
                //printf("\nTime difference for user:%lld is %u",iter->first,TD);
                if(TD>=USER_IDLE_TIMEOUT && TD<USER_TIMEOUT)//send out a u online hail
                {
                    //send through UDP
                    GeneralSend(iter->first,"UOK?",4,false,PACKET_STILL_ALIVE,0);
                }
                if(TD>=USER_TIMEOUT)
                {
                    printf("\nUSER HAS TIMED OUT:%lld",iter->first);
                    unsigned long long ID=iter->first;
                    UserLock.LOCK();
                    Users.PopUser(ID);
                    UserLock.unLOCK();
                    //delete all receiver tasks for this user
                    char offid[RECV_KEY_LENGTH];
                    CreateKey(offid,ID,0);
                    std::map<char * ,RecvStruct *,my_scmp>::iterator itr;
                    for(itr = ToRecv.RecvQ.begin();itr!=ToRecv.RecvQ.end();++itr)
                        if(!strncmp(offid,itr->first,19))//first 19 chars are for the user id
                        {
                            RecvLock.LOCK();
                            ToRecv.EndTask(itr->first,itr->second,false);
                            RecvLock.unLOCK();
                        }
                    iter = Users.UserQ.begin(); //needs reiterating after delete
                }
            }
        }*/
        //UserLock.unLOCK();
        pfin = GetSysTime();
        //if(pfin-now>0)
            //Sleep(NETWORK_HAIL_INTERVAL-(pfin-now));
        Sleep(NETWORK_HAIL_INTERVAL);
        if(TimeToExit)
            break;
    }
	//do not abruptly end the threads... ask them to exit and then exit main thread in stead
	closesocket(UDPsoc);
    ShutDownNetworking();
    return;
}
unsigned int INET_CHARtoH(char * ipv4str)
{
	unsigned int result=0;
	int IP_FIELD_VARS[4]={0,0,0,0};
	int nr=0,nodg=0;
	//check format first and extract values
	for(int i=0;ipv4str[i];++i)
	{
		if(ipv4str[i]>='0' && ipv4str[i]<='9')//this is a digit
		{
			IP_FIELD_VARS[nr]=IP_FIELD_VARS[nr]*10+(ipv4str[i]-'0');
			nodg++;
			if(nodg>3)//more than 3 digits are present in one field
				return 0;
		}
		else
		{
			nodg=0;
			if(ipv4str[i]!='.')
				return 0;
			else
			{
				if(nr>2)//more than 3 points are present
					return 0;
				nr++;
			}
		}
	}
	//forming the 32bit IP
	result=IP_FIELD_VARS[0];
	for(int i=1;i<4;++i)
	{
		result<<=8;
		result|=IP_FIELD_VARS[i];
	}
	return result;
}
char * BinToStr(unsigned long long no,int nrb)
{
	char * rez = new char[nrb];
	nrb--;
	while(nrb>-1)
	{
		rez[nrb--]=(char)(no&0x00000000000000ff);
		no>>=8;
	}
	return rez;
}
unsigned long long StrToBin(char * no,int from,int to)
{
	unsigned long long bff=0;
	for(int i=from;i<to;++i)
	{
		bff<<=8;
		bff|=((int)(no[i])&0xff);
	}
	return bff;
}
void FreeStreamIDName(char * name)
{
	if(name)
	{
		printf("\n***Freeing StreamID table of:%s",name);
		SIDLock.lock();
		FiletoStreamID.erase(name);//.DataSource.DataSource will contain a file name
		SIDLock.unlock();
		delete[] name;
	}
	else
		printf("\n***NOTHING TO FREE FROM!");
}
void PushToQ(PKT &item, char source)
{
	unsigned short pkt = StrToBin(item.packet,16,18);
	if( pkt == PACKET_STREAM_OFFER || pkt == PACKET_STREAM_DATA || pkt == PACKET_STREAM_ACK || pkt == PACKET_TCP_RETRANSMIT || pkt == PACKET_AVATAR_REQUEST)
	{
		//SpeedQ.add(item,source);
        //DataPacketHandler(item);
	}
	else
	{
		PacketQ.add(item,source);
	}
}
unsigned int RequestStreamID(char * FileName)
{
	/*unsigned int noloops = 0xffffffff;
	while(noloops>0)
	{
		if(FileName)
			if(ToSend.SendQ.find(CurrentStreamID)==ToSend.SendQ.end())
				break;
		noloops--;
		CurrentStreamID++;
		if(CurrentStreamID>MAX_STREAM_ID)
			CurrentStreamID=1;
	}
	printf("\nNR_LOOPS passed:%u",noloops);
	*/
	++CurrentStreamID;
	if(ToSend.SendQ.find(CurrentStreamID)==ToSend.SendQ.end())
	{
		return CurrentStreamID;
	}

	return 0;
}
char AssignStreamID(char * FileName,unsigned int & streamID,unsigned short pktype)
{
	char result = 1;
	//SIDLock.LOCK();
	printf("\nDebug::Checking if file has a StreamID...%s::%d",FileName,FiletoStreamID.find(FileName)!=FiletoStreamID.end());
	if(FiletoStreamID.find(FileName) == FiletoStreamID.end())
	{
		printf("\nDebug::Requesting StreamID...");
		streamID = RequestStreamID(FileName);
		printf("\nDebug::Obtained id:%u",streamID);
		if(!streamID)
		{
			printf("\nERROR: Could not assign unique StreamID to task:%s",FileName);
			//delete file
			//SIDLock.unLOCK();
			return -2;
		}
		char * kbfr= new char[MAX_FILESYS_PATH];
		strcpy(kbfr,FileName);
		FiletoStreamID[kbfr]=streamID;
		//successful lookup
		printf("\nDebug::Creating Send Task for File:%s ::TTYPE:%d",FileName,pktype);
		result = ToSend.CreateTask(streamID,FileName,pktype,true);//add the new task to the stack

		if(result<1)
		{
			printf("\nCould not create a send task!");
			//delete the file
			//SIDLock.unLOCK();
			return result;
		}
	}
	else
	{
		streamID=FiletoStreamID[FileName];
		printf("\nFound id:%u",streamID);
	}
	printf("\nDebug::Task was created! Moving on...");
	//successful creation of task
	//SIDLock.unLOCK();
	return 1;
}
char OfferFilesToUsers(char * fileList)
{
	//Step 1 attempt to obtain StreamIDs for every file in the task
	//if 1 file fails to get an id the whole job fails and the user is notified

	char result=1;
	char FileType=DATA_FILE;
	char buffer[MAX_FILESYS_PATH];//32000 maximum path lengt for NTFS
	char * IdFileLst=GetNewOutFileName();
	unsigned int streamID = 0;
	int nrusers=1;
	int cnrusers;
	unsigned long long * ToWho = 0;
	char InSection = 0;

	FILE * FD = fopen(fileList,"r");
	FILE * OD = fopen(IdFileLst,"w");
	if(!FD)//could not open list of files to send!
	{
		printf("\n***No such file!");
		fclose(OD);
		fclose(FD);
		delete[] IdFileLst;
		return -1;
	}
	while(1)
	{
		if( EOF!=fscanf(FD,"%s",buffer) )
		{
			if(InSection==2)
				InSection=3;
			if(!strcmp(buffer,"<files>"))
			{
				printf("\nMoving on to the offered files...");
				InSection=2;
			}
			if(InSection == 4)
			{
				int i=0;
				streamID = 0;
				//extracting file type
				//FileType = buffer[0];
				//for(i=0;buffer[i];++i)
				//	buffer[i]=buffer[i+1];
				//now checking if this file is currently being streamed
				printf("\n***Asigning stream id for file:%s",buffer);
				result = AssignStreamID(buffer,streamID,PACKET_STREAM_DATA);
				if(result<1)
				{
					printf("\nCould not create task %s %d",buffer,result);
					fclose(OD);
					delete[] IdFileLst;
					return result;
				}
				//adding user to send task
				printf("\n Adding users to Stream list");
				for(i=0;i<nrusers;++i)
				{
					printf("\n TaskID:%d Sending to %d",streamID,ToWho[i]);
					result = ToSend.AddUserToTask(streamID,ToWho[i]);

					printf("\nUSER:%lld SUCCESS:%d",ToWho[i],result);
				}
				char * id = BinToStr(streamID,4);
				for(int i=0;i<4;++i)
					fprintf(OD,"%c",id[i]);
				delete[] id;
				fprintf(OD,"%c",FileType);//file type ( 1 DATA_FILE 2 CLIPBOARD_FILE
				fprintf(OD,"%s",buffer);
				fprintf(OD,"%c",0);
			}
			else
			{
				if(InSection==1)
				{
					ToWho[cnrusers-1]=UnSerialize(buffer);
					printf("\n%lld",ToWho[cnrusers-1]);
					cnrusers--;
				}
				if(!InSection)
				{
					nrusers = UnSerialize(buffer);
					ToWho = new unsigned long long [nrusers+2];
					InSection=1;
					cnrusers = nrusers;
					printf("\n Number of receivers for this task:%d\nReceivers:",nrusers);
				}
			}
			if(InSection == 3)
            {
                //also signal the longest common path
                printf("\nLongest Common Path:%s",buffer);
                fprintf(OD,"%c%c%c%c%c",0,0,0,0,1);//file type ( 1 DATA_FILE 2 CLIPBOARD_FILE
				if(buffer[0])
                    fprintf(OD,"%s",buffer);
				fprintf(OD,"%c",0);
                InSection = 4;
            }
		}
		else
			break;
	}

	for(int i=0;i<MAX_MTU;++i)//padding
		fprintf(OD,"%c",0);

	fclose(OD);
	fclose(FD);
	///offer file to users
	//now checking if this file has is currently being streamed
	result = AssignStreamID(IdFileLst,streamID,PACKET_STREAM_OFFER);
	if(result>0)
	{
		//adding user to send task
		printf("\n Adding user to Stream list (for offer file)");
		int i=0;
		for(i=0;i<nrusers;++i)
		{
			result = ToSend.AddUserToTask(streamID,ToWho[i]);
			ToSend.UserAcceptedTask(streamID,ToWho[i]);
			printf("\nUSER:%lld SUCCESS:%d",ToWho[i],result);
		}
		//unpause the sender
		ToSend.SendQ[streamID]->first();
		KList * ptr = ToSend.SendQ[streamID]->next();

		while(ptr)
		{
			printf("\nUser to recive the offer:%lld",ptr->UID);
			ptr=ptr->next;
		}

		printf("\nWaking sender thread!");
		//ResumeThread(GeneralSndr);

		delete[] IdFileLst;
		printf("\n Deleted IdFileList");
		if(ToWho)
			delete[] ToWho;
		printf("\n Everything is done!");
		return result;
	}
	else
	{
		printf("\nFailed to send file offer!");
		delete[] IdFileLst;
		return result;
	}
}
char OfferFile(char * FileName , unsigned long long ToWho)
{
    printf("Creating offer descriptor");
	char * outF = "FileList.txt";
	FILE * FD = fopen(outF,"w");
	char * btstr = Serialize(1);
	fprintf(FD,"%s\n",btstr);
	delete[] btstr;
	btstr = Serialize(ToWho);
	fprintf(FD,"%s\n",btstr);
	delete[] btstr;

	fprintf(FD,"<files>\n");
	fprintf(FD,"%c\n",0);
	//fprintf(FD,"%c",DATA_FILE);//specify that the offered data is a file
	fprintf(FD,"%s",FileName);
	fclose(FD);
	OfferFilesToUsers(outF);
	delete[] outF;
	return true;
}
void OfferAvatar(unsigned long long ToWho)
{//warning: INSPECT THIS FUNCTION
// if avatar is requested reliably, multiple requests come one after another
// both general sender and the corresponding on this machine and on the receiver freeze
// the receiver on the other macine freezes
    char fname[15]="avatar/0.png";
    FILE *in = fopen(fname,"rb");
    printf("\nCreating avatar offer file...");
    if(in)
    {
        fclose(in);
        printf("\n Avatar Picture Exists");
        //now offer the file
        printf("\n Creating stream for it");
        unsigned int streamID;
        int result = AssignStreamID(fname,streamID,PACKET_AVATAR_OFFER);
        if(result>0)
        {
            //adding user to send task
            printf("\n Adding user to Stream list (for avatar file)");
            result = ToSend.AddUserToTask(streamID,ToWho);
            ToSend.UserAcceptedTask(streamID,ToWho);
            printf("\nUSER:%lld SUCCESS:%d",ToWho,result);
            //unpause the sender
            ToSend.SendQ[streamID]->first();
        }
    }
}
char RequestAvatar(unsigned long long FromWho)
{
    char data[5] = "....";
    return GeneralSend(FromWho,data,4,false,PACKET_AVATAR_REQUEST,0);
}
bool GeneralSend(unsigned long long To,char * data,int datalen,bool Reliable,unsigned short pktype,unsigned int sid)
{
	//printf("\nInvoked General Sender");
	if(Users.UserQ.find(To) != Users.UserQ.end())
	{
		UserStruct bfr = Users.UserQ[To];
		if(Reliable)
		{
			int streamID;
			char result;
			//create emtry in the To-Send Stack for recording when ACKs come in
			streamID = RequestStreamID(0);
			printf("\nGeneral Send creating Reliable Stream...StreamID:%d",streamID);
			if(streamID)
			{
				result = ToSend.CreateTask(streamID,0,pktype,false);
				//load data to task
				if(result>0)
				{
					printf("\n      Created task:%d",pktype);
					SendLock.lock();
					ToSend.SendQ[streamID]->DataSource.OpenReader(0,false);//open writer
					ToSend.SendQ[streamID]->DataSource.Preload(data,datalen);//load data to writer memory
					SendLock.unlock();

					result = ToSend.AddUserToTask(streamID,To);//add user to task
					if(result>0)
					{
						printf("\n         Accepted task + start data transfer...");
						ToSend.UserAcceptedTask(streamID,To);//auto accept task
						return true;
					}
					//else
						//delete task
					//SendLock.unLOCK();
				}
			}
		}
		//printf("\nGeneral Sender fallen to UDP");
		//if i could not send reliably try through UDP
		PACKET pout;
		pout.PackType = pktype;
		pout.SendID   = MyUID;
		pout.DestID   = To;
		pout.StreamID = sid;
		pout.SeqNo    = 1;
		pout.TotalSeq = 1;
		pout.PayloadSize = datalen;
		pout.NewData(data,datalen);
		pout.pack();
		sockaddr_in out;
		memset(&out, 0, sizeof(out));
		out.sin_family = AF_INET;
		if(bfr.UProto == PROTO_WWW_TCP)
		{
			//printf("\nGeneral Send using WWW TCP");
			return WWW_Tcp_Send(pout.data);
		}
		if(bfr.UProto == PROTO_LAN_TCP)
		{
			//printf("\nGeneral Send using WWW TCP");
			return P2P_Tcp_Send(pout);
		}
	}
	return 0;
}
bool UDP_Send(char * data,sockaddr_in & To)//used for network discovery and possibly sound and video
{
		if (sendto(UDPsoc, data, UDP_PACKET_SIZE, 0, (struct sockaddr*) &To, sizeof(sockaddr)) == SOCKET_ERROR)
		{
			printf("\nFailed to send UDP packet ERR:%d",GetLastError());
			return false;
		}
	return true;
}
void  EstablishChannel(int lpParam)
{
	PACKET out;
	sockaddr_in To;
	To.sin_family = AF_INET;
	bool idx = true;

	while(1)
	{

		idx=false;

		//UserLock.LOCK();
		std::map<unsigned long long ,UserStruct>::iterator iter;
		for (iter = Users.UserQ.begin(); iter != Users.UserQ.end(); iter++)
		{
			if(iter->second.UProto == PROTO_ESTABLISH)
			{
				if(GetTimestampDifference(GetSysTime(),iter->second.LastActive)>CHANNEL_ESTABLISH_TIME)
				{
					UserLock.lock();
					iter->second.UProto = PROTO_WWW_TCP;
					UserLock.unlock();
				}
				else
				{
					//establishing channel for each users
					out.EncodeMeOnline(MyData,iter->first,MyUID);
					printf("Establishing channel for user:%lld",iter->first);
					//try Local UDP for 1 second

					To.sin_addr.s_addr = iter->second.U_IP;
					To.sin_port = UDP_PORT;
					printf("\n***Trying UDP as channel IP:%u PRT:%d",To.sin_addr.s_addr,To.sin_port);
					out.pack();
					UDP_Send(out.data,To);
					idx=true;
				}
			}
		}
		Sleep(CHANNEL_HAIL_INTERVAL);

		if(!idx)
			Sleep(THREAD_IDLE_WAIT);
	}
}
//to delete
int lsn=0;
int nodisc=0;
int nopip=0;
//
unsigned long long st;
unsigned long long _end;
void DataPacketHandler(PACKET & recvd)
{
	unsigned long long gret=0;
	if(recvd.DestID == MyUID)//packet for me
	{
		if(recvd.PackType == PACKET_TCP_RETRANSMIT)//receive ACKs
		{
			ToSend.ResetSenderPos(recvd.StreamID,recvd.SendID,StrToBin(recvd.data,0,4));
			goto HEND;
		}
		if(recvd.PackType == PACKET_STREAM_ACK)//receive ACKs
		{
			if(recvd.StreamID)
			{
				ToSend.AckPacket(recvd.StreamID,recvd.SendID,StrToBin(recvd.data,0,4),StrToBin(recvd.data,4,5));
				//                 Task ID          SENDER ID                SEQUENCE BEING ACK-ed                     NR OF AVAILABLE TCP CONNECTIONS
				//printf("\n Stream ACK:From %d Sn:%d SID:%d",recvd.SendID,StrToBin(recvd.data,HDR_SIZE,4+HDR_SIZE),recvd.StreamID);
			}
			goto HEND;
		}
		if(recvd.PackType == PACKET_STREAM_DATA)
		{
			//printf("\nNew Data Packet:SN:%d TS:%d",recvd.SeqNo,recvd.TotalSeq);
			char finished[MAX_FILESYS_PATH+5];
			finished[0]=0;
			gret = ToRecv.UpdateTask( recvd , finished );//push the packet to the stack
			if( gret < recvd.SeqNo && gret )
			{
				char * dta = BinToStr(gret,5);
				GeneralSend(recvd.SendID,dta,5,false,PACKET_TCP_RETRANSMIT,recvd.StreamID);
				delete[] dta;
			}
			if(recvd.SeqNo == 1)
				MarkStart(rbenchmark);
			 if(finished[0])
             {
                ShowTimeElapsed(rbenchmark);
                char cmd[100];
                cmd[0]=0;
                char * id = Serialize(recvd.SendID);
                char * sid = Serialize(recvd.StreamID);
                AppendToString(cmd,"#b");
                AppendToString(cmd,id);
                AppendToString(cmd,"#b");
                AppendToString(cmd,sid);
                AppendToString(cmd,"#fdone");
                WriteToInterface(cmd);
                printf("Notifying interface:%s",cmd);
                delete[] id;
                delete[] sid;
            }
			goto HEND;
		}
		if(recvd.PackType == PACKET_AVATAR_OFFER)
        {
            char ret=1;
			gret  = 0;
			//printf("\nNew Offer Packet:SN:%d TS:%d",recvd.SeqNo,recvd.TotalSeq);
			if(recvd.SeqNo == 1)
			{
				printf("\n***GeneralReceiver::Received Avatar Offer");
				ret = 1;
				char key[RECV_KEY_LENGTH];
				CreateKey(key,recvd.SendID,recvd.StreamID);
				if(!key[0])
					ret = 0;
				if(ToRecv.RecvQ.find(key)!=ToRecv.RecvQ.end())//task already exists
				{
					printf("\nPACK_HNDL:THIS OFFER ALREADY EXISTS!:%s",key);
					ret = 0;
				}
				//delete[] key;
				if(ret)
				{
				    char fnm[40];
				    fnm[0]=0;
				    char * id =  Serialize(recvd.SendID);
                    AppendToString(fnm,"avatar/");
                    AppendToString(fnm,id);
                    AppendToString(fnm,".png");
                    delete[] id;
                    ret = ToRecv.CreateTask(recvd.SendID,recvd.StreamID,fnm,true,PACKET_AVATAR_OFFER,"");
				}
				if(ret)
                    ret = ToRecv.AcceptTask(recvd.SendID,recvd.StreamID,"");

			}
			if(ret)
			{
			    char finished[MAX_FILESYS_PATH+5];
				finished[0] = 0;
				gret = ToRecv.UpdateTask(recvd,finished);///!!!!!
                //printf("\nFin:%d",finished);
                //announce
                if(finished[0])
                {
                    printf("\nAnnouncing that an avatar is here! on IPC GRET:",gret);
                    char cmd[100];
                    cmd[0]=0;
                    char * id = Serialize(recvd.SendID);
                    AppendToString(cmd,"#b");
                    AppendToString(cmd,id);
                    AppendToString(cmd,"#cavatar");
                    WriteToInterface(cmd);
                    delete[] id;
                }
			}
			goto HEND;
        }
		if(recvd.PackType == PACKET_STREAM_OFFER)/////WARNING THIS MUST BE TREATED CAREFULLY AS IT CAN CAUSE
		{//////////////////////////////////SECURITY BREACHES IF HACKERS DISCOVER HOW TO ENCRYPT PACKAGES AND
		////////////////////////////////// WHAT TO SEND (IT CAN STORE FILES ON THE USER"S MACHINE WITHOUT CONSENT)
		//ask user for acceptance
			char ret=1;
			gret  = 0;
			//printf("\nNew Offer Packet:SN:%d TS:%d",recvd.SeqNo,recvd.TotalSeq);
			if(recvd.SeqNo==1)
			{
				printf("\n***GeneralReceiver::Received Data Stream Offer");
				ret = 1;
				char key[RECV_KEY_LENGTH];
				CreateKey(key,recvd.SendID,recvd.StreamID);
				if(!key[0])
					ret = 0;
				if(ToRecv.RecvQ.find(key)!=ToRecv.RecvQ.end())//task already exists
				{
					printf("\nPACK_HNDL:THIS OFFER ALREADY EXISTS!:%s",key);
					ret = 0;
				}
				//delete[] key;
				if(ret)
				{
				    char * fnm;
                    fnm = GetNewOfferFileName();
					ret = ToRecv.CreateTask(recvd.SendID,recvd.StreamID,fnm,true,PACKET_STREAM_OFFER,"");
					delete[] fnm;
				}
				if(ret)
                    ret = ToRecv.AcceptTask(recvd.SendID,recvd.StreamID,"");

			}
			if(ret)
			{
			    char finished[MAX_FILESYS_PATH+5];
				finished[0]=0;
				gret = ToRecv.UpdateTask(recvd,finished);///!!!!!
                //printf("\nFin:%d",finished);
                //announce
                if(finished[0])
                {
                    printf("\nSending message to IPC");
                    char cmd[100];
                    cmd[0]=0;
                    char * id = Serialize(recvd.SendID);
                    char * sid = Serialize(recvd.StreamID);
                    AppendToString(cmd,"#b");
                    AppendToString(cmd,id);
                    AppendToString(cmd,"#s");
                    AppendToString(cmd,finished);//filename of CoClipIn file
                    AppendToString(cmd,"#cfoff");
                    WriteToInterface(cmd);
                    delete[] id;
                    delete[] sid;
                }
			}
			goto HEND;
		}
		if(recvd.PackType == PACKET_STREAM_ACCEPT)
		{
			char ret = ToSend.UserAcceptedTask(StrToBin(recvd.data,0,4),recvd.SendID);//mark user as accepted to start file transfer
			//printf("\nYAAAAY:Packet stream accept received");
			goto HEND;
		}
		if(recvd.PackType == PACKET_STREAM_DENY)
		{
			printf("\nNooooooo:Packet stream deny received!");
			char * fname;
			char ret=ToSend.PopUserFromTask(StrToBin(recvd.data,0,4),recvd.SendID,fname);//User denied transfer: pop him from stack
			if(ret==2)
				FreeStreamIDName(fname);
			//goto HEND;
		}
		if(recvd.PackType == PACKET_STREAM_SCANCEL)
		{
			ToRecv.CancelTask(recvd.SendID,recvd.StreamID);
		}
		if(recvd.PackType == PACKET_STREAM_RCANCEL)
		{
			char * name;
			char ret = ToSend.PopUserFromTask(recvd.StreamID,recvd.SendID,name);
			if(ret==2)
				FreeStreamIDName(name);
		}
        if(recvd.PackType == PACKET_AVATAR_REQUEST)
            OfferAvatar(recvd.SendID);//user has requested my avatar

		//////ACK-ing
		HEND:
		if( recvd.PackType != PACKET_STREAM_ACK )
		{
			if(recvd.SeqNo == recvd.TotalSeq )//send ACKs //always ack last sequence to be sure it get sent
			{
				//printf("\n***ACK-ing packet:From:%d Strm:%d grt:%d seqno:%d",recvd.SendID,recvd.StreamID,gret,recvd.SeqNo);
				if(gret)
				{
					recvd.PackType = PACKET_STREAM_ACK;
					recvd.DestID = recvd.SendID;
					recvd.SendID = MyUID;
					recvd.SeqNo=1;
					recvd.TotalSeq=1;
					char * dta=BinToStr(gret,5);
					recvd.NewData(dta,5);
					delete[] dta;
					recvd.pack();

					P2P_Tcp_Send(recvd);
				}
				else
				{
					unsigned long long retCode=recvd.SeqNo;
					retCode<<=8;
					retCode|=100;
					char *dta = BinToStr(retCode,5);
					GeneralSend(recvd.SendID,dta,5,false,PACKET_STREAM_ACK,recvd.StreamID);
					delete[] dta;
				}
			}
		}
	}
	else//packet forwarding
	{
		printf("\n Forwarding TCP packet");
		P2P_Tcp_Send(recvd);
	}
}
void ChooseCorrectIP(unsigned int rcvip)
{
	int i;////////
	for(i=0;i<adapterIDX;++i)
		if((rcvip & NetExits[i].NetMask) ==(NetExits[i].IP & NetExits[i].NetMask))
		{
			MyData.U_IP = NetExits[i].IP;
			break;
		}//Reply with the correct IP address for the subnet the hail is coming from
}
void  UDPhandler(int lpParam)//currently leaking memory (very small leak)
{
	unsigned int noabnormal=0;
	unsigned int nodatastr=0;
	int NoPackets=0;
	////to delete above after debug
	int pkt=0;
	PKT first;
	PACKET recvd;
	while(1)
	{
		if(PacketQ.length())
		{
			first=PacketQ.GetFirst();
			recvd.NewPacket(first.packet,UDP_PACKET_SIZE);
			pkt = recvd.PackType;
			//maintain user timestamps
			if(recvd.PayloadSize<=( UDP_PACKET_SIZE - HDR_SIZE ))
			{
				if(recvd.SendID && Users.UserQ.find(recvd.SendID)!=Users.UserQ.end())//USER TIMESTAMP UPDATE
				{
					UserLock.lock();
					Users.UserQ[recvd.SendID].LastActive = GetSysTime();
					UserLock.unlock();
				}
				///////////////UZER DISCOVERY ROUTINES
				if(pkt == PACKET_WHO_ONLINE)
				{
					//printf("WhoOnlineReceived!\n");
					ChooseCorrectIP(first.From.sin_addr.s_addr);
					recvd.EncodeMeOnline(MyData,recvd.SendID,MyUID);
					recvd.pack();
					if(first.source == PROTO_UDP)
					{
						//printf("\nReplying to network hail to:%d",first.From.sin_addr.s_addr);
						UDP_Send(recvd.data,first.From);
						//MCast_Send(recvd.data);
					}
					goto HEND;
				}
				if((pkt == PACKET_ME_ONLINE || pkt == PACKET_ME_ONLINE2) && recvd.SendID!=MyUID)
				{
					UserStruct  newUser = recvd.DecodeMeOnline();
					//printf("\nFrom:%d To:%d ME_ONLINE RECEIVED fromdata:%d",recvd.SendID,recvd.DestID,first.From.sin_addr.s_addr);
					char ret = Users.AddUser(recvd.SendID,newUser, (first.source == PROTO_WWW_TCP) );
					if(ret)
					{
						if(ret == 2)//adding new user
						{
							P2P_Tcp_UserOnline(recvd.SendID);//announce P2P_System
							//Server must be asked for confirmation (this below may even be deleted for efficiency)
							PACKET CredentialEnquiry;
							CredentialEnquiry.SendID      = MyUID;
							CredentialEnquiry.DestID      = 0;
							CredentialEnquiry.PackType    = PACKET_CONTACT_DETAILS;
							CredentialEnquiry.PayloadSize = 8;
							char * dta = BinToStr(recvd.SendID,8);
							CredentialEnquiry.NewData(dta,8);
							delete[] dta;
							CredentialEnquiry.pack();
							WWW_Tcp_Send(CredentialEnquiry.data);
						}
						if( ret > 2)//new packet off WWW_TCP that needs channel establishing
						{
							printf("\nRECALCULATING BEST PATH :%d",ret);
						}
					}
					//if(!ret)
					//	printf("\nUser already exists!");
					char * cmd = new char[250];
					cmd[0]=0;
					AppendToString(cmd,"#b");
					char * serusr = Serialize(recvd.SendID);
					AppendToString(cmd,serusr);
					AppendToString(cmd,"#s");
					AppendToString(cmd,Users.UserQ[recvd.SendID].name);
					AppendToString(cmd,"#cuon");
                    //announce interface
					WriteToInterface(cmd);
					delete[] cmd;
					delete[] serusr;

					if(pkt == PACKET_ME_ONLINE && first.source==PROTO_UDP)
					{
						///////////////////////////////////////////////////////////
						ChooseCorrectIP(first.From.sin_addr.s_addr);
						recvd.EncodeMeOnline(MyData,recvd.SendID,MyUID);
						recvd.PackType=PACKET_ME_ONLINE2;
						recvd.pack();
						//printf("\nReplying to ME_ONLINE with ME_ONLINE2:");
						UDP_Send(recvd.data,first.From);
					}

					goto HEND;
				}
				///////////////////////////////////////////////////////////
				if(pkt == PACKET_BYE)
				{
					printf("\nRECEIVED PACKET BYE");
					if(Users.UserQ.find(recvd.SendID)!=Users.UserQ.end())
					{
						printf("\n***-> %d Went OFFLINE");
						Users.PopUser(recvd.SendID);
					}
					goto HEND;
				}
				if(pkt == PACKET_STILL_ALIVE)
				{
					if(first.source==PROTO_WWW_TCP)
					{
						recvd.PackType=PACKET_ME_ALIVE;
						recvd.SeqNo=1;
						recvd.TotalSeq=1;
						recvd.StreamID=0;
						recvd.DestID=0;
						recvd.SendID=MyUID;
						recvd.PayloadSize=3;
						recvd.NewData("OK!",3);
						recvd.pack();
						WWW_Tcp_Send(recvd.data);
					}
					else
						GeneralSend(recvd.SendID,"meok",4,false,PACKET_ME_ALIVE,0);
					goto HEND;
				}
				HEND:;
			}
			else
				printf("\n*&*&ABNORMAL PACKET %d!PT:%d PL:%d",++noabnormal,recvd.PackType,recvd.PayloadSize);

			NoPackets++;
			PacketQ.pop();
			_end = GetSysTime();
		}
		else
			Sleep(THREAD_IDLE_WAIT);
	}
}
void  UDPServer(int lpParam)// MEM OK
{
	int slen , recv_len;
	printf("\nWaiting for UDP data...");
	PKT data;
	slen = sizeof(data.From);
	while(1)
	{
		//clear the buffer by filling null, it might have previously received data
		memset(data.packet,'\0', UDP_PACKET_SIZE);
		//WARNING! THE EXCESS DATA IS LOST BECAUSE OF THE WAY UDP IS IMPLEMENTED
		//Workaround: A Separate Processing Thread Deals With The Packets
		if ((recv_len = recvfrom(UDPsoc, data.packet,UDP_PACKET_SIZE, 0, (struct sockaddr *) &data.From, &slen)) == SOCKET_ERROR)
			printf("\nReceiver failed with error code : %d" , WSAGetLastError());
		else
			PushToQ(data,PROTO_UDP);
	}
}
void  GeneralSender(int lpParam) //has an infinite loop problem
{
    //memory to fast store data form file;
    char * fname;
    char * data = new char[MAX_MTU+10];
    unsigned short dataLen;
    char ret;

	unsigned long long sbn;
	KList * current;
	PACKET tsdata;
	std::map<unsigned int ,boost::shared_ptr<KeyList>>::iterator iter;
	sockaddr_in To;
	To.sin_family = AF_INET;
	int idx=0;
	int nopk=0;
	//////////////////
	while(1)
	{
		idx = 0;
		for (iter = ToSend.SendQ.begin(); iter != ToSend.SendQ.end(); )
		{
			sbn=GetSysTime();
			//iterate each list of users looking for open trasmissions and keep sending
			auto ptr = iter->second;
			if(!ptr)//bad iterator
			{
				//fprintf(GSNDR,"***BAD POINTER REITERAGING GENERAL SENDER\n");
				printf("\nBAD POINTER REITERAGING GENERAL SENDER");
				idx=1;
				break;
			}
			if(ptr->DataSource.IsFileReady())
			{
				ptr->first();
				current=ptr->next();
				while(current)
				{
					if(Users.UserQ.find(current->UID)!=Users.UserQ.end())//if user is still online
					{
						UserStruct usrdta = Users.UserQ[current->UID];

						if(current->Data.Accepted)
						{
							tsdata.PackType = ptr->PacketType;
							tsdata.SendID   = MyUID;
							tsdata.DestID   = current->UID;
							tsdata.SeqNo    = current->Data.LastSent+1;
							tsdata.TotalSeq = ptr->DataSource.totalSeq;
							tsdata.StreamID = iter->first;
							if(tsdata.SeqNo==1)
							{
								current->Data.FirstSent=GetSysTime();
								MarkStart(sbenchmark);
							}

							ptr->DataSource.get(tsdata.SeqNo,data,dataLen);//add support for reading and fail safe in case file dissapears
							//assembling packet
							tsdata.PayloadSize = dataLen;
							tsdata.NewData(data,dataLen);
							tsdata.pack();
							idx++;
							//printf("\nGSNDR sending sn:%d ts:%d to:%lld",tsdata.SeqNo,tsdata.TotalSeq,tsdata.DestID);
							if(usrdta.UProto == PROTO_LAN_TCP)//Send the packet via TCP network
							{
								P2P_Tcp_Send(tsdata);
							}
							if(usrdta.UProto == PROTO_WWW_TCP)//send on last resort option ( through server )
								WWW_Tcp_Send(tsdata.data);
							if( current->Data.LastSent+1 == tsdata.TotalSeq)//if no retransmit has come in between
							{
								//SendLock.lock();
								current->Data.LastSent = current->Data.LastACK;
								//SendLock.unlock();
							}
							else
							{
								//SendLock.lock();
								current->Data.LastSent = tsdata.SeqNo;//LastSent is updated by sender (because connection is reliable)
								current->Data.LastACK  = tsdata.SeqNo;
								//SendLock.unlock();
							}

							//end of stream  //don't wait for ack for debugging
							if( (tsdata.SeqNo>=tsdata.TotalSeq) && (current->Data.LastACK>=tsdata.TotalSeq) )// if this was the last packet pop it from the Q
							{

								ret = 0;
								printf("\nTraditional popper:%d UPROTO:%d LS:%d LACK:%d",iter->first,usrdta.UProto,current->Data.LastSent+1,current->Data.LastACK);
								ret=ToSend.PopUserFromTask(iter->first,tsdata.DestID,fname);
								printf("\nRET=%d fname:%s",ret,fname);
								if(ret == 2)
								{
									FreeStreamIDName(fname);
									iter = ToSend.SendQ.begin();
									//reiterating
								}
								ShowTimeElapsed(sbenchmark);
								printf("\nAverage Speed:%f MB/s",(float)(tsdata.TotalSeq*(MAX_MTU-HDR_SIZE))/1024/1024/((GetSysTime()-sbenchmark)/1000));
							}//TASK AGING WILL BE DONE BY USER AGING! IF AN USER IS NOT ONLINE ANYMORE THEN THE TASKS DESTINED FOR HIM WILL BE DROPPED
						}
					}
					else   //user went offline while file transfer was in progress
					{
						printf("\n*&*Popping offline user form task:%lld",iter->first);
						idx=1;
						//needs lock
						char ret=ToSend.PopUserFromTask(iter->first,current->UID,fname);
						if(ret==2)
						{
							FreeStreamIDName(fname);
							iter = ToSend.SendQ.begin();
						}
					}
					current=ptr->next();//iterate through all recipients
				}
			}
			else
			{
				idx=1;
				if(ptr->DataSource.LoadFile(5)<0);//file loading error//to do abort task
			}
			//need lock
			iter++;//critical
		}
		if(!idx)
            Sleep(THREAD_IDLE_WAIT);
	}
	delete[] data;
}

void SetupMyData()
{
	strcpy(MyData.email,"test@coclip.com");
	MyData.TCP_Port      = 0;
	MyData.LastTimeStamp = 0;
	MyData.LastActive    = 0;
	MyData.NoAvConn      = P2P_Tcp_GetAvailableInLinks();
	//unsigned short TCP_Port;
}
void GatherInfo()
{
#ifdef _WIN64
    unsigned long long cpy=MyUID;
	MyData.name[0]=0;
	bool ret=GetUIDandLoadAddresses(MyUID,MyData.name);
	if(!ret)
	{
		printf("\nFATAL ERROR: Could not recognize machine!");
		TimeToExit=true;
	}
	printf("\nCOCLIP UNIQUE ID:%lld",MyUID);
	if(MyData.name[0])
		printf("\nLocal Machine Name:%s\n",MyData.name);
	MyUID = cpy;//do this just for now so that we can still chose the coclip id

	//delete this
	//this has to be adapted to a set of strings that will contain all the IPs the other users can see
	struct hostent *phe = gethostbyname(MyData.name);
	for (int i = 0; phe->h_addr_list[i] != 0; ++i)
	{
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
        MyData.U_IP=htonl(INET_CHARtoH(inet_ntoa(addr)));
		printf("Address: %s dec:%u\n",inet_ntoa(addr),MyData.U_IP);
	}
#elif _WIN32
    unsigned long long cpy=MyUID;
	MyData.name[0]=0;
	bool ret=GetUIDandLoadAddresses(MyUID,MyData.name);
	if(!ret)
	{
		printf("\nFATAL ERROR: Could not recognize machine!");
		TimeToExit=true;
	}
	printf("\nCOCLIP UNIQUE ID:%lld",MyUID);
	if(MyData.name[0])
		printf("\nLocal Machine Name:%s\n",MyData.name);
	MyUID = cpy;//do this just for now so that we can still chose the coclip id

	//delete this
	//this has to be adapted to a set of strings that will contain all the IPs the other users can see
	struct hostent *phe = gethostbyname(MyData.name);
	for (int i = 0; phe->h_addr_list[i] != 0; ++i)
	{
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
        MyData.U_IP=htonl(INET_CHARtoH(inet_ntoa(addr)));
		printf("Address: %s dec:%u\n",inet_ntoa(addr),MyData.U_IP);
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
}
char CoClipAcceptOrDenyTask(unsigned long long uid,unsigned int sid,bool accept,char * clcp)
{
	printf("\nAccepting task from %lld ID:%d ?::%d",uid,sid,accept);
	char mold[6];
    char * sidenc = BinToStr(sid,4);
    for(int k=0;k<4;++k)
            mold[k]=sidenc[k];
    delete[] sidenc;
    if(accept)
	{
		if(ToRecv.AcceptTask(uid,sid,clcp))
            return GeneralSend(uid,mold,4,true,PACKET_STREAM_ACCEPT,0);//send accept message reliabely
	}
	else
        return GeneralSend(uid,mold,4,true,PACKET_STREAM_DENY,0);//send deny message reliabely

	printf("Could Not Accept Task!");
	return 0;
}
char CoClipCancelSend(unsigned int tid)
{
	if(ToSend.SendQ.find(tid)!=ToSend.SendQ.end())
	{
		auto ptr = ToSend.SendQ[tid];
		//iterate the users and set the flag
		ptr->first();
		KList * iterator = ptr->next();

		while(iterator)
		{
			//announcing recipients that the task has ended
			GeneralSend(iterator->UID,"X!",2,true,PACKET_STREAM_SCANCEL,tid);
			iterator=iterator->next;
		}
		//ending task in sender stack
		char * fname;
		char ret = ToSend.CancelTask(tid,fname);
		FreeStreamIDName(fname);//free FiletoStreamID id
		printf("\nCanceled sending task:%d",tid);
		return ret;
	}
	return false;
}
char CoClipCancelRecv(unsigned long long fromwho,unsigned int tid)
{
	if(ToRecv.CancelTask(fromwho,tid))
	{//task was deleting
		//announcing sender
		GeneralSend(fromwho,"x!",2,true,PACKET_STREAM_RCANCEL,tid);
		printf("\nCanceled receiving of task:%d form %lld and notified sender!",tid,fromwho);
		return true;
	}
	return false;
}
void Commander(int lpParam)
{
	char cmd[50];
	while(1)
	{
		bool ok=false;
		printf("\nCoClipCMD#:");
		scanf("%s",cmd);
		if(!strcmp(cmd,"help"))
		{
			printf("\nCommands: users,accept,deny,scancel,rcancel,offer,tosend,torecv");
			ok=true;
		}
		if(!strcmp(cmd,"forceexit"))
		{
            //CancelSend=true;
			ok=true;
		}
		if(!strcmp(cmd,"stophail"))
		{
			//nohail=true;
			ok=true;
		}
		if(!strcmp(cmd,"forceudp"))
		{
			//ForceUDP=true;
			ok=true;
		}
		if(!strcmp(cmd,"IPCsend"))
		{
			char msg[100];
			printf("\nCoClipCMD#:What to send:");
			scanf("%s",msg);
			WriteToInterface(msg);
			ok=true;
		}
		if(!strcmp(cmd,"users"))
		{
			printf("\n******** USER LIST");
			int k=0;
			UserLock.lock();
			std::map<unsigned long long ,UserStruct>::iterator iter;
			for (iter = Users.UserQ.begin(); iter != Users.UserQ.end(); iter++)
			{
				printf("\n****%d:ID:%lld data:%s:%s UPROTO:%d TCP_PORT:%d",k,iter->first,iter->second.name,iter->second.email,iter->second.UProto,htons(iter->second.TCP_Port));
				k++;
			}
			UserLock.unlock();
			printf("\n********");
			ok=true;
		}
		if(!strcmp(cmd,"_offer"))
		{
			char fname[100];
			printf("\nFile descriptor for file transfer:");
			scanf("%s",fname);
			printf("\n...Starting file share using file %s",fname);
			OfferFilesToUsers(fname);
			ok=true;
		}
		if(!strcmp(cmd,"offer"))
		{
			char fname[100];
			unsigned long long ho;
			printf("\nOffer What File:");
			scanf("%s",fname);
			printf("\nTo Who:");
			scanf("%lld",&ho);
			printf("\n...Starting file share with %lu",ho);
			OfferFile(fname,ho);
			ok=true;
		}
		if(!strcmp(cmd,"torecv"))
		{
			printf("\n******** RECEIVE TASK LIST");
			int k=0;
			RecvLock.lock();
			std::map<char * ,boost::shared_ptr<RecvStruct>,my_scmp >::iterator iter;
			for (iter = ToRecv.RecvQ.begin(); iter != ToRecv.RecvQ.end(); iter++)
			{
				printf("\n**%d:From:%s Accepted?:%d Completed:%f Speed:%f B/s",k,iter->first,iter->second->Accepted,(iter->second->TotalSeq==0)?iter->second->TotalSeq:((float)iter->second->LastWSeq)/iter->second->TotalSeq*100,(float)iter->second->LastWSeq*(MAX_MTU-HDR_SIZE)/(GetTimestampDifference(GetSysTime(),iter->second->TimeStarts)/1000));
				k++;
			}
			RecvLock.unlock();
			printf("\n********");
			ok=true;
		}
		if(!strcmp(cmd,"tosend"))
		{
			printf("\n******** SENDER TASK LIST");
			int k=0;
			SendLock.lock();
			std::map<unsigned int ,boost::shared_ptr<KeyList>>::iterator iter;
			for (iter = ToSend.SendQ.begin(); iter != ToSend.SendQ.end(); iter++)
			{
				printf("\n****%d:TaskID:%d",k,iter->first);
				auto ptr = iter->second;
				if(ptr)
				{
					ptr->first();
					KList * kk;
					do
					{
						kk = ptr->next();
						if(kk)
							printf("\n****-> %lld(accepted %d) Completed:%f Speed:%f B/s",kk->UID,kk->Data.Accepted,(kk->Data.LastACK==0)?kk->Data.LastACK:((float)kk->Data.LastACK)/ptr->DataSource.totalSeq*100,(float)(kk->Data.LastACK*(MAX_MTU-HDR_SIZE))/(GetTimestampDifference(GetSysTime(),kk->Data.FirstSent)/1000));
                        else
                            break;
                    }
					while(kk);
					k++;
				}
			}
			printf("\n********");
			SendLock.unlock();
			ok=true;
		}
		if(!strcmp(cmd,"taskmap"))
		{
			printf("\n******** TASK MAP");
			int k=0;
			std::map<char * ,unsigned int>::iterator iter;
			for (iter = FiletoStreamID.begin(); iter != FiletoStreamID.end(); iter++)
			{
				printf("\n****%d:Name:%s::%d(StreamID)",k,iter->first,iter->second);
				k++;
			}
			printf("\n********");
			ok=true;
		}
		if(!strcmp(cmd,"accept")) ///WARNING ACCEPT MUST ALSO SEND THE NUMBER OF AVAILABLE TCP CONN SLOTS
		{
			unsigned int sid;
			unsigned long long id;
			char clcp[256] = "";
			printf("\nAccept task from who?(ID):");
			scanf("%lld",&id);
			printf("\nFrom %u accept what stream?(SID):");
			scanf("%d",&sid);
			printf("\nChosen Common Path?");
			char y = getch();
			if( y == 'y')
                scanf("%s",clcp);
			CoClipAcceptOrDenyTask(id,sid,true,clcp);
			//WWW_Tcp_Send(AcceptPacket.pack());
			ok=true;
		}
		if(!strcmp(cmd,"deny"))
		{
			unsigned int sid;
			unsigned long long id;
			printf("\nDeny task from who?(ID):");
			scanf("%lld",&id);
			printf("\nFrom %u deny what stream?(SID):");
			scanf("%d",&sid);
			CoClipAcceptOrDenyTask(id,sid,false,"");
			//WWW_Tcp_Send(AcceptPacket.pack());
			ok=true;
		}
		if(!strcmp(cmd,"scancel"))
		{
			int id;
			printf("What task to cancel?:");
			scanf("%lld",&id);
			//code a proper announcing method for the clients
			printf("\nSUCCESS:%d",CoClipCancelSend(id));
			ok=true;
		}
		if(!strcmp(cmd,"rcancel"))
		{
			int id;
			int tid;
			printf("Cancel task from who?:");
			scanf("%d",&id);
			printf("Cancel what task?:");
			scanf("%d",&tid);
			//code a proper annjouncing method for the sender
			printf("\nSUCCESS:%d",CoClipCancelRecv(id,tid));
			ok=true;
		}
		if(!strcmp(cmd,"ravatar"))
		{
			int id;
			printf("Ask for Avatar from who?:");
			scanf("%d",&id);
			RequestAvatar(id);
		}
		if(!ok)
			printf("CoClipCMD# UNKNOWN COMMAND!***");
	}
}
char * GetNewOfferFileName()
{
	lastOfferFN++;
	char * ret =  new char[45];
	unsigned long long cpy=lastOfferFN;
	char nr=0;
	ret[nr++]='c';
	ret[nr++]='c';
	ret[nr++]='i';
	ret[nr++]='o';
	ret[nr++]='/';
	do
	{
		ret[nr++]=cpy%10+48;
		cpy/=10;
	}
	while(cpy);
	ret[nr++]='.';
	ret[nr++]='C';
	ret[nr++]='o';
	ret[nr++]='C';
	ret[nr++]='l';
	ret[nr++]='i';
	ret[nr++]='p';
	ret[nr++]='I';
	ret[nr++]='n';
	ret[nr]=0;
	printf("\nGenerated file name:%s",ret);
	return ret;
}
char * GetNewOutFileName()
{
	lastOutFN++;
	char * ret =  new char[45];
	unsigned long long cpy=lastOutFN;
	char nr=0;
	ret[nr++]='c';
	ret[nr++]='c';
	ret[nr++]='i';
	ret[nr++]='o';
	ret[nr++]='/';
	do
	{
		ret[nr++]=cpy%10+48;
		cpy/=10;
	}
	while(cpy);
	ret[nr++]='.';
	ret[nr++]='C';
	ret[nr++]='o';
	ret[nr++]='C';
	ret[nr++]='l';
	ret[nr++]='i';
	ret[nr++]='p';
	ret[nr++]='O';
	ret[nr++]='u';
	ret[nr++]='t';
	ret[nr]=0;
	return ret;
}
char * Serialize(unsigned long long val)
{
	char * rez= new char[22];
	char idx=0;
	rez[0]=0;
	unsigned long long cpy = val;
	while(cpy)
	{
		idx++;
		cpy/=10;
	}
	rez[idx--]=0;
	while(val)
	{
		rez[idx--]=val%10+48;
		val/=10;
	}
	return rez;
}
unsigned long long UnSerialize(char * val)
{
	unsigned long long rez=0;
	char idx=0;
	for(idx=0;val[idx];++idx)
		rez = rez*10+(val[idx]-48);

	return rez;
}

