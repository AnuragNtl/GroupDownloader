#include<fstream.h>
class Splitter
{
private:
	char *data;
	int l;
protected:
	char* getData(char *data,int l);
public:
	Splitter()
	{
		data=NULL;
	}
	Splitter(char data[],int l);
	void setData(char data[],int l);
	char** splitInto(int n,int *len,int *lastlen);
	int getLength();
};
Splitter :: Splitter(char data[],int l)
{
	this->data=data;
	this->l=l;
}
void Splitter :: setData(char data[],int l)
{
	this->data=data;
	this->l=l;
}
char** Splitter :: splitInto(int n,int *len,int *lastlen)
{
	if(data==NULL)
		return NULL;
	int numbrOfChnks;
	if((l%n)==0)
		numbrOfChnks=l/n;
	else
		numbrOfChnks=(l/n)+1;
		char **r=new char*[numbrOfChnks];
}
class FileSplitter : public Splitter
{
public:
	FileSplitter();
	FileSplitter(char *file);
	char** splitInto(int n,int *sz);
	int getLength();
};
