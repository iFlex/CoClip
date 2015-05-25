void SenderSpeedCheck(char * name)
{
	printf("\nTesting Sender Speed");
	UserStruct newUser;
	newUser.UProto=0;             //0 LAN //1 WWW_TCP
	newUser.NoAgeing=false;
	Users.AddUser(5,newUser, false );
	MarkStart(sbenchmark);
	ToSend.CreateTask(1,name,PACKET_STREAM_DATA,true);
	ToSend.AddUserToTask(1,5);
	ToSend.UserAcceptedTask(1,5);
	//ShowTimeElapsed(sbenchmark);
}
void ReaderSpeedCheck(unsigned int length)
{
	printf("\nBenchmarking Receiver Speed:%d sequences = %lld bytes",length,length*(MAX_MTU-HDR_SIZE));
	MarkStart(rbenchmark);
	ToRecv.CreateTask(5,1,"test.txt",true,PACKET_STREAM_DATA);
	ToRecv.AcceptTask(5,1);
	PACKET pkt;
	pkt.PackType=PACKET_STREAM_DATA;
	pkt.SendID=5;
	pkt.DestID=MyUID;
	pkt.StreamID=1;
	pkt.SeqNo=1;
	pkt.TotalSeq=length;
	pkt.NewData("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",1000);
	while(pkt.SeqNo<=pkt.TotalSeq)
	{
		DataPacketHandler(pkt);
		//ToRecv.UpdateTask(pkt);///!!!!!
		pkt.SeqNo++;
	}
	ShowTimeElapsed(rbenchmark);
}
void CoClipReadWriteTest()
{
	char ch;
	do
	{
	char name[100];
	printf("File Name to read from for test:");
	scanf("%s",name);
	PACKET tsdata;
	tsdata.PackType = PACKET_STREAM_OFFER;
	tsdata.SendID   = MyUID;
	tsdata.DestID   = MyUID;
	
	KeyList * rdr = new KeyList;
	rdr->DataSource.OpenReader(name,true);
	
	tsdata.SeqNo    = 1;
	tsdata.TotalSeq = rdr->DataSource.totalSeq;
	tsdata.StreamID = 1;
	
	//ToRecv.CreateTask(1,1,"TestIO",true,PACKET_STREAM_DATA);
	ToRecv.AcceptTask(1,1);
	bool ok=false;
	printf("\nTesting read write functionality!");
	while(tsdata.SeqNo<=tsdata.TotalSeq)
	{
		char *data;
		char ret;
		unsigned short dataLen;
		//if(tsdata.SeqNo!=100)
		data = rdr->DataSource.get(tsdata.SeqNo,dataLen);//add support for reading and fail safe in case file dissapears
		//else
			//data = rdr->DataSource.get(tsdata.SeqNo-1,dataLen);
		//assembling packet
		tsdata.PayloadSize = dataLen;
		tsdata.NewData(data,dataLen);
		ToRecv.UpdateTask(tsdata);
		tsdata.SeqNo++;
	}
	printf("\nDONE!");
	printf("\n Try another file?(y/n)");
	ch = getch();	
	}
	while(ch=='y');
	
}
void MultipleTaskTest()
{
	for(int i=2;i<10;++i)
	{
		if(!ToRecv.CreateTask(5,i,GetNewOfferFileName(),true,PACKET_STREAM_OFFER))
			printf("\nFailed To Create Task %d\n",i);
		printf("\nAny destructors are above!");
	printf("\n******** RECEIVE TASK LIST");
	int k=0;
	std::map<char * ,RecvStruct*,my_scmp>::iterator iter;
	for (iter = ToRecv.RecvQ.begin(); iter != ToRecv.RecvQ.end(); iter++)
	{
		printf("\n****%d:From:%s Accepted?:%d Completed:%f\%",k,iter->first,iter->second->Accepted,(iter->second->TotalSeq==0)?iter->second->TotalSeq:((float)iter->second->LastWSeq)/iter->second->TotalSeq*100);
		k++;
	}				
	printf("\n********");
	
	}
	for(int i=2;i<10;++i)
	{
		if(!ToRecv.CreateTask(5,i,GetNewOfferFileName(),true,PACKET_STREAM_OFFER))
			printf("\nFailed To Create Task %d\n",i);
		ToRecv.AcceptTask(5,i);
		PACKET packet;
		packet.PackType=PACKET_STREAM_DATA;
		packet.StreamID=i;
		packet.SeqNo=1;
		packet.TotalSeq=2;
		packet.SendID=5;
		packet.DestID=MyUID;
		packet.PayloadSize=1;
		packet.data[0]='C';
		if(!ToRecv.UpdateTask(packet))
			printf("\n    Could not write!");
	}
	printf("\n******** RECEIVE TASK LIST");
	int k=0;
	std::map<char * ,RecvStruct *,my_scmp>::iterator iter;
	for (iter = ToRecv.RecvQ.begin(); iter != ToRecv.RecvQ.end(); iter++)
	{
		printf("\n****%d:From:%s Accepted?:%d Completed:%f\%",k,iter->first,iter->second->Accepted,(iter->second->TotalSeq==0)?iter->second->TotalSeq:((float)iter->second->LastWSeq)/iter->second->TotalSeq*100);
		k++;
	}				
	printf("\n********");
	printf("\ncheck...");
	CreateKey(RecvKey,5,2);
	if(ToRecv.RecvQ.find(RecvKey)==ToRecv.RecvQ.end())//task already exists
	{
		printf("\nOoooopa, problems in paradise!");
	}					
}
void RunCoClipTests()
{
	printf("\nCOCLIP TESTBED!\nTest no 1.");
	//MultipleTaskTest();
	ReaderSpeedCheck(1000000);
	SenderSpeedCheck("test.txt");
}
