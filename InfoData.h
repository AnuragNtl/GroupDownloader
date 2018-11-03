#ifndef INFO_DATA
#define INFO_DATA
#include<string>
#include<windows.h>
#include<winsock.h>
#include<wininet.h>
#include"Range.h"
#include<map>
using namespace std;
#define UNIV_BUF_SIZE 1024
#define DISTRIBUTION_LISTEN_PORT "9980"
#define DISTRIBUTION_TRANSFER_LISTEN_PORT 9890
#define REPLY_RESEND 0
#define REPLY_TERMINATE 1
#define REPLY_OK 2
#define PROGRESS_DELIMITER "\r\n\r\n"
#define INFO_DATA_DELIMITER "\0"
#define EACH_HEADER_DELIMITER "\n"
#define INFO_DATA_HEADERS_DELIMITER "\r\n\r\n"
struct InfoData
{
static const int ITYPE_DOWNLOAD=0;
static const int ITYPE_UPLOAD=1;
int type;
string host,path,rtype,hdrs,fdata;
Range r;
char *downloadId;
int start,stop;
int sendMergeData;
};
struct Progress
{
	string host,name;
short started;
short finished;
long progress;
short cancelled;
short error;
};
struct ReceiveProgress
{
	map<string,struct InfoData> *infoSend;
SOCKET *sc1;
struct sockaddr_in *clientAddr;
void (*onRecv)(struct Progress *,int *);
void (*onDisc)(struct Progress *);
void (*onReadFrom)(struct Progress *,char*,int);
int addrSize;
};
#endif
	/*
	Progress Message Format:
	Host(Alias Name)
	Progress
	StartedFinishedCancelledError
	*/
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
