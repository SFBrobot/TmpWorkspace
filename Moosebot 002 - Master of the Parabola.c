#pragma config(UART_Usage, UART1, uartVEXLCD, baudRate19200, IOPins, None, None)
#pragma config(UART_Usage, UART2, uartNotUsed, baudRate4800, IOPins, None, None)
#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl1,  redLed,         sensorLEDtoVCC)
#pragma config(Sensor, dgtl2,  yellowLed,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl3,  greenLed,       sensorLEDtoVCC)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           lift,          tmotorVex393HighSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           blWheel,       tmotorVex393HighSpeed_MC29, openLoop, driveLeft)
#pragma config(Motor,  port3,           flWheel,       tmotorVex393HighSpeed_MC29, openLoop, driveRight)
#pragma config(Motor,  port4,           brWheel,       tmotorVex393HighSpeed_MC29, openLoop, reversed, driveRight)
#pragma config(Motor,  port5,           frWheel,       tmotorVex393HighSpeed_MC29, openLoop, reversed, driveLeft)
#pragma config(Motor,  port6,           trFly,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           tlFly,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           brFly,         tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port9,           blFly,         tmotorVex393_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port10,          intake,        tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#define RKCOMP_LCD //Enable LCD stuff in rkCompetition

#include "rkCompetition002.h"
#include "rkUtil002.h"
#include "rkControl/base001.h"
//#include "rkControl/diff001.h"
#include "rkControl/tbh002.h"

float flyFwdPwr = 850;
const float autonFlyPwr = 835;
Tbh lFlyTbh, rFlyTbh;

const float greenThresh = 25;
const float yellowThresh = 75;

task lcd() {
	string str;
	float pwrBtns;
	const TTimers lcdTimer = T3;
	const float flyPwrIncrement = 10;

	while (true) {
		if (bIfiRobotDisabled || bIfiAutonomousMode || (nLCDButtons & kButtonCenter)) {
			sprintf(str, "%6.2f // %-6.2f", nImmediateBatteryLevel / 1000., BackupBatteryLevel / 1000.);

			clearLCD();
			displayLCDCenteredString(0, "Battery:");
			displayLCDString(1, 0, str);
		}
		else {
			clearLCD();

			if (time1[lcdTimer] < 2000) {
				displayLCDCenteredString(0, "Value:");
				displayLCDNumber(1, 0, flyFwdPwr);
			}
			else {
				sprintf(str, "%7.2f  %-7.2f", lFlyTbh.err, rFlyTbh.err);
				displayLCDString(0, 0, str);

				sprintf(str, "%7.2f  %-7.2f", lFlyTbh.out, rFlyTbh.out);
				displayLCDString(1, 0, str);
			}

			pwrBtns = twoWay((nLCDButtons & kButtonLeft) || vexRT[Btn7D], -flyPwrIncrement, (nLCDButtons & kButtonRight) || vexRT[Btn7U], flyPwrIncrement);

			if (pwrBtns != 0) {
				flyFwdPwr = fmaxf(0, flyFwdPwr + pwrBtns);

				clearLCDLine(1);
				displayLCDNumber(1, 0, flyFwdPwr);

				clearTimer(lcdTimer);
				while(((nLCDButtons & (kButtonLeft | kButtonRight)) || vexRT[Btn7D] || vexRT[Btn7U]) && time1[lcdTimer] < 300) { wait1Msec(25); EndTimeSlice(); }

				do {
					pwrBtns = twoWay((nLCDButtons & kButtonLeft) || vexRT[Btn7D], -flyPwrIncrement, (nLCDButtons & kButtonRight) || vexRT[Btn7U], flyPwrIncrement);

					flyFwdPwr = fmaxf(0, flyFwdPwr + pwrBtns);

					clearLCDLine(1);
					displayLCDNumber(1, 0, flyFwdPwr);

					wait1Msec(100);
					EndTimeSlice();
				} while(pwrBtns != 0);

				clearTimer(lcdTimer);
			}
		}

		if (!bIfiRobotDisabled) {
			SensorValue[redLed] =
				SensorValue[yellowLed] =
				SensorValue[greenLed] = 0;

			if (isTbhInThresh(&lFlyTbh, greenThresh) && isTbhInThresh(&rFlyTbh, greenThresh)) SensorValue[greenLed] = 1;
			else if (isTbhInThresh(&lFlyTbh, yellowThresh) && isTbhInThresh(&rFlyTbh, yellowThresh)) SensorValue[yellowLed] = 1;
			else SensorValue[redLed] = 1;
		}

		wait1Msec(25);
	}
}

