#include "rkUtil002.h";

TTimers pidTimer = T4;

bool doPidLoop = false;

void updatePids(float dt);

task pidLoop() {
	clearTimer(pidTimer);

	long time, lastTime = time1[pidTimer] - 1;
	float dt = 0;

	while (doPidLoop) {
		time = time1[pidTimer];
		dt = (float)(time - lastTime) / 1000.;

		updatePids(dt);

		lastTime = time;

		wait1Msec(1);

		EndTimeSlice();
	}
}

void startPidLoop() {
	doPidLoop = true;

	startTask(pidLoop);
}

//Note the intentional omission of stopTask
void stopPidLoop() {
	doPidLoop = false;
}

struct Pid {
	float deriv,
		derivFlt,
		err,
		errThresh,
		errXing,
		input,
		inLast,
		integ,
		integLim,
		integXing,
		kP, kI, kD,
		setpoint,
		tbhFactor,
		thresh,
		out;

  bool isRunning, isOnTgt;
}

void setPidDFlt(Pid *pid, float flt) { pid->derivFlt = pow(.1, flt / 10 - 5); }

void setPidTbhFactor(Pid *pid, float factor) { pid->tbhFactor = .5 * factor; }

void updatePidErr(Pid *pid) {
	pid->err = pid->setpoint - pid->input;

	pid->isOnTgt = abs(pid->err) < pid->thresh;

	if (pid->isOnTgt) pid->errThresh = 0;
  else pid->errThresh = pid->err - pid->thresh * sgn(pid->err); //Not sure if I'll keep this, but it might reduce jitter
}

void setPid(Pid *pid, float setpoint) {
	pid->setpoint = setpoint;
	pid->integXing = pid->errXing = 0;

	updatePidErr(pid);
}

void resetPid(Pid *pid, float input) {
	pid->input = pid->inLast = input;
	pid->integ = 0;

	setPid(pid, input);
}

void initPid(Pid *pid, float thresh, float kP, float kI, float kD, float integLim, float tbhFactor, float derivFlt) {
	pid->deriv = 0;
	pid->integLim = integLim;
	pid->kP = kP;
	pid->kI = kI;
	pid->kD = kD;
	pid->thresh = thresh;
	pid->out = 0;

	pid->isRunning = true;

	setPidDFlt(pid, derivFlt);
	setPidTbhFactor(pid, tbhFactor);
	resetPid(pid, 0);
}

float updatePid(Pid *pid, float input, float dt) {
	pid->input = input;

	updatePidErr(pid);

	if (!pid->isRunning) return pid->out = 0;

	pid->integ = fmaxf(-pid->integLim, fminf(pid->integLim, pid->integ + pid->kI * pid->err * dt));

	if (pid->err != 0 && sgn(pid->errXing) != sgn(pid->err)) {
		pid->integ = pid->integXing = (pid->integ + pid->integXing) * pid->tbhFactor;

		pid->errXing = pid->err;
	}

	pid->deriv += (1 - pow(.5, dt * pid->derivFlt)) * (pid->kD * (pid->inLast - pid->input) / dt - pid->deriv);

	pid->inLast = pid->input;

	return pid->out = pid->kP * pid->errThresh + pid->integ + pid->deriv;
}

struct Tbh {
	float err,
		errThresh,
		errXing,
		input,
		integ,
		integLim,
		integXing,
		kI,
		setpoint,
		tbhFactor,
		thresh,
		out;

	bool doSgnLock,
		hasXed,
		isRunning,
		isOnTgt;
}

void setTbhTbhFactor(Tbh *tbh, float factor) { tbh->tbhFactor = .5 * factor; }

void updateTbhErr(Tbh *tbh) {
	tbh->err = tbh->setpoint - tbh->input;

	tbh->isOnTgt = abs(tbh->err) < tbh->thresh;

	if (tbh->isOnTgt) tbh->errThresh = 0;
	else tbh->errThresh = tbh->err - tbh->thresh * sgn(tbh->err);
}

void setTbh(Tbh *tbh, float setpoint) {
	tbh->setpoint = setpoint;
	tbh->integXing = tbh->errXing = 0;
	tbh->hasXed = false;

	updateTbhErr(tbh);
}

void resetTbh(Tbh *tbh, float input) {
	tbh->input = input;
	tbh->integ = 0;

	setTbh(tbh, input);
}

void initTbh(Tbh *tbh, float thresh, float kI, float integLim, float tbhFactor, bool doSgnLock) {
	tbh->integLim = integLim;
	tbh->kI = kI;
	tbh->thresh = thresh;
	tbh->out = 0;

	tbh->doSgnLock = doSgnLock;
	tbh->isRunning = true;

	setTbhTbhFactor(tbh, tbhFactor);
	resetTbh(tbh, 0);
}

float updateTbh(Tbh *tbh, float input, float dt) {
	tbh->input = input;

	updateTbhErr(tbh);

	if (!tbh->isRunning) return tbh->out = 0;

	tbh->integ = fmaxf(-tbh->integLim, fminf(tbh->integLim, tbh->integ + tbh->kI * tbh->err * dt));

	if (tbh->err != 0 && sgn(tbh->errXing) != sgn(tbh->err)) {
		if (tbh->hasXed)
			tbh->integ = (tbh->integ + tbh->integXing) * tbh->tbhFactor;
		else
			tbh->hasXed = true;

		tbh->integXing = tbh->integ;

		tbh->errXing = tbh->err;
	}

	tbh->out = tbh->integ;

	if (tbh->doSgnLock) {
		if (tbh->input > 0 && tbh->out < 0) tbh->out = 0;
		else if (tbh->input < 0 && tbh->out > 0) tbh->out = 0;
	}

	return tbh->out;
}
