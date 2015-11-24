#include "diff002.h"

typedef struct {
	float deriv,
		err,
		errThresh,
		errXing,
		input,
		integ,
		integLim,
		integXing,
		kI,
		kD,
		setpoint,
		thresh,
		out;

	bool doSgnLock,
		hasXed,
		doRun,
		doUpdate,
		isOnTgt;

	FltDiff inDiff;
} Tbh;

bool isTbhInThresh(Tbh *tbh, float thresh) { return abs(tbh->err) <= thresh; }

bool isTbhAccelInThresh(Tbh *tbh, float thresh) { return abs(tbh->inDiff.out) <= thresh; }

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

void setTbhDoRun(Tbh *tbh, bool doRun) {
	tbh->doRun = doRun;

	if (doRun) tbh->doUpdate = true;
}

void initTbh(
	Tbh *tbh,
	float thresh,
	float kI,
	float kD,
	float integLim,
	float derivFlt,
	bool doSgnLock) {
	tbh->integLim = integLim;
	tbh->kI = kI;
	tbh->kD = kD;
	tbh->thresh = thresh;
	tbh->out = 0;

	tbh->doSgnLock = doSgnLock;
	tbh->doUpdate = true;

	setFltDiffDFlt(tbh->inDiff, derivFlt);
	resetFltDiff(tbh->inDiff, 0);

	resetTbh(tbh, 0);
	setTbhDoRun(tbh, false);
}

float updateTbh(Tbh *tbh, float input, float dt) {
	tbh->input = input;

	updateTbhErr(tbh);
	updateFltDiff(tbh->inDiff, input, dt);

	if (!tbh->doRun) {
		tbh->doUpdate = false;
		return tbh->out = 0;
	}

	tbh->integ = fmaxf(-tbh->integLim, fminf(tbh->integLim, tbh->integ + tbh->errThresh * tbh->kI * dt));

	tbh->deriv = tbh->inDiff.out * -tbh->kD;

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

	return tbh->out = tbh->integ + tbh->deriv;
}
