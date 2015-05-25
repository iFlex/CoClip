/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// CoClip WEB MODULE /////////////////////////////////
///////////////////////////// TCP Implementation ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////To start it just call WWW_Tcp_Start()///////////////////////
//data
#include "CoClipConstants.h"
#define WWW_Tcp_MaxServer 20
short WWW_Tcp_NoServers=0;
unsigned int WWW_Tcp_Servers[WWW_Tcp_MaxServer];
HANDLE WWW_Tcp_ConnectorHndl;
HANDLE WWW_Tcp_ListenerHndl;
SOCKET WWW_Tcp_link;
unsigned short WWW_Tcp_port;//standard HTTP port
unsigned int WWW_Tcp_ip;
bool WWW_Tcp_connected=false;
///functions
void WWW_Tcp_SetServers(unsigned int * ser);
bool WWW_Tcp_Send(char * data);
void WWW_Tcp_Start();
void WWW_Tcp_AddServer(char * server);
void WWW_Tcp_TryServers(int lpParam);
void WWW_Tcp_Listener(int lpParam);

//implementation
void WWW_Tcp_INIT()
{
	WWW_Tcp_link=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	WWW_Tcp_port = 106;//PUT BACK TO 80
}
void WWW_Tcp_AddServer(char * server)
{
	int i=0;
	for(i=0;i<WWW_Tcp_NoServers ;++i);
	if(i!=WWW_Tcp_MaxServer)
	{
		WWW_Tcp_Servers[i]=htonl(INET_CHARtoH(server));
		WWW_Tcp_NoServers++;
	}
}

void WWW_Tcp_SetServers(unsigned int * Ser)
{
	for(int i=0;i<WWW_Tcp_MaxServer && Ser[i]>0 ;++i)
		WWW_Tcp_Servers[i]=Ser[i];
}

