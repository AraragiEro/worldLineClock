#define PT_USE_TIMER

#include "class_initialization.h"
#include <pt.h>
#include <pt-timer.h>
#include <pins_arduino.h> 
#include <SPI.h>
#include <DS3231.h>
#include <IRremote.h>
#include <math.h>

//#include <variant.h> 

//-----------------------实例化----------------------------
Time E_Time = Time();

DS3231 E_clock;

world_line_change E_world_line_change = world_line_change();

Glow_tube E_Glow_tube = Glow_tube();

digital_way E_digital_way;

analog_in_way ON_worldline = analog_in_way();

GPSdata E_GPSdata;

analog_in_way E_analog_in_way=analog_in_way();

IRrecv irrecv(RECV_PIN);//红外类声明，端口初始化，解析初始化。
decode_results results;

prooftime E_prooftime;


extern struct pt pt_autoprof_time;//自动校时

//Time类函数定义----------------------------------------------------------
Time::Time()
{
	//E_clock.getTime(Year,Month,Date,DoW,hour,min,sec);
	h24 = false;
	PM = false;
	Century = false;
}

void Time::GetTime()
{
	//E_clock.getTime(Year, Month, Date, DoW, hour, min, sec);
	hour = E_clock.getHour(h24,PM);
	min = E_clock.getMinute();
	sec = E_clock.getSecond();
	Year = E_clock.getYear();
	Month = E_clock.getMonth(Century);
	Date = E_clock.getDate();
}

uchar Time::get_sec()
{
	return sec;
}



void GPSdata::restart_EGPS()
{
	hour = 0;
	min = 0;
	sec = 0;
	Date = 0;
	Month = 0;
	Year = 0;
	E_GPSdata.isGetData = false;
	E_GPSdata.isParseData = false;
	E_GPSdata.isUsefull = false;
}

//GPS类----------------------------------------------------------------------
void GPSdata::gpsRead()
{
	for (int a = 0; a < 20000; a++)
	{
		if (isGetData == false)
		{
			for (uchar a = 0; a < 10; a++)
			{
				while (GpsSerial.available())//有数据时
				{
					gpsRxBuffer[ii++] = GpsSerial.read();
					if (ii == gpsRxBufferLength)clrGpsRxBuffer();
				}

				char* GPS_BufferHead;
				char* GPS_BufferTail;
				if ((GPS_BufferHead = strstr(gpsRxBuffer, "$GPRMC,")) != NULL || (GPS_BufferHead = strstr(gpsRxBuffer, "$GNRMC,")) != NULL)/*缓冲内有GPRMC或GNRMC字节*/
				{
					if (((GPS_BufferTail = strstr(GPS_BufferHead, "\r\n")) != NULL) && (GPS_BufferTail > GPS_BufferHead))
					{
						memcpy(E_GPSdata.GPS_Buffer, GPS_BufferHead, GPS_BufferTail - GPS_BufferHead);
						E_GPSdata.isGetData = true;//更改标志：已获得信息

						clrGpsRxBuffer();
					}
				}
			}
		}
		else break;
	}
	if (isGetData == false)
	{
		errorLog(0);		//GPS无法获得信息
	}
}

void GPSdata::clrGpsRxBuffer()
{
	memset(gpsRxBuffer, 0, gpsRxBufferLength);      //将某一块内存前N字节置ch
	ii = 0;
}

void GPSdata::errorLog(int num)
{

	E_Glow_tube.change_state(0,time_state);
	E_Time.time_mission_key = 0;
	for (uchar b = 0; b < 3; b++)
	{
		E_Glow_tube.set_number(num);
		E_digital_way.digital_mission();
		delay(700);
		E_Glow_tube.set_number(12);
		E_digital_way.digital_mission();
		delay(700);
	}
}

void GPSdata::parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	if (E_GPSdata.isGetData)
	{
		E_GPSdata.isGetData = false;
		for (int i = 0; i <= 6; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(E_GPSdata.GPS_Buffer, ",")) == NULL)
				{
					errorLog(1);	//解析错误
				}
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2];
					switch (i)
					{
					case 1:memcpy(E_GPSdata.UTCTime, subString, subStringNext - subString); break;	//获取UTC时间
					case 2:memcpy(usefullBuffer, subString, subStringNext - subString); break;	//定位状态
					/*case 3:memcpy(E_GPSdata.latitude, subString, subStringNext - subString); break;	//获取纬度信息
					case 4:memcpy(E_GPSdata.N_S, subString, subStringNext - subString); break;	//获取N/S
					case 5:memcpy(E_GPSdata.longitude, subString, subStringNext - subString); break;	//获取纬度信息
					case 6:memcpy(E_GPSdata.E_W, subString, subStringNext - subString); break;	//获取E/W*/

					default:break;
					}

					subString = subStringNext;
					E_GPSdata.isParseData = true;
					if (usefullBuffer[0] == 'A')
						E_GPSdata.isUsefull = true;
					else if (usefullBuffer[0] == 'V')
						E_GPSdata.isUsefull = false;
				}
				else
				{
					if(i==1)
					errorLog(2);	//解析错误2
				}
			}
		}
	}
}

