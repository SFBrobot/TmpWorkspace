#pragma platform(VEX)

#ifdef RKCOMP_DEBUG
#include "rkUtil003.h"

#include "rkLogic/dlatch001.h"

bool _rkBotDisabled = bIfiRobotDisabled,
	_rkAutonMode = bIfiAutonomousMode,
	rkModeRestart = false;

#define rkBotDisabled _rkBotDisabled
#define rkAutonMode _rkAutonMode

#define __rkCompDisabled__ rkBotDisabled && !rkModeRestart
#define __rkCompAutonMode__ rkAutonMode && !rkBotDisabled && !rkModeRestart
#define __rkCompUserOpMode__ !rkAutonMode && !rkBotDisabled && !rkModeRestart

#define __rkCompDebuggerListen__ if (risingEdge(&dbgMenuLatch, RKCOMP_DEBUG_MENU_COND))\
	rkCompDebugMenu(&dbgMenuLatch, &dbgDisableLatch, &dbgAutonLatch, &dbgDriverLatch, &dbgRestartLatch);

void rkCompDebugMenu(
	DLatch *menuLatch,
	DLatch *disableLatch,
	DLatch *autonLatch,
	DLatch *driverLatch,
	DLatch *restartLatch) {

	hogCPU();

	const int nMotors = 10;
	word motors[nMotors];

	for (int i = 0; i < nMotors; i++) {
		motors[i] = motor[i];
		motor[i] = 0;
	}

	while (!fallingEdge(menuLatch, RKCOMP_DEBUG_MENU_COND));

	while (true) {
		if (fallingEdge(menuLatch, RKCOMP_DEBUG_MENU_COND)) break;

		if (fallingEdge(disableLatch, RKCOMP_DEBUG_DISABLE_COND)) {
			_rkBotDisabled = true;
			break;
		}

		if (fallingEdge(autonLatch, RKCOMP_DEBUG_AUTON_COND)) {
			_rkBotDisabled = false;
			_rkAutonMode = true;
			break;
		}

		if (fallingEdge(driverLatch, RKCOMP_DEBUG_DRIVER_COND)) {
			_rkBotDisabled =
				_rkAutonMode = false;
			break;
		}

		if (fallingEdge(restartLatch, RKCOMP_DEBUG_RESTART_COND)) {
			rkModeRestart = true;
			break;
		}

		hog1Msec(50);
	}

	for (int i = 0; i < nMotors; i++) motor[i] = motors[i];

	releaseCPU();
}

#else
#pragma competitionControl(Competition);

#define rkBotDisabled bIfiRobotDisabled
#define rkAutonMode bIfiAutonomousMode

#define __rkCompDisabled__ rkBotDisabled
#define __rkCompAutonMode__ rkAutonMode && !rkBotDisabled
#define __rkCompUserOpMode__ !rkAutonMode && !rkBotDisabled

#define __rkCompDebuggerListen__

#endif

//Forward definitions of the necessary functions
void init(); //Runs at the program's start
void endAuton(); //Runs when the autonomous period ends
void endUserOp(); //Runs when the user-op period ends
task auton(); //Runs during the autonomous period
task userOp(); //Runs during the user-op period

#ifdef RKCOMP_LCD
task lcd();

#ifdef RKCOMP_DEBUG
#define __rkCompLcdTask__ _rkCompLcd

task _rkCompLcd() {
	long flashTs = nSysTime;

	byte nFlips = 0;

	bLCDBacklight = true;

	do {
		clearLCD();

		if (nSysTime - flashTs > 500) {
			bLCDBacklight = !bLCDBacklight;
			flashTs = nSysTime;
			nFlips++;
		}

		displayLCDCenteredString(0, "rkCompetition");

		displayLCDCenteredString(1, "DEBUG MODE");

		wait1Msec(50);
	} while (nFlips < 6);

	startTask(lcd);
}
#else
#define __rkCompLcdTask__ lcd
#endif

#endif

task main() {

#ifdef RKCOMP_LCD
	startTask(__rkCompLcdTask__);
#endif

#ifdef RKCOMP_DEBUG
	DLatch dbgMenuLatch,
		dbgDisableLatch,
		dbgAutonLatch,
		dbgDriverLatch,
		dbgRestartLatch;

	resetDLatch(&dbgMenuLatch, 0);
	resetDLatch(&dbgDisableLatch, 0);
	resetDLatch(&dbgAutonLatch, 0);
	resetDLatch(&dbgDriverLatch, 0);
	resetDLatch(&dbgRestartLatch, 0);
#endif

	init();

	while(true) {
#ifdef RKCOMP_DEBUG
		rkModeRestart = false;
#endif

		if (rkBotDisabled) {
			for (int i = 0; i < 10; i++) motor[i] = 0;

			while(__rkCompDisabled__) {
				__rkCompDebuggerListen__
				wait1Msec(25);
			} //Wait until the robot isn't disabled
		}
		else if(rkAutonMode) { //Are we in autonomous now?
			startTask(auton); //Begin the autonomous routine

			while(__rkCompAutonMode__) {
				__rkCompDebuggerListen__
				wait1Msec(25);
			} //Wait until autonomous ends or the robot's disabled

			stopTask(auton); //End the autonomous routine
			endAuton(); //Clean up afterwards
		}
		else { //We must be in user-op
		  startTask(userOp); //Begin the user-op routine

		  while(__rkCompUserOpMode__) {
		  	__rkCompDebuggerListen__
		  	wait1Msec(25);
		 	} //Wait until user-op ends or the robot's disabled

		  stopTask(userOp); //End the user-op routine
		  endUserOp(); //Clean up afterwards
		}
	}
}

#undef __rkCompAutonMode__
#undef __rkCompUserOpMode__
#undef __rkCompDebuggerListen__
#undef __rkCompLcdTask__
