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

#include "rkLogic/dlatch001.h"

#include "rkControl/base002.h"
#include "rkControl/diff003.h"
#include "rkControl/tbh006.h"
#include "rkControl/tbhController002.h"

float flyLRPwr = 800, flySRPwr = 550;
const float autonFlyPwr = 800,
	velThresh = 50,
	accelThresh = 200;

ADiff flyDiff, fly2Diff, liftDiff, lift2Diff;
Tbh flyTbh, liftTbh;
TbhController flyCtl, liftCtl;

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
	bool dispPwr = false, doUseLRPwr = true, flash = false;

	const word battThresh = 7800;

	while (true) {
		if ((bIfiRobotDisabled || bIfiAutonomousMode) && nImmediateBatteryLevel < battThresh) {
			if (time1[lcdTimer] > 500) {
				bLCDBacklight = flash = !flash;
				clearTimer(lcdTimer);
			}
		}

		if (nLCDButtons & kButtonCenter) {
			bLCDBacklight = true;

			sprintf(str, "%6.2f // %-6.2f", nImmediateBatteryLevel / 1000., BackupBatteryLevel / 1000.);

			clearLCD();
			displayLCDCenteredString(0, "Battery:");
			displayLCDString(1, 0, str);
		}
		else if (bIfiRobotDisabled || bIfiAutonomousMode) {
			clearLCD();

			dispPwr = false;

			if (nImmediateBatteryLevel < battThresh) {
				sprintf(str, "%.2f Volts", nImmediateBatteryLevel / 1000.);

				if (flash) displayLCDCenteredString(0, "BATTERY WARNING:");
				displayLCDCenteredString(1, str);
			}
			else {
				bLCDBacklight = flash = false;

				displayLCDCenteredString(0, "Moosebot Mk. II");
				displayLCDCenteredString(1, "4800Buckets");
			}
		}
		else {
			bLCDBacklight = true;

			if (vexRT[Btn7L] ^ vexRT[Btn7R])
				doUseLRPwr = vexRT[Btn7L];

			clearLCD();

			if (dispPwr && (time1[lcdTimer] < 2000 || vexRT[Btn7L] || vexRT[Btn7R])) lcdDispPwr(doUseLRPwr);
			else {
				if (flyTbh.doRun) {
					sprintf(str, "% 07.2f  % 07.2f",
						fmaxf(-999.99, fminf(999.99, flyDiff.out)),
						fmaxf(-999.99, fminf(999.99, flyTbh.err)));
					displayLCDString(0, 0, str);

					sprintf(str, "% 07.2f  % 07.2f",
						fmaxf(-999.99, fminf(999.99, fly2Diff.out)),
						fmaxf(-999.99, fminf(999.99, flyTbh.out)));
					displayLCDString(1, 0, str);
				}
				else {
					sprintf(str, "%-8s%8s", "Speed", "Error");
					displayLCDString(0, 0, str);

					sprintf(str, "%-8s%8s", "Accel", "Out");
					displayLCDString(1, 0, str);
				}
			}

			pwrBtns = twoWay((nLCDButtons & kButtonLeft) || vexRT[Btn7D], -flyPwrIncrement, (nLCDButtons & kButtonRight) || vexRT[Btn7U], flyPwrIncrement);

			if (pwrBtns != 0) {
				dispPwr = true;
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

		if (bIfiRobotDisabled) {
			if (nImmediateBatteryLevel < battThresh) {
				SensorValue[redLed] =
					SensorValue[yellowLed] =
					SensorValue[greenLed] = flash;
			}

			SensorValue[testLed] = 0;
		}
		else {
			if (flyTbh.doRun) {
				if (isTbhInThresh(&flyTbh, velThresh)) {
					if (abs(fly2Diff.out) <= accelThresh) SensorValue[greenLed] = 1;
					else  SensorValue[yellowLed] = 1;
				}
				else SensorValue[redLed] = 1;
			}

			SensorValue[testLed] = SensorValue[liftLim];
		}

		wait1Msec(50);
	}
}

void startFlyTbh(bool useCtl) {
	resetTbh(&flyTbh, 0);

	resetDiff(&flyDiff, 0);
	resetDiff(&fly2Diff, 0);

	if (useCtl) updateTbhController(&flyCtl, 0);
	else setTbhDoRun(&flyTbh, true);

	SensorValue[flyEnc] = 0;
}

void startLiftTbh(bool useCtl) {
  resetTbh(&liftTbh, 0);

  resetDiff(&liftDiff, 0);
  resetDiff(&lift2Diff, 0);

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
  ctlLoopInterval = 50;

  initTbh(&flyTbh, 5, .25, .15, 127, 52, true);
  initTbh(&liftTbh, 10, .5, 0, 127, 40, false);

  initTbhController(&flyCtl, &flyTbh, false);
  initTbhController(&liftCtl, &liftTbh, false);
}

void updateCtl(float dt) {
	updateDiff(&flyDiff, SensorValue[flyEnc], dt);
	updateDiff(&fly2Diff, flyDiff.out, dt);
	updateDiff(&liftDiff, SensorValue[liftEnc], dt);
	updateDiff(&lift2Diff, liftDiff.out, dt);

	if (flyTbh.doUpdate)
		motor[blFly] = motor[tlFly] =
			motor[brFly] = motor[trFly] = updateTbh(&flyTbh, flyDiff.out, fly2Diff.out, dt);

	if (liftTbh.doUpdate)
		motor[lift] = updateTbh(&liftTbh, -liftDiff.out, -lift2Diff.out, dt);
}

task auton() {
	const float recoilThresh = 75,
		recoil2Thresh = 300;

	startFlyTbh(false);
	startCtlLoop();

	setTbh(&flyTbh, autonFlyPwr);

	block(!(isTbhInThresh(&flyTbh, velThresh) &&
		isTbhDerivInThresh(&flyTbh, accelThresh)));

	while (true) {
		motor[intake] = motor[lift] = 80;

		block(isTbhInThresh(&flyTbh, recoilThresh) &&
			isTbhDerivInThresh(&flyTbh, accelThresh));

		motor[intake] = motor[lift] = 0;

		block(!(isTbhInThresh(&flyTbh, velThresh) &&
			isTbhDerivInThresh(&flyTbh, accelThresh)));

		wait1Msec(20);
	}
}

void endAuton() {
	stopCtls();
}

task userOp() {
	startFlyTbh(true);
	startCtlLoop();

	const float cutFac = 3;
	bool cutDrive = false,
		flipDrive = false;

	DLatch cutLatch, flipLatch, flyLRLatch, flySRLatch;

	word driveX, driveY, flyDir = 0;

	resetDLatch(&cutLatch, 0);
	resetDLatch(&flipLatch, 0);
	resetDLatch(&flyLRLatch, 0);
	resetDLatch(&flySRLatch, 0);

	while (true) {
		driveX = vexRT[ChRX];
		driveY = vexRT[ChLY];

		if (risingBistable(&cutLatch, vexRT[Btn8U])) {
			driveX = (word)(driveX / cutFac);
			driveY = (word)(driveY / cutFac);
		}

		if (risingBistable(&flipLatch, vexRT[Btn8D])) driveY *= -1;

		arcade4(driveX, driveY, flWheel, blWheel, frWheel, brWheel);

		if (risingEdge(&flyLRLatch, vexRT[Btn5U]))
			flyDir = flyDir == 1 ? 0 : 1;

		if (risingEdge(&flySRLatch, vexRT[Btn5D]))
			flyDir = flyDir == 2 ? 0 : 2;

		switch (flyDir) {
			case 1: updateTbhController(&flyCtl, flyLRPwr); break;
			case 2: updateTbhController(&flyCtl, flySRPwr); break;
			default: updateTbhController(&flyCtl, 0); break;
		}

		motor[intake] = motor[lift] =
			joyDigi2(Btn6U, 127, Btn6D, -127);

		EndTimeSlice();
	}
}

void endUserOp() {
	stopCtls();
}
