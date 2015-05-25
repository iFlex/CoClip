//CoClipDataStructures
#include <map>
#include <functional>
#include <boost/shared_ptr.hpp>
using namespace std;

char * Extract(char * data,int from,int to);
void CreateKey(char * key, unsigned long long ip, unsigned int port);
//default comparator for MAPs with char * keys
//////////////////////////////////////////////////////////////////////////////////////////////////////
class RingBuffer//////////////////////////////////Apparent small memory leak!
{
public:
	int length;
	int usefulLength;
	int start;
	int end;
	unsigned short *dataLen;
	char ** Buffer;

	RingBuffer();
	~RingBuffer();
	void push(char * data,unsigned short len);
	void pop();
	char get(int pos,char * data,unsigned short & dlen);
};

class SmartReader//////////////////////////////////Apparent Small Memory Leak depending on upper structure
{
	private:
		char isOpen;
		char isFile;
		char isLoaded;
		FILE * FD;
		int DSlen;
		unsigned int readerPos;

		bool Load(unsigned int seq,char * buffer,unsigned short & len);

	public:
		unsigned int totalSeq;
		char * DataSource;

		SmartReader();
		~SmartReader();
		inline bool IsFileReady(){return (isLoaded>0);}
		char OpenReader(char * data,char isfile);//opens the reader and sets up the parameters ( preparing for read )
		char Preload(char * data,int len);
		char LoadFile(int nrb); //if data is in file it incrementally loads it into the memory and calculates total seq
		char get(unsigned int seqNo,char * data,unsigned short & len);
		void CloseReader();
};

class SmartWriter  ///////////////////////////////////OK!
{
	public:
		char isFile;
		FILE *FD;
		int pos;
		char * DataSource;
		char lcp[MAX_FILESYS_PATH+5];
		char chosenLCP[MAX_FILESYS_PATH+5];
		char isOpen;
		void NormalizePath();
		SmartWriter(){pos=0;DataSource=0;isOpen=0;isFile=0;lcp[0]=0;chosenLCP[0]=0;}
		~SmartWriter(){CloseWriter();printf(" Call by Destructor");}
		void SetLongestCommonPath(char * LCP){strcpy(lcp,LCP);}
		void SetDestinationCommonPath(char * LCP){strcpy(chosenLCP,LCP);}
		void PreloadWriter(char * name)
		{
			if(!isFile && !isOpen)
			{
				isFile = true;
				int i=0;
				for(i=0;name[i];++i);

				DataSource = new char[i+1];
				for(i=0;name[i];++i)
					DataSource[i]=name[i];
				DataSource[i]=0;
			}
		}
		char OpenWriter();//opens file preloaded with Preloadwriter
		char OpenWriter(char * name,char isfile);
		bool WriteNext(char * data,int len)
		{
			if(isOpen)
			{
				if(isFile)
				{
					//printf("\nSMARTWRITER:Writting data...");
					fwrite(data,len,1,FD);
					//if(ferror(FD);
					/*Signal error while writting file*/
					return true;
				}
				//else
				//{
				//	for(int i=0;i<len;++i)
				//		DataSource[pos++]=data[i];
				//	DataSource[pos]=0;
				//}
			}
			return false;
		}
		void CloseWriter()
		{
			if(isFile)
			{
				if(isOpen)
					fclose(FD);
				isFile=false;
				delete[] DataSource;
				printf("\n Smart Writer Closed!");
			}//signal job is complete
		}
};

struct LinkedList{////////////////////////////////////OK!
	unsigned int seq;
	unsigned short plen;
	char *data;
	LinkedList * next;
	//LinkedList *inner;
};

class List{///////////////////////////////////////////OK!
private:
	int size;
	int length;
	int max_length;
	LinkedList *list;
	//functions
	void pop();//pops element i from root

public:
	List();
	List(int packet_length);
	~List();
	char push(unsigned int seq,unsigned int LastW,char * dta,unsigned short dlen,int dOffset);//pushes to root
	char * GetNext(unsigned int lastSeq,unsigned short & lng);
	int len(){return length;}
	char AutoGap();
};

struct UserStruct{
	char ServerValidated;
	unsigned int U_IP;
	unsigned short TCP_Port;
	unsigned short NoAvConn;
	char UProto;             //0 LAN //1 WWW_TCP
	char NoAgeing;
	unsigned long long LastTimeStamp;
	unsigned long long LastActive;
	char name[150];
	char email[100];
};

struct RecvStruct{
	char Accepted;
	unsigned int LastWSeq;
	unsigned long long TimeRecvd;
	unsigned long long TimeStarts;
	unsigned int TotalSeq;
	unsigned int PacketType;
	List PacketBuffer;
	SmartWriter DataDest;
};

struct SendStruct{
	//unsigned int TotalSeq;
	unsigned int LastSent;
	unsigned int LastACK;
	unsigned long long LastActive;
	unsigned long long FirstSent;
	char Accepted;
	float AutoGap;
};

class PACKET
{
	public:
		unsigned short HdrPos;
		//packet HDR
		unsigned short PackType;
		unsigned long long SendID;
		unsigned long long DestID;
		unsigned int SeqNo;
		unsigned int TotalSeq;
		unsigned int StreamID;
		unsigned short PayloadSize;
		//packet data
		char data[MAX_MTU];

		void NewPacket(char *,int packet_size);
		void NewPacket();
		PACKET();//manually build the packet
		PACKET(char *);//build packet form char encoded string
		PACKET(char *,int packet_size);//build packet form char encoded string
		void NewData(char *,int len);
		void AppendRawData(char *,int len);
		void pack();
		UserStruct DecodeMeOnline();
		void EncodeMeOnline(UserStruct,unsigned long long,unsigned long long);
};

struct KList /////////////////////////////////////////////////////////////OK
{
	unsigned long long UID;
	SendStruct Data;
	KList * next;
};

class KeyList{ ///////////////////////////////////////////////////////////OK
private:
	int idx;
	int len;
public:
	KList * Record;
	SmartReader DataSource;
	unsigned short PacketType;

	KeyList();//constructor
	~KeyList();//destructor

