#define _WIN32_WINNT 0x501
#include<windows.h>
#include<winsock.h>
#include<ws2tcpip.h>
#include<winsock2.h>
#include<wininet.h>
#include"InfoData.h"
#include<vector>
#include<stdlib.h>
#include<map>
struct ProgressBufferS
{
	static const int BUFFER_WAIT=0;
	static const int BUFFER_READY=1;
	static const int BUFFER_EMPTY=2;
string buf;
int bufIndx;
int bufState;
string result;
};
typedef ProgressBufferS ProgressBuffer;
	void getInitBuffer(ProgressBuffer *);
	int readNextIntoBuffer(string s,ProgressBuffer *);
class Distribution
{
private:
	static DWORD WINAPI start(LPVOID d);
	static DWORD WINAPI startRead(LPVOID recvProg);
	map<string,struct InfoData> infoSend;
	DWORD thrdId;
	void (*onRecv)(struct Progress *,int *);
	void (*onDisc)(struct Progress*);
	void (*onRdFrm)(struct Progress*,char*,int);
	static void sendInfoData(struct InfoData *,SOCKET*);
	static string getClientName(SOCKET *sc1);
public:
	static struct Progress* parseProgressData(char *data,int dlen,struct Progress *p,ProgressBuffer *);
	Distribution();
	int startListen();
	void onReceive(void (*onR)(struct Progress *,int *))
	{
		onRecv=onR;
	}
	void onClientDisconnect(void (*onDisc)(struct Progress *))
	{
		this->onDisc=onDisc;
	}
	void onReadFrom(void (*rdFrm)(struct Progress*,char *,int))
	{
		onRdFrm=rdFrm;
	}
	void addInfoData(string hostID,struct InfoData id1)
	{
		infoSend.insert(pair<string,struct InfoData>(hostID,id1));
	}
};
Distribution :: Distribution()
{
startListen();
}
int Distribution :: startListen()
{
HANDLE h1=CreateThread(NULL,0,start,(LPVOID)(this),0,&thrdId);
return (h1?1:0);
}
DWORD WINAPI Distribution :: start(LPVOID d)
{
	Distribution *dist=(Distribution*)d;
	WSAData wd1;
SOCKET lsn,*clnt;
struct addrinfo *r=NULL,hints;
int ir1;
ir1=WSAStartup(MAKEWORD(2,2),&wd1);
if(ir1!=0)
return 1;
ZeroMemory(&hints,sizeof(hints));
hints.ai_family=AF_INET;
hints.ai_socktype=SOCK_STREAM;
hints.ai_protocol=IPPROTO_TCP;
hints.ai_flags=AI_PASSIVE;
ir1=getaddrinfo(NULL,DISTRIBUTION_LISTEN_PORT,&hints,&r);
if(ir1!=0)
{
	WSACleanup();
	return 1;
}
lsn=socket(r->ai_family,r->ai_socktype,r->ai_protocol);
if(lsn==INVALID_SOCKET)
{
	freeaddrinfo(r);
	WSACleanup();
	return 1;
}
ir1=bind(lsn,r->ai_addr,(int)r->ai_addrlen);
if(ir1==SOCKET_ERROR)
{
	freeaddrinfo(r);
	closesocket(lsn);
	WSACleanup();
	return 1;
}
freeaddrinfo(r);
ir1=listen(lsn,SOMAXCONN);
if(ir1==SOCKET_ERROR)
{
	closesocket(lsn);
	WSACleanup();
	return 1;
}

while(1)
{
	struct sockaddr_in addr1;
	ZeroMemory(&addr1,sizeof(struct sockaddr_in));
	int sz=sizeof(addr1);

//clnt=accept(lsn,&addr1,&sz);
	clnt=new SOCKET;
*clnt=accept(lsn,NULL,NULL);
if(*clnt==INVALID_SOCKET)
{

	closesocket(lsn);
	WSACleanup();
	return 1;
}
getpeername(*clnt,(struct sockaddr*)&addr1,&sz);
DWORD tid;
struct ReceiveProgress *rp1=new ReceiveProgress;
rp1->sc1=clnt;
rp1->onRecv=dist->onRecv;
rp1->onDisc=dist->onDisc;
rp1->onReadFrom=dist->onRdFrm;
rp1->clientAddr=&addr1;
rp1->addrSize=sz;
rp1->infoSend=&(dist->infoSend);
cerr <<"Sending socket " <<*rp1->sc1 <<endl;
HANDLE h1=CreateThread(NULL,0,startRead,rp1,0,&tid);
if(!h1)
{
	closesocket(lsn);
	WSACleanup();
	return 1;
}
}
}
void getInitBuffer(ProgressBuffer *pb)
{
	pb->bufIndx=0;
	pb->bufState=ProgressBuffer::BUFFER_EMPTY;
}
int readNextIntoBuffer(string s,ProgressBuffer *p)
{

	int len=s.size();
	string findS=p->buf+s;
	int f1=findS.find(PROGRESS_DELIMITER);
	if(f1==-1)
	{
		(p->buf).insert(p->bufIndx,s,0,len);
		(p->bufIndx)+=len;
		p->bufState=ProgressBuffer::BUFFER_WAIT;
	}
	else
	{

		(p->buf).insert(p->bufIndx,findS,0,f1+1);
		p->result="";
		p->result.insert(0,p->buf,0,p->buf.size());
		if(f1<(len-string(PROGRESS_DELIMITER).size()))
		{
		string t;
		t.insert(0,findS,f1+string(PROGRESS_DELIMITER).size(),len-f1-string(PROGRESS_DELIMITER).size()-1);
		p->buf=t;
		p->bufIndx=len-f1-1;
		}
		else
		{
		p->buf="";
		p->bufIndx=0;
		}
		p->bufState=ProgressBuffer::BUFFER_READY;
	}

	return p->bufState;
}
struct Progress* Distribution :: parseProgressData(char *rd,int rlen,struct Progress *prog,ProgressBuffer *progBuf)
{
	rd[rlen]='\0';

