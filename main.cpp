# include <stdio.h>
# include <stdlib.h>
# include <termios.h>
# include <unistd.h>
# include <errno.h>
#include <iostream>
# include <string.h>
#include <thread>
#include <sqlite3.h>

#include "broadcast_server.h"

int flag_select_bleid;
unsigned char deviceid[60];
unsigned char deviceid_t[60];

unsigned char buf[256]={0};
unsigned int StringToHex(/*IN*/  const unsigned char * pByteString,
						 /*IN */ unsigned int        dwStrLen,
						 /*OUT*/ unsigned char       * pbHex)
{
	unsigned int i;
	unsigned int dwRet;
	unsigned char * pTempBuf=NULL;

	if (pByteString==NULL || pbHex==NULL)
		return 1;

	if(dwStrLen <= 0)
		return 2;

	if ((dwStrLen%2)!=0)
	{
		pTempBuf=buf;
		pTempBuf[0]=0x30;
		memcpy(&pTempBuf[1],pByteString,sizeof(unsigned char)*dwStrLen);
		dwRet=StringToHex(pTempBuf,dwStrLen+1,pbHex);
		return dwRet;
	}
	else
	{
		for(i=0;i<dwStrLen;i++)
		{
			if(i%2==0)
			{
				if(pByteString[i]>='0' && pByteString[i]<='9')
					pbHex[i/2]=(pByteString[i]-0x30)<<4;
				else if(pByteString[i]>='a' && pByteString[i]<='f')
					pbHex[i/2]=(pByteString[i]-0x57)<<4;
				else if(pByteString[i]>='A' && pByteString[i]<='F')
					pbHex[i/2]=(pByteString[i]-0x37)<<4;
				else
					return 3;
			}
			else
			{
				if(pByteString[i]>='0' && pByteString[i]<='9')
					pbHex[i/2]|=pByteString[i]-0x30;
				else if(pByteString[i]>='a' && pByteString[i]<='f')
					pbHex[i/2]|=pByteString[i]-0x57;
				else if(pByteString[i]>='A' && pByteString[i]<='F')
					pbHex[i/2]|=pByteString[i]-0x37;
				else
					return 4;
			}
		}
	}
	return 0;
}

const char *DeviceName = "/dev/ttyUSB3";

static void Error(const char *Msg)
{
	fprintf (stderr, "%s\n", Msg);
	fprintf (stderr, "strerror() is %s\n", strerror(errno));
	exit(1);
}
static void Warning(const char *Msg)
{
	fprintf (stderr, "Warning: %s\n", Msg);
}

static inline void WaitFdWriteable(int Fd)
{
  fd_set WriteSetFD;
  FD_ZERO(&WriteSetFD);
  FD_SET(Fd, &WriteSetFD);
  if (select(Fd + 1, NULL, &WriteSetFD, NULL, NULL) < 0) {
    Error(strerror(errno));
  }

}