	char push(unsigned long long uid,SendStruct Data);//pushes person with uid and with Data at the back of the list
	char pop(int pos);
	char pop(unsigned long long id);
	int length(){return len;}
	KList * get(int pos);
	void first();//sets idx to -1 use before next
	KList * next();//returns next klist entry
};

class UserStack{//undergoing testing
	//Users Panel
	public:
		map<unsigned long long, UserStruct> UserQ;
	public:
		char AddUser(unsigned long long key,UserStruct Data,char override);//this also chooses who is the right user out of 2 colliding
		char UpdateUserData(unsigned long long key, UserStruct Data);
		void PopUser(unsigned long long key);
		void UpdateTimeStamp( unsigned long long key, unsigned long long stamp);
		char UserStillActive(unsigned long long Key);
};

class SendStack{
	//SenderQ
	public:
		map<unsigned int,boost::shared_ptr<KeyList>> SendQ;
	public:
		char CreateTask(unsigned int TaskID,char * name,unsigned short PacketType,char isfile);
		char AddUserToTask(unsigned int TaskID,unsigned long long UID);
		char UserAcceptedTask(unsigned int TaskID,unsigned long long UID);
		char AckPacket(unsigned int TaskID,unsigned long long UID,unsigned int Seq,unsigned char AutoGap);
		char PopUserFromTask(unsigned int TaskID,unsigned long long UID,char * &fname);//this also deletes whole task if all userse have received
		char ResetSenderPos(unsigned int TaskID,unsigned long long UID,unsigned int Seq);
		char CancelTask(unsigned int TaskID,char * fname);
};

class RecvStack{
	//ReceiverQ
	public:
		map<char *, boost::shared_ptr<RecvStruct>,my_scmp> RecvQ;
	public:
		char CreateTask(unsigned long long UID, unsigned int TaskID,char * name, char isfile,unsigned int pktype,char * lcp);//the functions must be rewritten
		char AcceptTask(unsigned long long UID, unsigned int TaskID,char * clcp);
		unsigned long long UpdateTask(PACKET & Data,char * finished);//also deletes the task if it's done
		//char AckPacket(unsigned long long UID,unsigned int TaskID,unsigned int Seq);
		char EndTask(char * key,boost::shared_ptr<RecvStruct> Rbuff,char load);//implement
		char CancelTask(unsigned long long UID, unsigned int TaskID);//implement
		char AttemptWrite(boost::shared_ptr<RecvStruct> Rbuff,char * key,char * finished);
		char LoadTasksToReceiver(char * filename,unsigned long long who);
		bool IsAckReady(unsigned long long UID,unsigned int TaskID);
};


/////////////////////////////////IMPLEMENTATION
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CoClipDataStructures
//#include "IPC_Signaling.h"
//#include "CoClipDataStructures.h"
#include "CoClipConstants.h"
#include "Timestamping.h"
#include <stdio.h>
//#include "IPC_Signaling.h"
//////////////////////////////BASE FUNCTIONS////////////////////////////////////////////

char * Extract(char * data,int from,int to)
{
	if(from>to)
		return 0;
	int len=to-from;
	char * rez = new char[len+1];
	for(int i=0;i<len;++i)
		rez[i]=data[i+from];
	rez[len]=0;
	return rez;
}
void CreateKey(char * rez,unsigned long long ip, unsigned int port)
{
	char lenip=19;
	char lenpt=10;
	int idx=0;
	while(lenip)
	{
		rez[idx++]=(ip%10)+48;
		ip/=10;
		lenip--;
	}
	while(lenpt)
	{
		rez[idx++]=(port%10)+48;
		port/=10;
		lenpt--;
	}
	rez[idx]=0;
}

PACKET::PACKET()
{
	PackType=0;
	SeqNo=1;
	TotalSeq=1;
	StreamID=0;
	PayloadSize=0;
	SendID=0;
	DestID=0;
}