void init() {
	bLCDBacklight = true;
	clearLCD();

  ctlLoopInterval = 50;

  initTbh(&lFlyTbh, 0, .45, 1 / 500., 1000, 127, true);
  initTbh(&rFlyTbh, 0, .45, 1 / 500., 1000, 127, true);
}

long lFlyEnc = 0,
	rFlyEnc = 0,
	lastLFlyEnc = 0,
	lastRFlyEnc = 0;

float dLFlyEnc = 0,
	dRFlyEnc = 0,
	lastDLFlyEnc = 0,
	lastDRFlyEnc = 0,
	avgDLFlyEnc = 0,
	avgDRFlyEnc = 0;

void updateCtl(float dt) {
	lastLFlyEnc = lFlyEnc;
	lastRFlyEnc = rFlyEnc;

	lFlyEnc = nMotorEncoder[blFly];
	rFlyEnc = nMotorEncoder[brFly];

	lastDLFlyEnc = dLFlyEnc;
	lastDRFlyEnc = dRFlyEnc;

	dLFlyEnc = (lFlyEnc - lastLFlyEnc) / dt;
	dRFlyEnc = (rFlyEnc - lastRFlyEnc) / dt;

	avgDLFlyEnc = (dLFlyEnc + lastDLFlyEnc) / 2.;
	avgDRFlyEnc = (dRFlyEnc + lastDRFlyEnc) / 2.;

	motor[blFly] = motor[tlFly] = updateTbh(&lFlyTbh, avgDLFlyEnc, dt);
	motor[brFly] = motor[trFly] = updateTbh(&rFlyTbh, avgDRFlyEnc, dt);
}

void startFlyTbh() {
	resetTbh(&lFlyTbh, 0);
	resetTbh(&rFlyTbh, 0);

	lFlyTbh.doRun =
		rFlyTbh.doRun = true;

	nMotorEncoder[blFly] = 0;
	nMotorEncoder[brFly] = 0;

	startCtlLoop();
}

task auton() {
	startFlyTbh();

	setTbh(&lFlyTbh, autonFlyPwr);
	setTbh(&rFlyTbh, autonFlyPwr);

	//motor[flWheel] = motor[frWheel] =
	//	motor[blWheel] = motor[brWheel] = 63;

	//wait1Msec(900);

	//motor[flWheel] = motor[frWheel] =
	//	motor[blWheel] = motor[brWheel] = 0;

	blockLim(4000, !(isTbhInThresh(&lFlyTbh, 75) && isTbhInThresh(&rFlyTbh, 75)));

	motor[intake] = 80;

	while (true) {
		motor[lift] = 127;

		wait1Msec(600);

		motor[lift] = 0;

		wait1Msec(1500);
	}
}

void endAuton() {
	stopCtlLoop();
}

task userOp() {
	startFlyTbh();

	float flyPwr = 0, lastFlyPwr = 0;
	const float cutFac = 3;
	bool isFlyOn, cutDrive = false;
	word driveX, driveY, cutBtn = 0, lastCutBtn = 0;

	while (true) {
		driveX = vexRT[ChRX];
		driveY = vexRT[ChLY];

		cutBtn = vexRT[Btn8D];

		if (cutBtn && !lastCutBtn) {
			cutDrive = !cutDrive;
		}

		lastCutBtn = cutBtn;

		if (cutDrive) {
			driveX = (word)(driveX / cutFac);
			driveY = (word)(driveY / cutFac);
		}

		arcade4(driveX, driveY, flWheel, blWheel, frWheel, brWheel);

		flyPwr = joyDigi2(Btn5U, flyFwdPwr, Btn5D, -200);

		if (flyPwr != lastFlyPwr) {
			isFlyOn = flyPwr != 0;

			setTbh(&lFlyTbh, flyPwr);
			setTbh(&rFlyTbh, flyPwr);

			lFlyTbh.doRun =
				rFlyTbh.doRun =
				isFlyOn;

			lastFlyPwr = flyPwr;
		}

		joyDigiMtr2(intake, Btn6U, 127, Btn6D, -127);

		joyAnalogMtr(ChRYXmtr2, lift, 3);
	}
}

void endUserOp() {
	stopCtlLoop();
}