void GPSdata::tr_gps_time()
{
	hour = (UTCTime[0] - 48) * 10 + UTCTime[1] - 48 + 8;
	min = (UTCTime[2] - 48) * 10 + UTCTime[3] - 48;
	sec = (UTCTime[4] - 48) * 10 + UTCTime[5] - 48;
	Date = ((UTCdate[0] - 48) * 10 + UTCTime[1] - 48) + hour / 24;
	Month = (UTCdate[2] - 48) * 10 + UTCTime[3] - 48;
	Year = (UTCdate[4] - 48) * 10 + UTCTime[5] - 48;
	hour = hour % 24;
}

void GPSdata::prooftime()
{
	E_clock.setHour(hour);
	E_clock.setMinute(min);
	E_clock.setSecond(sec);
	E_clock.setDate(Date);
	E_clock.setMonth(Month);
	E_clock.setYear(Year);
}

//世界线类-----------------------------------------------------------------------
world_line_change::world_line_change()
{
	flash_number[8] = { 0 };
	stop_number[8] = { 0 };
	srand(millis());
	main_worldline = rand() % 10;
}

void world_line_change::get_new_flashworldline()
{
	srand(millis());
	for (uchar i=0;i<8;i++)
	{
		flash_number[i] = rand()%10;
		//flash_number[i] = 99;
	}
}

void world_line_change::new_worldline()
{
	srand(millis());
	for (uchar i = 7; i >= 1; i--)
	{
		stop_number[i] = rand() % 10;
	}
	stop_number[0] = 0;//main_worldline * 2 ^ 4 + 10;
	stop_number[1] = 10;//main_worldline * 2 ^ 4 + 10;
}

uchar world_line_change::get_flashnum(uchar i)
{
	return flash_number[i];
}

uchar world_line_change::get_stopnum(uchar i)
{
	return stop_number[i];
}

void world_line_change::new_stopqueue()
{
	for (char i=-1;i<8;i++)
	{
		stop_queue[i] = i;
	}
	srand(millis());
	char value;
	char median;
	for (int index=0;index<8;index++)
	{
		value = rand() % 8;
		median = stop_queue[index];
		stop_queue[index] = stop_queue[value];
		stop_queue[value] = median;
	}
}

char world_line_change::get_stopqueue(char i)
{
	return stop_queue[i];
}

void world_line_change::change_allstopstate_ON()
{
	for (char i=7;i>=0;i--)
	{
		stop_state[i] = true;
	}
}

void world_line_change::close_stopstate(char i)
{
	stop_state[i] = false;
}

bool world_line_change::get_stopstate(char i)
{
	return stop_state[i];
}

void world_line_change::new_stop_type()
{
	stop_type = 1 + rand() % 3;
}

char world_line_change::get_stoptype()
{
	return stop_type;
}



//辉光管类------------------------------------------------------------------------
Glow_tube::Glow_tube()
{
	binary_queue[4] = { 0 };
	state[0] = 0;
}



