# include <stdio.h>
# include <stdlib.h>
# include <termios.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
#include <thread>


unsigned char Address[5];
unsigned char newid[10];


const char *DeviceName = "/dev/ttyUSB2";

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
//------------------------------------- read datas from zigbee --------------------------------------------
// succese return 1
// error   return 0
int read_zigbee_datas(int fd, char *rcv_buf)
{
	int retval;
	fd_set rfds;

	int ret,pos,count;


	pos = 0;
	ret = 0;
	count = 0;

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
			ret = read(fd, rcv_buf+pos, 2048);

			pos += ret;

			if(pos >4)
			{
				if(pos >= rcv_buf[3]+5)
				{

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



int read_zigbee_temhum(int fd, char *rcv_buf)
{
	int retval;
	fd_set rfds;

	int ret,pos,count;

	pos = 0;
	ret = 0;
	count = 0;

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
			ret = read(fd, rcv_buf+pos, 2048);

			pos += ret;

			if(pos >=46)
			{
				break;

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


float readtemhum(int fd, int temorhum)
{
	char crc=0,i;
	unsigned char buftozigbee[100];
	unsigned char buftozigbee1[20];
	unsigned char bufrxcom[100];
	unsigned char temp[100];
	int rxcount=0;
	unsigned char temhum[3];
	unsigned int *ptemhum;
	unsigned int  result = 0;
	float fresult=0;

	crc = 0;

	buftozigbee[0] =  0x26;
	buftozigbee[1] =  0x52; //RAS
	buftozigbee[2] =  0x53;
	buftozigbee[3] =  0x48;

	buftozigbee[4] =  newid[0];
	buftozigbee[5] =  newid[1];
	buftozigbee[6] =  newid[2];
	buftozigbee[7] =  newid[3];
	buftozigbee[8] =  newid[4];
	buftozigbee[9] =  newid[5];
	buftozigbee[10] = newid[6];
	buftozigbee[11] = newid[7];

	buftozigbee[12] = Address[0];
	buftozigbee[13] = Address[1];

	if(temorhum == 1)
	{
            buftozigbee[14] = 0x57;//wd
	    buftozigbee[15] = 0x44;
	}
	else
	{
            buftozigbee[14] = 0x53;//sd
	    buftozigbee[15] = 0x44;
	}

	buftozigbee[16] = 0x30;
	buftozigbee[17] = 0x30;
	buftozigbee[18] = 0x30;
	buftozigbee[19] = 0x30;
	buftozigbee[20] = 0x30;
	buftozigbee[21] = 0x30;
	buftozigbee[22] = 0x30;
	buftozigbee[23] = 0x30;
	buftozigbee[24] = 0x30;
	buftozigbee[25] = 0x30;
	buftozigbee[26] = 0x30;
	buftozigbee[27] = 0x30;
	buftozigbee[28] = 0x30;
	buftozigbee[29] = 0x30;


	buftozigbee[30] = 0x30;
	buftozigbee[31] = 0x2A;


	buftozigbee1[0]= 0xFE;
	buftozigbee1[1]= 0x01;
	buftozigbee1[2]= 0xA0;
	buftozigbee1[3]= 35;
	buftozigbee1[4]= 0x00;

	buftozigbee1[5]= *(buftozigbee+13);
	buftozigbee1[6]= *(buftozigbee+12);
	buftozigbee1[7]='\0';

	read(fd, bufrxcom, 2048);

	for(i=1;i<7;i++)
	{
	    crc ^= buftozigbee1[i];
	}

	WaitFdWriteable(fd);
	if (write(fd, &buftozigbee1[0], 7) < 0) {
	    Error(strerror(errno));
	}

	for(i=0;i<32;i++)
	{
	    crc ^= buftozigbee[i];
	}
	buftozigbee[32] = crc;

	WaitFdWriteable(fd);
	if (write(fd, &buftozigbee[0], 33) < 0) {
	    Error(strerror(errno));
	}

	rxcount = read_zigbee_temhum(fd,reinterpret_cast<char *>(bufrxcom));
	result |= (bufrxcom[29] <<8);
	result |= bufrxcom[30] ;

	if(temorhum == 1)
	{
		return  result*0.01 - 39.6;
	}
	return  result * 0.0405 - 4 - result * result * 2.8 * 0.000001;
}

void  startzigbee(int fd)
{
	char buftozigbee[100];
	unsigned char bufrxcom[100];
	int  count;

	buftozigbee[0]= 0xFE;
	buftozigbee[1]= 0x0A;
	buftozigbee[2]= 0x18;
	buftozigbee[3]= 0x00;
	buftozigbee[4]= 0x12;
	buftozigbee[5]= '\0';

	WaitFdWriteable(fd);
	if (write(fd, &buftozigbee[0], 5) < 0) {
		Error(strerror(errno));
	}

	count = read_zigbee_datas(fd,reinterpret_cast<char *>(bufrxcom));

	//FE 1A 18 01 01 02

	if((bufrxcom[0]==0xFE)&&(bufrxcom[1]==0x1A)&&(bufrxcom[2]==0x18))
	{
		printf("Successful startup\n");
	}
	else
	{
		printf("Failed to start\n");
	}
}

void print_prompt()
{
	printf ("Select what you want to do:\n");
	printf ("1 : Read temperature \n");
	printf ("2 : Read humidity \n");
	printf ("3 : Quit\n");
	printf (">");
}

int main() {
	int CommFd;

	struct termios TtyAttr;
	struct termios BackupTtyAttr;

	int DeviceSpeed = B38400;
	int TtySpeed = B115200;
	int ByteBits = CS8;
	int OutputHex = 0;
	int OutputToStdout = 0;
	int UseColor = 0;
	int  tocomFd;
	unsigned char buftozigbee[100];
	unsigned char buftozigbee1[20];
	unsigned char bufrxcom[100];
	char i,j,crc;
	int flag_select_func;
	unsigned char temp[100];

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

	system("echo 2  > /dev/swzb");

	startzigbee(CommFd);

	for (;;) {

		printf ("Waiting for ZIGBEE node to join \n");
		read_zigbee_datas(CommFd,reinterpret_cast<char *>(bufrxcom));

		if((bufrxcom[0]==0xFE)&&(bufrxcom[1]==0x01)&&(bufrxcom[2]==0xA1))
		{

			if((bufrxcom[7]=='&')&&(bufrxcom[8]=='J')&&(bufrxcom[9]=='O')&&(bufrxcom[10]=='N'))
			{

				memcpy(newid,&bufrxcom[11],8);

				sprintf(reinterpret_cast<char *>(temp),"%02x%02x%02x%02x%02x%02x%02x%02x",newid[0],newid[1],newid[2],newid[3],
	   newid[4],newid[5],newid[6],newid[7]);
				printf("New entrants to the zigbee node ID:%s\n",temp);

				Address[0] = bufrxcom[19];
				Address[1] = bufrxcom[20];
				sprintf(reinterpret_cast<char *>(temp),"%02x%02x",Address[0],Address[1]);
				printf("Network Address:%s\n",temp);

				while (1)
				{

					float temp = readtemhum(CommFd,1);
					float hum = readtemhum(CommFd,0);

					printf("Temperature: %.3f\n",temp);
					printf("Humidity: %.3f\n",hum);
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}
			}
		}
	}
	return 0;
}