	string data=string(rd);
	if(readNextIntoBuffer(data,progBuf)==ProgressBuffer::BUFFER_WAIT)
	{
		return NULL;
	}
	else
		data=progBuf->result;
	int len=data.size();
	string host;
	int i,k;
	for(i=0;i<len;i++)
	{
		if(data[i]=='\n')
			break;
		else
			host.insert(i,data,i,1);
	}
	if(i>=len)
		return NULL;
	string prgrs;
	for(k=i+1;k<len;k++)
	{
		if(data[k]=='\n')
			break;
		else
			prgrs.insert(k-i-1,data,k,1);
	}
	if(k>=len)
		return NULL;
	int progress=atoi(prgrs.c_str());
	string strtd,fnshd,cncld,err;
	strtd.insert(0,data,k+1,1);
	fnshd.insert(0,data,k+2,1);
	cncld.insert(0,data,k+3,1);
	err.insert(0,data,k+4,1);
	short started=(short)(atoi(strtd.c_str())),finished=(short)atoi(fnshd.c_str());
	short cancelled=(short)atoi(cncld.c_str());
	short error=(short)atoi(err.c_str());
	prog->started=started;
	prog->finished=finished;
	prog->cancelled=cancelled;
	prog->error=error;
	prog->progress=progress;
	prog->name=host;
	return prog;
	/*
	Message Format:
	Host(Alias Name)
	Progress
	StartedFinishedCancelledError
	*/
}
void Distribution :: sendInfoData(struct InfoData *id1,SOCKET *ss1)
{
	/*
InfoDataMessage Format:
DownloadID
Host
Path
rtype
fdata
Range-Lower Range-Upper
startstopsendMergeData
hdr
hdr
hdr...\r\n\r\n
*/
string ssend;
char *rstrt=new char[40];
char *rend=new char[40];
ltoa(id1->r.start,rstrt,10);
ltoa(id1->r.end,rend,10);
ssend=string(id1->downloadId)+"\n"+id1->host+"\n"+id1->path+"\n"+id1->rtype+"\n"+id1->fdata+"\n"+string(rstrt)+" "+string(rend)+"\n";
char *strt=new char[2],*stop=new char[2],*sendMergeData=new char[2];
strt[0]=(id1->start?'1':'0');
stop[0]=(id1->stop?'1':'0');
sendMergeData[0]=(id1->sendMergeData?'1':'0');
strt[1]='\0';
stop[1]='\0';
sendMergeData[1]='\0';
ssend=ssend+strt+string(stop)+sendMergeData+string("\n");
ssend=ssend+id1->hdrs+"\r\n\r\n";
int sz=ssend.size();
send(*ss1,ssend.c_str(),sz,0);
delete[] rstrt;
delete[] rend;
delete[] strt;
delete[] stop;
delete[] sendMergeData;
}
DWORD WINAPI Distribution :: startRead(LPVOID rp)
{
ProgressBuffer pb1;
getInitBuffer(&pb1);
	char rply[]="1";
struct ReceiveProgress *recvProg=(struct ReceiveProgress *)rp;
SOCKET *sc1=recvProg->sc1;
void (*onRecv)(struct Progress *,int *)=recvProg->onRecv;
void (*onDisc)(struct Progress *)=recvProg->onDisc;
void (*onReadFrom)(struct Progress*,char*,int)=recvProg->onReadFrom;
map<string,struct InfoData> *infoSend=recvProg->infoSend;
string clntName=getClientName(sc1);
if(clntName=="")
{
	cout <<"Name not given\n";
	closesocket(*sc1);
	return 1;
}
map<string,struct InfoData> :: iterator pos=infoSend->find(clntName);
if(pos==infoSend->end())
{
	cout <<"Name not found: " <<clntName <<"\n";
	closesocket(*sc1);
return 1;
}
else
{
	cout <<"Sending infodata to " <<clntName <<endl;
sendInfoData(&(pos->second),sc1);
}
struct sockaddr_in *clientAddr=recvProg->clientAddr;
//char *addr=new char[recvProg->addrSize+1];
//strcpy(addr,clientAddr->sa_data);
char *addr=inet_ntoa(clientAddr->sin_addr);
struct Progress prgrs;
prgrs.host=string(addr);
prgrs.started=0;
prgrs.finished=0;
prgrs.progress=0;
prgrs.cancelled=0;
prgrs.error=0;
int r1=0;
char *rd1=new char[UNIV_BUF_SIZE+1];

r1=recv(*sc1,rd1,UNIV_BUF_SIZE,0);
while(r1>0)
{
cout <<"-------------" <<rd1 <<"--------------" <<endl;
	struct Progress *prgrss=parseProgressData(rd1,r1,&prgrs,&pb1);

	int reply=0;
if(prgrss!=NULL)
cout <<prgrss->name <<endl;

	if(prgrss!=NULL)
	(*onRecv)(prgrss,&reply);
	if(reply)
	{
		send(*sc1,rply,strlen(rply),0);
	}
	if(prgrss!=NULL)
	if(prgrss->finished)
	{
		do
		{
			r1=recv(*sc1,rd1,UNIV_BUF_SIZE,0);
			if(r1>0)
			{
				(*onReadFrom)(prgrss,rd1,r1);
			}
		}
		while(r1>0);
	}
	else
	r1=recv(*sc1,rd1,UNIV_BUF_SIZE,0);
else
	r1=recv(*sc1,rd1,UNIV_BUF_SIZE,0);
}
if(r1<0)
	{
		closesocket(*sc1);
		prgrs.error=1;
		(*onDisc)(&prgrs);
	}
	return 0;
}
string Distribution :: getClientName(SOCKET *sc1)
{
	cerr <<"Getting Client Name for socket (" <<*sc1 <<")\n";
	string r="";
	char *rd=new char[2];
	cout <<(void*)rd <<endl;
	int r1=0;
	do
	{
	r1=recv(*sc1,rd,1,0);
	if(r1>0 && rd[0]!='@')
	{
		cout <<rd <<endl;
		rd[1]='\0';
		r=r+rd;
	}
	}
	while(r1>0 && rd[0]!='@');
	cout <<"Client Name " <<r <<endl;
	//delete[] rd;
	return r;
}
