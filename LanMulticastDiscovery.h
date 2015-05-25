#define MCastPort      60000
#define MULTICAST_IP   "240.0.0.88"//
HANDLE MHNDL;
SOCKET msoc;
SOCKADDR_IN local_sin,snd_sin;
int LMD_JoingSourceGroup()
{
   /*struct ip_mreq imr; 
   
   imr.imr_multiaddr.s_addr  = inet_addr(MULTICAST_IP);
   imr.imr_interface.s_addr  = INADDR_ANY;
   return setsockopt(msoc, IPPROTO_IP, IP_SOURCE_MEMBERSHIP, (char *) &imr, sizeof(imr));  
*/
	return 0;
}

int LMD_LeaveSourceGroup()
{
  /* struct ip_mreq imr;

   imr.imr_multiaddr.s_addr  = inet_addr(MULTICAST_IP);
   imr.imr_interface.s_addr  = INADDR_ANY;
   return setsockopt(msoc, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *) &imr, sizeof(imr));
*/
	return 0;
}
DWORD WINAPI  MCastServer(LPVOID lpParam)
{
	int slen , recv_len;
	printf("\nWaiting for Multicast data...");
	PKT data;
	slen = sizeof(data.From);
	while(1)
	{
		//clear the buffer by filling null, it might have previously received data
		memset(data.packet,'\0', UDP_PACKET_SIZE);
		//WARNING! THE EXCESS DATA IS LOST BECAUSE OF THE WAY UDP IS IMPLEMENTED
		//Workaround: A Separate Processing Thread Deals With The Packets
		if ((recv_len = recvfrom(msoc, data.packet,UDP_PACKET_SIZE, 0, (struct sockaddr *) &data.From, &slen)) == SOCKET_ERROR)
			printf("\nMulticast Receiver failed with error code : %d" , WSAGetLastError());
		else
			PushToQ(data,PROTO_UDP);
	}
}
bool MCast_Send(char * data)//used for network discovery and possibly sound and video
{
		if (sendto(msoc, data, UDP_PACKET_SIZE, 0, (struct sockaddr*) &snd_sin, sizeof(snd_sin)) == SOCKET_ERROR)
		{
			printf("\nFailed to send UDP packet ERR:%d",GetLastError());
			return false;
		}
	return true;
}
char InitMulticastDiscovery()
{
//create UDP socket
	if((msoc = socket(AF_INET,SOCK_DGRAM,0))==SOCKET_ERROR)
	{
		printf("\nMulticast Discovery:: failed to create socket");
		return 0;
	}
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons(MCastPort);
	local_sin.sin_addr.s_addr = htonl(INADDR_ANY);
	snd_sin = local_sin;
	snd_sin.sin_addr.s_addr  = inet_addr(MULTICAST_IP);
	//bind socket
	if(bind(msoc,(struct sockaddr *) &local_sin,sizeof(local_sin))==SOCKET_ERROR)
	{
		printf("\nMulticast Discovery:: failed to bind socket");
		return 0;
	}
	//join multicast group
	if(LMD_JoingSourceGroup()==SOCKET_ERROR)
	{
		printf("\nMulticast Discovery:: Unable to join multicast group");
		return 0;
	}
	int NoParam1;
	MHNDL = CreateThread( NULL, 0, MCastServer, 	   &NoParam1, 0, NULL);  
	if(!MHNDL)
	{
		printf("\nMulticast Discovery: Could not start listener!");
		return 0;
	}
}