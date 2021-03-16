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



//Time类函数定义----------------------------------------------------------
Time::Time()
{
	//E_clock.getTime(Year,Month,Date,DoW,hour,min,sec);
	h24 = false;
	PM = false;
}

void Time::GetTime()
{
	E_clock.getTime(Year, Month, Date, DoW, hour, min, sec);
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
	while (isGetData == false)
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

void GPSdata::clrGpsRxBuffer()
{
	memset(gpsRxBuffer, 0, gpsRxBufferLength);      //将某一块内存前N字节置ch
	ii = 0;
}

void GPSdata::errorLog(int num)
{
}

void GPSdata::parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	if (E_GPSdata.isGetData)
	{
		E_GPSdata.isGetData = false;
		for (int i = 0; i <= 9; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(E_GPSdata.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
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
					case 9:memcpy(E_GPSdata.UTCdate, subString, subStringNext - subString); break;	//获取E/W

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
					errorLog(2);	//解析错误
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
		stop_number[i] = 0;//rand() % 100;
		stop_number[0] = 0;//main_worldline * 2 ^ 4 + 10;
	}
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
	state = 0;
}


int Glow_tube::trans_new_number(char a)
{
	unsigned int b = ~(0b100000000000 >> a);
	return b;
}

void Glow_tube::get_new_number()//修改输出数据
{
	switch (E_Glow_tube.state)
	{
	case time_state://走时
	{
		binary_queue[7] = trans_new_number(E_clock.getHour(E_Time.h24, E_Time.PM) / 10);
		binary_queue[6] = trans_new_number(E_clock.getHour(E_Time.h24, E_Time.PM) % 10);
		binary_queue[5] = 10;
		binary_queue[4] = trans_new_number(E_clock.getMinute() / 10);
		binary_queue[3] = trans_new_number(E_clock.getMinute() % 10);
		binary_queue[2] = 10;
		binary_queue[1] = trans_new_number(E_clock.getSecond() / 10);
		binary_queue[0] = trans_new_number(E_clock.getSecond() % 10);
		}; break;

	case worldlinechange_state://世界线闪烁
	{
		for (char c=7;c>=0;c--)
		{
			binary_queue[c] =
				(E_world_line_change.get_stopstate(c) == true ? trans_new_number(E_world_line_change.get_flashnum(c)) : trans_new_number(E_world_line_change.get_stopnum(c)));
		}
	}; break;

	default:
		break;
	}
}

void Glow_tube::anime_number()
{

}



uchar Glow_tube::return_number(uchar i)
{
	return binary_queue[i];
}

void Glow_tube::change_state(uchar new_state)
{
	last_state = state;
	state = new_state;
}

uchar Glow_tube::get_state()
{
	return state;
}


//输出方法-----------------------------------------------------------------

void digital_way::digital_mission()//输出一次数据并点灯
{
	
	for (char glow = 0; glow < 8; glow++)
	{
		for (char d = 0; d < 12; d++)
		{
			char temp = fmod(E_Glow_tube.return_number(glow) / pow(2, d), 2);
			digitalWrite(CLK, HIGH);
			NOP;
			digitalWrite(CLK, LOW);
			NOP;
		}
		if ((glow + 1) % 4 == 0)
		{
			for (char d = 0; d < 16; d++)
			{
				digitalWrite(CLK, HIGH);
				NOP;
				digitalWrite(CLK, LOW);
				NOP;
			}
		}
	}
	digitalWrite(LE, HIGH);
	NOP;
	digitalWrite(LE, LOW);
	NOP;
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
				Serial.println(E_analog_in_way.IR_code);
			}
			else
			{
				irrecv.resume();
			}
		}
		PT_TIMER_DELAY(pt,100);
	}
	PT_END(pt);


}