bool WWW_Tcp_Send(char * data)
{
	//printf("\n TCP Attempting to send...");
	try
	{
		if (send(WWW_Tcp_link, data, UDP_PACKET_SIZE, 0) != SOCKET_ERROR)
		{
			return false;
		}
		//else
			//printf("\nSend function failed with error: %ld", WSAGetLastError());
	}
	catch(...)
	{
		printf("\n Exception caught on TCP WWW SENDER");
	}
	return true;
}
void WWW_Tcp_EndConnection()
{
	printf("\n***###*** Ending TCP connection!");
	closesocket(WWW_Tcp_link);//close socket
	WWW_Tcp_link=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);//get a new socket
	WWW_Tcp_connected = false;
	Sleep(3000);
	printf("\nRestarting WWW TCP Sweeper");
}
void WWW_Tcp_Listener(int lpParam)//check for errors
{
    //Ask who else is online (out of friends and on same network)
    PACKET AuthOnWWW;
    AuthOnWWW.PackType = PACKET_WHO_ONLINE;
    AuthOnWWW.NewData("? ",2);
    AuthOnWWW.pack();
    WWW_Tcp_Send(AuthOnWWW.data);

	int recv_len;
	//printf("\n Starting TCP Listener");
	PKT ServerData;
	//ServerData.FromConn = WWW_Tcp_link;
	char Reconstruct[UDP_PACKET_SIZE];
	ServerData.source               = PROTO_WWW_TCP;//www tcp source;
	ServerData.From.sin_family      = AF_INET;/////DAMN YOU MICROSOFT DAMN YOU!
	ServerData.From.sin_port        = htons(WWW_Tcp_port);
	ServerData.From.sin_addr.s_addr = WWW_Tcp_ip;
	printf("\n TCP Listener thread started...");
	short ReadUnit = UDP_PACKET_SIZE;

	while(true)
	{
		try
		{
		if ((recv_len = recv(WWW_Tcp_link, ServerData.packet, UDP_PACKET_SIZE, 0)) != SOCKET_ERROR)
		{
			//push packet to packet Q
			//printf("\nTcp data in Sizeof ServerData.packet:%d",sizeof(ServerData.packet));
			if(recv_len == UDP_PACKET_SIZE)
				PushToQ(ServerData,PROTO_WWW_TCP);
			else
			{
				printf("\n**SERVER TCP::received less bytes than normal %d RU:%d",recv_len,ReadUnit);
				if(!recv_len)
				{
					printf("\nWWW TCP CONNECTION LOST!");
					printf("\n*** Attempting to restore");
					WWW_Tcp_EndConnection();
					//unpause the server sweeping thread!
					return;
				}
				printf("\n***Fetching rest of packet...");
				ReadUnit = UDP_PACKET_SIZE-recv_len;
				int old_len = recv_len;
				if ((recv_len = recv(WWW_Tcp_link, Reconstruct, ReadUnit, 0)) == SOCKET_ERROR)
				{
					printf("\n****OW SNAP...SOMETHING IS WRONG WITH THE TCP PACKET RECEIVER");
					printf("\n****Dropping this packet...");
					continue;
				}
				if(recv_len==ReadUnit)
				{
					printf("\nTCP Pushing reconstructed packet");
					for(int i=0;i<recv_len;++i)
						ServerData.packet[i+old_len]=Reconstruct[i];
					PushToQ(ServerData,PROTO_WWW_TCP);
					ReadUnit=UDP_PACKET_SIZE;
				}
				else
					printf("\nDropped packet due to inconsistency!");
			}
		}
		else
		{
			printf("\nCould not receive Web TCP packet, There was an error!");
			printf("\n Ending connection...");
			WWW_Tcp_EndConnection();
			return;
		}
		}
		catch(...)
		{
			printf("\nCould not receive Web TCP packet...");
			//WWW_Tcp_EndConnection();
			WWW_Tcp_connected = false;
			return;
		}
	}
}
void WWW_Tcp_TryServers(int lpParam)//problematic?
{
	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	WWW_Tcp_connected = false;
	int i;
	printf("\n Trying WWW servers...");
	while(1)
	{
		//printf("\nStarting sweep:%d %d",WWW_Tcp_MaxServer,WWW_Tcp_Servers[0]);
		for(i=0;i<WWW_Tcp_NoServers;++i)
		{
			try
			{
				sa.sin_family = AF_INET;
				sa.sin_port = htons(WWW_Tcp_port);
				sa.sin_addr.s_addr = WWW_Tcp_Servers[i];
				WWW_Tcp_ip = WWW_Tcp_Servers[i];
				if(connect(WWW_Tcp_link,(struct sockaddr*) &sa,sizeof(sa)) != SOCKET_ERROR)
				{
					//exiting connection thread
					sockaddr_in hints;
					sockaddr hnts;
					int *size;
					int *sz;
					int rt1;
					int rt2;
					rt1=getpeername(UDPsoc,(struct sockaddr*) &hints,size); //get peer name works for connected sockets
					rt2=getsockname(UDPsoc,(struct sockaddr*) &hints,sz);

					//authenticate immediatly after connection was established
					PACKET AuthOnWWW;
					AuthOnWWW.EncodeMeOnline(MyData,0,MyUID);
					AuthOnWWW.pack();
					WWW_Tcp_Send(AuthOnWWW.data);
					//wait for newtork acceptance
					int recv_len=0;
					char bffr[UDP_PACKET_SIZE];
					if ((recv_len = recv(WWW_Tcp_link, bffr, UDP_PACKET_SIZE, 0)) != SOCKET_ERROR)
					{
						//received answer1
						PACKET Answer(bffr);
						if(Answer.PackType == PACKET_WELLCOME)
						{
							//successfully joined network
							WWW_Tcp_connected = true;
							printf("\nSUCCESS! SERVER has accepted me on the WWW network!");
						}
						else
						{
							//acces to network was denied
							WWW_Tcp_connected = false;
							//notify the user to enter the correct password
							printf("\nOOPS! SERVER has denied me acces to the WWW network!");
							//return;
						}
					}
					//starting listener thread
					int NoParam;
					printf("\n...Starting TCP Listener");
					WWW_Tcp_Listener(0);
				}
				else
					printf("\nWWW_TCP:connect function failed with error: %ld", WSAGetLastError());
			}
			catch(...)
			{
				printf("\n**EX**Exception in the WWW TCP SERVER PROBE");
			}
		}
		//printf("\n %d Unsuccessful sweeps",WWW_Tcp_FailedSweeps);
	}
}
void WWW_Tcp_Start()
{
    boost::thread WWW_Tcp_watcher(WWW_Tcp_TryServers,0);
}
