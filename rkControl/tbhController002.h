typedef struct {
  Tbh *tbh;

  float lastValue;

  bool doRunAtZero;
} TbhController;

void updateTbhController(TbhController *ctl, float value) {
  if (value != ctl->lastValue) {
    setTbhDoRun(ctl->tbh, ctl->doRunAtZero || value != 0);

    setTbh(ctl->tbh, value);

    ctl->lastValue = value;
  }
}

void resetTbhController(TbhController *ctl, float value) {
  ctl->lastValue = value;
}

void initTbhController(TbhController *ctl, Tbh *tbh, bool doRunAtZero) {
  ctl->tbh = tbh;

  ctl->doRunAtZero = doRunAtZero;

  resetTbhController(ctl, 0);
}