int read_blue_result(int fd, char *rcv_buf,int rxcount)
{
  int  tlen=0;
  int  len=0;
  int state=0;
  char c;
  int i;
  char dat[10];
  int retval;
  fd_set rfds;
  unsigned char temp[100];

  int ret,pos;

  pos = 0;
  ret = 0;

  while (1)
  {
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    retval = select(fd+1 , &rfds, NULL, NULL, NULL);

    if (retval == -1)
    {
      perror("select()");
      break;
    }
    else if (retval)
    {
      if(rxcount==0)
      {
        ret = read(fd, rcv_buf+pos, 2048);
        pos += ret;
#if 0
        printf("\nret=%d,len=%d\n",ret,pos);
        for(i=0;i<pos;i++)
        {
          sprintf(dat,"%02x ",rcv_buf[i]);
          printf(dat);
        }
        printf("\n==============\n");
#endif

        for(i=0;i<pos;i++)
        {
          if((rcv_buf[i]==0x04)&&(rcv_buf[i+1]==0xff)&&(rcv_buf[i+3]==0x01)&&(rcv_buf[i+4]==0x06))
          {
            state=1;
            tlen=i+rcv_buf[i+2]+3;
            len=rcv_buf[i+2]+3;
#if 0
            printf("\ni=%d,tlen=%d,len=%d\n",i,tlen,len);
#endif
            break;
          }
        }

        if(state==1)
        {
          if(pos >= tlen)
          {
            memcpy(rcv_buf,rcv_buf+i,len);
#if 0
            printf("\n*******************\n");
            for(i=0;i<len;i++)
            {
              sprintf(dat,"%02x ",rcv_buf[i]);
              printf(dat);
            }
            printf("\n-------------------\n");
#endif

            state=0;
            break;
          }
        }
      }
      else if(rxcount==1)
      {
        ret = read(fd, rcv_buf+pos, 2048);
        pos += ret;
#if 0
        printf("\nret=%d,len=%d\n",ret,pos);
        for(i=0;i<pos;i++)
        {
          sprintf(dat,"%02x ",rcv_buf[i]);
          printf(dat);
        }
        printf("\n==============\n");
#endif

        for(i=0;i<pos;i++)
        {
          //04 ff 0a 09 05 00 00 00 04 03 34 00 1a
          if((rcv_buf[i]==0x04)&&(rcv_buf[i+1]==0xff)&&(rcv_buf[i+2]==0x0a)&&(rcv_buf[i+3]==0x09)&&(rcv_buf[i+4]==0x05))
          {
            state=1;
            tlen=i+rcv_buf[i+2]+3;
            len=rcv_buf[i+2]+3;
#if 0
            printf("\ni=%d,tlen=%d,len=%d\n",i,tlen,len);
#endif
            break;
          }
        }

        if(state==1)
        {
          if(pos >= tlen)
          {
            memcpy(rcv_buf,rcv_buf+i,len);
#if 0
            printf("\n*******************\n");
            for(i=0;i<len;i++)
            {
              sprintf(dat,"%02x ",rcv_buf[i]);
              printf(dat);
            }
            printf("\n-------------------\n");
#endif

            state=0;
            break;
          }
        }
      }
      else
      {
        ret = read(fd, rcv_buf+pos, 2048);
        pos += ret;


        if(pos >=rxcount)
        {
#if 0
          printf("ret=%d,len=%d\n",ret,pos);
          for(i=0;i<pos;i++)
          {
            sprintf(dat,"%02x ",rcv_buf[i]);
            printf(dat);
          }
          printf("\n");
#endif

          break;
        }

      }
    }
    else
    {
      printf("No data\n");
      break;
    }
  }

  return pos;
}

int  readlightsensor(int fd)
{
  int i;
  char dat[10];

  unsigned char buftoblue[100];
  int ptr=0;

  //01 B4 FD 08 00 00 01 00 FF FF B2 FF

  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0xB4;
  buftoblue[ptr++] =  0xFD;
  buftoblue[ptr++] =  0x08;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0xFF;
  buftoblue[ptr++] =  0xFF;
  buftoblue[ptr++] =  0xB2;
  buftoblue[ptr++] =  0xFF;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),1);//3
#if 0
  printf("\n*******************\n");
  for(i=0;i<31;i++)
  {
    sprintf(dat,"%02x ",buftoblue[i]);
    printf(dat);
  }
  printf("\n-------------------\n");
#endif

  //04 ff 0a 09 05 00 00 00 04 03 34 00 35
	return (int)buftoblue[12];
}

void  enablelightsensor(int fd)
{
  unsigned char buftoblue[100];
  int ptr=0;

  //01 92 FD 05 00 00 29 00 01

  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x92;
  buftoblue[ptr++] =  0xFD;
  buftoblue[ptr++] =  0x05;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;

  buftoblue[ptr++] =  0x29;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x01;


  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),18);//18
  printf("enable light sensor ok \n");
}