void Glow_tube::get_new_number()//修改输出数据
{
	switch (E_Glow_tube.state[0])
	{
	case time_state://走时
	{
		if (E_Glow_tube.state[1] == 1)
		{
			binary_queue[0] = E_clock.getHour(E_Time.h24, E_Time.PM) / 10;
			binary_queue[1] = E_clock.getHour(E_Time.h24, E_Time.PM) % 10;
			binary_queue[2] = 10;
			binary_queue[3] = E_clock.getMinute() / 10;
			binary_queue[4] = E_clock.getMinute() % 10;
			binary_queue[5] = 10;
			binary_queue[6] = E_clock.getSecond() / 10;
			binary_queue[7] = E_clock.getSecond() % 10;
		}
		
		else if (E_Glow_tube.state[1] == 2)
		{
			binary_queue[0] = 2;
			binary_queue[1] = 0;
			binary_queue[2] = int(E_clock.getYear() / 10) % 10;
			binary_queue[3] = E_clock.getYear() % 10;
			binary_queue[4] = E_clock.getMonth(E_Time.Century) / 10;
			binary_queue[5] = E_clock.getMonth(E_Time.Century) % 10;
			binary_queue[6] = E_clock.getDate() / 10;
			binary_queue[7] = E_clock.getDate() % 10;
		}

		else if (E_Glow_tube.state[1] == 3)
		{
			binary_queue[E_clock.getDoW() - 1] = E_clock.getDoW();
		}
		}; break;

	case worldlinechange_state://世界线闪烁
	{
		for (char c=7;c>=0;c--)
		{
			binary_queue[c] =
				(E_world_line_change.get_stopstate(c) == true ? (E_world_line_change.get_flashnum(c)) : (E_world_line_change.get_stopnum(c)));
		}
	}; break;

	case close_state:			//关灯
	{
		for (char c = 0; c < 8; c++)
		{
			binary_queue[c] = 12;
		}
	}; break;

	case timeproof_state:		//校时
	{
		if (E_Glow_tube.state[1] == timeproof_state_date)
		{
			for (uchar a = 0; a < 8; a++)
			{
				E_Glow_tube.binary_queue[a] = E_prooftime.pDateQueue[a];
			}
		}
		
		else if (E_Glow_tube.state[1] == timeproof_state_day)
		{
			binary_queue[E_prooftime.pDay-1] = E_prooftime.pDay;
		}

		else if (E_Glow_tube.state[1] == timeproof_state_time)
		{
			binary_queue[0] = E_prooftime.get_hour() / 10;
			binary_queue[1] = E_prooftime.get_hour() % 10;
			binary_queue[2] = 10;
			binary_queue[3] = E_prooftime.get_Min() / 10;
			binary_queue[4] = E_prooftime.get_Min() % 10;
			binary_queue[5] = 10;
			binary_queue[6] = E_prooftime.get_sec() / 10;
			binary_queue[7] = E_prooftime.get_sec() % 10;
		}
	}; break;

	default:
		break;
	}
}

void Glow_tube::anime(int a)
{
	if (a == 1)			//开机动画
	{
		set_number(12);							//开始清零
		for (char k = 0; k < 8; k++) {			//流水01
			binary_queue[k] = 1;
			E_digital_way.digital_mission();
			delay(100);
		}
		for (char k = 0; k < 8; k++) {
			binary_queue[k] = 0;
			E_digital_way.digital_mission();
			delay(100);
		}
		set_number(12);							//清零

		for (char b = 0; b < 8; b++)			//流水点
		{
			binary_queue[b] = 1;
			E_digital_way.digital_mission();
			delay(1000 / (b+1));
		}
		delay(1000);
	}

	else if (a == 2)							//自动校时动画
	{
		set_number(12);
		for (char b = 0; b < 2; b++)
		{
			for (char c = 0; c < 4; c++)
			{
				binary_queue[3 - c] = 1;
				binary_queue[4 + c] = 1;
				E_digital_way.digital_mission();
				delay(100);
			}
			for (char d = 0; d < 4;d++)
			{
				binary_queue[3 - d] = 12;
				binary_queue[4 + d] = 12;
				E_digital_way.digital_mission();
				delay(100);
			}
		}
	}

	else if (a == 3)									//世界线启动动画
	{
		set_number(12);
		for (uchar c = 0; c < 4; c++)
		{
			for (uchar b = 0; b < 8; b++)
			{
				binary_queue[b] = 1;
				if (b != 0) binary_queue[b - 1] = 12;
				else binary_queue[7] = 12;
				E_digital_way.digital_mission();
				delay(50);
			}
		}
	}

	else if (a == 4)								//【暂定gps】错误动画
	{
		set_number(12);
		E_digital_way.digital_mission();

		for (uchar b = 0; b < 3; b++)
		{
			set_number(0);
			E_digital_way.digital_mission();
			delay(700);
			set_number(12);
			E_digital_way.digital_mission();
			delay(700);
		}
	}
}



uchar Glow_tube::return_number(uchar i)
{
	return binary_queue[i];
}

void Glow_tube::change_state(uchar state_num, uchar new_state)
{
	last_state[state_num] = state[state_num];
	state[state_num] = new_state;
}

uchar Glow_tube::get_state(int a)
{
	return state[a];
}

void Glow_tube::set_number(uchar a)
{
	for (char k = 0; k < 8; k++) {
		binary_queue[k] = a;
	}
	E_digital_way.digital_mission();
}

void Glow_tube::set_queue(uchar a,uchar b)
{
	binary_queue[a] = b;
}


//输出方法-----------------------------------------------------------------

