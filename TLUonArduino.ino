/*
 Threshold Logic Unit implementation on Arduino.
 
 Programmed by tomi.nori at gmail.com
 
 Refference:
  McCulloch, W. and Pitts, W. (1943)."A logical calculus of the ideas immanent in nervous activity.", 
  Bulletin of Mathematical Biophysics, 7:115 - 133.
*/
//---------------------------------------------------
//Pin Settings
//---------------------------------------------------
int pushLearnButton = 2;   //Learn 学習ボタン
int pusCorrectPin   = 12;  //CorrectInput 教師信号ボタン
int pushButtonX1    = 6;   //Input 入力 x1
int pushButtonX2    = 11;  //Input 入力 x2

int ledPinOut       = 8;   //Output 出力表示
int ledPinLearn     = 10;  //CorrectOutput 教師信号表示

//---------------------------------------------------
//Initialize
//---------------------------------------------------
void setup() {
  //Out
  pinMode(ledPinOut, OUTPUT);
  pinMode(ledPinLearn, OUTPUT);
  //In
  pinMode(pushLearnButton, INPUT_PULLUP);
  pinMode(pusCorrectPin, INPUT_PULLUP);
  pinMode(pushButtonX1, INPUT_PULLUP);
  pinMode(pushButtonX2, INPUT_PULLUP);

  //setting interruput int_0
  attachInterrupt(0, LearningTLU, FALLING);//CHANGE

  //initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  //Hello World of embedded system. L Tika!
  for ( int i = 0; i < 2; i++ )
  {
    digitalWrite(ledPinOut, HIGH);
    digitalWrite(ledPinLearn, LOW);
    delay(100);
    digitalWrite(ledPinOut, LOW);
    digitalWrite(ledPinLearn, HIGH);
    delay(100);
    digitalWrite(ledPinOut, HIGH);
    digitalWrite(ledPinLearn, HIGH);
    delay(100);
    digitalWrite(ledPinOut, LOW);
    digitalWrite(ledPinLearn, LOW);
    delay(100);
  }

  //check
  DebugCheck();
}

//---------------------------------------------------
//Loop
//---------------------------------------------------
//neulon
const float ALPHA            = 0.08;//学習係数
const float NEULONACTIVATION = 0.5; //ニューロン発火の閾値
const int   MAXLEARNING      = 100; //1回あたりの最大学習回数

//weight
float W0 = 0;
float W1 = 0.1;
float W2 = 0.1;

//InputNow
int gInputX1 = 0;
int gInputX2 = 0;
int gCorrect = 0;

void loop() {
  //INPUT_PULLUP なので 反転
  if ( digitalRead(pushButtonX1) == HIGH )
  {
    gInputX1 = LOW;
  }
  else
  {
    gInputX1 = HIGH;
  }
  if ( digitalRead(pushButtonX2) == HIGH )
  {
    gInputX2 = LOW;
  }
  else
  {
    gInputX2 = HIGH;
  }

  //Correct
  if ( digitalRead(pusCorrectPin) == HIGH )
  {
    digitalWrite(ledPinLearn, LOW);
    gCorrect = LOW;
  }
  else
  {
    digitalWrite(ledPinLearn, HIGH);
    gCorrect = HIGH;
  }

  //OUTPUT
  int outputTemp = EvaluateTLU( gInputX1, gInputX2 );
  digitalWrite(ledPinOut, outputTemp);

  //Debug using Serial Monitor
  if ( digitalRead(pushLearnButton) == LOW )
  {
    DebugCheck();
  }
}

