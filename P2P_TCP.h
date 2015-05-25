/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// CoClip TCP MODULE /////////////////////////////////
///////////////////////////// Peer to peer TCP Implementation ///////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////To start it just call P2P_Tcp_Start()///////////////////////

#define MAX_INLINKS           10
#define MAX_OUTLINKS          3
unsigned char NoActiveInlinks  = 0;
struct NetLink{
SOCKET soc;
unsigned long long UID;
};
map<unsigned long long,SOCKET> InLinks;
NetLink OutLinks[MAX_OUTLINKS];
//LockOBJ InLinkLock;
//LockOBJ OutLinkLock;

#include "CoClipConstants.h"
#include <Windows.h>
HANDLE P2P_Tcp_ServerHndl;
SOCKET P2P_Tcp_link;
unsigned short P2P_Port;
///functions
//utility functions
int  P2P_Tcp_GetAvailableInLinks();
void P2P_Tcp_UserOnline(unsigned long long UID);
//core functions
void SetSocOptions(SOCKET & soc);
bool P2P_Tcp_Start();
void P2P_Tcp_GenericListener(SOCKET soc);
void P2P_Tcp_InLinkListener(unsigned long long DestID);
void P2P_Tcp_OutLinkListener(unsigned long long UID);
void P2P_Tcp_Server(int lpParam);
void P2P_Tcp_Listener(int lpParam);

