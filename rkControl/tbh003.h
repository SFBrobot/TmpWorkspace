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
		thresh,
		out;

	bool doSgnLock,
		hasXed,
		doRun,
		isOnTgt;
}

bool isTbhInThresh(Tbh *tbh, float thresh) { return abs(tbh->err) <= thresh; }

void updateTbhErr(Tbh *tbh) {
	tbh->err = tbh->setpoint - tbh->input;

	tbh->isOnTgt = isTbhInThresh(tbh, tbh->thresh);

	if (tbh->isOnTgt) tbh->errThresh = 0;
	else tbh->errThresh = tbh->err - tbh->thresh * sgn(tbh->err);
}

void setTbh(Tbh *tbh, float setpoint) {
	tbh->setpoint = setpoint;
	tbh->integXing = 0;
	tbh->hasXed = false;

	updateTbhErr(tbh);

	tbh->errXing = tbh->err; //Don't immediately trigger a crossing
}

void resetTbh(Tbh *tbh, float input) {
	tbh->input = input;
	tbh->integ = 0;

	setTbh(tbh, input);
}

void initTbh(
	Tbh *tbh,
	float thresh,
	float kI,
	float integLim,
	bool doSgnLock) {
	tbh->integLim = integLim;
	tbh->kI = kI;
	tbh->thresh = thresh;
	tbh->out = 0;

	tbh->doSgnLock = doSgnLock;
	tbh->doRun = true;

	resetTbh(tbh, 0);
}

float updateTbh(Tbh *tbh, float input, float dt) {
	tbh->input = input;

	updateTbhErr(tbh);

	if (!tbh->doRun) return tbh->out = 0;

	tbh->integ = fmaxf(-tbh->integLim, fminf(tbh->integLim, tbh->integ + tbh->errThresh * tbh->kI * dt));

	if ((tbh->errXing <= 0 && tbh->err > 0) || (tbh->errXing >= 0 && tbh->err < 0)) {
		if (tbh->hasXed)
			tbh->integ = (tbh->integ + tbh->integXing) / 2.;
		else
			tbh->hasXed = true;

		tbh->integXing = tbh->integ;

		tbh->errXing = tbh->err;
	}

	if (tbh->doSgnLock) {
		if (tbh->input > 0 && tbh->integ < 0) tbh->integ = 0;
		else if (tbh->input < 0 && tbh->integ > 0) tbh->integ = 0;
	}

	return tbh->out = tbh->integ;
}
