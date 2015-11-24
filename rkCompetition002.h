#pragma platform(VEX)
#pragma competitionControl(Competition);

//Forward definitions of the necessary functions
void init(); //Runs at the program's start
void endAuton(); //Runs when the autonomous period ends
void endUserOp(); //Runs when the user-op period ends
task auton(); //Runs during the autonomous period
task userOp(); //Runs during the user-op period

#ifdef RKCOMP_LCD
task lcd();
#endif

task main() {
#ifdef RKCOMP_LCD
	startTask(lcd);
#endif

	init();

	while(true) {
		while(bIfiRobotDisabled) { wait1Msec(25); EndTimeSlice(); } //Wait until the robot isn't disabled

		//Are we in autonomous now?
		if(bIfiAutonomousMode) {
			startTask(auton); //Begin the autonomous routine

			while(bIfiAutonomousMode && !bIfiRobotDisabled) { wait1Msec(25); EndTimeSlice(); } //Wait until autonomous ends or the robot's disabled

			stopTask(auton); //End the autonomous routine
			endAuton(); //Clean up afterwards
		}
		else { //We must be in user-op
		  startTask(userOp); //Begin the user-op routine

		  while(!bIfiAutonomousMode && !bIfiRobotDisabled) { wait1Msec(25); EndTimeSlice(); } //Wait until user-op ends or the robot's disabled

		  stopTask(userOp); //End the user-op routine
		  endUserOp(); //Clean up afterwards
		}
	}
}
