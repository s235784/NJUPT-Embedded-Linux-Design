# include <stdio.h>
# include <stdlib.h>
# include <termios.h>
# include <unistd.h>
# include <fcntl.h>
# include <getopt.h>
# include <time.h>
# include <errno.h>
# include <string.h>


unsigned char deviceid[10];


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


static int SerialSpeed(const char *SpeedString)
{
    int SpeedNumber = atoi(SpeedString);
#   define TestSpeed(Speed) if (SpeedNumber == Speed) return B##Speed
    TestSpeed(1200);
    TestSpeed(2400);
    TestSpeed(4800);
    TestSpeed(9600);
    TestSpeed(19200);
    TestSpeed(38400);
    TestSpeed(57600);
    TestSpeed(115200);
    TestSpeed(230400);
    Error("Bad speed");
    return -1;
}

static void PrintUsage(void)
{

   fprintf(stderr, "comtest - interactive program of comm port\n");
   fprintf(stderr, "press [ESC] 3 times to quit\n\n");

   fprintf(stderr, "Usage: comtest [-d device] [-t tty] [-s speed] [-7] [-c] [-x] [-o] [-h]\n");
   fprintf(stderr, "         -7 7 bit\n");
   fprintf(stderr, "         -x hex mode\n");
   fprintf(stderr, "         -o output to stdout too\n");
   fprintf(stderr, "         -c stdout output use color\n");
   fprintf(stderr, "         -h print this help\n");
   exit(-1);
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



int read_blue_result(int fd, unsigned char *rcv_buf,int rxcount)
{
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
                        ret = read(fd, rcv_buf+pos, 2048);

                        pos += ret;

			if(pos >=rxcount)
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


void print_prompt(void)
{
        printf ("Select what you want to do:\n");
        printf ("1 : Open the buzzer \n");
        printf ("2 : close the buzzer \n");
        printf ("3 : Read  temperature sensor \n");
        printf ("4 : Read  light sensors \n");
        printf ("5: Quit\n");
        printf (">");
} 

void  openbeep(int fd)
{
        unsigned char buftoblue[100];
	int ptr=0;


  //01 12 FD 07 00 00 00 00 36 00 01
   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x12;
   	buftoblue[ptr++] =  0xFD;
   	buftoblue[ptr++] =  0x07;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;

   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x36;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x01;


	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}    
       
       read_blue_result(fd,buftoblue,18);//18


       printf("openbeep ok \n");
}

void  closebeep(int fd)
{
        unsigned char buftoblue[100];
	int ptr=0;

  //01 12 FD 07 00 00 00 00 36 00 01
   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x12;
   	buftoblue[ptr++] =  0xFD;
   	buftoblue[ptr++] =  0x07;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;

   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x36;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;


	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}    
       
       read_blue_result(fd,buftoblue,18);//18


       printf("closebeep ok \n");
}


void  readtemsensor(int fd)
{

        unsigned char buftoblue[100];
	int ptr=0;

  //01 0A FD 04 00 00 29 00 

   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x0A;
   	buftoblue[ptr++] =  0xFD;
   	buftoblue[ptr++] =  0x04;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x31;
   	buftoblue[ptr++] =  0x00;



	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}
      
       
       read_blue_result(fd,buftoblue,19);//18
       printf("temperature:%d\n",buftoblue[18]);




}



void  enabletemsensor(int fd)
{
        unsigned char buftoblue[100];
	int ptr=0;

  //01 12 FD 07 00 00 00 00 36 00 01
   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x12;
   	buftoblue[ptr++] =  0xFD;
   	buftoblue[ptr++] =  0x07;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;

   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x2E;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x01;


	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}
      
       
       read_blue_result(fd,buftoblue,18);//18


}

void  readlightsensor(int fd)
{

        unsigned char buftoblue[100];
	int ptr=0;

  //01 0A FD 04 00 00 29 00 

   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x0A;
   	buftoblue[ptr++] =  0xFD;
   	buftoblue[ptr++] =  0x04;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x29;
   	buftoblue[ptr++] =  0x00;



	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}
      
       
       read_blue_result(fd,buftoblue,19);//18
       printf("light:%d\n",buftoblue[18]);


}

void  enablelightsensor(int fd)
{
        unsigned char buftoblue[100];
	int ptr=0;


  //01 12 FD 07 00 00 00 00 36 00 01
   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x12;
   	buftoblue[ptr++] =  0xFD;
   	buftoblue[ptr++] =  0x07;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;

   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x26;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x01;


	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}
      
       
       read_blue_result(fd,buftoblue,18);//18

  

}