void digital_way::digital_mission()//输出一次数据并点灯
{

	for (char glow = 0; glow < 8; glow++)
	{
		fourway_digital_once(1, 1, 1, 1, 0);
		fourway_digital_once(1, 1, 1, 1, 0);
		fourway_digital_once(1, 1, 1, 1, 0);
		fourway_digital_once(1, 1, 1, 1, 0);

		E_digital_way.digital_onenumber(E_Glow_tube.return_number(7), 0);
		E_digital_way.digital_onenumber(E_Glow_tube.return_number(6), 0);
		E_digital_way.digital_onenumber(E_Glow_tube.return_number(5), 0);
		E_digital_way.digital_onenumber(E_Glow_tube.return_number(4), 0);

		fourway_digital_once(1, 1, 1, 1, 0);
		fourway_digital_once(1, 1, 1, 1, 0);
		fourway_digital_once(1, 1, 1, 1, 0);
		fourway_digital_once(1, 1, 1, 1, 0);

		E_digital_way.digital_onenumber(E_Glow_tube.return_number(3), 1);
		E_digital_way.digital_onenumber(E_Glow_tube.return_number(2), 1);
		E_digital_way.digital_onenumber(E_Glow_tube.return_number(1), 1);
		E_digital_way.digital_onenumber(E_Glow_tube.return_number(0), 1);
		PORTD = PORTD | B00001000;
		PORTD = PORTD & B11110111;
	}
}

	void digital_way::fourway_digital_once(bool a, bool b, bool c, bool d, bool rl)
	{

		if (rl == 0)
		{
			unsigned int k1 = a * 16
				+ b * 32
				+ c * 64
				+ d * 128 + 8 + 4 + 2 + 1;

			PORTD = PORTD | B11110000;
			PORTD = PORTD & k1;
		}
		else
		{
			unsigned int k1 = d * 16
				+ c * 32
				+ b * 64
				+ a * 128 + 8 + 4 + 2 + 1;

			PORTD = PORTD | B11110000;
			PORTD = PORTD & k1;
		}
		//Serial.print(PORTD,BIN);
		PORTD = PORTD | B00000100;
		PORTD = PORTD & B11111011;
	}

	void digital_way::digital_onenumber(int a, bool rl)
	{
		///Serial.println(a);
		switch (a)
		{
		case 0: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(0, 1, 1, 1, rl);//2
		}; break;

		case 1: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 0, 1, 1, rl);//2
		}; break;

		case 2: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 0, 1, rl);//2
		}; break;

		case 3: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 0, rl);//2
		}; break;

		case 4: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(0, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 5: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 0, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 6: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 0, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;
		case 7: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 0, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 8: {
			fourway_digital_once(0, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 9: {
			fourway_digital_once(1, 0, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 10: {
			fourway_digital_once(1, 1, 0, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 11: {
			fourway_digital_once(1, 1, 1, 0, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;

		case 12: {
			fourway_digital_once(1, 1, 1, 1, rl);//数字1
			fourway_digital_once(1, 1, 1, 1, rl);//2
			fourway_digital_once(1, 1, 1, 1, rl);//2
		}; break;
		default:break;
		}
		//Serial.println("-----");
	}

void digital_way::proofsuccess_anime()
{
}

void digital_way::proofailed_anime()
{
}

//输入方法-----------------------------------------------------------
analog_in_way::analog_in_way()
{
	IR_state = false;
	IR_code = 0;
}

void analog_in_way::restart_analog()
{
	IR_state = false;
	last_IRcode = IR_code;
	IR_code = 0;
	irrecv.resume();
}

//手动调时模块-------------------------------------------------------

void prooftime::getproof_data()
{
	if (E_Glow_tube.get_state(1) == timeproof_state_date)
	{
		pDateQueue[0] = 2;
		pDateQueue[1] = 0;
		pDateQueue[2] = int(E_clock.getYear() / 10) % 10;
		pDateQueue[3] = E_clock.getYear() % 10;
		pDateQueue[4] = E_clock.getMonth(E_Time.Century) / 10;
		pDateQueue[5] = E_clock.getMonth(E_Time.Century) % 10;
		pDateQueue[6] = E_clock.getDate() / 10;
		pDateQueue[7] = E_clock.getDate() % 10;
	}

	else if (E_Glow_tube.get_state(1) == timeproof_state_day)
	{
		pDay = E_clock.getDoW();
	}

	else if (E_Glow_tube.get_state(1) == timeproof_state_time)
	{
		pDateQueue[0] = E_clock.getHour(E_Time.h24, E_Time.PM) / 10;
		pDateQueue[1] = E_clock.getHour(E_Time.h24, E_Time.PM) % 10;
		pDateQueue[2] = 10;
		pDateQueue[3] = E_clock.getMinute() / 10;
		pDateQueue[4] = E_clock.getMinute() % 10;
		pDateQueue[5] = 10;
		pDateQueue[6] = E_clock.getSecond() / 10;
		pDateQueue[7] = E_clock.getSecond() % 10;
	}
}

void prooftime::cofirm()
{
	if (E_Glow_tube.get_state(1) == timeproof_state_date)
	{
		uchar year =pDateQueue[2] * 10 + pDateQueue[3];
		uchar month = pDateQueue[4] * 10 + pDateQueue[5];
		uchar day = pDateQueue[6] * 10 + pDateQueue[7];
		E_clock.setYear(year);
		E_clock.setMonth(month);
		E_clock.setDate(day);
	}
	else if (E_Glow_tube.get_state(1) == timeproof_state_day)
	{
		E_clock.setDoW(pDay);
	}
}

void prooftime::get_time()
{
	sec = E_clock.getSecond();
	min = E_clock.getMinute();
	hour = E_clock.getHour(E_Time.h24, E_Time.PM);
}

int prooftime::get_Min()
{
	return min;
}

int prooftime::get_hour()
{
	return hour;
}

int prooftime::get_sec()
{
	return sec;
}

//--------------线程任务定义-----------------------
static int pt_IR_mission(struct pt *pt)//红外接收线程，每隔100ms检测红外输入。
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt,E_analog_in_way.IR_state == false);
		if (irrecv.decode(&results))
		{
			if (results.value!=0xFFFFFFFF)
			{
				E_analog_in_way.IR_code = results.value;
				E_analog_in_way.IR_state = true;
				Serial.println(E_analog_in_way.IR_code,HEX);
			}
			else
			{
				irrecv.resume();
			}
		}
		PT_TIMER_DELAY(pt,1000);
	}
	PT_END(pt);
}

static int pt_close_mission(struct  pt *pt)//关灯线程
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state(0) == close_state);
		E_Glow_tube.get_new_number();
		E_digital_way.digital_mission();

		while (E_Glow_tube.get_state(0) == close_state)
		{
			PT_TIMER_DELAY(pt, 1000);
		}
	}
	PT_END(pt);
}


//----------------------------------------------------------------------------------
static int pt_time_mission(struct  pt *pt)//显时线程，显时状态触发
{
	PT_BEGIN(pt);
	//-----任务-----
	
	E_Time.GetTime();
	while (1)
	{
		//时间模式触发
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state(0) == time_state);
		if (E_Glow_tube.get_state(1) == time_state_time)		//时间显示分支
		{
			if (E_clock.getSecond() - E_Time.get_sec() != 0)
			{
				E_Glow_tube.get_new_number();
				E_digital_way.digital_mission();
				//Serial.println(E_clock.getSecond());
				E_Time.GetTime();
				E_Time.time_mission_key++;
			}
			PT_TIMER_MICRODELAY(pt, 100000);
			if (E_Time.time_mission_key == 10)
			{
				if (
					(E_Glow_tube.last_state[0] == worldlinechange_state) ||
					(E_analog_in_way.last_IRcode != B_POWER)
					)
				{
					E_Glow_tube.change_state(0, close_state);
					E_Time.time_mission_key = 0;
				}
			}
			else if (E_Time.time_mission_key == 10)
			{
				E_Time.time_mission_key = 0;
			}
		}
		
		else if (E_Glow_tube.get_state(1) == time_state_date)	//日期显示分支
		{
			E_Glow_tube.get_new_number();
			E_digital_way.digital_mission();
			E_Time.time_mission_key++;
			delay(1000);
			PT_TIMER_DELAY(pt,1000);
			if (E_Time.time_mission_key == 5)
			{
				E_Glow_tube.change_state(0, close_state);
				E_Time.time_mission_key = 0;
			}
		}

		else if (E_Glow_tube.get_state(1) == time_state_day)
		{
			E_Time.GetTime();
			{static int a;
			for (a = 0; a < 8; a++)
			{
				E_Glow_tube.set_number(12);
				E_Glow_tube.set_queue(a, 10);
				E_Glow_tube.get_new_number();
				E_digital_way.digital_mission();
				PT_TIMER_DELAY(pt, 300);
			}}
			if (E_clock.getSecond() - E_Time.get_sec() != 0)
			{
				E_Time.GetTime();
				E_Time.time_mission_key++;
			}
			if (E_Time.time_mission_key == 5)
			{
				E_Glow_tube.change_state(0, close_state);
				E_Time.time_mission_key = 0;
			}
		}
	}
	PT_END(pt);
}