void establishLinkRequest(int fd)
{
  int i;
  char dat[10];
  unsigned char buftoblue[100];
  int ptr=0;

  //01 09 FE 09 00 00 00 33 75 1D 65 70 90

  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x09;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x09;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;

  buftoblue[ptr++] =  deviceid_t[5];
  buftoblue[ptr++] =  deviceid_t[4];
  buftoblue[ptr++] =  deviceid_t[3];
  buftoblue[ptr++] =  deviceid_t[2];
  buftoblue[ptr++] =  deviceid_t[1];
  buftoblue[ptr++] =  deviceid_t[0];

  // buftoblue[ptr++] =  deviceid[flag_select_bleid*6+0];
  // buftoblue[ptr++] =  deviceid[flag_select_bleid*6+1];
  // buftoblue[ptr++] =  deviceid[flag_select_bleid*6+2];
  // buftoblue[ptr++] =  deviceid[flag_select_bleid*6+3];
  // buftoblue[ptr++] =  deviceid[flag_select_bleid*6+4];
  // buftoblue[ptr++] =  deviceid[flag_select_bleid*6+5];

#if 0
  printf("\n==============\n");
  for(i=0;i<ptr;i++)
  {
    sprintf(dat,"%02x ",buftoblue[i]);
    printf(dat);
  }
  printf("\n==============\n");
#endif

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),31);//31

  printf("establish Link Request ok \n");
}

void terminatelinkrequest(int fd)
{

	unsigned char buftoblue[100];
	int ptr=0;

	//01 0A FE 02 00 00

	buftoblue[ptr++] =  0x01;
	buftoblue[ptr++] =  0x0A;
	buftoblue[ptr++] =  0xFE;
	buftoblue[ptr++] =  0x02;
	buftoblue[ptr++] =  0x00;
	buftoblue[ptr++] =  0x00;

	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
		Error(strerror(errno));
	}

	read_blue_result(fd,reinterpret_cast<char *>(buftoblue),18);//18

	printf("terminate Link Request ok \n");
}

void stopscanningequipment(int fd)
{

  unsigned char buftoblue[100];
  int ptr=0;

  //01 05 FE 00
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x05;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x00;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),16);//16

  printf("stop scanning equipment ok \n");
}


void scanningequipment(int fd)
{
  int i;
  unsigned char buftoblue[200];
  int ptr=0;
  unsigned char temp[100];

  //01 04 FE 03 03 01 00
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x04;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x03;
  buftoblue[ptr++] =  0x03;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x00;

  printf("scanning equipment,please wait......\n");

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),0);//0

  //04 ff 14 01 06 00 02 00 00 33 75 1d 65 70 90 00 01 ea 73 e5 7c 4b de

  printf("New entrants to the bluetooth:\n");
  for(i=0;i<buftoblue[6];i++)
  {
    memcpy(&deviceid[i*6],&buftoblue[9+8*i],6);
    sprintf(reinterpret_cast<char *>(temp),"%02x%02x%02x%02x%02x%02x",deviceid[i*6+5],deviceid[i*6+4],deviceid[i*6+3],deviceid[i*6+2],
            deviceid[i*6+1],deviceid[i*6+0]);
    printf("  ID%d:%s\n",i,temp);
  }

  printf ("Select ble [ID] you want to established\n");
  scanf("%d",&flag_select_bleid);	// user input select
  getchar();
  printf ("Now ble node [ID%d]  will be establish\n",flag_select_bleid);

}

void getsupervisiontimout(int fd)
{
  unsigned char buftoblue[100];
  int ptr=0;

  //01 31 FE 01 1A
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x31;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x19;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),11);//11

  printf("get supervision timout ok \n");

}

void getslavelatency(int fd)
{
  unsigned char buftoblue[100];
  int ptr=0;

  //01 31 FE 01 1A
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x31;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x1A;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),11);//11

  printf("get slave latency ok \n");
}

void getmaxconnectinterval(int fd)
{

  unsigned char buftoblue[100];
  int ptr=0;

  //01 31 FE 01 15
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x31;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x16;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),11);//11

  printf("get max connect interval  ok \n");
}

void getminconnectinterval(int fd)
{

  unsigned char buftoblue[100];
  int ptr=0;
  unsigned char temp[100];
  //01 31 FE 01 15
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x31;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x15;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),11);//11


  printf("get min connect interval  ok \n");
}


void startblue(int fd)
{
  //01 00 FE 26 08 05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00
  unsigned char buftoblue[100];
  int ptr=0;

  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0xFE;
  buftoblue[ptr++] =  0x26;
  buftoblue[ptr++] =  0x08;
  buftoblue[ptr++] =  0x05;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x01;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;
  buftoblue[ptr++] =  0x00;

  WaitFdWriteable(fd);
  if (write(fd, &buftoblue[0], ptr) < 0) {
    Error(strerror(errno));
  }

  read_blue_result(fd,reinterpret_cast<char *>(buftoblue),56);//56

  printf("startblue  ok \n");
}