void PACKET::NewPacket()
{
	DestID      = StrToBin(data,0,8);  //8B
	SendID      = StrToBin(data,8,16); //8B
	PackType    = StrToBin(data,16,18);//2B
	SeqNo       = StrToBin(data,18,22);//4B
	TotalSeq    = StrToBin(data,22,26);//4B
	StreamID    = StrToBin(data,26,30);//4B
	PayloadSize = StrToBin(data,30,32);//2B
    for(int i=0;i<PayloadSize;++i)
        data[i] = data[i+HDR_SIZE];
}
void PACKET::NewPacket(char * stream,int packet_size)
{
	DestID      = StrToBin(stream,0,8); //8B
	SendID      = StrToBin(stream,8,16);//8B
	PackType    = StrToBin(stream,16,18);  //2B
	SeqNo       = StrToBin(stream,18,22);//4B
	TotalSeq    = StrToBin(stream,22,26);//4B
	StreamID    = StrToBin(stream,26,30);//4B
	PayloadSize = StrToBin(stream,30,32);//2B
	for(int i=0;i<PayloadSize && i+HDR_SIZE<packet_size;++i)
		data[i]=stream[i+HDR_SIZE];
}
PACKET::PACKET(char * stream)
{
	NewPacket(stream,MAX_MTU);
}
PACKET::PACKET(char * stream,int packet_size)
{
	NewPacket(stream,packet_size);
}
void PACKET::NewData(char * newd,int len)
{
	int i=0;
	int maxPayload=MAX_MTU-HDR_SIZE;
	for(i=0;i<maxPayload && i<len;++i)
		data[i]=newd[i];
	PayloadSize=i;
}
void PACKET::AppendRawData(char *newd,int len)
{
	int i=0;
	int maxPayload=MAX_MTU-HDR_SIZE;
	for(i=PayloadSize+HDR_SIZE;i<maxPayload && i<len;++i)
		data[i+HDR_SIZE]=newd[i];
	PayloadSize=i;
}
void PACKET::pack()
{
	char * Destr = BinToStr(DestID,8);
	char * Sendr = BinToStr(SendID,8);
	char * type  = BinToStr(PackType,2);
	char * seq   = BinToStr(SeqNo,4);
	char * tseq  = BinToStr(TotalSeq,4);
	char * strs  = BinToStr(StreamID,4);
	char * psize = BinToStr(PayloadSize,2);

	int nr=0;
	int i=0;

    for(i=PayloadSize-1;i>=0;--i)
        data[i+HDR_SIZE] = data[i];

	for(i=0;i<8;++i)
		data[nr++]=Destr[i];

	for(i=0;i<8;++i)
		data[nr++]=Sendr[i];

    //building packet
	data[nr++]=type[0];
	data[nr++]=type[1];

	for(i=0;i<4;++i)
		data[nr++]=seq[i];

	for(i=0;i<4;++i)
		data[nr++]=tseq[i];

	for(i=0;i<4;++i)
		data[nr++]=strs[i];

	data[nr++]=psize[0];
	data[nr++]=psize[1];
	//packing padding to fill up to BUFLEN
	delete [] Destr;
	delete [] Sendr;
	delete [] type;
	delete [] seq;
	delete [] tseq;
	delete [] strs;
	delete [] psize;
}
UserStruct PACKET::DecodeMeOnline()
{
	UserStruct rez;
	rez.LastTimeStamp = StrToBin(data,0,8);
	rez.U_IP          = StrToBin(data,8,12);
	rez.TCP_Port      = StrToBin(data,12,14);
	rez.NoAvConn      = StrToBin(data,14,16);
	int i=16;
	int nr=0;
	//decoding email and name
	for(;i<MAX_MTU && data[i];++i)
		rez.email[nr++]=data[i];
	rez.email[nr]=0;
	nr=0;
	i++;
	for(;i<MAX_MTU && data[i];++i)
		rez.name[nr++]=data[i];
	rez.name[nr]=0;

	return rez;
}
void PACKET::EncodeMeOnline(UserStruct UserData,unsigned long long To,unsigned long long From)
{
	PackType=PACKET_ME_ONLINE;
	SeqNo=1;
	TotalSeq=1;
	StreamID=0;
	SendID=From;
	DestID=To;

	char * ts = BinToStr(GetSysTime(),8);
	int nr = 0;
	int i;
	for( i=0; i<8 ; ++i )
		data[nr++]=ts[i];
	delete[] ts;
	ts = BinToStr(UserData.U_IP,4);
	for( i=0; i<4 ; ++i )
		data[nr++]=ts[i];
	ts = BinToStr(UserData.TCP_Port,2);
	for( i=0; i<2 ; ++i )
		data[nr++]=ts[i];
	delete[] ts;
	ts = BinToStr(UserData.NoAvConn,2);
	for( i=0; i<2 ; ++i )
		data[nr++]=ts[i];
	delete[] ts;

	for( i=0;UserData.email[i] && nr < UDP_PACKET_SIZE;++i)
		data[nr++]=UserData.email[i];
	data[nr++]=0;
	if(nr<MAX_MTU)
	{
		for( i=0;UserData.name[i] && nr < UDP_PACKET_SIZE;++i)
			data[nr++]=UserData.name[i];
		data[nr++]=0;
	}
	PayloadSize=nr;
}

void UserStack::UpdateTimeStamp(unsigned long long Key,unsigned long long stamp)
{
    //UserLock.lock();
	UserQ[Key].LastTimeStamp=stamp;
    //UserLock.unlock();
}
char UserStack::AddUser(unsigned long long Key,UserStruct Data,char override)
{
	//change this to be able to check for colliding users!
		char ret=1;
		Data.UProto = PROTO_LAN_TCP;
		if(UserQ.find(Key)!=UserQ.end())
		{
			if(!override)
			{
				//do timestamp check and choose prefered source
				unsigned long long SysTime = GetSysTime();
				if( GetTimestampDifference(SysTime,UserQ[Key].LastTimeStamp) < GetTimestampDifference(SysTime,Data.LastTimeStamp))
					return 0;
			}
			else
				if(Data.U_IP!=UserQ[Key].U_IP)//Server reported correct IP. establishing channel
					Data.UProto = PROTO_ESTABLISH;
		}
		else
		{
			printf("\nNew User Came ONLINE %lld:%s",Key,Data.name);
			if(override)//packet comes from server for the first time (not directly from network) so channel must be chosen
			{
				Data.UProto = PROTO_ESTABLISH;
				ret = 3;//new packet from WWW_SERVER
			}
			else
				ret = 2;//new packet form LAN
		}
	Data.LastActive = GetSysTime();

	//UserLock.lock();
	UserQ[Key]=Data;
	//UserLock.unlock();

	return ret;
}
char UserStack::UserStillActive(unsigned long long Key)
{
	if(UserQ.find(Key)==UserQ.end())
		return false;

	//UserLock.lock();
	UserQ[Key].LastActive = GetSysTime();
	//UserLock.unlock();
	return true;
}
char UserStack::UpdateUserData(unsigned long long Key,UserStruct Data)
{
	//change this to be able to check for colliding users!
		printf("\nAttempting to update: %lld",Key);
		if(!UserQ.count(Key))
			return false;
		//update field by field
		//UserLock.lock();
		if(Data.U_IP)
			UserQ[Key].U_IP=Data.U_IP;
		if(Data.TCP_Port)
			UserQ[Key].TCP_Port=Data.TCP_Port;
		if(Data.NoAvConn)
			UserQ[Key].NoAvConn=Data.NoAvConn;
		if(Data.LastTimeStamp)
			UserQ[Key].LastTimeStamp = Data.LastTimeStamp;
		if(Data.LastActive)
			UserQ[Key].LastActive    = GetSysTime();
		if(Data.email[0])
			for(int i=0;Data.email[i];++i)
				UserQ[Key].email[i]=Data.email[i];
		if(Data.name[0])
			for(int i=0;Data.name[i];++i)
				UserQ[Key].name[i]=Data.name[i];
        //UserLock.unlock();

		return true;
}
void UserStack::PopUser(unsigned long long Key)
{
	printf("\nErasing user:%lld",Key);
	//UserLock.lock();
	UserQ.erase(Key);
    //UserLock.unlock();
}

