#ifndef RKCONTROL_BASE
#define RKCONTROL_BASE

bool doCtlLoop = false;
int ctlLoopInterval = 50;

void updateCtl(float dt);

task ctlLoop() {
	bool doRun = doCtlLoop;

	long time, lastTime = nSysTime - 1;
	float dt = 0;

	while (doRun) {
		doRun = doCtlLoop;
		time = nSysTime;

		if (time < lastTime) dt = 0.;
		else dt = (float)(time - lastTime) / 1000.;

		updateCtl(dt);

		lastTime = time;

		wait1Msec(ctlLoopInterval);
	}
}

void startCtlLoop() {
	if (!doCtlLoop) {
		doCtlLoop = true;

		startTask(ctlLoop);
	}
}

void stopCtlLoop() {
	doCtlLoop = false;
}
#endif
