#pragma config(UART_Usage, UART2, uartNotUsed, baudRate4800, IOPins, None, None)
#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl1,  lDown,          sensorDigitalOut)
#pragma config(Sensor, dgtl2,  rDown,          sensorDigitalOut)
#pragma config(Sensor, dgtl3,  rRelease,       sensorDigitalOut)
#pragma config(Sensor, dgtl4,  lRelease,       sensorDigitalOut)
#pragma config(Sensor, I2C_1,  lDriveEnc,      sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  rDriveEnc,      sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_3,  flyEnc,         sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           lift,          tmotorVex393HighSpeed_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           brDrive,       tmotorVex393HighSpeed_MC29, openLoop, reversed, driveRight, encoderPort, I2C_2)
#pragma config(Motor,  port3,           mrDrive,       tmotorVex393HighSpeed_MC29, openLoop, reversed, driveRight)
#pragma config(Motor,  port4,           blDrive,       tmotorVex393HighSpeed_MC29, openLoop, driveLeft, encoderPort, I2C_1)
#pragma config(Motor,  port5,           mlDrive,       tmotorVex393HighSpeed_MC29, openLoop, driveLeft)
#pragma config(Motor,  port6,           frDrive,       tmotorVex393HighSpeed_MC29, openLoop, driveRight)
#pragma config(Motor,  port7,           flDrive,       tmotorVex393HighSpeed_MC29, openLoop, reversed, driveLeft)
#pragma config(Motor,  port8,           rFly,         tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_3)
#pragma config(Motor,  port9,           lFly,          tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          intake,        tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//NB: *Release pistons are inverted but *Down pistons ARE NOT INVERTED.

#pragma platform(VEX)

#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "rkCompetition002.h"
#include "rkUtil002.h"

#include "rkLogic/dlatch001.h"

#include "rkControl/base002.h"
//#include "rkControl/diff002.h" //Included by tbh
#include "rkControl/tbh005.h"
#include "rkControl/tbhController002.h"

float flyLRPwr = 1000, flySRPwr = 800;
ADiff flyDiff, fly2Diff;
Tbh flyTbh;
TbhController flyCtl;

const float autonFlyPwr = 800,
	velThresh = 50,
	accelThresh = 200;

void setFly(word value) {
	motor[lFly] = motor[rFly] = value;
}

void setDown(word value) {
	SensorValue[lDown] = SensorValue[rDown] = value;
}

void setRelease(word value) {
	SensorValue[lRelease] = SensorValue[rRelease] = value;
}

task retractPneumatics() {
	setDown(1);
	wait1Msec(200);
	setRelease(1);
	wait1Msec(1000);
	setDown(0);
}

void startFlyTbh(bool doUseController) {
	resetTbh(&flyTbh, 0);

	resetADiff(&flyDiff, 0);
	resetADiff(&fly2Diff, 0);

	if (doUseController) updateTbhController(&flyCtl, 0);
	else setTbhDoRun(&flyTbh, true);

	SensorValue[flyEnc] = 0;
}

void stopCtls() {
	setTbhDoRun(&flyTbh, false);

	stopCtlLoop();
}

void init() {
	ctlLoopInterval = 50;

	initTbh(&flyTbh, 5, .25, .15, 127, 52, true);

	initTbhController(&flyCtl, &flyTbh, false);

	setRelease(0);
	setDown(0);
}

void updateCtl(float dt) {
	updateADiff(&flyDiff, -SensorValue[flyEnc], dt);
	updateADiff(&fly2Diff, flyDiff.out, dt);

	if (flyTbh.doUpdate)
		motor[lFly] = motor[rFly] = updateTbh(&flyTbh, flyDiff.out, dt);
}

task auton() {
	const float recoilThresh = 75,
		recoil2Thresh = 300;

	startFlyTbh(false);
	startCtlLoop();

	setTbh(&flyTbh, autonFlyPwr);

	motor[blDrive] = motor[mlDrive] = motor[flDrive] =
		motor[brDrive] = motor[mrDrive] = motor[frDrive] = 127;

	wait1Msec(3000);

	motor[blDrive] = motor[mlDrive] = motor[flDrive] =
		motor[brDrive] = motor[mrDrive] = motor[frDrive] = 0;

	block(!(isTbhInThresh(&flyTbh, velThresh) &&
		isTbhAccelInThresh(&flyTbh, accelThresh) &&
		(abs(fly2Diff.out) <= accelThresh)));

	while (true) {
		motor[intake] = motor[lift] = 80;

		block(isTbhInThresh(&flyTbh, recoilThresh) &&
			(abs(fly2Diff.out) <= recoil2Thresh));

		motor[intake] = motor[lift] = 0;

		block(!(isTbhInThresh(&flyTbh, velThresh) &&
			(abs(fly2Diff.out) <= accelThresh)));

		wait1Msec(20);
	}
}

void endAuton() {
	stopCtls();
}

task userOp() {
	startFlyTbh(true);
	startCtlLoop();

	DLatch flyLRLatch, flySRLatch;

	word driveX, driveY,
		flyDir = 0;

	resetDLatch(&flyLRLatch, 0);
	resetDLatch(&flySRLatch, 0);

	while (true) {
		driveX = vexRT[ChLX];
		driveY = vexRT[ChLY];

		motor[blDrive] = motor[mlDrive] = motor[flDrive] =
			arcadeLeft(driveX, driveY);

		motor[brDrive] = motor[mrDrive] = motor[frDrive] =
			arcadeRight(driveX, driveY);

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

		if (vexRT[Btn7U] && vexRT[Btn8U])
			startTask(retractPneumatics);

		EndTimeSlice();
	}
}

void endUserOp() {
	stopCtls();
}