///////////////////////////////END OF SIMPLE FUNCTION
//////////////////////////SENDER FUNCTIONS//////////////////////////////////////////
KeyList::KeyList()
{
	Record=0;
	len=0;
}
KeyList::~KeyList()
{
	//delete procedure
	printf("\nKEY LIST(SendStack):Clering list(~Destructor Call)s...!");
	while( Record && len>0)
		pop(0);
}
char KeyList::push(unsigned long long uid,SendStruct Data)
{
	if(!Record)
	{
		printf("\nPusghin user to new list");
		Record = new KList;
		printf("\nHave allocated KLIST");
		Record->UID =uid;
		printf("\nCOPIED UID");
		//memcpy(&Record->Data,&Data,sizeof(SendStruct));
		Record->Data=Data;
		printf("\nCopied user data");
		Record->next=0;
		len++;
		printf("\nDone");
	}
	else
	{
		printf("\nAppending to old list");
		KList *iterator = Record;
		while(iterator->next)
		{
			if(iterator->UID==uid)
				return false;
			iterator=iterator->next;
		}
		if(iterator->UID==uid)
			return false;

		KList * addee = new KList;
		addee->UID=uid;
		addee->Data=Data;
		addee->next=0;
		iterator->next=addee;
		len++;
	}
	return true;
}
char KeyList::pop(int pos)
{
	if( pos>-1 && len )
	{
		KList * iterator = Record;
		KList * last=0;

		while(iterator && pos)
		{
			pos--;
			last=iterator;
			iterator=iterator->next;
		}
		if(!iterator && !last)
			return false;
		if(pos>0)
			return false;

		if(!last && iterator)
		{
			KList * nxt = iterator->next;
			delete iterator;
			iterator=nxt;
			if(len==1)//last element was deleted
				Record=0;
		}
		else
		{
			if(iterator)
			{
				last->next = iterator->next;
				delete iterator;
			}
			else
			{
				delete last->next;
				last->next = 0;
			}
		}
		len--;
		return true;
	}
	return false;
}
char KeyList::pop(unsigned long long id)
{
	if(len)
	{
		KList * iterator = Record;
		KList *last=0;
		char found=false;
		while( iterator )
		{
			if(iterator->UID == id)
			{
				found=true;
				break;
			}
			last=iterator;
			iterator=iterator->next;
		}
		if(!iterator && !last)
			return false;
		if(found)
		{
			if(!last)
			{
				KList * nxt = iterator->next;
				printf("\n*FINAL**Damage?:%d %d",iterator->UID,iterator->next);
				delete iterator;
				iterator=nxt;
				printf("\n*FINAL Damage?:%d",iterator);
				if(len==1)//last element was deleted
					Record=0;
			}
			else
			{
				if(iterator)
				{
					KList * nxt = iterator->next;
					last->next=nxt;
					delete iterator;
				}
				if(!iterator)
					return false;
			}
			len--;
			return true;
		}
	}
	return false;
}
KList * KeyList::get(int pos)
{
	if(!Record)
		return 0;
	KList * iterator = Record;

	while(iterator && pos)
	{
		pos--;
		iterator=iterator->next;
	}
	if(!pos)
		return iterator;

	return 0;
}
void KeyList::first()
{
	idx=-1;
}
KList * KeyList::next()
{
	idx++;
	return get(idx);
}

