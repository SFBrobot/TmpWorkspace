#pragma config(UART_Usage, UART1, uartVEXLCD, baudRate19200, IOPins, None, None)
#pragma config(UART_Usage, UART2, uartNotUsed, baudRate4800, IOPins, None, None)
#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl1,  redLed,         sensorLEDtoVCC)
#pragma config(Sensor, dgtl2,  yellowLed,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl3,  greenLed,       sensorLEDtoVCC)
#pragma config(Sensor, dgtl4,  liftLim,        sensorTouch)
#pragma config(Sensor, dgtl12, testLed,        sensorLEDtoVCC)
#pragma config(Sensor, I2C_1,  flyEnc,         sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  liftEnc,        sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_3,  rWheelEnc,      sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_4,  lWheelEnc,      sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           frWheel,       tmotorVex393HighSpeed_HBridge, openLoop, reversed, driveRight)
#pragma config(Motor,  port2,           brWheel,       tmotorVex393HighSpeed_MC29, openLoop, reversed, driveRight, encoderPort, I2C_3)
#pragma config(Motor,  port3,           blWheel,       tmotorVex393HighSpeed_MC29, openLoop, driveLeft, encoderPort, I2C_4)
#pragma config(Motor,  port4,           intake,        tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           lift,          tmotorVex393HighSpeed_MC29, openLoop, driveLeft, encoderPort, I2C_2)
#pragma config(Motor,  port6,           trFly,         tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           brFly,         tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port8,           blFly,         tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           tlFly,         tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port10,          flWheel,       tmotorVex393HighSpeed_HBridge, openLoop, driveLeft)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#define RKCOMP_LCD //Enable LCD stuff in rkCompetition

#include "rkCompetition002.h"
#include "rkUtil002.h"
#include "rkControl/base002.h"
#include "rkControl/diff001.h"
#include "rkControl/tbh004.h"
#include "rkControl/tbhController002.h"

float flyLRPwr = 800, flySRPwr = 400;
const float autonFlyPwr = 800;
Tbh flyTbh, liftTbh;
ADiff flyDiff, liftDiff;
TbhController flyCtl, liftCtl;

const float greenThresh = 50;
const float yellowThresh = 75;

void lcdDispPwr(bool doUseLRPwr) {
	if (doUseLRPwr) {
		displayLCDCenteredString(0, "Long  Power:");
		displayLCDNumber(1, 0, flyLRPwr);
	}
	else {
		displayLCDCenteredString(0, "Short Power:");
		displayLCDNumber(1, 0, flySRPwr);
	}
}

task lcd() {
	string str;
	float pwrBtns;
	const TTimers lcdTimer = T3;
	const float flyPwrIncrement = 10;
	bool doUseLRPwr = true;

	while (true) {
		if (bIfiRobotDisabled || (nLCDButtons & kButtonCenter)) {
			sprintf(str, "%6.2f // %-6.2f", nImmediateBatteryLevel / 1000., BackupBatteryLevel / 1000.);

			clearLCD();
			displayLCDCenteredString(0, "Battery:");
			displayLCDString(1, 0, str);
		}
		else {
			if (vexRT[Btn7L] ^ vexRT[Btn7R])
				doUseLRPwr = vexRT[Btn7L];

			clearLCD();

			if (time1[lcdTimer] < 2000 || vexRT[Btn7L] || vexRT[Btn7R]) lcdDispPwr(doUseLRPwr);
			else {
				if (flyTbh.doRun) {
					displayLCDString(0, 0, "Error        Out");

					sprintf(str, "%7.2f  %-7.2f", flyTbh.err, flyTbh.out);
					displayLCDString(1, 0, str);
				}
				else {
					displayLCDCenteredString(0, "<Flywheel off>");
				}
			}

			pwrBtns = twoWay((nLCDButtons & kButtonLeft) || vexRT[Btn7D], -flyPwrIncrement, (nLCDButtons & kButtonRight) || vexRT[Btn7U], flyPwrIncrement);

			if (pwrBtns != 0) {
				if (doUseLRPwr) flyLRPwr = fmaxf(0, flyLRPwr + pwrBtns);
				else flySRPwr = fmaxf(0, flySRPwr + pwrBtns);

				clearLCD();
				lcdDispPwr(doUseLRPwr);

				clearTimer(lcdTimer);
				while(((nLCDButtons & (kButtonLeft | kButtonRight)) || vexRT[Btn7D] || vexRT[Btn7U]) && time1[lcdTimer] < 300) { wait1Msec(25); EndTimeSlice(); }

				do {
					pwrBtns = twoWay((nLCDButtons & kButtonLeft) || vexRT[Btn7D], -flyPwrIncrement, (nLCDButtons & kButtonRight) || vexRT[Btn7U], flyPwrIncrement);

					if (doUseLRPwr) flyLRPwr = fmaxf(0, flyLRPwr + pwrBtns);
					else flySRPwr = fmaxf(0, flySRPwr + pwrBtns);

					clearLCD();
					lcdDispPwr(doUseLRPwr);

					wait1Msec(100);
					EndTimeSlice();
				} while(pwrBtns != 0);

				clearTimer(lcdTimer);
			}
		}

		SensorValue[redLed] =
			SensorValue[yellowLed] =
			SensorValue[greenLed] = 0;

		if (!bIfiRobotDisabled) {
			if (flyTbh.doRun) {
				if (isTbhInThresh(&flyTbh, greenThresh)) SensorValue[greenLed] = 1;
				else if (isTbhInThresh(&flyTbh, yellowThresh)) SensorValue[yellowLed] = 1;
				else SensorValue[redLed] = 1;
			}

			SensorValue[testLed] = SensorValue[liftLim];
		}
		else SensorValue[testLed] = 0;

		wait1Msec(25);
	}
}

