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
#include"DownloadInfo.h"
#include"PartialDownload.h"
using namespace std;
void download(struct InfoData &,struct Progress &,DownloadInfo *);
void sendProgress(struct Progress *);
void writeDownloadInfo(string file,struct InfoData *);
int readDownloadInfo(string file,struct InfoData *);
void showHelpMsg();
DWORD fileSize(string file);
int main(int argc,char *argv[])
{
	string infoDataFile;
	struct InfoData *info;
	if(argc<=2)
	{
		showHelpMsg();
		return 0;
	}
	else
	{
		string opt=argv[1];
	DownloadInfo di(argv[2]);
	if(!di.connectToServer(argv[3]))
	{
		cout <<"Error: Cannot Connect To Server\n" <<endl;
		if(opt=="-n")
		return 1;
	}
	if(opt=="-l" && argc>=5)
	{
		info=new struct InfoData;
		readDownloadInfo(argv[4],info);
	long size=fileSize(string(argv[3])+info->downloadId);
	info->r.start+=size;
	}
	else if(opt=="-n" && argc>=4)
	{
	info=di.getInfoData();	
		if(argc>=5)
			writeDownloadInfo(argv[4],info);
	}
	else
	{
	cout <<"Error: Invalid Arguments Passed\n";
	showHelpMsg();
	return 1;
	}
	cout <<"Got InfoData\n";
	struct Progress p;
	p.host=argv[2];
	p.name=argv[3];
	p.started=0;
	p.finished=0;
	p.progress=0;
	p.cancelled=0;
	p.error=0;
	download(*info,p,&di);
	return 0;
		
		}
}
void download(struct InfoData &idata,struct Progress &p,DownloadInfo *d)
{
	p.started=1;
p.finished=0;
p.progress=0;
p.cancelled=0;
p.error=0;
cout <<"Got Info Data:: :: " <<idata.host <<" " <<idata.path <<" " <<idata.r.start <<" " <<idata.r.end <<endl;
if(idata.r.start>idata.r.end)
{
	if(idata.r.start>idata.r.end+1)
	{
	cout <<"Error:Infodata File Corrupted or wrong Range Info\n";
	return;
	}
	else
		cout <<"Download Already Completed\nJust need to Send Now\n";
}
else
{
	PartialDownloader pd(idata.host,idata.path);
pd.setRange(&(idata.r));
pd.setHeaders(idata.hdrs);
pd.setRequestType(idata.rtype);
pd.setFormData(idata.fdata);
pd.startDownload(p.name+idata.downloadId);
cout <<"Downloaded Started\n";
int prog=pd.getProgress();
while(pd<100)
{
	if(prog!=pd.getProgress())
	{
		prog=pd.getProgress();
		cout <<"Progress " <<prog <<endl;
		p.progress=prog;
		d->sendProgress(&p);
	}
}
}
p.finished=1;
p.progress=100;
if(!d->sendProgress(&p))
	return;
//cout <<"Progress " <<p.name <<" " <<pd.getProgress() <<endl;
cout <<"Sent Progress\n";
cout <<"Sending Data\n";
ifstream in((p.name+idata.downloadId).c_str(),ios :: in | ios :: binary);
int rd=0;
char *readData=new char[UNIV_BUF_SIZE];
do
{
in.read(readData,UNIV_BUF_SIZE);
rd=in.gcount();
if(rd>0)
	d->sendMergeData(readData,rd);
}
while(rd>0);
cout <<"Completed\n";
}
void writeDownloadInfo(string file,struct InfoData *dInfo)
{
ofstream out(file.c_str());
out <<dInfo->host.c_str() <<"\n" <<dInfo->path.c_str() <<"\n" <<dInfo->rtype.c_str() <<"\n" <<dInfo->fdata.c_str() <<"\n";
out <<dInfo->r.start <<"\n" <<dInfo->r.end <<endl;
out <<dInfo->downloadId;
out <<"\n" <<dInfo->hdrs.c_str();
out.close();
}
int readDownloadInfo(string file,struct InfoData *dInfo)
{
	char *buf=new char[UNIV_BUF_SIZE];
ifstream in(file.c_str());
in.getline(buf,UNIV_BUF_SIZE);
dInfo->host=buf;
in.getline(buf,UNIV_BUF_SIZE);
dInfo->path=buf;
in.getline(buf,UNIV_BUF_SIZE);
dInfo->rtype=buf;
in.getline(buf,UNIV_BUF_SIZE);
dInfo->fdata=buf;
in.getline(buf,UNIV_BUF_SIZE);
dInfo->r.start=atoi(buf);
in.getline(buf,UNIV_BUF_SIZE);
dInfo->r.end=atoi(buf);
in.getline(buf,UNIV_BUF_SIZE);
dInfo->downloadId=new char[strlen(buf)+1];
strcpy(dInfo->downloadId,buf);
while(!in.eof())
{
in.getline(buf,UNIV_BUF_SIZE);
dInfo->hdrs=dInfo->hdrs+(dInfo->hdrs.size()==0?"":"\r\n")+buf;
}
delete[] buf;
}
void showHelpMsg()
{
		cout <<"\t\tGroup Downloader Client:\n";
		cout <<"DClient -n/l host aliasname [info_data_file]\n";
		cout <<"Options:\n";
		cout <<"n:New Download\n";
		cout <<"l:Load(Resume) Download From File\n";
}
DWORD fileSize(string file)
{
DWORD size=0;
WIN32_FIND_DATA fd1;
HANDLE handle=FindFirstFile(file.c_str(),&fd1);
if(handle!=INVALID_HANDLE_VALUE)
size=fd1.nFileSizeLow;
else
cout <<"File Not Found\n";
return size;
}