//---------------------------------------------------
//int_0 割り込み データセットによる学習を行う
//---------------------------------------------------
int EvaluateTLU(int x1, int x2) {
  if ( (W0 + W1 * x1 + W2 * x2) >= NEULONACTIVATION )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

//Interrupt Function
void LearningTLU() {
  int inputX1 = gInputX1;
  int inputX2 = gInputX2;
  int correct = gCorrect;

  if ( EvaluateTLU(inputX1, inputX2) == correct) {
    return;
  }
  
  //Learn
  float diff  = correct - EvaluateTLU(inputX1, inputX2);
  W0 += ALPHA * diff;
  W1 += ALPHA * diff * inputX1;
  W2 += ALPHA * diff * inputX2;
}

//---------------------------------------------------
//debug
//---------------------------------------------------
//float to string
//reffrence:http://playground.arduino.cc/Main/FloatToString
char * floatToString(char * outstr, float value, int places, int minwidth = 0, bool rightjustify = false) {
  // this is used to write a float value to string, outstr.  oustr is also the return value.
  int digit;
  float tens = 0.1;
  int tenscount = 0;
  int i;
  float tempfloat = value;
  int c = 0;
  int charcount = 1;
  int extra = 0;
  // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
  // if this rounding step isn't here, the value  54.321 prints as 54.3209

  // calculate rounding term d:   0.5/pow(10,places)
  float d = 0.5;
  if (value < 0)
    d *= -1.0;
  // divide by ten for each decimal place
  for (i = 0; i < places; i++)
    d /= 10.0;
  // this small addition, combined with truncation will round our values properly
  tempfloat +=  d;

  // first get value tens to be the large power of ten less than value
  if (value < 0)
    tempfloat *= -1.0;
  while ((tens * 10.0) <= tempfloat) {
    tens *= 10.0;
    tenscount += 1;
  }

  if (tenscount > 0)
    charcount += tenscount;
  else
    charcount += 1;

  if (value < 0)
    charcount += 1;
  charcount += 1 + places;

  minwidth += 1; // both count the null final character
  if (minwidth > charcount) {
    extra = minwidth - charcount;
    charcount = minwidth;
  }

  if (extra > 0 and rightjustify) {
    for (int i = 0; i < extra; i++) {
      outstr[c++] = ' ';
    }
  }

  // write out the negative if needed
  if (value < 0)
    outstr[c++] = '-';

  if (tenscount == 0)
    outstr[c++] = '0';

  for (i = 0; i < tenscount; i++) {
    digit = (int) (tempfloat / tens);
    itoa(digit, &outstr[c++], 10);
    tempfloat = tempfloat - ((float)digit * tens);
    tens /= 10.0;
  }

  // if no places after decimal, stop now and return

  // otherwise, write the point and continue on
  if (places > 0)
    outstr[c++] = '.';

  // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
  for (i = 0; i < places; i++) {
    tempfloat *= 10.0;
    digit = (int) tempfloat;
    itoa(digit, &outstr[c++], 10);
    // once written, subtract off that digit
    tempfloat = tempfloat - (float) digit;
  }
  if (extra > 0 and not rightjustify) {
    for (int i = 0; i < extra; i++) {
      outstr[c++] = ' ';
    }
  }

  outstr[c++] = '\0';
  return outstr;
}

void DebugCheck() {
  char buf[50];
  Serial.println("Learning Result");
  Serial.println("-----------Weight------------");
  Serial.println("W0=");
  Serial.println(floatToString(buf, W0, 5));
  Serial.println("W1=");
  Serial.println(floatToString(buf, W1, 5));
  Serial.println("W2=");
  Serial.println(floatToString(buf, W2, 5));
  Serial.println("-----------OutputCheck-------");
  Serial.println(" InputA , InputB , TLUOutput");
  snprintf(buf, sizeof(buf), "    0   ,    0   ,    %d", EvaluateTLU(0, 0));
  Serial.println(buf);
  snprintf(buf, sizeof(buf), "    1   ,    0   ,    %d", EvaluateTLU(1, 0));
  Serial.println(buf);
  snprintf(buf, sizeof(buf), "    0   ,    1   ,    %d", EvaluateTLU(0, 1));
  Serial.println(buf);
  snprintf(buf, sizeof(buf), "    1   ,    1   ,    %d", EvaluateTLU(1, 1));
  Serial.println(buf);
}

