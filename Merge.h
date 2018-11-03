#include<iostream.h>
#include<fstream.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<windows.h>
#define BSIZE 1024
class Merger
{
protected:
	char *file;
	void createRandomName(char rName[],int len,char *appnd);
void addFile(ofstream *out,ifstream *in);
public:
	Merger()
	{
		file=NULL;
	}
	Merger(char *file);
	void setFile(char *file)
	{
		this->file=file;
	}
	void renameTo(char *name);
	void prependFile(char *file);
	void appendFile(char *file);
	Merger operator+(char *file);
	char* getFile();
};
Merger :: Merger(char *file)
{
	this->file=file;
}
Merger Merger :: operator+(char *file)
{
appendFile(file);
return *this;
}
void Merger :: createRandomName(char rName[],int len,char *appnd)
{
//randomize();
for(int i=0;i<len;i++)
{
	int n=rand();
	while(n>=10)
		n/=10;
	cout <<n <<endl;
	rName[i]=(char)(n+48);
}
rName[len]='\0';
strcat(rName,appnd);
}
void Merger :: prependFile(char *file)
{
	cout <<file <<" " <<(this->file) <<endl;
	char rn[24];
	createRandomName(rn,20,".dat");
	ofstream out(rn,ios :: out | ios :: binary);
	ifstream in(file,ios :: in | ios :: binary);
	ifstream in1(this->file,ios :: in | ios :: binary);
	addFile(&out,&in);
	addFile(&out,&in1);
	in.close();
	in1.close();
	out.close();
	DeleteFile(this->file);
	DeleteFile(file);
	MoveFile(rn,this->file);
	}
void Merger :: addFile(ofstream *out,ifstream *in)
{
	int r=0;
	char *rd1=new char[BSIZE];
	do
	{
		if(r>0)
		{
out->write(rd1,r);
		}
	in->read(rd1,BSIZE);
	r=in->gcount();
	}
	while(r>0);
in->close();
delete rd1;
}
void Merger :: appendFile(char *file)
{
	ofstream out(this->file,ios :: out | ios :: binary | ios :: app);
	ifstream in(file,ios :: in | ios :: binary);
	addFile(&out,&in);
	in.close();
	out.close();
}
void Merger :: renameTo(char *name)
{
MoveFile(file,name);
}
class DMerger : public Merger
{
public:
	DMerger()
	{
		this->file=NULL;
	}
	DMerger(char *file);
	void appendData(char *data,int l);
	void prependData(char *data,int l);
};
DMerger :: DMerger(char *file)
{
	this->file=file;
}
void DMerger :: appendData(char *data,int l)
{
ofstream out(file,ios :: out | ios :: binary | ios :: app);
out.write(data,l);
out.close();
}
void DMerger :: prependData(char *data,int l)
{
	char rN[24];
	createRandomName(rN,20,".dat");
	cout <<rN <<endl;
ofstream out(rN,ios :: out | ios :: binary);
out.write(data,l);
ifstream in(file,ios :: in | ios :: binary);
addFile(&out,&in);
in.close();
out.close();
DeleteFile(this->file);
MoveFile(rN,this->file);
}
Merger operator+(char *file,Merger m)
{
m.prependFile(file);
return m;
}
