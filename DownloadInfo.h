#define _WIN32_WINNT 0x501
#include<string>
//Download,Progress,Send
#include"InfoData.h"
#include<winsock.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<fstream.h>
using namespace std;
class DownloadInfo
{
private:
	string host;
	int reply;
	SOCKET sc1;
	int connected;
	int readInfoData();
	struct InfoData info;
public:
	DownloadInfo()
	{
		reply=-1;
		connected=0;
		host="";
	}
	DownloadInfo(string host);
	int connectToServer(string name);
	int isConnected()
	{
		return connected;
	}
	int sendProgress(struct Progress *p);
	int getReply();
	struct InfoData* getInfoData();
	int sendMergeData(char *data,int len);
};
DownloadInfo :: DownloadInfo(string host)
{
	reply=-1;
	connected=0;
	this->host=host;
}
int DownloadInfo :: connectToServer(string name)
{
	cout <<"Begining Connection:\n";
int ir1;
WSAData wd1;
sc1=INVALID_SOCKET;
struct addrinfo *r=NULL,*p=NULL,hints;
ir1=WSAStartup(MAKEWORD(2,2),&wd1);
if(ir1!=0)
	return 0;
ZeroMemory(&hints,sizeof(hints));
hints.ai_family=AF_UNSPEC;
hints.ai_socktype=SOCK_STREAM;
hints.ai_protocol=IPPROTO_TCP;
ir1=getaddrinfo(host.c_str(),DISTRIBUTION_LISTEN_PORT,&hints,&r);
if(ir1!=0)
{
	WSACleanup();
	return 0;
}
for(p=r;p!=NULL;p=p->ai_next)
{
sc1=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
if(sc1==INVALID_SOCKET)
{
	WSACleanup();
	return 0;
}
ir1=connect(sc1,p->ai_addr,(int)p->ai_addrlen);
if(ir1==SOCKET_ERROR)
{
	closesocket(sc1);
	sc1=INVALID_SOCKET;
	continue;
}
break;
}
freeaddrinfo(r);
if(sc1==INVALID_SOCKET)
{
	WSACleanup();
	return 0;
}
cout <<"Connected to Server\n";
name=name+"@";
cout <<"Sending name " <<name <<endl;
if(send(sc1,name.c_str(),name.size(),0)!=SOCKET_ERROR)
{
	cout <<"Sent Name\n";
if(readInfoData())
{
connected=1;
return 1;
}
else
{
	cout <<"Unable to read Info Data\n";
	return 0;
}
}
else
{
	cout <<"Error in sending\n";
	return 0;
}
}
int DownloadInfo :: sendProgress(struct Progress *p)
{
	int ir1;
	char *rd1=new char[2];
char *name=new char[p->name.size()+1];
strcpy(name,p->name.c_str());
char *progress=new char[32];
itoa(p->progress,progress,10);
char *sendData=new char[strlen(name)+strlen(progress)+11];
strcpy(sendData,name);
strcat(sendData,"\n");
strcat(sendData,progress);
strcat(sendData,"\n");
int len=strlen(sendData);
sendData[len]=(p->started?'1':'0');
sendData[len+1]=(p->finished?'1':'0');
sendData[len+2]=(p->cancelled?'1':'0');
sendData[len+3]=(p->error?'1':'0');
sendData[len+4]='\0';
strcat(sendData,"\r\n\r\n");
ir1=send(sc1,sendData,strlen(sendData),0);
cout <<sendData <<endl;
if(ir1==SOCKET_ERROR)
{
	delete[] rd1;
	delete[] name;
	delete[] progress;
	delete[] sendData;
	closesocket(sc1);
	WSACleanup();
	return 0;
}
ir1=recv(sc1,rd1,1,0);
if(ir1<=0)
{
	delete[] rd1;
	delete[] name;
	delete[] progress;
	delete[] sendData;
	closesocket(sc1);
	WSACleanup();
	return 0;	
}
reply=rd1[0]-48;
return 1;
}
struct InfoData* DownloadInfo :: getInfoData()
{
	return &info;
}
int DownloadInfo :: readInfoData()
{
string buf;
char *rd=new char[UNIV_BUF_SIZE+1];
int r1=recv(sc1,rd,UNIV_BUF_SIZE,0);
cout <<"InfoData:\n";
while(r1>0)
{
	rd[r1]='\0';
	cout <<rd;
	buf=buf+rd;
	if(buf.find("\r\n\r\n")!=-1)
		break;
	r1=recv(sc1,rd,UNIV_BUF_SIZE,0);
}
	if(r1<=0)
		return 0;
cout <<"Done Reading\n";
cout <<"Response: " <<buf <<endl;
int ppos=0;
int pos=buf.find("\n");
string id;
id.insert(0,buf,ppos,pos);
info.downloadId=new char[id.size()+1];
strcpy(info.downloadId,id.c_str());
pos++;
ppos=pos;
pos=buf.find("\n",pos);
info.host.insert(0,buf,ppos,pos-ppos);
pos++;
ppos=pos;
pos=buf.find("\n",pos);
info.path.insert(0,buf,ppos,pos-ppos);
pos++;
ppos=pos;
pos=buf.find("\n",pos);
info.rtype.insert(0,buf,ppos,pos-ppos);
pos++;
ppos=pos;
pos=buf.find("\n",pos);
info.fdata.insert(0,buf,ppos,pos-ppos);
pos++;
ppos=pos;
pos=buf.find(" ",pos);
string rlower,rupper;
rlower.insert(0,buf,ppos,pos-ppos);
info.r.start=atoi(rlower.c_str());
pos++;
ppos=pos;
pos=buf.find("\n",pos);
rupper.insert(0,buf,ppos,pos);
info.r.end=atoi(rupper.c_str());
info.start=(buf[++pos]=='1'?1:0);
info.stop=(buf[++pos]=='1'?1:0);
info.sendMergeData=(buf[++pos]=='1'?1:0);
pos+=2;
ppos=pos;
pos=buf.find("\r\n\r\n",pos);
info.hdrs.insert(0,buf,ppos,pos-ppos);
cout <<"Done Parsing\n";
return 1;
}
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
int DownloadInfo :: sendMergeData(char *data,int len)
{	
int r1=send(sc1,data,len,0);
return (r1!=SOCKET_ERROR);
}