//----------------------------------------------------------------------------------
static int pt_worldline_mission(struct pt *pt)//世界线线程，世界线状态触发
{
	PT_BEGIN(pt);
	while (1)
	{
		//世界线线模式时触发
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state(0) == worldlinechange_state);
		
		//------二，检测世界线阶段
		{static char jishu;
		for (jishu=0;jishu<3;)//计数满三次进入下一阶段
		{
			E_world_line_change.change_allstopstate_ON();//刷新位重置
			E_world_line_change.new_worldline();//固定世界线
			E_world_line_change.new_stop_type();//获得本次停止模式
			//前置刷新时间1.5s
			{static char a;
			for (a = 0; a < 90; a++)
			{
				E_world_line_change.get_new_flashworldline();
				E_Glow_tube.get_new_number();
				E_digital_way.digital_mission();
				PT_TIMER_DELAY(pt, wl_speed);
				if (a % 5 == 0)
				{
					if (E_Glow_tube.get_state(0) != worldlinechange_state)
						PT_RESTART(pt);
				}
			}}
			//-----------------------------------------------------
			if (E_world_line_change.get_stoptype() == 1)//①逐位停止刷新
				{
					{static char e;
					for (e = 7; e >= -1; e--)
					{
						{static char j;
						for (j = 0; j < 30; j++)
						{
							E_world_line_change.get_new_flashworldline();
							E_Glow_tube.get_new_number();
							E_digital_way.digital_mission();
							PT_TIMER_DELAY(pt, wl_speed);
						}}
						E_world_line_change.close_stopstate(e);
						if (E_Glow_tube.get_state(0) != worldlinechange_state)
							PT_RESTART(pt);
					}}
					jishu++;//计数一次
					PT_TIMER_DELAY(pt, 600);
				}
			//------------------------------------------------
			else if (E_world_line_change.get_stoptype() == 2)//②选位停止刷新
				{
					E_world_line_change.new_stopqueue();//获取本次停止队列
					{static char e;
					for (e = 7; e >= -1; e--)
					{
						{static char j;
						for (j = 0; j < 30; j++)
						{
							E_world_line_change.get_new_flashworldline();
							E_Glow_tube.get_new_number();
							E_digital_way.digital_mission();
							PT_TIMER_DELAY(pt, wl_speed);
						}}
						E_world_line_change.close_stopstate(E_world_line_change.get_stopqueue(e));
						if (E_Glow_tube.get_state(0) != worldlinechange_state)
							PT_RESTART(pt);
					}}
					jishu++;//计数一次
					PT_TIMER_DELAY(pt, 600);
				}
			//-----------------------------------------------------
			else if (E_world_line_change.get_stoptype() == 3)//③同时停止
				{
					E_world_line_change.get_new_flashworldline();
					E_Glow_tube.get_new_number();
					E_digital_way.digital_mission();
					PT_TIMER_DELAY(pt, 300);
				}
		}}
		

		//------三，确认世界线
		{
			E_world_line_change.change_allstopstate_ON();//刷新位重置
			E_world_line_change.new_stopqueue();//获取本次停止队列
			{static char e;
			for (e = 7; e >= -1; e--)
			{
				{static uchar j;
				for (j = 0; j < 30+(15*(7-e)); j++)
				{
					E_world_line_change.get_new_flashworldline();
					E_Glow_tube.get_new_number();
					E_digital_way.digital_mission();
					PT_TIMER_DELAY(pt, wl_speed);
				}}
				E_world_line_change.close_stopstate(E_world_line_change.get_stopqueue(e));
				if (E_Glow_tube.get_state(0) != worldlinechange_state)
					PT_RESTART(pt);
			}}
			PT_TIMER_DELAY(pt, 5000);
		}
		E_Glow_tube.change_state(0,time_state);
		E_Glow_tube.change_state(1, time_state_time);
		E_Time.time_mission_key = 0;
	}
	PT_END(pt);
}


