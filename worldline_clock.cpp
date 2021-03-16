/*
Name:		worldline_clock.ino
Created:	2017/7/7 1:25:19
Author:	AraragiEro
*/

//----------------------附加库-------------------------

#include "class_initialization.h"
#include <SPI.h>
#define PT_USE_TIMER
#include <pt.h>
#include <pt-timer.h>
#include <SPI.h>
#include <DS3231.h>


//-------------------线程初始化--------------
static struct pt pt_time;
static struct pt pt_worldline;
static struct pt pt_digital;

//-----------------------setup----------------------------
void setup()
{

	SPI.setClockDivider(SPI_CLOCK_DIV2);
	//线程初始化--------------------
	PT_INIT(&pt_worldline);//
	PT_INIT(&pt_time);
	PT_INIT(&pt_digital);
	E_Glow_tube.change_state(worldlinechange_state);

	//引脚按钮初始化
	pinMode(worldline_button, INPUT_PULLUP);
	pinMode(SPIdataPin, OUTPUT);
	pinMode(SPIclockPin, OUTPUT);
	pinMode(HC595, OUTPUT);

}

//----------------------loop-----------------------------
void loop()
{
	pt_worldline_mission(&pt_worldline);
}


