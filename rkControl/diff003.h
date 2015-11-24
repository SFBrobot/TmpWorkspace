#ifndef RKCONTROL_DIFF
#define RKCONTROL_DIFF

typedef struct {
	float input,
		inLast,
		out;
} Differentiator;

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

typedef struct {
	Diff diff;

	float diffLast,
		out;
} AvgDifferentiator;

typedef AvgDifferentiator ADiff; //Yep, still lazy.

void resetDiff(ADiff *aDiff, float value) {
	resetDiff(aDiff->diff, value);

	aDiff->diffLast = 0;
}

float updateDiff(ADiff *aDiff, float input, float dt) {
	aDiff->diffLast = aDiff->diff.out;

	return aDiff->out = (aDiff->diffLast + updateDiff(&aDiff->diff, input, dt)) / 2;
}

typedef struct {
	Diff diff;

	float flt,
		out;
} FltDifferentiator;

typedef FltDifferentiator FltDiff;

void setDiffDFlt(FltDiff *fDiff, float flt) { fDiff->flt = pow(.1, flt / 10 - 5); }

void resetDiff(FltDiff *fDiff, float value) {
	resetDiff(fDiff->diff, value);

	fDiff->out = fDiff->diff.out; //Defer to resetDiff for value handling
}

float updateDiff(FltDiff *fDiff, float input, float dt) {
	updateDiff(fDiff->diff, input, dt);

	return fDiff->out += (1 - pow(.5, dt * fDiff->flt)) * (fDiff->diff.out - fDiff->out);
}
#endif