void startFlyTbh(bool useCtl) {
	resetTbh(&flyTbh, 0);

	resetADiff(&flyDiff, 0);

	if (useCtl) updateTbhController(&flyCtl, 0);
	else setTbhDoRun(&flyTbh, true);

	SensorValue[flyEnc] = 0;
}

void startLiftTbh(bool useCtl) {
  resetTbh(&liftTbh, 0);

  resetADiff(&liftDiff, 0);

  if (useCtl) updateTbhController(&liftCtl, 0);
  else setTbhDoRun(&liftTbh, true);

  SensorValue[liftEnc] = 0;
}

void stopCtls() {
	setTbhDoRun(&flyTbh, false);
	setTbhDoRun(&liftTbh, false);

	stopCtlLoop();
}

void init() {
	bLCDBacklight = true;
	clearLCD();

  ctlLoopInterval = 50;

  initTbh(&flyTbh, 5, .125, 127, true);
  initTbh(&liftTbh, 10, .5, 127, false);

  initTbhController(&flyCtl, &flyTbh, false);
  initTbhController(&liftCtl, &liftTbh, false);
}

void updateCtl(float dt) {
	updateADiff(&flyDiff, SensorValue[flyEnc], dt);
	updateADiff(&liftDiff, SensorValue[liftEnc], dt);

	if (flyTbh.doUpdate)
		motor[blFly] = motor[tlFly] =
			motor[brFly] = motor[trFly] = updateTbh(&flyTbh, flyDiff.out, dt);

	if (liftTbh.doUpdate)
		motor[lift] = updateTbh(&liftTbh, -liftDiff.out, dt);
}

task auton() {
	const float spinupThresh = 50,
		recoilThresh = 20;

	startFlyTbh(false);
	startCtlLoop();

	setTbh(&flyTbh, autonFlyPwr);

	blockLim(4000, !isTbhInThresh(&flyTbh, spinupThresh));

	while (true) {
		motor[lift] = 127;
		motor[intake] = 127;

		wait1Msec(600);

		motor[lift] = 0;
		motor[intake] = 80;

		wait1Msec(500);
		blockLim(1500, !isTbhInThresh(&flyTbh, recoilThresh));
	}
}

void endAuton() {
	stopCtls();
}

task userOp() {
	startFlyTbh(true);
	startCtlLoop();

	const float cutFac = 3;
	bool cutDrive = false;

	word driveX, driveY,
		cutBtn = 0, lastCutBtn = 0,
		flyFwdBtn = 0, lastFlyFwdBtn = 0,
		flyBackBtn = 0, lastFlyBackBtn = 0,
		flyDir = 0;

	while (true) {
		driveX = vexRT[ChRX];
		driveY = vexRT[ChLY];

		cutBtn = vexRT[Btn8D];

		if (cutBtn && !lastCutBtn)
			cutDrive = !cutDrive;

		lastCutBtn = cutBtn;

		if (cutDrive) {
			driveX = (word)(driveX / cutFac);
			driveY = (word)(driveY / cutFac);
		}

		arcade4(driveX, driveY, flWheel, blWheel, frWheel, brWheel);

		lastFlyFwdBtn = flyFwdBtn;
		lastFlyBackBtn = flyBackBtn;

		flyFwdBtn = vexRT[Btn5U];
		flyBackBtn = vexRT[Btn5D];

		if (flyFwdBtn && !lastFlyFwdBtn)
			flyDir = flyDir == 1 ? 0 : 1;

		if (flyBackBtn && !lastFlyBackBtn)
			flyDir = flyDir == 2 ? 0 : 2;

		switch (flyDir) {
			case 1: updateTbhController(&flyCtl, flyLRPwr); break;
			case 2: updateTbhController(&flyCtl, flySRPwr); break;
			default: updateTbhController(&flyCtl, 0); break;
		}

		motor[intake] = motor[lift] =
			joyDigi2(Btn6U, 127, Btn6D, -127);

		//updateTbhController(&liftCtl, joyDigi2(Btn6U, 600, Btn6D, -600));
	}
}

void endUserOp() {
	stopCtls();
}
