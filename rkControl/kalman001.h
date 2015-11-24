typedef struct {
	float envNoise,
		errCovar,
		gain,
		input,
		out;

} KalmanFilter;

typedef KalmanFilter KFlt;

void resetKFlt(KFlt *flt, float value, float errCovar) {
	flt->errCovar = errCovar;
	flt->out = value;
}

void initKFlt(KFlt *flt, float envNoise) {
	flt->envNoise = envNoise;
	flt->gain = 0;
	flt->input = 0;

	resetKFlt(flt, 0, 0);
}

void updateKFlt(KFlt *flt, float input, float dt) {
	flt->input = input;

	flt->gain = flt->errCovar / (flt->errCovar + flt->envNoise);

	flt->out = flt->out + flt->gain * (flt->input - flt->out);

	flt->errCovar *= (1 - flt->gain);
}
