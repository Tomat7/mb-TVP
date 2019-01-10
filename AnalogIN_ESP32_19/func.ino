void IRAM_ATTR onTimer() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  lastIsrAt = millis();
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  uint32_t TMRwrite;
  if (isrCounter > 10) TMRwrite = 800000;
  if (isrCounter > 30) TMRwrite = 500000;
  timerWrite(timer, TMRwrite);
  if (isrCounter == 41) timerAlarmDisable(timer);
}

void IRAM_ATTR onADCTimer()
{
  int16_t Inow = adcEnd(Ipin) - I_ZERO;
  int16_t Unow = adcEnd(Upin) - U_ZERO;
  if (ADCcounter > 0)
  {
    uint32_t I2 = Inow * Inow;
    uint32_t U2 = Unow * Unow;
    // Increment the counter and set the time of ISR
    portENTER_CRITICAL_ISR(&adcMux);
    SummI += I2;
    SummU += U2;
    ADCcounter--;
    portEXIT_CRITICAL_ISR(&adcMux);
    // Give a semaphore that we can check in the loop
    xSemaphoreGiveFromISR(ADCSemaphore[2], NULL);
  }
  adcStart(Ipin);
  adcStart(Upin);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void setupTimer()
{
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void setupADCTimer()
{
  ADCSemaphore[2] = xSemaphoreCreateBinary();
  ADCtimer = timerBegin(1, 80, true);
  timerAttachInterrupt(ADCtimer, &onADCTimer, true);
  timerAlarmWrite(ADCtimer, adcRate, true);
  timerAlarmEnable(ADCtimer);
}

void setupADC()
{
  adcAttachPin(Ipin);
  adcAttachPin(Upin);
  adcStart(Ipin);
  adcStart(Upin);
  //portENTER_CRITICAL_ISR(&adcMux);
  ADCcounter = ADCcounts;
  //portEXIT_CRITICAL_ISR(&adcMux);
}

int ShiftTest(int sPin, uint16_t counts)
{
  uint64_t Summ = 0;
  int16_t Value = 0;
  for (int i = 0; i < counts; i++)
  {
    Value = analogRead(sPin);
    Summ += Value;
  }
  int ret = int(Summ / counts);
  return ret;
}

void RMSintr()
{
  if (ADCcounter == 0)
  {
    msReadTime = micros() - msMicros;
    I = sqrt((float)SummI / (float)ADCcounts) * I_RATIO;
    U = sqrt((float)SummU / (float)ADCcounts) * U_RATIO;
    portENTER_CRITICAL_ISR(&adcMux);
    SummI = 0;
    SummU = 0;
    ADCcounter = ADCcounts;
    portEXIT_CRITICAL_ISR(&adcMux);
    msMicros = micros();
    ReadCycles++;
  }
}

void setupNetMB()   //Config Modbus IP
{
  //serialMAC();
  //displayMAC();
  initETH();
  serialIP();
  displayIP();
}