static int pt_time_mission(struct  pt *pt)//显时线程，显时状态触发
{
	PT_BEGIN(pt);
	//-----任务-----
	
	E_Time.GetTime();
	static int time_mission_key;
	time_mission_key = 0;
	while (1)
	{
		//时间模式触发
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state() == time_state);
		if(E_clock.getSecond() - E_Time.get_sec() != 0)
		{
			E_Glow_tube.get_new_number();
			E_digital_way.digital_mission();
			//Serial.println(E_clock.getSecond());
			E_Time.GetTime();
		}
		time_mission_key++;
		PT_TIMER_MICRODELAY(pt, 100000);
		if (time_mission_key == 100 && E_analog_in_way.last_IRcode != B_POWER&&E_analog_in_way.last_IRcode != B_HOME&&E_Glow_tube.last_state == close_state ||
			time_mission_key == 100 && E_Glow_tube.last_state == worldlinechange_state ||
			time_mission_key == 100 && E_Glow_tube.last_state == timeproof_state)
		{
			E_Glow_tube.change_state(close_state);
			time_mission_key = 0;
		}
		else
		{
			time_mission_key = 0;
		}
	}
	PT_END(pt);
}

static int pt_worldline_mission(struct pt *pt)//世界线线程，世界线状态触发
{
	PT_BEGIN(pt);
	while (1)
	{
		//世界线线模式时触发
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state() == worldlinechange_state);
		//------一，启动动画

		//------二，检测世界线阶段
		{static char jishu;
		for (jishu=0;jishu<4;)//计数满三次进入下一阶段
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
				PT_TIMER_DELAY(pt, 17);
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
							PT_TIMER_DELAY(pt, 17);
						}}
						E_world_line_change.close_stopstate(e);
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
							PT_TIMER_DELAY(pt, 17);
						}}
						E_world_line_change.close_stopstate(E_world_line_change.get_stopqueue(e));
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
					PT_TIMER_DELAY(pt, 17);
				}}
				E_world_line_change.close_stopstate(E_world_line_change.get_stopqueue(e));
			}}
			PT_TIMER_DELAY(pt, 5000);
		}
		E_Glow_tube.change_state(time_state);
	}
	PT_END(pt);
}

static int pt_autoprooftime_mission(struct  pt *pt)//自动校时线程
{
	PT_BEGIN(pt);
	static char proof_key;
	static bool proof_state = false;
	while (1)
	{
		PT_WAIT_UNTIL(pt, E_Glow_tube.get_state() == timeproof_state);
		E_GPSdata.restart_EGPS();
		E_GPSdata.clrGpsRxBuffer();

		/*while (E_GPSdata.sec==0&&E_GPSdata.hour==0)
		{
			E_GPSdata.gpsRead();
			E_GPSdata.parseGpsBuffer();
			E_GPSdata.tr_gps_time();
			PT_TIMER_DELAY(pt,10)
		}*/
		for (proof_key = 0; proof_key < 1000; proof_key++)
		{
			E_GPSdata.gpsRead();
			E_GPSdata.parseGpsBuffer();
			E_GPSdata.tr_gps_time();
			PT_TIMER_DELAY(pt, 10);
		}
		if (E_GPSdata.sec == 0 && E_GPSdata.hour == 0)
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
			E_Glow_tube.change_state(time_state);
		}
		
		
	}
	PT_END(pt);
}

static int pt_menu_state_mission(struct pt *pt)//遥控器操作，常驻
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt, E_analog_in_way.IR_state==true);
		if (E_Glow_tube.get_state()==time_state)//else if 结构分状态判断输入
		{
			if (E_analog_in_way.IR_code == B_POWER)//开关灯
			{
				E_Glow_tube.change_state(close_state);
				E_analog_in_way.restart_analog();
			}
			else if (E_analog_in_way.IR_code == B_HOME)
			{
				E_Glow_tube.change_state(worldlinechange_state);//世界线
				E_analog_in_way.restart_analog();
			}
			else if (E_analog_in_way.IR_code==B_CANCEL)
			{
				E_Glow_tube.change_state(timeproof_state);//自动校准
				E_analog_in_way.restart_analog();
			}
		}
		
		else if (E_Glow_tube.get_state() == close_state)
		{
			if (E_analog_in_way.IR_code==B_HOME)
			{
				E_Glow_tube.change_state(worldlinechange_state);//世界线
				E_analog_in_way.restart_analog();
			}
			else
			{
				E_Glow_tube.change_state(time_state);//时间，常亮或延迟判断在时间线程
				E_analog_in_way.restart_analog();
			}
		}
		else
		{
			E_analog_in_way.restart_analog();//不符合，IR输入清零		
		}
	}
	PT_END(pt);
}