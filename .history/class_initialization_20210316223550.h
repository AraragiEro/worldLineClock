#define close_state				0	//鍏崇伅寰呮満鐘舵€�
#define worldlinechange_state	1	//涓栫晫绾跨姸鎬�

#define time_state			2	//鏄炬椂鐘舵€�
	#define time_state_time			1		//鏃堕棿
	#define time_state_date			2		//鏃ユ湡
	#define time_state_day			3		//鏄熸湡

#define autotimeproof_state		3	//鑷姩鏍″鐘舵€�

#define timeproof_state	4	//鎵嬪姩鏍℃椂-鏃堕棿
	#define timeproof_state_time	1		//鏃堕棿
	#define timeproof_state_date	2		//鏃ユ湡
	#define timeproof_state_day		3		//鏄熸湡



#define digitalmission_ON		1
#define digitalmission_OFF		0

#define LE						3	//鏃堕挓
#define CLK						2

#define RECV_PIN				9	//绾㈠

#define light_PIN				8

#define gpsRxBufferLength		300

#define wl_speed				10


#define PT_out					PT_EXIT(pt)


//绾㈠閬ユ帶鍣ㄦ寜閿紪鐮�
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
	
#define B_POWER					0x6A68351E	//鎵嬫満绾㈠缂栫爜
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

//鏃堕棿绫�--------------------------------------------------------------
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
	void GetTime();//DS绫籫etTime鐨勭畝鍖栥€�
	uchar get_sec();
	bool h24;
	bool PM;
	uchar time_mission_key;
};

//GPS绫�------------------------------------------------------------
class GPSdata
{
private:
	char GPS_Buffer[80];
	
	bool isParseData;	//鏄惁瑙ｆ瀽瀹屾垚
	char latitude[11];		//绾害
	char N_S[2];		//N/S
	char longitude[12];		//缁忓害
	char E_W[2];		//E/W
	char UTCdate[6];		//UTC鏃ユ湡
	bool isUsefull;		//瀹氫綅淇℃伅鏄惁鏈夋晥
	
public:
	char UTCTime[11];
	uchar hour;
	uchar min;
	uchar sec;
	uchar Date;
	uchar Month;
	uchar Year;
	bool isGetData;		//鏄惁鑾峰彇鍒癎PS鏁版嵁
	char gpsRxBuffer[gpsRxBufferLength];
	unsigned int ii = 0;
	void restart_EGPS();
	void gpsRead();//鑾峰彇GPS鏁版嵁
	void clrGpsRxBuffer();//娓呯┖
	void errorLog(int num);
	void parseGpsBuffer();//瑙ｆ瀽GPS鏁版嵁
	void tr_gps_time();
	void prooftime();
};

//涓栫晫绾�--------------------------------------------------------------
class world_line_change
{
private:
	uchar flash_number[8];//鍒锋柊鏃剁殑8涓暟瀛�
	uchar stop_number[8];//鍚笅鏃剁殑8涓暟瀛�
	char main_worldline;//鏈€楂樹綅涓讳笘鐣岀嚎
	bool stop_state[8];//鏌愪綅鏄惁宸茬粡鍋滀笅
	char stop_queue[8];//閫変綅鍋滄闃熷垪
	char stop_type;
	
public:
	world_line_change();
	void get_new_flashworldline();//闅忔満涓€缁�8浣嶆暟瀛�
	void new_worldline();//鏂扮殑鍋滄鏁板瓧
	uchar get_flashnum(uchar i);
	uchar get_stopnum(uchar i);
	void new_stopqueue();//閫変綅娲楃墝
	char get_stopqueue(char i);
	void change_allstopstate_ON();//寮€鍚墍鏈変綅鍒锋柊
	void close_stopstate(char i);//鍏抽棴鏌愪竴浣嶅埛鏂�
	bool get_stopstate(char i);
	void new_stop_type();
	char get_stoptype();

};

//杈夊厜绠�---------------------------------------------------------------
class Glow_tube
{
private:
	int binary_queue[8];//8涓�16浣嶄簩杩涘埗鏁�
	uchar state[2];//褰撳墠鐨勫伐浣滅姸鎬�
public:
	Glow_tube();
	uchar last_state[2];
	void get_new_number();//鑾峰彇鏂版暟瀛椼€愮姸鎬併€�
	void anime(int a);//鑾峰彇涓嶅悓鍔ㄧ敾
	uchar return_number(uchar i);//閫佸嚭鏁版嵁
	void change_state(uchar state_num, uchar new_state);//鍒囨崲妯″紡
	uchar get_state(int a);
	void set_number(uchar a);//鍏ㄩ儴缃暟
	void set_queue(uchar a,uchar b);
};

//杈撳嚭鏂规硶-----------------------------------------------------------------
class digital_way
{
private:
public:
	void digital_mission();//杈撳嚭鍑芥暟
	void proofsuccess_anime();
	void proofailed_anime();
	void fourway_digital_once(bool a, bool b, bool c, bool d, bool rl);//涓€娆″洓浣嶅苟琛岃緭鍑�
	void digital_onenumber(int a, bool rl);//杈撳嚭涓€涓暟瀛�
};

//杈撳叆鏂规硶-----------------------------------------------------------
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

//鑿滃崟妯″潡--------------------------------------
class menu
{
private:
public:
};

//鎵嬪姩璋冩椂妯″潡
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