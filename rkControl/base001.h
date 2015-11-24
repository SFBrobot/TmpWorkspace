#ifndef RKCONTROL_BASE
#define RKCONTROL_BASE

TTimers ctlTimer = T4;

bool doCtlLoop = false;
int ctlLoopInterval = 20;

void updateCtl(float dt);

task ctlLoop() {
	clearTimer(ctlTimer);

	long time, lastTime = time1[ctlTimer] - 1;
	float dt = 0;

	while (doCtlLoop) {
		time = time1[ctlTimer];
		dt = (float)(time - lastTime) / 1000.;

		updateCtl(dt);

		lastTime = time;

		wait1Msec(ctlLoopInterval);

		EndTimeSlice();
	}
}

void startCtlLoop() {
	if (!doCtlLoop) {
		doCtlLoop = true;

		clearTimer(ctlTimer);

		startTask(ctlLoop);
	}
}

void stopCtlLoop() {
	doCtlLoop = false;
}
#endif
