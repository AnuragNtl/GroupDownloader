#include<iostream.h>
#include<iomanip.h>
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>
#include<process.h>
#include<fstream.h>
#include<dos.h>
#include<dir.h>
#include<time.h>
#include<io.h>
#include<direct.h>
#include<windows.h>
#include"InfoData.h"
#include"Distribution.h"
#include"PartialDownload.h"
#include"Merge.h"
void onRecv(struct Progress *,int *);
void onDisc(struct Progress *);
struct PartFileData
{
int length;
int written;
};
vector<string>* splitValues(string);
vector<string> hosts;
vector<struct InfoData> hostInfoData;
map<string,struct Progress> hostProgress;
map<string,ofstream*> partFiles;
map<string,struct PartFileData> partFileProgress;
HANDLE progressMutex;
int cnttLen;
void displayStats();
short nFinished=0;
void onReadFrom(struct Progress *p,char *data,int len)
{
	map<string,ofstream*> :: iterator findHost=partFiles.find(p->name);
if(findHost==partFiles.end())
{
	ofstream *out=new ofstream((p->name+".partFile").c_str(),ios :: out | ios :: binary);
	partFiles.insert(pair<string,ofstream*>(p->name,out));
	findHost=partFiles.find(p->name);
}
ofstream *out=findHost->second;
struct PartFileData *pd1=&partFileProgress.find(p->name)->second;
if(pd1->written>=pd1->length)
return;
out->write(data,len);
out->flush();
pd1->written+=len;
if(pd1->written>=pd1->length)
{
	out->close();
	partFiles.erase(findHost);
	delete out;
}
	//cout <<pd1->written <<" " <<pd1->length <<endl;
}
void showMsg();
string createRandomId(int);
int main(int argc,char *argv[])
{
	string targetServer,targetPath,reqType,frmData;
	vector<string> reqHeaders;
	progressMutex=CreateMutex(NULL,FALSE,NULL);
	if(argc==1)
		showMsg();
	else if(argc==2)
	{
		//cout <<"Please Specify targetServer\n";
		showMsg();
	}
	else
	{
			hosts=*splitValues(argv[1]);
		switch(argc)
		{
			case 3:
			targetServer=argv[2];
			break;
			case 4:
			targetServer=argv[2];
			targetPath=argv[3];
			break;
			case 5:
			targetServer=argv[2];
			targetPath=argv[3];
			reqType=argv[4];
			break;
			case 6:
			targetServer=argv[2];
			targetPath=argv[3];
			reqType=argv[4];
			reqHeaders=*splitValues(argv[5]);
			break;
			case 7:
			targetServer=argv[2];
			targetPath=argv[3];
			reqType=argv[4];
			reqHeaders=*splitValues(argv[5]);
			frmData=argv[6];
			break;
		}
	}
	if(argc<=2)
		return 0;
	int rHdrSize=reqHeaders.size();
	//cout <<rHdrSize <<endl;
	string eachHostHeaders="";
	for(int i=0;i<rHdrSize;i++)
	{
		if(i<rHdrSize-1)
		eachHostHeaders=eachHostHeaders+reqHeaders[i]+"\n";
	else
		eachHostHeaders=eachHostHeaders+reqHeaders[i];
	}
	eachHostHeaders=eachHostHeaders+"\r\n\r\n";
	//cout <<eachHostHeaders <<endl;
	int hostNumber=hosts.size();
	hostInfoData=vector<struct InfoData>(hostNumber);
	cnttLen=getContentLength(targetServer,targetPath,"Host: "+targetServer,"");
	if(cnttLen==-1)
	{
		cerr <<"Could not find the length of the content from the server\n";
		return 1;
	}
	//cout <<cnttLen <<endl;
	int eachLength=cnttLen/hostNumber;
	Distribution downloadDistribute;
	for(int i=0,k=0;i<hostNumber;i++,k+=eachLength)
	{
	hostInfoData[i].type=1;
	hostInfoData[i].host=targetServer;
	hostInfoData[i].path=targetPath;
	hostInfoData[i].rtype=reqType;
	hostInfoData[i].hdrs=eachHostHeaders;
	hostInfoData[i].fdata=frmData;
	hostInfoData[i].downloadId="DownloadID";
	hostInfoData[i].start=0;
	hostInfoData[i].stop=0;
	hostInfoData[i].sendMergeData=0;
	hostInfoData[i].r.start=k;
	if(i<hostNumber-1)
	hostInfoData[i].r.end=k+eachLength-1;
	else
	hostInfoData[i].r.end=cnttLen-1;
	//cout <<hosts[i] <<":" <<hostInfoData[i].r.start <<"-" <<hostInfoData[i].r.end <<"\n";
	struct PartFileData pd1;
	pd1.length=hostInfoData[i].r.end-hostInfoData[i].r.start+1;
	pd1.written=0;
	partFileProgress.insert(pair<string,struct PartFileData>(hosts[i],pd1));
	downloadDistribute.addInfoData(hosts[i],hostInfoData[i]);
	}
	downloadDistribute.onReceive(onRecv);
	downloadDistribute.onClientDisconnect(onDisc);
	downloadDistribute.onReadFrom(onReadFrom);
	while(1)
	{
		system("cls");
		displayStats();
		cout <<nFinished <<" " <<partFiles.size() <<endl;
		map<string,ofstream*> :: iterator it1=partFiles.begin();
		while(it1!=partFiles.end())
		{
			cout <<it1->first <<endl;
			it1++;
		}
			if(nFinished==hostNumber && partFiles.size()==0)
			break;
		Sleep(1000);
	}
	char *rd1=new char[MAX_PATH];
	strcpy(rd1,(hosts[0]+".partFile").c_str());
	Merger merger1(rd1);
			cout <<rd1 <<(void*)rd1 <<endl;
	for(int i=1;i<hostNumber;i++)
	{
		rd1=new char[MAX_PATH];
		strcpy(rd1,(hosts[i]+".partFile").c_str());
		cout <<rd1 <<(void*)(rd1) <<endl;
		merger1=merger1+rd1;
	}
	merger1.renameTo("DownloadedFile.dat");
	return 0;
}
void onRecv(struct Progress *p,int *re)
{
	//cout <<"Received From " <<p->name <<endl;
DWORD waitResult=WaitForSingleObject(progressMutex,INFINITE);
if(waitResult==WAIT_OBJECT_0)
{
	map<string,struct Progress> :: iterator fd=hostProgress.find(p->name);
if(fd==hostProgress.end())
{
hostProgress.insert(pair<string,struct Progress>(p->name,*p));
}
else
{
fd->second=*p;
}
if(p->finished)
nFinished++;
*re=1;
ReleaseMutex(progressMutex);
}
//cout <<"Done Receive\n";
}
void onDisc(struct Progress *p)
{
	//cout <<"Disconnected " <<(p->name) <<"\n";
}
void showMsg()
{
cout <<"\t\tDistribution Server:\n";
cout <<"\t\tUsage:\n";
cout <<"DServer  hostalias1,hostalias2... targetServer [targetPath requestType requestHeader1,requestHeader2... formData]\n";
}
vector<string>* splitValues(string hsts)
{
	vector<string> *r=new vector<string>;
int len=hsts.size();
int ppos=-1;
int pos=hsts.find(",");
while(pos!=-1)
{
	string s;
	s.insert(0,hsts,ppos+1,pos-ppos-1);
	r->push_back(s);
	ppos=pos;
pos=hsts.find(",",pos+1);
}
string s;
s.insert(0,hsts,ppos+1,len-ppos-1);
r->push_back(s);
return r;
}
string createRandomId(int len)
{
return "";
}
void displayStats()
{
	cout <<"Total Content Length: " <<cnttLen <<"\n";
	int hostsSize=hosts.size();
	cout <<setw(12) <<"Host Name " <<setw(16) <<" Progress " <<setw(20) <<"Range\n";
for(int i=0;i<hostsSize;i++)
{
	cout <<setw(12) <<hosts[i] <<" ";
	map<string,struct Progress> :: iterator fd=hostProgress.find(hosts[i]);
	if(fd==hostProgress.end())
	cout <<setw(16) <<"Not Connected";
	else
		cout <<setw(16) <<fd->second.progress <<(fd->second.finished?"Fs":"Dw");
	cout <<setw(20) <<hostInfoData[i].r.start <<"-" <<hostInfoData[i].r.end <<"\n";
}
}
