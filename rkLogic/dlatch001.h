#ifndef RKLOGIC_DLATCH
#define RKLOGIC_DLATCH

typedef struct {
	bool inLast,
		input,
		out;
} DLatch;

void resetDLatch(DLatch *latch, bool input) {
	latch->inLast = input;
	latch->out = 0;
}

void updateDLatch(DLatch *latch, bool input) {
	latch->inLast = latch->input;
	latch->input = input;
}

bool transient(DLatch *latch, bool input) {
	updateDLatch(latch, input);

	return latch->out = latch->input ^ latch->inLast;
}

bool risingEdge(DLatch *latch, bool input) {
	updateDLatch(latch, input);

	return latch->out = latch->input && !latch->inLast;
}

bool fallingEdge(DLatch *latch, bool input) {
	updateDLatch(latch, input);

	return latch->out = !latch->input && latch->inLast;
}

bool risingBistable(DLatch *latch, bool input) {
	updateDLatch(latch, input);

	if (latch->input && !latch->inLast) latch->out = !latch->out;

	return latch->out;
}

#endif
