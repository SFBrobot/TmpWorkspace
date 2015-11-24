int nTest = 0, nMain = 0;

task test() {
	while (true) {
		nTest++;

		wait1Msec(100);
	}
}


task main() {
	clearDebugStream();

	startTask(test);

	hogCPU();

	while (true) {
		nMain++;
	}

	releaseCPU();
}
