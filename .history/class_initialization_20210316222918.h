#define close_state				0	//关灯待机状态
#define worldlinechange_state	1	//世界线状态

#define time_state			2	//显时状态
	#define time_state_time			1		//时间
	#define time_state_date			2		//日期
	#define time_state_day			3		//星期

#define autotimeproof_state		3	//自动校对状态

#define timeproof_state	4	//手动校时-时间
	#define timeproof_state_time	1		//时间
	#define timeproof_state_date	2		//日期
	#define timeproof_state_day		3		//星期



#define digitalmission_ON		1
#define digitalmission_OFF		0

#define LE						3	//时钟
#define CLK						2

#define RECV_PIN				9	//红外

#define light_PIN				8

#define gpsRxBufferLength		300

#define wl_speed				10


#define PT_out					PT_EXIT(pt)


//红外遥控器按键编码
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
	
#define B_POWER					0x6A68351E	//手机红外编码
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

//时间类--------------------------------------------------------------
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
	void GetTime();//DS类getTime的简化。
	uchar get_sec();
	bool h24;
	bool PM;
	uchar time_mission_key;
};

//GPS类------------------------------------------------------------
class GPSdata
{
private:
	char GPS_Buffer[80];
	
	bool isParseData;	//是否解析完成
	char latitude[11];		//纬度
	char N_S[2];		//N/S
	char longitude[12];		//经度
	char E_W[2];		//E/W
	char UTCdate[6];		//UTC日期
	bool isUsefull;		//定位信息是否有效
	
public:
	char UTCTime[11];
	uchar hour;
	uchar min;
	uchar sec;
	uchar Date;
	uchar Month;
	uchar Year;
	bool isGetData;		//是否获取到GPS数据
	char gpsRxBuffer[gpsRxBufferLength];
	unsigned int ii = 0;
	void restart_EGPS();
	void gpsRead();//获取GPS数据
	void clrGpsRxBuffer();//清空
	void errorLog(int num);
	void parseGpsBuffer();//解析GPS数据
	void tr_gps_time();
	void prooftime();
};

//世界线--------------------------------------------------------------
class world_line_change
{
private:
	uchar flash_number[8];//刷新时的8个数字
	uchar stop_number[8];//听下时的8个数字
	char main_worldline;//最高位主世界线
	bool stop_state[8];//某位是否已经停下
	char stop_queue[8];//选位停止队列
	char stop_type;
	
public:
	world_line_change();
	void get_new_flashworldline();//随机一组8位数字
	void new_worldline();//新的停止数字
	uchar get_flashnum(uchar i);
	uchar get_stopnum(uchar i);
	void new_stopqueue();//选位洗牌
	char get_stopqueue(char i);
	void change_allstopstate_ON();//开启所有位刷新
	void close_stopstate(char i);//关闭某一位刷新
	bool get_stopstate(char i);
	void new_stop_type();
	char get_stoptype();

};

//辉光管---------------------------------------------------------------
class Glow_tube
{
private:
	int binary_queue[8];//8个16位二进制数
	uchar state[2];//当前的工作状态
public:
	Glow_tube();
	uchar last_state[2];
	void get_new_number();//获取新数字【状态】
	void anime(int a);//获取不同动画
	uchar return_number(uchar i);//送出数据
	void change_state(uchar state_num, uchar new_state);//切换模式
	uchar get_state(int a);
	void set_number(uchar a);//全部置数
	void set_queue(uchar a,uchar b);
};

//输出方法-----------------------------------------------------------------
class digital_way
{
private:
public:
	void digital_mission();//输出函数
	void proofsuccess_anime();
	void proofailed_anime();
	void fourway_digital_once(bool a, bool b, bool c, bool d, bool rl);//一次四位并行输出
	void digital_onenumber(int a, bool rl);//输出一个数字
};

//输入方法-----------------------------------------------------------
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

//菜单模块--------------------------------------
class menu
{
private:
public:
};

//手动调时模块
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