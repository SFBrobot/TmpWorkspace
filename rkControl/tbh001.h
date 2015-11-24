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
		doRun,
		isOnTgt;
}

void setTbhTbhFactor(Tbh *tbh, float factor) { tbh->tbhFactor = .5 * factor; }

bool isTbhInThresh(Tbh *tbh, float thresh) { return abs(tbh->err) <= thresh; }

void updateTbhErr(Tbh *tbh) {
	tbh->err = tbh->setpoint - tbh->input;

	tbh->isOnTgt = isTbhInThresh(tbh, tbh->thresh);

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
	tbh->doRun = true;

	setTbhTbhFactor(tbh, tbhFactor);
	resetTbh(tbh, 0);
}

float updateTbh(Tbh *tbh, float input, float dt) {
	tbh->input = input;

	updateTbhErr(tbh);

	if (!tbh->doRun) return tbh->out = 0;

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
