#include<string>
#include<winsock.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<wininet.h>
#include<stdlib.h>
#include<string.h>
#include<iostream.h>
#include<fstream.h>
#ifndef INFO_DATA
#include"Range.h"
#endif
using namespace std;
int getContentLength(string host,string path);
class Downloader
{
protected:
	static DWORD WINAPI start(LPVOID d);
	string host,path;
	string file;
	string hdrs,frm,rtype;
	int flen,downloaded;
	DWORD thrdId;
	int rcode;
	int connected;
	string *data;
	int downloadToData;
	int completd;
	void setHostAndPath(string url);
public:
		static const int BUF_SIZE=1024;
	Downloader()
	{
	downloadToData=0;
	completd=0;
	flen=-1;
	downloaded=0;
	}
	Downloader(string url);
	Downloader(string host,string path);
	void setUrl(string url);
	void setHost(string host);
	void setPath(string path);
	int startDownload(string file);
	int startDownload(string *data);
	void setHeaders(string hdrs);
	void setFormData(string frm);
	void setRequestType(string rtype);
	int getResponseCode();
	int getProgress();
	int getLength()
	{
		return flen;
	}
	int isConnected()
	{
		return connected;
	}
	int operator<(int pr)
	{
		return (getProgress()<pr);
	}
	int operator>(int pr)
	{
		return (getProgress()>pr);
	}
	int isCompleted()
	{
		return completd;
	}
	string getHost()
	{
		return host;
	}
	string getPath()
	{
		return path;
	}
};
Downloader :: Downloader(string url)
{
	flen=-1;
	downloaded=0;
	downloadToData=0;
	completd=0;
	setHostAndPath(url);
}
Downloader :: Downloader(string host,string path)
{
	flen=-1;
	downloaded=0;
	downloadToData=0;
	completd=0;
	this->host=host;
	this->path=path;
}
void Downloader :: setUrl(string url)
{
	setHostAndPath(url);
}
void Downloader :: setHost(string h)
{
host=h;
}
void Downloader :: setPath(string p)
{
	path=p;
}
void Downloader :: setRequestType(string r)
{
	rtype=r;
}
void Downloader :: setHeaders(string h)
{
	hdrs=h;
}
void Downloader :: setFormData(string f)
{
	this->frm=f;
}
void Downloader :: setHostAndPath(string url)
{
	int l1=url.size();
	if(l1<=7)
		return;
			string s1;
			s1.insert(0,url,0,7);
			if(s1!="http://")
				return;
			string s4="";
			int hostset=0; 
			for(int i=7;i<l1;i++)
			{
				if((url[i]=='/') && !hostset)
				{
					host=s4;
					s4="";
					hostset=1;
				}
				else
				{
					s4=s4+url[i];
				}
			}
			if(hostset)
			path="/"+s4;
}
int Downloader :: startDownload(string file)
{
	completd=0;
	this->file=file;
	HANDLE h1=CreateThread(NULL,0,start,(LPVOID)(this),0,&thrdId);
	return (h1?1:0);
}
int Downloader :: startDownload(string *data)
{
	completd=0;
	this->data=data;
	downloadToData=1;
	HANDLE h1=CreateThread(NULL,0,start,(LPVOID)(this),0,&thrdId);
	return (h1?1:0);
}
DWORD WINAPI Downloader :: start(LPVOID d)
{
	Downloader *pd1=(Downloader*)d;
HINTERNET h1,con,req;
do
{
h1=InternetOpen("Partial Downloader",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
}
while(!h1);
do
{
	const char *host1=pd1->host.c_str();
con=InternetConnect(h1,host1,INTERNET_DEFAULT_HTTP_PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,1);
}
while(!con);
pd1->connected=1;
const char *reqtype=pd1->rtype.c_str(),*path1=pd1->path.c_str();
req=HttpOpenRequest(con,reqtype,path1,NULL,NULL,NULL,0,1);
if(req)
{
	const char *hdrs=pd1->hdrs.c_str(),*fdata=pd1->frm.c_str();
	char *hd,*fd;
	hd=new char[strlen(hdrs)+1];
	fd=new char[strlen(fdata)+1];
	strcpy(hd,hdrs);
	strcpy(fd,fdata);
int rsend=HttpSendRequest(req,hd,strlen(hd),fd,strlen(fd));
	if(rsend)
	{
		char *flenS=new char[100];
		strcpy(flenS,"Content-Length");
		DWORD fsz=99;
		if(HttpQueryInfo(req,HTTP_QUERY_CUSTOM,(LPVOID)flenS,&fsz,NULL))
			pd1->flen=atoi(flenS);
		else
			pd1->flen=-1;
		delete[] flenS;
		ofstream out;
		char *rd1=new char[BUF_SIZE];
		DWORD bRead=0;
		int r1=0;
		const char *file=pd1->file.c_str();
		if(pd1->downloadToData)
			pd1->data=new string;
			else
		out.open(file,ios :: out | ios :: binary | ios :: app);
		pd1->downloaded=0;
		do
		{
		r1=InternetReadFile(req,rd1,BUF_SIZE,&bRead);
		if(bRead>0)
		{
			if(pd1->downloadToData)
				*(pd1->data)=*(pd1->data)+rd1;
				else
			out.write(rd1,bRead);
		pd1->downloaded+=bRead;
		}
		}
		while(r1 && (bRead>0));
		if(!(pd1->downloadToData))
		out.close();
	pd1->completd=1;
	}
}
}
int Downloader :: getResponseCode()
{
	return rcode;
}
int Downloader :: getProgress()
{
	return (downloaded*100)/flen;
}
class PartialDownloader : public Downloader
{
protected:
	struct RangeS *r;
public:
	PartialDownloader(Range *r);
	PartialDownloader(string url);
	PartialDownloader(string host,string path) : Downloader(host,path)
	{	
		flen=-1;
	downloaded=0;
	}
	void setRange(Range *r)
	{
		this->r=r;
	}
	int startDownload(string file);
	int startDownload(string *data);
};
PartialDownloader :: PartialDownloader(Range *r)
{
this->r=r;
	flen=-1;
	downloaded=0;
}
PartialDownloader :: PartialDownloader(string url) : Downloader(url){}
int getContentLength(string host,string path,string hdrs,string fdata)
{
int cnttLen;
WSAData wd1;
SOCKET sc1=INVALID_SOCKET;
struct addrinfo *r=NULL,*p=NULL,hints;
int ir1=WSAStartup(MAKEWORD(2,2),&wd1);
if(ir1!=0)
	return -1;
ZeroMemory(&hints,sizeof(hints));
hints.ai_family=AF_UNSPEC;
hints.ai_socktype=SOCK_STREAM;
hints.ai_protocol=IPPROTO_TCP;
ir1=getaddrinfo(host.c_str(),"80",&hints,&r);
for(p=r;p!=NULL;p=p->ai_next)
{
	sc1=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
	if(sc1==INVALID_SOCKET)
	{
		WSACleanup();
		return -1;
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
if(sc1==INVALID_SOCKET)
{
	WSACleanup();
	return -1;
}
string sendHd="HEAD "+path+" HTTP/1.1\r\n"+hdrs+"\r\n\r\n"+fdata;
if(send(sc1,sendHd.c_str(),sendHd.size(),0)==SOCKET_ERROR)
{
	closesocket(sc1);
	WSACleanup();
	return -1;
}
char *rd1=new char[UNIV_BUF_SIZE+1];
ir1=recv(sc1,rd1,UNIV_BUF_SIZE,0);
if(ir1>0)
{
	rd1[ir1]='\0';
	string hdrsData=rd1;
	string cnttLenHdr="Content-Length: ";
	int hdrFind=hdrsData.find(cnttLenHdr);
	if(hdrFind>=0)
	{
		string hData;
		int lt=hdrsData.find("\r\n",hdrFind+cnttLenHdr.size());
		hData.insert(0,hdrsData,hdrFind+cnttLenHdr.size(),lt-hdrFind-cnttLenHdr.size());
		return atoi(hData.c_str());
	}
	else
	{
		closesocket(sc1);
		WSACleanup();
		return -1;
	}
}
else
{
closesocket(sc1);
WSACleanup();
return -1;
}
}
int PartialDownloader :: startDownload(string file)
{
	char st[10],e[10];
	ltoa(r->start,st,10);
	ltoa(r->end,e,10);
	setHeaders(string("Range: bytes=")+st+string("-")+e);
		completd=0;
	this->file=file;
	HANDLE h1=CreateThread(NULL,0,start,(LPVOID)(this),0,&thrdId);
	return (h1?1:0);
}
int PartialDownloader :: startDownload(string *data)
{
		char st[10],e[10];
	ltoa(r->start,st,10);
	ltoa(r->end,e,10);
	setHeaders(string("Range: bytes=")+st+string("-")+e);
		completd=0;
	this->data=data;
	downloadToData=1;
	HANDLE h1=CreateThread(NULL,0,start,(LPVOID)(this),0,&thrdId);
	return (h1?1:0);	
}
