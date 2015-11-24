#ifndef RKCONTROL_DIFF
#define RKCONTROL_DIFF

struct Differentiator {
	float input,
		inLast,
		out;
}

typedef Differentiator Diff; //I'm lazy

void resetDiff(Diff *diff, float value) {
	diff->out = 0;
	diff->inLast = diff->input = value;
}

float updateDiff(Diff *diff, float input, float dt) {
	diff->inLast = diff->input;
	diff->input = input;

	if (dt == 0) return diff->out = 0;

	return diff->out = (diff->input - diff->inLast) / dt;
}

struct AvgDifferentiator {
	Diff diff;

	float diffLast,
		out;
}

typedef AvgDifferentiator ADiff; //Yep, still lazy.

void resetADiff(ADiff *aDiff, float value) {
	resetDiff(aDiff->diff, value);

	aDiff->diffLast = 0;
}

float updateADiff(ADiff *aDiff, float input, float dt) {
	aDiff->diffLast = aDiff->diff.out;

	return aDiff->out = (aDiff->diffLast + updateDiff(&aDiff->diff, input, dt)) / 2;
}

#endif
