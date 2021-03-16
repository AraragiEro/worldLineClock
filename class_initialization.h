#define close_state				0	//�صƴ���״̬
#define worldlinechange_state	1	//������״̬

#define time_state			2	//��ʱ״̬
	#define time_state_time			1		//ʱ��
	#define time_state_date			2		//����
	#define time_state_day			3		//����

#define autotimeproof_state		3	//�Զ�У��״̬

#define timeproof_state	4	//�ֶ�Уʱ-ʱ��
	#define timeproof_state_time	1		//ʱ��
	#define timeproof_state_date	2		//����
	#define timeproof_state_day		3		//����



#define digitalmission_ON		1
#define digitalmission_OFF		0

#define LE						3	//ʱ��
#define CLK						2

#define RECV_PIN				9	//����

#define light_PIN				8

#define gpsRxBufferLength		300

#define wl_speed				10


#define PT_out					PT_EXIT(pt)


//����ң������������
/*#define B_POWER					0xFFA25D	
#define B_MENUE					0xFFE21D
#define B_TEST					0xFF22DD
#define B_CANCEL				0xFFC23D
#define B_ADD					0xFF02FD
#define B_MINUS					0xFF9867
#define B_Last					0xFFE01F
#define B_Next					0xFF906F
#define B_Confirm				0xFFA857
#define B_0						0xFF6897
#define B_1						0xFF30CF
#define B_2						0xFF18E7
#define B_3						0xFF7A85
#define B_4						0xFF10EF
#define B_5						0xFF38C7
#define B_6						0xFF5AA5
#define B_7						0xFF42BD
#define B_8						0xFF4AB5
#define B_9						0xFF52AD
#define B_C						0xFFB04F*/
	
#define B_POWER					0x6A68351E	//�ֻ��������
#define B_MENUE					0xDC95DD23
#define B_ANALOGSOURCE			0x40C1789F
#define B_HOME					0xCD9141E
#define B_CANCEL				0x7E940902
#define B_UP					0x1039789F
#define B_DOWN					0xBB8B3E9E
#define B_ADD					0x22D912BB
#define B_MINUS					0x776C6E7A
#define B_LEFT					0x45481702
#define B_RIGHT					0xF0B4BB43
#define B_Confirm				0xB8E7B4FE
#define B_0						0x898FDF7A
#define B_1						0x6BC6597B
#define B_2						0x735B797E
#define B_3						0x1EC81DBF
#define B_4						0x450753D6
#define B_5						0xBA0F4EDF
#define B_6						0x4AC4DA9A
#define B_7						0xF6317EDB
#define B_8						0xF9000E7E
#define B_9						0xC7291B77

#define uchar						unsigned char
#define GpsSerial  Serial
#define DebugSerial Serial
#define NOP do { __asm__ __volatile__ ("nop"); } while (0)

//ʱ����--------------------------------------------------------------
class Time
{
private:
	uchar hour;
	uchar min;
	uchar sec;
	uchar DoW;
	uchar Date;
	uchar Month;
	uchar Year;
public:
	bool Century;
	Time();
	void GetTime();//DS��getTime�ļ򻯡�
	uchar get_sec();
	bool h24;
	bool PM;
	uchar time_mission_key;
};

//GPS��------------------------------------------------------------
class GPSdata
{
private:
	char GPS_Buffer[80];
	
	bool isParseData;	//�Ƿ�������
	char latitude[11];		//γ��
	char N_S[2];		//N/S
	char longitude[12];		//����
	char E_W[2];		//E/W
	char UTCdate[6];		//UTC����
	bool isUsefull;		//��λ��Ϣ�Ƿ���Ч
	
public:
	char UTCTime[11];
	uchar hour;
	uchar min;
	uchar sec;
	uchar Date;
	uchar Month;
	uchar Year;
	bool isGetData;		//�Ƿ��ȡ��GPS����
	char gpsRxBuffer[gpsRxBufferLength];
	unsigned int ii = 0;
	void restart_EGPS();
	void gpsRead();//��ȡGPS����
	void clrGpsRxBuffer();//���
	void errorLog(int num);
	void parseGpsBuffer();//����GPS����
	void tr_gps_time();
	void prooftime();
};

//������--------------------------------------------------------------
class world_line_change
{
private:
	uchar flash_number[8];//ˢ��ʱ��8������
	uchar stop_number[8];//����ʱ��8������
	char main_worldline;//���λ��������
	bool stop_state[8];//ĳλ�Ƿ��Ѿ�ͣ��
	char stop_queue[8];//ѡλֹͣ����
	char stop_type;
	
public:
	world_line_change();
	void get_new_flashworldline();//���һ��8λ����
	void new_worldline();//�µ�ֹͣ����
	uchar get_flashnum(uchar i);
	uchar get_stopnum(uchar i);
	void new_stopqueue();//ѡλϴ��
	char get_stopqueue(char i);
	void change_allstopstate_ON();//��������λˢ��
	void close_stopstate(char i);//�ر�ĳһλˢ��
	bool get_stopstate(char i);
	void new_stop_type();
	char get_stoptype();

};

//�Թ��---------------------------------------------------------------
class Glow_tube
{
private:
	int binary_queue[8];//8��16λ��������
	uchar state[2];//��ǰ�Ĺ���״̬
public:
	Glow_tube();
	uchar last_state[2];
	void get_new_number();//��ȡ�����֡�״̬��
	void anime(int a);//��ȡ��ͬ����
	uchar return_number(uchar i);//�ͳ�����
	void change_state(uchar state_num, uchar new_state);//�л�ģʽ
	uchar get_state(int a);
	void set_number(uchar a);//ȫ������
	void set_queue(uchar a,uchar b);
};

//�������-----------------------------------------------------------------
class digital_way
{
private:
public:
	void digital_mission();//�������
	void proofsuccess_anime();
	void proofailed_anime();
	void fourway_digital_once(bool a, bool b, bool c, bool d, bool rl);//һ����λ�������
	void digital_onenumber(int a, bool rl);//���һ������
};

//���뷽��-----------------------------------------------------------
class analog_in_way
{
private:
public:
	analog_in_way();
	uchar menu_state;
	bool IR_state;
	unsigned long IR_code;
	unsigned long last_IRcode;
	void restart_analog();
};

//�˵�ģ��--------------------------------------
class menu
{
private:
public:
};

//�ֶ���ʱģ��
class prooftime
{
private:
	int sec;
	int min;
	int hour;
public:
	uchar pDateQueue[8];
	uchar pDay;
	uchar queue;
	void getproof_data();
	void cofirm();
	void get_time();
	int get_Min();
	int get_hour();
	int get_sec();

};


int pt_IR_mission(struct pt *pt);
int pt_worldline_mission(struct pt *pt);
int pt_time_mission(struct  pt *pt);
int pt_autoprooftime_mission(struct  pt *pt);
int pt_desktop_mission(struct pt *pt);
int pt_close_mission(struct pt *pt);
int pt_prooftime_mission(struct pt *pt);