void establishLinkRequest(int fd)
{
        unsigned char buftoblue[100];
	int ptr=0;
 
  //01 09 FE 09 00 00 00 11 FF FF FF FF 22

   	buftoblue[ptr++] =  0x01;
   	buftoblue[ptr++] =  0x09;
   	buftoblue[ptr++] =  0xFE;
   	buftoblue[ptr++] =  0x09;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;
   	buftoblue[ptr++] =  0x00;

   	// buftoblue[ptr++] =  deviceid[0];
   	// buftoblue[ptr++] =  deviceid[1];
   	// buftoblue[ptr++] =  deviceid[2];
   	// buftoblue[ptr++] =  deviceid[3];
   	// buftoblue[ptr++] =  deviceid[4];
   	// buftoblue[ptr++] =  deviceid[5];
	
   	buftoblue[ptr++] =  0x66;
   	buftoblue[ptr++] =  0x66;
   	buftoblue[ptr++] =  0x66;
   	buftoblue[ptr++] =  0x66;
   	buftoblue[ptr++] =  0x66;
   	buftoblue[ptr++] =  0x66;	

	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}     
       
       read_blue_result(fd,buftoblue,31);//31

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
       
       read_blue_result(fd,buftoblue,18);//18

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
      
       read_blue_result(fd,buftoblue,9);//9

       printf("stop scanning equipment ok \n");

}


void scanningequipment(int fd)
{

        unsigned char buftoblue[100];
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

	printf("scanning equipment......... \n");

	WaitFdWriteable(fd);
	if (write(fd, &buftoblue[0], ptr) < 0) {
	    Error(strerror(errno));
	}
      
        read_blue_result(fd,buftoblue,74);//11

        memcpy(deviceid,&buftoblue[68],6);
        sprintf((char *)temp,"%02x%02x%02x%02x%02x%02x",deviceid[0],deviceid[1],deviceid[2],deviceid[3],
		deviceid[4],deviceid[5]);
         printf("New entrants to the bluetooth ID:%s\n",temp);


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
        
        read_blue_result(fd,buftoblue,11);//11
	
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
      
        read_blue_result(fd,buftoblue,11);//11

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
      
       read_blue_result(fd,buftoblue,11);//11

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
      
       read_blue_result(fd,buftoblue,11);//11


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
      
        read_blue_result(fd,buftoblue,56);//56

	printf("startblue  ok \n");	


}



int main(int argc, char **argv)
{
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

    

    opterr = 0;
    for (;;) {
        int c = getopt(argc, argv, "d:s:t:7xoch");
        if (c == -1)
            break;
        switch(c) {
        case 'd':
            DeviceName = optarg;
            break;
        case 's':
	    if (optarg[0] == 'd') {
		DeviceSpeed = SerialSpeed(optarg + 1);
	    } else if (optarg[0] == 't') {
		TtySpeed = SerialSpeed(optarg + 1);
	    } else
            	TtySpeed = DeviceSpeed = SerialSpeed(optarg);
            break;
	case 'o':
	    OutputToStdout = 1;
	    break;
	case '7':
	    ByteBits = CS7;
	    break;
        case 'x':
            OutputHex = 1;
            break;
	case 'c':
	    UseColor = 1;
	    break;
        case '?':
        case 'h':
        default:
	    PrintUsage();
        }
    }
    if (optind != argc)
        PrintUsage();

    CommFd = open(DeviceName, O_RDWR, 0);  // 打开指定设备
    if (CommFd < 0)
	Error("Unable to open device");
    if (fcntl(CommFd, F_SETFL, O_NONBLOCK) < 0) // 非阻塞模式
     	Error("Unable set to NONBLOCK mode");


    memset(&TtyAttr, 0, sizeof(struct termios));  // 设置串口参数
    TtyAttr.c_iflag = IGNPAR;
    TtyAttr.c_cflag = DeviceSpeed | HUPCL | ByteBits | CREAD | CLOCAL;

    TtyAttr.c_cc[VMIN] = 1;

    if (tcsetattr(CommFd, TCSANOW, &TtyAttr) < 0)
        Warning("Unable to set comm port");


    system("echo 1  > /dev/swzb");

    startblue(CommFd);  // 启动蓝牙连接
    getminconnectinterval(CommFd);
    getmaxconnectinterval(CommFd);
    getslavelatency(CommFd);
    getsupervisiontimout(CommFd);

    //scanningequipment(CommFd);

    //stopscanningequipment(CommFd);
    establishLinkRequest(CommFd);  // 建立蓝牙连接

    enabletemsensor(CommFd);
    enablelightsensor(CommFd);
    
 
    for (;;) 
     {
		    print_prompt();
		    scanf("%d",&flag_select_func);	// user input select
		    getchar();

		    switch(flag_select_func)
		    {
			case 1        : {openbeep(CommFd);        break;}
			case 2        : {closebeep(CommFd);       break;}
			case 3        : {readtemsensor(CommFd);   break;}
			case 4        : {readlightsensor(CommFd);  break;}
			case 5        :
				      {
					terminatelinkrequest(CommFd);
				         printf("Quit blue function. \n");
				         return 0;
				       }
			default :
			{
				printf("please input your select use 1 to 5\n");
			}
		  }

       }


    return 0;
}
