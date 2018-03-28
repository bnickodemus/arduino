#define POTPIN 0       // The potentiometer is connected to A0
#define PHOTORESPIN 1  // The photo resistor is connected to A1
#define ONBTNPIN 4
#define  OFFBTNPIN 3

bool onState = false;
float samplingRate;

void setup()
{
  Serial.begin(9600);
  
  // Set up the pushButton pins to be an input
  pinMode(ONBTNPIN, INPUT);
  pinMode(OFFBTNPIN, INPUT);

  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_OVF_vect)        // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{
  if (onState) { // onBtn was pressed
    noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = samplingRate;
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
    interrupts();             // enable all interrupts

    int lightLevel = analogRead(PHOTORESPIN);
    Serial.println(lightLevel);
  }
}

void loop()
{    
  // read potPin and set samplingRate
  if (onState == false) {
    int sensorValue = analogRead(POTPIN);
    float value = map(sensorValue, 0, 1023, 1, 200); // 0 - 1023 to 0.5Hz to 100Hz (divide double the value)
    samplingRate = 65536-(16*1000000/256/value); // timer1: 65536-(16MHz/256/1) = 0.5Hz = 3036 
  }
  
  if ((digitalRead(ONBTNPIN) == HIGH)) { // onBtn pressed
    onState = true;
  }
  if ((digitalRead(OFFBTNPIN) == HIGH)) { // offBtn pressed
    onState = false;
  }
}

