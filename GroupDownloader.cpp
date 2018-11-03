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
#include<winsock.h>
#include<winsock2.h>
//#include<ws2tcpip.h>
#include<wininet.h>
#include<windows.h>
class T1
{
protected:
	int a;
public:
	T1()
	{
		a=10;
	}
	void showA()
	{
		cout <<"ee" <<a <<endl;
	}
};
class T2 : public T1
{
protected:
	int a;
public:
	T2()
	{
		a=20;
	}
	void showA()
	{
		cout <<a+a <<endl;
	}
};
int main()
{
T1 *t1=new T2;
t1->showA();
}
