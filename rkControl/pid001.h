typedef struct {
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

  bool doSgnLock,
  	doRun,
  	isOnTgt;
} Pid;

void setPidDFlt(Pid *pid, float flt) { pid->derivFlt = pow(.1, flt / 10 - 5); }

bool isPidInThresh(Pid *pid, float thresh) { return abs(pid->err) <= thresh; }

void updatePidErr(Pid *pid) {
	pid->err = pid->setpoint - pid->input;

	pid->isOnTgt = isPidInThresh(pid, pid->thresh);

	if (pid->isOnTgt) pid->errThresh = 0;
  else pid->errThresh = pid->err - pid->thresh * sgn(pid->err);
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

void initPid(Pid *pid, float thresh, float kP, float kI, float kD, float integLim, float derivFlt, bool doSgnLock) {
	pid->deriv = 0;
	pid->integLim = integLim;
	pid->kP = kP;
	pid->kI = kI;
	pid->kD = kD;
	pid->thresh = thresh;
	pid->out = 0;

	pid->doSgnLock = doSgnLock;
	pid->doRun = true;

	setPidDFlt(pid, derivFlt);
	resetPid(pid, 0);
}

float updatePid(Pid *pid, float input, float dt) {
	pid->input = input;

	updatePidErr(pid);

	if (!pid->doRun) return pid->out = 0;

	if ((pid->errXing <= 0 && pid->err > 0) || (pid->errXing >= 0 && pid->err < 0)) {
		pid->integ = 0;

		pid->errXing = pid->err;
	}
	else 	pid->integ = fmaxf(-pid->integLim, fminf(pid->integLim, pid->integ + pid->kI * pid->err * dt));

	pid->deriv += (1 - pow(.5, dt * pid->derivFlt)) * (pid->kD * (pid->inLast - pid->input) / dt - pid->deriv);

	pid->inLast = pid->input;

	pid->out = pid->kP * pid->errThresh + pid->integ + pid->deriv;

	if (pid->doSgnLock) {
		if (pid->input > 0 && pid->out < 0) pid->out = 0;
		else if (pid->input < 0 && pid->out > 0) pid->out = 0;
	}

	return pid->out;
}
