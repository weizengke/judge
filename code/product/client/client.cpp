// client.cpp : Defines the entry point for the console application.
//

#include <winsock2.h>
#include <stdio.h>
#define PORT 5000

//#pragma comment(lib,"ws2_32")

SOCKET sClient;

int initSocket(int port,char *ip)
{
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);

	if(WSAStartup(sockVersion, &wsaData) != 0)
		return 0;

    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sClient == INVALID_SOCKET)
	{
		printf("socket error\n");
		return 0;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.S_un.S_addr =inet_addr(ip);

	if(connect(sClient,(sockaddr*)&servAddr,sizeof(servAddr))==SOCKET_ERROR)
	{
		closesocket(sClient);
		return 0;
	}
	return 1;
}

int main(int argc, char* argv[])
{

	SetErrorMode(SEM_NOGPFAULTERRORBOX );
	char INI_filename[256]="D:\\OJ\\conf\\config.ini";
	char IP[30]="127.0.0.1";
	int languageId=1;
	int sulotionId=0;
	char buff[1024] = {0};

	if(argc>0){
		sulotionId=atoi(argv[1]);
	}else{
		return 0;
	}
	if(argc>1){
		languageId=atoi(argv[2]);
	}
	if(argc>2){
		strcpy(INI_filename,argv[3]);
	}

	int port;
	char languageName[100]={0};
	char keyname[110]={0};
	char szCmd[128] = {0};
	sprintf(keyname,"Language%d",languageId);
	GetPrivateProfileString("Language",keyname,"",languageName,sizeof(languageName),INI_filename);
	GetPrivateProfileString("JudgeIP",languageName,"127.0.0.1",IP,sizeof(IP),INI_filename);

	port=GetPrivateProfileInt("System","sock_port",PORT,INI_filename);

	initSocket(port,IP);

	sprintf(szCmd, "judge solution %u", sulotionId);
	sprintf(buff, "abcddcba0001%04x%s", strlen(szCmd), szCmd);
	send(sClient,(const char*)buff,sizeof(buff),0);

	closesocket(sClient);
	WSACleanup();
	Sleep(1000);
	return 0;
}