char SendStack::CreateTask(unsigned int TaskID,char * name,unsigned short pktype,char isfile)
{
	if(SendQ.find(TaskID)!=SendQ.end())//alerady exists
	{
		printf("\nSEND_STACK:TASK ALREADY EXISTS!");
		return 0;//already existing user
	}
	boost::shared_ptr<KeyList> _empty (new KeyList);
	_empty->PacketType = pktype;
	//open the writer
	if(!_empty->DataSource.OpenReader(name,isfile))
	{
	    _empty->DataSource.CloseReader();
		printf("\nDebug::Can't open source file!");
		return -1;//could not open file
	}
	//assigning the data structure to the stack
	//SendLock.lock();
	SendQ[TaskID]=_empty;
	//SendLock.unlock();
	printf("\nSEND_STACK:TASK WAS CREATED! :NO SEQUENCES:%d",SendQ[TaskID]->DataSource.totalSeq);
	return 1;//created empty task to populate with users
}
char SendStack::AddUserToTask(unsigned int TaskID,unsigned long long UID)
{
		SendStruct Data;
		//Data.TotalSeq=DataSource.totalSeq;
		Data.LastSent = 0;
		Data.LastACK  = 0;
		Data.Accepted = false;
		Data.AutoGap  = 100;//%100

		if(SendQ.find(TaskID)!=SendQ.end())
		{
		    //SendLock.lock();
		    SendQ[TaskID]->push(UID,Data);
            //SendLock.unlock();
		}
		else
			return 0;
		return 1;
}
char SendStack::UserAcceptedTask(unsigned int TaskID,unsigned long long UID)
{
	//check if task exists
    if(SendQ.find(TaskID) != SendQ.end())
	{
		//iterate the users and set the flag
		//SendLock.lock();
		SendQ[TaskID]->first();
		KList * iterator = SendQ[TaskID]->next();
		//SendLock.unlock();
		while(iterator)
		{
			if(iterator->UID == UID)
			{
				if(!iterator->Data.Accepted)
				{
					//SendLock.lock();
					iterator->Data.Accepted=true;
                    //SendLock.unlock();
					return true;
				}
				return false;
			}
			iterator=iterator->next;
		}
	}
	return false;
}
char SendStack::AckPacket(unsigned int TaskID,unsigned long long UID,unsigned int Seq,unsigned char AutoGap)
{
	//check if task exists
	if(SendQ.find(TaskID)==SendQ.end())
		return false;
	auto ptr = SendQ[TaskID];
	//iterate the users and set the flag
	ptr->first();
	KList * iterator = ptr->next();

	while(iterator)
	{
		if(iterator->UID == UID)
		{
			if(iterator->Data.LastACK<Seq)//consider ack if it is greater than the last ACK
			{
				//printf("\nHave Updated SeqNo:%d total S:%d GAP:%d",Seq,ptr->DataSource.totalSeq,iterator->Data.LastSent-Seq);
				//SendLock.lock();
				iterator->Data.LastACK=Seq;
				iterator->Data.LastActive=GetSysTime();
				if(AutoGap>0 && AutoGap<256)
					iterator->Data.AutoGap = AutoGap;
				else
					iterator->Data.AutoGap = 100;//%100
                //SendLock.unlock();
				//if(iterator->Data.LastACK >= ptr->DataSource.totalSeq)//user has ACKed all data
					//printf("\n *** MUST ALSO POP USER FROM TASK:%d USR:%d",TaskID,UID);
			}
			return true;
		}
		iterator=iterator->next;
	}
	return false;
}
char SendStack:: ResetSenderPos(unsigned int TaskID,unsigned long long UID,unsigned int Seq)
{
//check if task exists
	if(SendQ.find(TaskID)==SendQ.end())
	{
		//printf("\nNO SUCH TASK TO ACK!");
		printf("\n****Retransmit request for absent task!");
		return false;
	}
	auto ptr = SendQ[TaskID];
	//iterate the users and set the flag
	ptr->first();
	KList * iterator = ptr->next();

	while(iterator)
	{
		if(iterator->UID == UID)
		{
			printf("\n***Retransmit Request Received SeqNo:%d",Seq);
			//SendLock.lock();
			iterator->Data.LastSent = Seq;
			iterator->Data.LastACK  = Seq;
			//SendLock.unlock();
			return true;
		}
		iterator=iterator->next;
	}
	printf("\n**Have not found user that asked for retransmit!");
	return false;
}
char SendStack::PopUserFromTask(unsigned int TaskID,unsigned long long UID,char * &fname)//no locking! //add & in both
{
		fname=0;
		printf("\n  Popping user :%lld from task %d",UID,TaskID);
		if(SendQ.find(TaskID) == SendQ.end())
		{
			printf("\n!!!!No such task!");
			return 0;
		}
		auto ptr = SendQ[TaskID];
		//SendLock.lock();
		if(ptr->pop(UID))//pop the person that has received all the stuff
            if(!ptr->length())//if there is no other person waiting to receive(task complete)
            {
                int i=0;
                for(;ptr->DataSource.DataSource[i];++i);
                fname = new char[i+1];
                for(i=0;ptr->DataSource.DataSource[i];++i)
                    fname[i]=ptr->DataSource.DataSource[i];
                fname[i]=0;

                //close reader and delete the pointers
                ptr->DataSource.CloseReader();//close the Reader
                printf("\nDeleting task from list since all users are done!:%s",fname);
                SendQ.erase(TaskID);//delete task
                //SendLock.unlock();
                return 2;//sender must be signaled to break and reiterate
            }
        //SendLock.unlock();
		return 1;
}
char SendStack::CancelTask(unsigned int TaskID,char * fname)
{
	//proper ending of task
	auto ptr = SendQ[TaskID];
	if(ptr)
	{
		int i=0;
		int len=0;
		for(;ptr->DataSource.DataSource[i];++len);
			fname = new char[len+1];
		for(i=0;ptr->DataSource.DataSource[i];++i)
			fname[i]=ptr->DataSource.DataSource[i];
		fname[i]=0;
		//close reader and delete the pointers
		//SendLock.lock();
		ptr->DataSource.CloseReader();//close the Reader
		printf("\nDeleting task from list since all users are done!");
		SendQ.erase(TaskID);//delete task
        //SendLock.unlock();

		return 2;//sender must be signaled to break and reiterate
	}

	return 0;
}
void SmartWriter::NormalizePath()
{
    if(lcp[0] && chosenLCP[0])
    {
        printf("\nOriginal Path:%s",DataSource);
        printf("\nLongest CPath:%s",lcp);
        printf("\nChosen c Path:%s",chosenLCP);
        int i = 0;
        for(i=0;i<MAX_FILESYS_PATH && lcp[i] && DataSource[i] && lcp[i] == DataSource[i];++i);
        int startP = i;
        for(;DataSource[i];++i);
        int len = i+1;
        char * newDataS = new char[strlen(chosenLCP)+len];
        //load chosenLCP
        for(i=0;chosenLCP[i] && i<MAX_FILESYS_PATH;++i)
            newDataS[i] = chosenLCP[i];
        newDataS[i]=0;
        printf("\nStep1:%s",newDataS);
        for(;i<MAX_FILESYS_PATH && DataSource[startP] && startP<MAX_FILESYS_PATH;++i)
            newDataS[i] = DataSource[startP++];
        newDataS[i]=0;
        printf("\nStep2:%s",newDataS);
        delete[] DataSource;
        DataSource = newDataS;
        printf("\nFINAL   Path:%s",DataSource);
    }
}
char SmartWriter::OpenWriter()
{
	if(isFile && DataSource && !isOpen)
	{
		//adapt path to local FS
        NormalizePath();
        printf("\nSmartWriter:Attempting to open write file:%s",DataSource);
		BuildPathToFile(DataSource);//create the necessary folders
		FD = fopen(DataSource,"wb");
		if(!FD)
			return false;
		isOpen=1;
	}
	return true;
}
char SmartWriter::OpenWriter(char * name,char isfile)//set to file and assign name
{
	isFile=isfile;
	if(isFile)
	{
		if(!isOpen)
		{
		    NormalizePath();
			printf("\nSmartWriter:Attempting to open write file:%s",name);
			BuildPathToFile(name);//create the necessary folders
			FD = fopen(name,"wb");
			if(!FD)
				return false;
			isOpen=true;
		}
	}
	//else
	//	DataSource = name;
	return true;
}
SmartReader::SmartReader()
{
	readerPos=0;
	FD=0;
	DataSource=0;
	DSlen=0;
	isOpen=false;
	isLoaded=0;
}
char SmartReader::OpenReader(char * name, char isfile)
{
	if(!isOpen)
	{
		isFile=isfile;
		if(isFile)
		{
			printf("\nOpening file! %s",name);
			FD = fopen(name,"rb");
			//buffer = new char[MAX_MTU-HDR_SIZE+1];
			readerPos = 0;
			if(!FD)//failed to open file
				return false;
			//Load file name
			DataSource = new char[strlen(name)+1];
			totalSeq=0;
			int i=0;
			for(i=0;name[i] && i<MAX_FILESYS_PATH;++i)
				DataSource[i]=name[i];
			DataSource[i]=0;
		}
		else
		{
			printf("\nSmart Reader ready for Preloading...");
			totalSeq=1;
			isLoaded = true;
			DataSource=new char[MAX_MTU-HDR_SIZE+1];
			DSlen=MAX_MTU-HDR_SIZE;
		}
		isOpen=true;
		return true;
	}
	return false;
}
char SmartReader::Preload(char * data,int len)
{
    printf("\nSmart Reader Preloading...");
    for(int i=0;i<len;++i)
        DataSource[i] = data[i];
    DSlen = len;
}
char SmartReader::get(unsigned int seqNo,char * data,unsigned short & length)
{
	if(isFile)
        return Load(seqNo,data,length);
	else
	{
		length = DSlen;
		for(int i=0;i<DSlen;++i)
            data[i] = DataSource[i];
        return 1;
	}
	return 0;
}
char SmartReader::LoadFile(int nrs)
{
	if(isOpen && isFile && !isLoaded)
	{
		//establish size
		int nrr=0;
		int len;
		char buffer[(MAX_MTU-HDR_SIZE)];
		for(int i=0;i<nrs;++i)
		{
			len=fread(buffer,(MAX_MTU-HDR_SIZE),1,FD);
			if(ferror (FD))//if error abort task
			{
				printf("\nGENERAL_SENDER:: ERROR while reading offered file!");
				fclose(FD);
				isLoaded=-1;
			}
			totalSeq++;
			if(feof(FD))
			{
				isLoaded=1;
				fseek(FD,0,SEEK_SET);//return file pointer to beginning
				printf("\nFile is ready for transfer:%s total sequences:%lld",DataSource,totalSeq);
				break;
			}
		}
	}
	return isLoaded;
}
bool SmartReader::Load(unsigned int seq,char * buffer,unsigned short & len)
{
	if(isOpen && isFile)
	{
	    readerPos = seq-1;
		fseek(FD,(long long)readerPos * (MAX_MTU-HDR_SIZE),SEEK_SET);
		len = fread(buffer,1,(MAX_MTU-HDR_SIZE),FD);
		readerPos = seq;
        return 1;
	}
	return 0;
}
void SmartReader::CloseReader()
{
if(isOpen)
{
	totalSeq=0;
	if(isFile)//close the file descriptor
	{
		printf("\nSmartReader Freeing memory");
		if(isLoaded>-1)
			fclose(FD);
		//delete FD;
	}
	delete[] DataSource;
	isOpen=false;
}
}
SmartReader::~SmartReader()
{
	CloseReader();
}
///////////////////////////////////////////END OF SENDER FUNTIONS
///////////////////////////////////RECEIVER FUNCTIONS/////////////////////////////////////
//////////PRIVATE FUNCTIONS//////////////////////////
List::List()
{
	size = MAX_MTU;
	max_length = 1500;//set back to 4 or 5
	list = 0;
	length = 0;
}
List::List(int packet_size)
{
	size = packet_size;
	max_length = 1500;//set back to 4 or 5
	list = 0;
	length = 0;
}
List::~List()
{
	//delete the whole list
	printf("\n List destructor called!");
	while(list)
		pop();
}
char List::AutoGap()
{

	if(!list)
		return 100;

	if(list && !(list->next)) //segmentation fault at this point for some reason
		return 100;//100%

	int diff = list->next->seq - list->seq;
	if(diff<0)
		diff=-diff;
	if(!diff)
		diff=1;
	return (max_length/(max_length-diff))*100;
}
char List::push(unsigned int place,unsigned int LastW, char * dta,unsigned short pklen,int dOffset)//pushes to subpath
{
	if( LastW < place)
	{
		if( abs((long long)place - LastW) < max_length )
		{
			if( length < max_length || !list )//if found
			{
				if( list != 0 )//null
				{
					//	printf("R");
					//append to root
					LinkedList *iterator = list;
					LinkedList *last = 0;
					while( iterator )//null
					{
						if(place < iterator->seq)
							break;
						if(place == iterator->seq)
							return true;
						last = iterator;
						iterator = iterator->next;
					}
					//printf("F");
					LinkedList * appendee = new LinkedList;
					appendee->data  = new char[pklen];
					appendee->plen  = pklen;
					appendee->seq   = place;
					appendee->next  = 0;//null
					////copy data
					for ( int i=0;i<pklen;++i)
						appendee->data[i]=dta[i+dOffset];
					//rebind list
					//printf("At");
					if(last)
					{
						if(!iterator)
						{
							appendee->next=0;
							last->next=appendee;
						//	printf("E");
						}
						else
						{
							appendee->next=iterator;
							last->next=appendee;
							//printf("M");
						}
					}
					else
					{
						appendee->next=iterator;
						list=appendee;
						//printf("S");
					}
					//return root;
				}
				else
				{
					//add the first element
					//printf(" NL ");
					list = new LinkedList();
					list->data = new char[pklen];
					for ( int i=0;i<pklen;++i)
						list->data[i]=dta[i+dOffset];
					list->next  = 0;//null
					list->plen  = pklen;
					list->seq   = place;
				}
				length++;
				//printf("D");
				return true;
			}
		}
	}
	return false;
}
void List::pop()
{
	if(list)
	{
		//popped first
		//printf("\nLIST POPED TOP");
		delete[] list->data;
		LinkedList * nxt = list->next;
		delete list;
		list=nxt;
		length--;
		if(!length)
			list=0;
	}
}