//----------------------------------------------------------------------------------
static int pt_autoprooftime_mission(struct  pt *pt)//自动校时线程
{
	PT_BEGIN(pt);
	while (1)
	{
		static bool proof_state = false;
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state(0) == autotimeproof_state);

		E_GPSdata.restart_EGPS();
		E_GPSdata.clrGpsRxBuffer();
		
		{
			E_GPSdata.gpsRead();
			E_GPSdata.parseGpsBuffer();
			if (E_Glow_tube.get_state(0) != autotimeproof_state)
				PT_RESTART(pt);
			E_GPSdata.tr_gps_time();
		}
		if (E_GPSdata.sec != 0 && E_GPSdata.hour != 8)
		{
			proof_state = true;
		}
		/*Serial.println(E_GPSdata.UTCTime);
		Serial.println(E_clock.getMinute());
		Serial.println(E_GPSdata.hour);
		Serial.println(E_GPSdata.min);
		Serial.println(E_GPSdata.sec);*/
		if (proof_state == true)
		{
			E_GPSdata.prooftime();
			E_Glow_tube.change_state(0,time_state);
			E_Time.time_mission_key = 0;
		}
		else
		{
			E_GPSdata.errorLog(3);	//数据异常3
		}
		
		
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------------
static int pt_desktop_mission(struct pt *pt)//遥控器操作，常驻
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt, E_analog_in_way.IR_state == true);
		//else if 结构分状态判断输入
		if (E_Glow_tube.get_state(0) == time_state)	//时间状态时分支
		{
			if (E_analog_in_way.IR_code == B_POWER)				//开关灯
			{
				E_Glow_tube.change_state(0, close_state);
				E_analog_in_way.restart_analog();
			}
			else if (E_analog_in_way.IR_code == B_HOME)			//世界线
			{
				E_Glow_tube.change_state(0, worldlinechange_state);
				E_Glow_tube.anime(3);
				E_analog_in_way.restart_analog();
			}
			else if (E_analog_in_way.IR_code == B_CANCEL)			//自动校准
			{
				E_Glow_tube.change_state(0, autotimeproof_state);
				E_Glow_tube.anime(2);
				E_analog_in_way.restart_analog();
			}
			else if (E_analog_in_way.last_IRcode == B_ANALOGSOURCE)		//手动校时
			{
				E_Glow_tube.change_state(0, timeproof_state);
				E_Glow_tube.change_state(1, timeproof_state_time);
				E_prooftime.getproof_data();
				E_analog_in_way.restart_analog();
			}

			else if (E_Glow_tube.get_state(1) == time_state_time)	//二级时间分支
			{
				if (E_analog_in_way.IR_code == B_UP)
				{
					E_Glow_tube.change_state(1, time_state_day);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_DOWN)
				{
					E_Glow_tube.change_state(1, time_state_date);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else E_analog_in_way.restart_analog();
			}
			else if (E_Glow_tube.get_state(1) == time_state_date)
			{
				if (E_analog_in_way.IR_code == B_UP)
				{
					E_Glow_tube.change_state(1, time_state_time);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_DOWN)
				{
					E_Glow_tube.change_state(1, time_state_day);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else E_analog_in_way.restart_analog();
			}
			else if (E_Glow_tube.get_state(1) == time_state_day)
			{
				if (E_analog_in_way.IR_code == B_UP)
				{
					E_Glow_tube.change_state(1, time_state_date);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_DOWN)
				{
					E_Glow_tube.change_state(1, time_state_time);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else E_analog_in_way.restart_analog();
			}
			else E_analog_in_way.restart_analog();
		}
		//=====================================================================
		else if (E_Glow_tube.get_state(0) == close_state)	//待机状态时分支
		{
			if (E_analog_in_way.IR_code == B_HOME)
			{
				E_Glow_tube.change_state(0, worldlinechange_state);//世界线
				E_Glow_tube.anime(3);
				E_analog_in_way.restart_analog();
			}
			else if (E_analog_in_way.IR_code == B_POWER)		//时间，常亮或延迟判断在时间线程
			{
				E_Glow_tube.change_state(0, time_state);
				E_Glow_tube.change_state(1, time_state_time);
				E_Time.time_mission_key = 0;
				E_analog_in_way.restart_analog();
			}
			else
			{
				E_Glow_tube.change_state(0, time_state);	//普通唤醒，延时关闭
				E_Glow_tube.change_state(1, time_state_time);
				E_Time.time_mission_key = 0;
				E_analog_in_way.restart_analog();
			}
		}
		//=====================================================================
		else if (E_Glow_tube.get_state(0) == worldlinechange_state)//世界线状态分支
		{
			if (E_analog_in_way.IR_code != 0) {
				E_Glow_tube.change_state(0, time_state);
				E_Glow_tube.change_state(1, time_state_time);
				E_Time.time_mission_key = 0;
				E_analog_in_way.restart_analog();
			}
			else E_analog_in_way.restart_analog();
		}
		//=====================================================================
		else if (E_Glow_tube.get_state(0) == timeproof_state)		//手动校时分支
		{
			if (E_analog_in_way.IR_code == B_ANALOGSOURCE)
			{
				E_Glow_tube.change_state(0, time_state);	//返回时间
				E_Glow_tube.change_state(1, time_state_time);
				E_Time.time_mission_key = 0;
				E_analog_in_way.restart_analog();
			}
			else if (E_Glow_tube.get_state(1) == timeproof_state_date)	//日期二级分支
			{
				if (E_analog_in_way.IR_code == B_ADD)			//上下左右换位调时
				{
					E_prooftime.pDateQueue[E_prooftime.queue] = (E_prooftime.pDateQueue[E_prooftime.queue] + 1) % 10;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_MINUS)
				{
					if (E_prooftime.pDateQueue[E_prooftime.queue] != 0)
						E_prooftime.pDateQueue[E_prooftime.queue] = (E_prooftime.pDateQueue[E_prooftime.queue] - 1) % 10;
					else
						E_prooftime.pDateQueue[E_prooftime.queue] = 9;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_LEFT)
				{
					if (E_prooftime.queue != 0)
						E_prooftime.queue--;
					else
						E_prooftime.queue = 7;
					E_analog_in_way.restart_analog();
				}

				else if (E_analog_in_way.IR_code == B_RIGHT)
				{
					E_prooftime.queue = (E_prooftime.queue + 1) % 8;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_UP)
				{
					E_Glow_tube.change_state(1, timeproof_state_time);
					E_prooftime.getproof_data();
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_DOWN)
				{
					E_Glow_tube.change_state(1, timeproof_state_day);
					E_prooftime.getproof_data();
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_Confirm)
				{
					E_prooftime.cofirm();
					E_Glow_tube.change_state(0, time_state);	//返回时间
					E_Glow_tube.change_state(1, time_state_time);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else E_analog_in_way.restart_analog();
			}

			else if (E_Glow_tube.get_state(1) == timeproof_state_day)
			{
				if (E_analog_in_way.IR_code == B_ADD)			//上下左右换位调时
				{
					E_prooftime.pDay = (E_prooftime.pDay + 1) % 7;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_MINUS)
				{
					if (E_prooftime.pDay != 1)
						E_prooftime.pDay--;
					else
						E_prooftime.pDay = 7;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_UP)
				{
					E_Glow_tube.change_state(1, timeproof_state_date);
					E_prooftime.getproof_data();
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_DOWN)
				{
					E_Glow_tube.change_state(1, timeproof_state_time);
					E_prooftime.getproof_data();
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_Confirm)
				{
					E_prooftime.cofirm();
					E_Glow_tube.change_state(0, time_state);	//返回时间
					E_Glow_tube.change_state(1, time_state_time);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else E_analog_in_way.restart_analog();
			}

			else if (E_Glow_tube.get_state(1) == timeproof_state_time)
			{
				if (E_analog_in_way.IR_code == B_ADD)			//上下左右换位调时
				{
					if (E_prooftime.queue == 5 || E_prooftime.queue == 7)
					{

					}

					E_prooftime.pDateQueue[E_prooftime.queue] = (E_prooftime.pDateQueue[E_prooftime.queue] + 1) % 10;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_MINUS)
				{
					if (E_prooftime.pDateQueue[E_prooftime.queue] != 0)
						E_prooftime.pDateQueue[E_prooftime.queue] = (E_prooftime.pDateQueue[E_prooftime.queue] - 1) % 10;
					else
						E_prooftime.pDateQueue[E_prooftime.queue] = 9;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_LEFT)
				{
					if (E_prooftime.queue != 0)
						E_prooftime.queue--;
					else
						E_prooftime.queue = 7;
					E_analog_in_way.restart_analog();
				}

				else if (E_analog_in_way.IR_code == B_RIGHT)
				{
					E_prooftime.queue = (E_prooftime.queue + 1) % 8;
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_UP)
				{
					E_Glow_tube.change_state(1, timeproof_state_day);
					E_prooftime.getproof_data();
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_DOWN)
				{
					E_Glow_tube.change_state(1, timeproof_state_date);
					E_prooftime.getproof_data();
					E_analog_in_way.restart_analog();
				}
				else if (E_analog_in_way.IR_code == B_Confirm)
				{
					E_prooftime.cofirm();
					E_Glow_tube.change_state(0, time_state);	//返回时间
					E_Glow_tube.change_state(1, time_state_time);
					E_Time.time_mission_key = 0;
					E_analog_in_way.restart_analog();
				}
				else E_analog_in_way.restart_analog();
			}
			//=====================================================================
			else E_analog_in_way.restart_analog();//不符合，IR输入清零		
		}
	}
	PT_END(pt);
}


static int pt_prooftime_mission(struct pt *pt)				//手动校时显示【暂定
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt,E_Glow_tube.get_state(0) == timeproof_state);
		if (E_Glow_tube.get_state(1) == timeproof_state_time)
		{
			{static uchar d;
			for (d = 0; d < 12; d++)
			{
				if (d < 6)
				{
					E_Glow_tube.get_new_number();
					E_digital_way.digital_mission();
					PT_TIMER_DELAY(pt, 25);
				}
				else
				{
					E_Glow_tube.set_queue(E_prooftime.queue, 12);
					E_digital_way.digital_mission();
					PT_TIMER_DELAY(pt, 25);
				}
			}
			}
		}
		
		else if (E_Glow_tube.get_state(1) == timeproof_state_date)
		{
			{static uchar b;
			for (b = 0; b < 12; b++)
			{
				if (b < 6)
				{
					E_Glow_tube.get_new_number();
					E_digital_way.digital_mission();
					PT_TIMER_DELAY(pt, 25);
				}
				else
				{
					E_Glow_tube.set_queue(E_prooftime.queue, 12);
					E_digital_way.digital_mission();
					PT_TIMER_DELAY(pt, 25);
				}}
			}
		}
		else if (E_Glow_tube.get_state(1) == timeproof_state_day)
		{
			{static int a;
			for (a = 0; a < 8; a++)
			{
				E_Glow_tube.set_number(12);
				E_Glow_tube.set_queue(a, 10);
				E_Glow_tube.get_new_number();
				E_digital_way.digital_mission();
				PT_TIMER_DELAY(pt, 300);
			}}
		}
	}
	PT_END(pt);
}