//implementation
inline void SetSocOptions(SOCKET & soc)
{
	char * OptVal = new char;
	//experimental
	//*OptVal=true;
	//setsockopt(soc,SOL_SOCKET,TCP_NODELAY,OptVal,sizeof(OptVal));
	//*OptVal=100;
	//setsockopt(soc,SOL_SOCKET,SO_SNDTIMEO,OptVal,sizeof(OptVal));
	delete OptVal;
	//set socket buffer sizes
	//int OptVali = (MAX_MTU*1);
	//setsockopt(soc,SOL_SOCKET,SO_SNDBUF,(char *)&OptVali,sizeof(OptVali));
	//setsockopt(soc,SOL_SOCKET,SO_RCVBUF,(char *)&OptVali,sizeof(OptVali)); //enlarge the receiver buffer up to 15000 bytes
}
bool P2P_Tcp_Start()
{
	P2P_Tcp_link=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	P2P_Port=80;
	MyData.TCP_Port = htons(P2P_Port);

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = MyData.TCP_Port;
	printf("\nP2P_TCP:: initialising server...(Looking for port)");
	//finding port
	bool ft=true;
	int iResult;

	while(bind(P2P_Tcp_link, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR && MyData.TCP_Port)
	{
		if(ft && MyData.TCP_Port )
		{
			P2P_Port = 0xffff;
			ft=false;
		}
		MyData.TCP_Port = htons(P2P_Port);
		sa.sin_port = MyData.TCP_Port;
		P2P_Port--;
	}
	if(!MyData.TCP_Port)
		return false;

	iResult = listen(P2P_Tcp_link, MAX_INLINKS+3);
    if (iResult == SOCKET_ERROR)
	{
        printf("\nP2P_TCP::listen failed with error: %d\n", WSAGetLastError());
        closesocket(P2P_Tcp_link);
        return false;
    }
	printf("\nP2P_TCP:Establishing P2P server on port:%d htnos:%d %d",P2P_Port,htons(P2P_Port),htons(htons(P2P_Port)));
	//FIND A WAY TO CHECK IF THREAD STARTS
	boost::thread P2P_Tcp_srv(P2P_Tcp_Server,0);
	//if(!P2P_Tcp_srv)
    //{
    //  printf("\nP2P_TCP::Could not start TCP server");
	//	return false;
	//}
	return true;
}
void P2P_Tcp_Server(int lpParam)//problematic?
{
	printf("\nP2P_TCP:Waitting for connections");
	while(1)
	{//and
		// Accept a client socket
		SOCKET CliSoc=INVALID_SOCKET;;
		sockaddr_in From;
		int flen=sizeof(From);
		CliSoc = accept(P2P_Tcp_link, (struct sockaddr*) &From, &flen);
		SetSocOptions(CliSoc);
		if (CliSoc == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			//closesocket(ListenSocket);
		}
		else
		{
			//check who the connection is from
			printf("\nAccepted new InLink! Assessing...");
			unsigned long long FromID=0;
			//UserLock.LOCK();
			map<unsigned long long,UserStruct>::iterator iter;
			for( iter = Users.UserQ.begin(); iter != Users.UserQ.end() ; ++iter )
			{
				printf("\n    User:%lld ip:%lld   inconnection IP:%lld",iter->first,iter->second.U_IP,From.sin_addr.s_addr);
				if(iter->second.U_IP == From.sin_addr.s_addr)//found em
					FromID=iter->first;
			}
			//UserLock.unLOCK();
			//now check if there is no available connection to this person
			printf("\n  Connection is from:%lld",FromID);
			if(InLinks.find(FromID)==InLinks.end())
			{
				//InLinkLock.LOCK();
				InLinks[FromID]=CliSoc;
				//InLinkLock.unLOCK();
				//////////////////////////FIND A WAY TO CHECK IF THREAD STARTS
				boost::thread P2P_ill(P2P_Tcp_InLinkListener,FromID);
                //if(P2P_ill)
                //{
                //    NoActiveInlinks++;
                //    printf("\nP2P_TCP::OPENED NEW CONNECTION! Left slots:%d",MAX_INLINKS-NoActiveInlinks);
                //}
                //else
                //    printf("\nP2P_TCP::Could not start inlinklistener");
			}
			else
				closesocket(CliSoc);//close the connection since one already exists
		}
	}
}
void P2P_Tcp_InLinkListener(unsigned long long DestID)
{
	printf("\nP2P_TCP::A new InLink is here from:%lld",DestID);
	P2P_Tcp_GenericListener(InLinks[DestID]);

	//InLinkLock.LOCK();

	printf("\nP2P_TCP:: An InLink has closed!");
	closesocket(InLinks[DestID]);
	NoActiveInlinks--;
	InLinks.erase(DestID);

	//InLinkLock.unLOCK();
}
void P2P_Tcp_OutLinkListener(unsigned long long UID)
{
	unsigned long long minVal = OutLinks[0].UID;
	int minPos=0;
	int i=0;
	printf("\nP2P_TCP::Sweeping OutLinks to find place to put new conn!");
	for(i=0;i<MAX_OUTLINKS;++i)
	{
		if(OutLinks[i].UID == UID)
		{
			printf("\nUser already exists in connection table");
			return;
		}
		if(OutLinks[i].UID < minVal)
		{
			minPos = i;
			minVal = OutLinks[i].UID;
		}
	}
	//OutLinkLock.LOCK();
	if(	i<MAX_OUTLINKS)
	{
		minPos=i;
		OutLinks[minPos].soc=socket(AF_INET,SOCK_STREAM,0);
		SetSocOptions(OutLinks[minPos].soc);
		OutLinks[minPos].UID=UID;
	}
	//OutLinkLock.unLOCK();
	//case 2 list is full
	if( i == MAX_OUTLINKS )
    {
		if( UID < OutLinks[minPos].UID )
		{
			printf("\nUser exceeds Redundancy depth! No Connection initiated: UID:%lld SelectedUID:%lld",UID,OutLinks[minPos].UID);
			return;
		}
		//OutLinkLock.LOCK();

		closesocket(OutLinks[minPos].soc);
		OutLinks[minPos].soc=socket(AF_INET,SOCK_STREAM,0);
		SetSocOptions(OutLinks[minPos].soc);
		OutLinks[minPos].UID=UID;

		//OutLinkLock.unLOCK();
	}
	//get USER dest IP and Port;
	sockaddr_in sa;
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = Users.UserQ[UID].U_IP;
	sa.sin_port        = Users.UserQ[UID].TCP_Port;
	//connect
	printf("\nP2P_TCP::Connecting to OutLink:%lld",UID);
	printf("\n          Connecting to:%s:%d",inet_ntoa(sa.sin_addr),htons(sa.sin_port));
	if(connect(OutLinks[minPos].soc,(struct sockaddr*) &sa,sizeof(sa)) != SOCKET_ERROR)
	{
		printf("\nP2P_TCP::Starting listener thread for OutLink:%lld",UID);
		P2P_Tcp_GenericListener(OutLinks[minPos].soc);

		//OutLinkLock.LOCK();
		for(i=0;i<MAX_OUTLINKS;++i)
			if(OutLinks[i].UID==UID)
			{
				printf("\nP2P_TCP:: An OutLink has closed!");
				closesocket(OutLinks[i].soc);
				OutLinks[i].UID=0;
				break;
			}
		//OutLinkLock.unLOCK();
	}
	else
		printf("\nP2P_TCP::OutLink Connect failed:%d",GetLastError());
}
int P2P_Tcp_GetAvailableInLinks()
{
	return MAX_INLINKS-NoActiveInlinks;
}
void P2P_Tcp_UserOnline(unsigned long long UID)
{
	if( UID<MyUID && UID!=MyUID )//only outlinks are initiated by a CoClip app
	//if(UID<=MyUID)
	{
		printf("\nA User is online! Initiating connecting if necessary");
		//FIND A WAY TO CHECK IF THREAD STARTS
		boost::thread P2P_Tcp_oll(P2P_Tcp_OutLinkListener,UID);//connect to peer and liste
	}
}
FILE *ReadS=fopen("ReadSpeed.txt","w");
unsigned long long rbn;
void P2P_Tcp_GenericListener(SOCKET soc)
{
	int noFailed=0;
	int MAX_FAIL;

	int last_len=0;
	int recv_len;

	int payload=0;
	PACKET pkt;
	char bfr[MAX_MTU];

	while(1)
	{
		noFailed=0;
		payload=0;
		last_len=0;
		do
		{
			//printf(" R...");
			rbn=GetSysTime();
			if((recv_len=recv(soc,bfr,(payload+HDR_SIZE),MSG_PEEK))==SOCKET_ERROR)
				return;

			//printf(" Peeking at message:%d LL:%d PL:%d TS:%d",recv_len,last_len,payload,(HDR_SIZE+payload));
			if(recv_len==last_len)
				noFailed++;
			if(!payload && recv_len >= HDR_SIZE)
				payload = StrToBin(bfr,HDR_SIZE-2,HDR_SIZE);
			last_len=recv_len;
		}
		while(recv_len<(HDR_SIZE+payload) && noFailed < MAX_FAIL );
		//printf(" R");
		if((recv_len=recv(soc,bfr,(HDR_SIZE+payload),0))==SOCKET_ERROR)
			return;
		//printf(" D:L:%d EL:%d",recv_len,HDR_SIZE+payload);
		if(recv_len == HDR_SIZE+payload)
		{
			//printf("\nNew Packet:: Size:%d",recv_len);
			pkt.NewPacket(bfr,recv_len);
			DataPacketHandler(pkt);
		}
	//	else
		//	printf(" DISCARDED!!!");
		fprintf(ReadS,"%d\n",GetSysTime()-rbn);
	}
}
void P2P_Tcp_GenericListener2(SOCKET soc)
{
	int MAX_FAIL=3;
	int noFailed;
	int last_len;
	int recv_len;
	PACKET pkt;
	char bfr[MAX_MTU];

	while(1)
	{
		last_len=0;
		noFailed=0;
		do
		{
			if((recv_len=recv(soc,bfr,HDR_SIZE,MSG_PEEK))==SOCKET_ERROR)
				return;
			printf(" Peeking at message:%d LL:%d TS:%d",recv_len,last_len,MAX_MTU);
			if(recv_len==last_len)
				noFailed++;
			last_len=recv_len;
		}
		while(recv_len<MAX_MTU && noFailed < MAX_FAIL );
		printf(" R");
		if((recv_len=recv(soc,bfr,MAX_MTU,0))==SOCKET_ERROR)
			return;
		printf(" D:L:%d",recv_len);
		if(recv_len == MAX_MTU)
		{
			printf("\nNew Packet:: Size:%d",recv_len);
			pkt.NewPacket(bfr,recv_len);
			DataPacketHandler(pkt);
		}
	}
}
bool P2P_Tcp_Send(PACKET &tsnd)
{
	//printf("\n TCP Attempting to send...");
	//rudimentar sending system with no load ballancing ( chooses only the closest path )
	//looking up correct socket
	int plen = tsnd.PayloadSize+HDR_SIZE;
	if(tsnd.DestID < MyUID)
	{
		//lookup outlinks
		unsigned long long min = OutLinks[0].UID;
		int minPos=0;
		//printf("\nLooking up OutLinks");
		for(int i=0;i<MAX_OUTLINKS;++i)
		{
			if(OutLinks[i].UID)
			{
				if(OutLinks[i].UID == tsnd.DestID)
				{
					minPos=i;//destination is next hop
					break;
				}
				if( OutLinks[i].UID<min && OutLinks[i].UID>tsnd.DestID)
				{
					min = OutLinks[i].UID;//destination needs to be routed
					minPos = i;
				}
			}
		}
		//printf(" Sending packet to:%d",min);
		if (send(OutLinks[minPos].soc, tsnd.data, plen, 0) == SOCKET_ERROR)
		{
			//printf("\nP2P_TCP::Send function failed with error: %ld", WSAGetLastError());
			return false;
		}
	}
	else //lookup InLinks
	{
	//	printf("\nP2P_TCP:: Sender looking up InLink table...");
		map<unsigned long long,SOCKET>::iterator iter = InLinks.begin();
		unsigned long long max = iter->first;
		SOCKET outs = iter->second;
		//may need lock
		for(;iter!=InLinks.end();++iter)
		{
			if(iter->first == tsnd.DestID)
			{
				//comment line below after debug
				max=tsnd.DestID;
				outs=iter->second;
				break;
			}
			if(iter->first > max && iter->first < tsnd.DestID )
			{
				max = iter->first;
				outs = iter->second;
			}
		}
		//printf("\nP2P_TCP:: Sending to:%lld ",max);
		if (send(outs, tsnd.data, plen, 0) == SOCKET_ERROR)
		{
			//printf("\nP2P_TCP::Send function failed with error: %ld", WSAGetLastError());
			return false;
		}
	}
	return true;
}

