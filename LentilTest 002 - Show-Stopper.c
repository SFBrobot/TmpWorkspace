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

#define RKCOMP_DEBUG //Comment to disable debug mode

#define RKCOMP_DEBUG_MENU_COND vexRT[Btn8R]
#define RKCOMP_DEBUG_DISABLE_COND vexRT[Btn8U]
#define RKCOMP_DEBUG_AUTON_COND vexRT[Btn8L]
#define RKCOMP_DEBUG_DRIVER_COND vexRT[Btn8D]
#define RKCOMP_DEBUG_RESTART_COND vexRT[Btn6U]

#include "rkUtil003.h"

#include "rkCompetition003a.h"

unsigned long x = 0;

void init() { }

task auton() {
	while (true) {
		x++;
		EndTimeSlice();
	}
}

void endAuton() { }

void endAutonDebug() { }

task userOp() {
	while (true) {
		x++;
		EndTimeSlice();
	}
}

void endUserOp() { }
