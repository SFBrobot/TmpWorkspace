float fmaxf(float a, float b) { return a > b ? a : b; }

float fminf(float a, float b) { return a < b ? a : b; }

//Simple multi-task-friendly blocking loop
//	cond: blocks while this is true
#define block(cond) while(cond) EndTimeSlice()

//While loop with a time limit
//	lim: the time limit, in milliseconds
//	cond: loops while this is true
#define whileLim(lim, cond) clearTimer(T1); while((cond) && time1[T1] <= lim)

//Simple multi-task-friendly blocking loop with a time limit
//	lim: the time limit, in milliseconds
//	cond: blocks while this is true
#define blockLim(lim, cond) whileLim(lim, cond) EndTimeSlice()

//Clears both lines of the LCD
#define clearLCD() clearLCDLine(0); clearLCDLine(1)

//Joystick analog channels
#define ChLX Ch4
#define ChLY Ch3
#define ChRX Ch1
#define ChRY Ch2

//Partner joystick analog channels
#define ChLXXmtr2 Ch4Xmtr2
#define ChLYXmtr2 Ch3Xmtr2
#define ChRXXmtr2 Ch1Xmtr2
#define ChRYXmtr2 Ch2Xmtr2

//EasyC-style motor value assignment
//	mtr: the motor to set
//	val: the value to set it to
#define setMotor(mtr, val) motor[mtr] = val

//Arcade drive value for the left wheels
//	x: the rotation speed
//	y: the forward speed
#define arcadeLeft(x, y) (y + x)

//Arcade drive value for the right wheels
//	x: the rotation speed
//	y: the forward speed
#define arcadeRight(x, y) (y - x)

//Tank drive value for the left wheels
//	l: the left speed
//	r: the right speed
#define tankLeft(l, r) l

//Tank drive value for the right wheels
//	l: the left speed
//	r: the right speed
#define tankRight(l, r) r

//Holonomic drive value for the front left wheel
//	x: the strafe speed
//	y: the forward speed
//	r: the rotation speed
#define holoFL(x, y, r) (y + x + r)

//Holonomic drive value for the front right wheel
//	x: the strafe speed
//	y: the forward speed
//	r: the rotation speed
#define holoFR(x, y, r) (y - x - r)

//Holonomic drive value for the back left wheel
//	x: the strafe speed
//	y: the forward speed
//	r: the rotation speed
#define holoBL(x, y, r) (y - x + r)

//Holonomic drive value for the back right wheel
//	x: the strafe speed
//	y: the forward speed
//	r: the rotation speed
#define holoBR(x, y, r) (y + x - r)

//Two-motor arcade drive
//	x: the rotation speed
//	y: the forward speed
//	ml: the left motor
//	mr: the right motor
#define arcade2(x, y, ml, mr)\
	motor[ml] = arcadeLeft(x, y);\
	motor[mr] = arcadeRight(x, y)

//Four-motor arcade drive
//	x: the rotation speed
//	y: the forward speed
//	ml1: the first left motor
//	ml2: the second left motor
//	mr1: the first right motor
//	mr2: the second right motor
#define arcade4(x, y, ml1, ml2, mr1, mr2)\
	motor[ml1] = motor[ml2] = arcadeLeft(x, y);\
	motor[mr1] = motor[mr2] = arcadeRight(x, y)

//Two-motor tank drive
//	l: the left speed
//	r: the right speed
//	ml: the left motor
//	mr: the right motor
#define tank2(l, r, ml, mr)\
  motor[ml] = tankLeft(l, r);\
  motor[mr] = tankRight(l, r)

//Four-motor tank drive
//	l: the left speed
//	r: the right speed
//	ml1: the first left motor
//	ml2: the second left motor
//	mr1: the first right motor
//	mr2: the second right motor
#define tank4(l, r, ml1, ml2, mr1, mr2)\
  motor[ml1] = motor[ml2] = tankLeft(l, r);\
  motor[mr1] = motor[mr2] = tankRight(l, r)

//Four-motor holonomic drive
//	x: the strafe speed
//	y: the forward speed
//	r: the rotation speed
//	mfl: the front-left motor
//	mbl: the back-left motor
//	mfr: the front-right motor
//	mbr: the back-right motor
#define holonomic(x, y, r, mfl, mbl, mfr, mbr)\
	motor[mfl] = holoFL(x, y, r);\
	motor[mfr] = holoFR(x, y, r);\
	motor[mbl] = holoBL(x, y, r);\
	motor[mbr] = holoBR(x, y, r)

//If val is above thresh, returns val; otherwise zero.
//	val: the input value
//	thresh: the threshold to check val against
#define deadbandThresh(val, thresh) (abs(val) > thresh ? val : 0)

//Returns a value for analog joystick control
//	ch: the joystick channel
//	thresh: the deadband threshold
#define joyAnalog(ch, thresh) deadbandThresh(vexRT[ch], thresh)

//Analog joystick control of a motor
//	ch: the joystick channel
//	mtr: the motor
//	thresh: the deadband threshold
#define joyAnalogMtr(ch, mtr, thresh) motor[mtr] = joyAnalog(ch, thresh)

//Returns a value for two-way digital control
//	cond1: the conditon for the first direction
//	val1: the value for the first direction
//	cond2: the condition for the second direction
//	val2: the value for the second direction
#define twoWay(cond1, val1, cond2, val2) (cond1 ^ cond2 ? (cond1 ? val1 : val2) : 0)

//Returns a value for one-way digital joystick control
//	btn: the channel of the joystick button
//	val: the value for the joystick button
#define joyDigi1(btn, val) (vexRT[btn] ? val : 0)

//Returns a value for two-way digital joystick control
//	btn1: the channel of the joystick button for the first direction
//	val1: the value for the first direction
//	btn2: the channel of the joystick button for the second direction
//	val2: the value for the second direction
#define joyDigi2(btn1, val1, btn2, val2) twoWay(vexRT[btn1], val1, vexRT[btn2], val2)

//See joyDigi1
#define joystickDigital(btn, val) joyDigi1(btn, val)

//See joyDigi2
#define joystickDigitalTwoWay(btn, val) joyDigi2(btn, val)

//One-way digital joystick control of a motor
//	btn: the channel of the joystick button
//	val: the motor value for the joystick button
#define joyDigiMtr1(mtr, btn, val) motor[mtr] = joyDigi1(btn, val)

//Two-way digital joystick control of a motor
//	btn1: the channel of the joystick button for the first direction
//	val1: the motor value for the first direction
//	btn2: the channel of the joystick button for the second direction
//	val2: the motor value for the second direction
#define joyDigiMtr2(mtr, btn1, val1, btn2, val2) motor[mtr] = joyDigi2(btn1, val1, btn2, val2)

//See joyDigiMtr1
#define joystickDigitalToMotor(mtr, btn, val) joyDigiMtr1(mtr, btn, val)

//See joyDigiMtr2
#define joystickDigitalToMotorTwoWay(mtr, btn1, val2, btn2, val2) joyDigiMtr2(mtr, btn1, val2, btn2, val2)