char * List::GetNext(unsigned int last,unsigned short & lng)
{
	if(list)
	{
		if(list->seq == last+1)
		{
			char * ret = new char[list->plen];
			for(int i=0;i<list->plen;++i)
				ret[i]=list->data[i];
			lng = list->plen;
			pop();
			return ret;
		}
		//else
			//printf("\n Sequence not continuous");
	}
	//else
	//	printf("\n Inexistent list");
	return 0;
}
char RecvStack:: CreateTask(unsigned long long UID, unsigned int TaskID,char * name, char isfile,unsigned int pktype,char * lcp)
{
	char *key = new char[RECV_KEY_LENGTH+1];////WARNING MAY CAUSE MEM LEAK
	CreateKey(key,UID,TaskID);
	if(!key[0])
		return GENERAL_ERROR;
	if(RecvQ.find(key)!=RecvQ.end())//task already exists
		return GENERAL_ERROR;

	printf("\nRECV_STACK:Creating new task...");
	boost::shared_ptr<RecvStruct> Data(new RecvStruct);
	printf("\nKey:%s",key);
	//preset paramteters to accept packets
	Data->TotalSeq=0;
	Data->PacketType=pktype;
	Data->LastWSeq=0;
	Data->Accepted=false;
	//open writer to be ready to write the packets
	Data->DataDest.PreloadWriter(name);
	Data->DataDest.SetLongestCommonPath(lcp);
	//pass the data structure to the Que
	//RecvLock.lock();
	RecvQ[key]=Data;
    //RecvLock.unlock();

    return SUCCESS;
}
char RecvStack::AcceptTask(unsigned long long uid,unsigned int tid,char * clcp)
{
	//printf("\n3Does the key exist?:%d",RecvQ.find(key)!=RecvQ.end());
	char key[RECV_KEY_LENGTH];
	CreateKey(key,uid,tid);
	printf("\nAttempting to accept task of key:%s",key);
	if(RecvQ.find(key)==RecvQ.end())//RecvQ.count(key))//task already exists
		return GENERAL_ERROR;
	//RecvLock.lock();
	RecvQ[key]->DataDest.SetDestinationCommonPath(clcp);
	if(!RecvQ[key]->DataDest.OpenWriter())
	{
		printf("\nSMART_WRITER:failed to create file on disk!");
		//RecvLock.unlock();
		return GENERAL_ERROR;
	}
	RecvQ[key]->Accepted = true;
	//RecvLock.unlock();
	return SUCCESS;
}
unsigned long long RecvStack::UpdateTask(PACKET & Data,char * finished)
{
	char key[RECV_KEY_LENGTH];
	CreateKey(key,Data.SendID,Data.StreamID);
	//check if tasks exists
	if(RecvQ.find(key)==RecvQ.end())
	{
		//printf("\nRECV_STACK::inexistent task");
		return GENERAL_ERROR;
	}
	//push the packet and update the parameters
 	auto Rbuff = RecvQ[key]; //reduce number of binary searches on the map by copying the pointer to the result
	char rez;
	unsigned long long ret=0;
	finished[0] = 0;
	if(Rbuff->Accepted)
	{
		//printf("\n*RECEIVER_STACK ID:%d SN:%d TS:%d",Data.StreamID,Data.SeqNo,Data.TotalSeq);
		if(!Rbuff->TotalSeq)//set the total number of expected sequences for this task from the first packet
		{
		    //RecvLock.lock();
			Rbuff->TotalSeq=Data.TotalSeq;
			Rbuff->TimeStarts=GetSysTime();
            //RecvLock.unlock();
		}
		//RecvLock.lock(); //this lock is preventive ( normally the same recv list will not be concurrently accessed )
		rez = Rbuff->PacketBuffer.push(Data.SeqNo,Rbuff->LastWSeq,Data.data,Data.PayloadSize,0);//HDR_SIZE);//push the packet
		//RecvLock.unlock();
		unsigned char ag=Rbuff->PacketBuffer.AutoGap();
		if(rez)
		{
            rez = AttemptWrite(Rbuff,key,finished);//attempt to write continuous segments
            if(rez)
            {
                ret = Rbuff->LastWSeq;
                ret <<=8;
                ret |=ag;
            }
            //else
            //	printf("\nSmart Writer could not write to disk");
		}
		//else
			//printf("\nRECV_STACK: could not push to buffer");
	}
	return ret;
}
char RecvStack::AttemptWrite(boost::shared_ptr<RecvStruct> Rbuff,char * key,char *finished)
{
	//this runs under the lock in the above function
	unsigned short len;
	char ret;
	while(Rbuff)
	{
		char * packet = Rbuff->PacketBuffer.GetNext(Rbuff->LastWSeq,len);
		if(packet)
		{
		    //RecvLock.lock();
			Rbuff->LastWSeq++;
			ret = Rbuff->DataDest.WriteNext(packet,len);
			//RecvLock.unlock();
			delete[] packet;
			if(!ret)
			{
				//printf("\n SMART WRITER CANNOT WRITE TO DISK");
				return GENERAL_ERROR;
			}
			if(Rbuff->LastWSeq == Rbuff->TotalSeq)//finished writting file
			{
				printf("\n**EVENT:New file received:%s",Rbuff->DataDest.DataSource);
				strcpy(finished,Rbuff->DataDest.DataSource);
				//now load the tasks in the file to RecvTask (by setting the load param to true)
				EndTask(key,Rbuff,Rbuff->PacketType==PACKET_STREAM_OFFER);//pop the task from the list and clean up
				return FINAL_SUCCESS;
            }
		}
		else
		{
			//printf("\n   RECV_STACK::Could not retrieve data from BUFFER");
			return GENERAL_ERROR;
		}
	}
	//printf("\nBad RBUFF address");
	return GENERAL_ERROR;
}
char RecvStack::LoadTasksToReceiver(char * filename,unsigned long long who)
{
	FILE *FD = fopen(filename,"rb");
	if(!FD)
	{
		printf("\nDEBUG: Could not open task file...");
		return 0;
	}
    unsigned int B;
	char CB[4];
	char FTP;//file type
	char intNRB=4;/////////////IF STREAM ID size changes this must change as well!!!!!!!!!!1
	char fname[MAX_FILESYS_PATH+5];
	char lcp[MAX_FILESYS_PATH+5];
	char cr;
	int nrb=0;//general index

	while(1)//read task by task
	{
		fread(CB,1,intNRB,FD);
		fread(&FTP,1,1,FD);
		B = StrToBin(CB,0,4);
		if(ferror (FD))//if error abort task
		{
			printf("\n Debug:ERROR while reading offer file!");
			fclose(FD);
			return 0;
		}
		if(feof(FD))
		{
			printf("\nDebug:End of offer file!");
			break;
		}
		if(B)
		{
			do
			{
				fread(&cr,1,1,FD);

				if(ferror (FD))//if error abort task
				{
					printf("\n Debug:ERROR while reading offer file!");
					fclose(FD);
					return 0;
				}
				if(feof(FD))
				{
					printf("\nDebug:End of offer file!");
					break;
				}

				if(nrb>=MAX_FILESYS_PATH)
				{
					printf("\n@!#%$%THIS FILE IS NOT OF CORRECT FORMAT! POSSIBLE SECURITY BREACH ATTEMPT");
					fclose(FD);
					return 0;
				}
				else
				{
					if(!cr)
					{
						//end of line
						fname[nrb]=0;
						//time to create the task
						////TO DELETE AFTER SPEED TEST
						//fname[0]='s';
						//fname[1]='.';
						//fname[2]='t';
						//fname[3]= 0;
						//////////////////////////////
						printf("\nDebug::LOADING task ID:%u File:%s len:%d",B,fname,nrb);
						CreateTask(who,B,fname,true,PACKET_STREAM_DATA,lcp);
						//now the longest common path must be added to all the created tasks
						//////////////////////////////WARNING : case when stack is full must be treated
						break;
					}
					else
						fname[nrb]=cr;
				}
				nrb++;
			}
			while(1);
		}
		else
        {
            //reached end of offer file
            //reading the longest common path
            nrb = 0;
            if(FTP)
            {
                printf("\nLoading longest common path...");
                do
                {
                    fread(&cr,1,1,FD);
                    if(ferror (FD))//if error abort task
                    {
                        fclose(FD);
                        return 0;
                    }
                    if(feof(FD))
                        break;

                    if(nrb<MAX_FILESYS_PATH)
                        lcp[nrb++] = cr;
                }
                while(cr);
                printf("\nlcp:%s",lcp);
            }
        }
		nrb = 0;
	}
	fclose(FD);
	return 1;
}
char RecvStack::EndTask(char * key,boost::shared_ptr<RecvStruct> Rbuff,char load)
{
	//clean up the list
	char fileName[MAX_FILESYS_PATH];
	unsigned long long who=0;
	unsigned long long mul=1;
	printf("\n Closing Smart Writer...");
	if(load)
	{
		int i;
		for( i=0;Rbuff->DataDest.DataSource[i] && i<MAX_FILESYS_PATH;++i)
			fileName[i]=Rbuff->DataDest.DataSource[i];
		fileName[i]=0;
		for( i=0;i<19 && key[i];++i)//////////////////////WARNING if user id size changes this must change as well!!!!!!!
		{
			who+=(key[i]-48)*mul;
			mul*=10;
		}
		printf("Identified Person:%lld",who);
	}
	//RecvLock.lock();
	Rbuff->DataDest.CloseWriter();//close the file writer
	//delete the record form the
	RecvQ.erase(key);
	//RecvLock.unlock();
	//now load the data in the file if it is set to load
	if(load)
	{
		//loading...
		printf("\nReceiver Stack loading tasks from file:%s",fileName);
		if(!LoadTasksToReceiver(fileName,who))
		{
			printf("\nFailed to load from file.");
			//////////MUST DELETE FILE HERE!
            return GENERAL_ERROR;
		}
		else
		{
			printf("\nSUCCESSFULLY LOADED TASKS(***MIKE's interface must be notified now)");
			//////////NOTIFY INTERFACE THAT TASKS ARE AVAILABLE
            return FINAL_SUCCESS;
		}
	}
	return SUCCESS;
}
char RecvStack::CancelTask(unsigned long long UID, unsigned int TaskID)
{
	char key[RECV_KEY_LENGTH];
	CreateKey(key,UID,TaskID);
	if(RecvQ.find(key)!=RecvQ.end())
		return EndTask(key,RecvQ[key],false);
	return GENERAL_ERROR;
}
bool RecvStack::IsAckReady(unsigned long long UID,unsigned int TaskID)
{
	char key[RECV_KEY_LENGTH];
	CreateKey(key,UID,TaskID);
	auto ptr = RecvQ[key];
	char ret = false;
	if(!ptr->TimeRecvd)
		ret = true;
	//RecvLock.lock();
	ptr->TimeRecvd = GetSysTime();
	//RecvLock.unlock();

	if( ret || GetTimestampDifference(ptr->TimeRecvd , GetSysTime()) > ACK_SEND_INTERV )
		return true;
	return false;
}
RingBuffer::RingBuffer()
{
	unsigned long long mem = GetAvailableMem();
	int allowed = (int)((double)(mem)*0.005);
	printf("\nTotal Mem:%lld Allowed mem %d",mem,allowed);
	if(allowed<MAX_MTU)
	{
		if(mem>MAX_MTU+1)
			allowed=MAX_MTU+1;
		else
		{
			printf("\nNot enough system memory!");
			length=0;
			return;
		}
	}

	length=allowed/MAX_MTU;
	//length=262929;
	printf("\nBuffer length:%d",length);

	Buffer  = new char * [length];
	dataLen = new unsigned short [length];
	start=end=0;
	usefulLength=0;

	for(int i=0;i<length;++i)
	{
		Buffer[i]=new char[MAX_MTU+1];
		dataLen[i]=0;
	}
}
RingBuffer::~RingBuffer()
{
	printf("\nFreeing buffer memory!");
	for(int i=0;i<length;++i)
		delete[] Buffer[i];
	delete[]  Buffer;
	delete[]  dataLen;
}
void RingBuffer::pop()
{
	start++;
	if(start==length)
		start=0;

	usefulLength--;
}
void RingBuffer::push(char * data,unsigned short len)
{
	if(length)
	{
		usefulLength++;
		if(usefulLength>length)
			pop();

		dataLen[end]=len;
		for(int i=0;i< len ;++i)
			Buffer[end][i]=data[i];

		end++;
		if(end==length)
			end=0;
	}
}
char RingBuffer::get( int pos,char * data,unsigned short & dlen)
{
	if(pos>=usefulLength)
	{
		//printf("\nDebug position higher than possible positions:%d available:%d",pos,usefulLength);
		dlen = 0;
		return 0;
	}

	int i=start+pos;
	if(i>=length)
		i-=length;

	dlen = dataLen[i];

	data = Buffer[i];
	//char * ret = new char[dlen];
	//for(int j=0;j<dlen;++j)
	//	ret[j]=Buffer[i][j];
	//return ret;
}