sqlite3* db;
const std::string dbName = "data.db";

bool init_sqlite() {
	char* errMsg = nullptr;
	int rc;

	rc = sqlite3_open(dbName.c_str(), &db);
	if (rc) {
		std::cerr << "can't open: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	std::string sql = "CREATE TABLE IF NOT EXISTS Data ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"Light REAL NOT NULL, "
		"Time TEXT NOT NULL);";
	rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::cerr << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return false;
	}
	std::cout << "Table Created Successes." << std::endl;
	return true;
}

bool insertSensorData(int light) {
	char* errMsg = nullptr;  //用于存储错误信息

	//准备插入数据的SQL语句
	std::string sql = "INSERT INTO Data (Light, Time) VALUES (" +
					  std::to_string(light) + ", " +
						  "strftime('%Y-%m-%d %H:%M:%S', 'now'));";

	int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::cerr << "mistake of inserting data : " << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return false;
	}
	return true;
}

void close_sqlite() {
	sqlite3_close(db);
}


int main() {
	if (!init_sqlite()) {
		return 0;
	}

	broadcast_server server;

	std::thread server_thread([&] {
		constexpr uint16_t port = 8888;
		std::cout << "WebSocket Listening port: " + std::to_string(port) << std::endl;
		server.run(port);
	});

	int CommFd;

	struct termios TtyAttr;
	struct termios BackupTtyAttr;

	int DeviceSpeed = B57600;
	int TtySpeed = B115200;
	int ByteBits = CS8;
	int OutputHex = 0;
	int OutputToStdout = 0;
	int UseColor = 0;
	int  tocomFd;

	unsigned char bufrxcom[100];
	char i,j,crc;
	int flag_select_func;
	unsigned char temp[100];

	int opt;
	char *optstring = "d:h:";
	unsigned char dat[100];

	std::string input = "F8333162491E";
	StringToHex(reinterpret_cast<const unsigned char *>(input.c_str()),12,deviceid_t);

	CommFd = open(DeviceName, O_RDWR, 0);
	if (CommFd < 0)
		Error("Unable to open device");
	if (fcntl(CommFd, F_SETFL, O_NONBLOCK) < 0)
		Error("Unable set to NONBLOCK mode");


	memset(&TtyAttr, 0, sizeof(struct termios));
	TtyAttr.c_iflag = IGNPAR;
	TtyAttr.c_cflag = DeviceSpeed | HUPCL | ByteBits | CREAD | CLOCAL;

	TtyAttr.c_cc[VMIN] = 1;

	if (tcsetattr(CommFd, TCSANOW, &TtyAttr) < 0)
		Warning("Unable to set comm port");

	startblue(CommFd);
	getminconnectinterval(CommFd);
	getmaxconnectinterval(CommFd);
	getslavelatency(CommFd);
	getsupervisiontimout(CommFd);

	//scanningequipment(CommFd);
	printf("establishLink To ");
	for(i=0;i<6;i++)
	{
		sprintf(reinterpret_cast<char *>(dat),"%02x ",deviceid_t[i]);
		printf(reinterpret_cast<char *>(dat));
	}
	printf("\n");
	//stopscanningequipment(CommFd);
	establishLinkRequest(CommFd);

	enablelightsensor(CommFd);

	std::atomic running(true);
	std::thread timer_thread([&] {
		while (running) {
			printf("Read\n");
			int result = readlightsensor(CommFd);
			printf("Light:%d\n",result);
			insertSensorData(result);
			server.send_light(result);
			printf("Read End\n");
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	});

	// 等待线程
	std::cout << "Press Enter to stop the server..." << std::endl;
	std::cin.get();  // 等待用户按下回车键

	// 停止定时器和服务器
	running = false;  // 停止定时器线程
	timer_thread.join();  // 等待定时器线程结束

	server.stop();
	server_thread.join();
	close_sqlite();
	terminatelinkrequest(CommFd);

	return 0;
}