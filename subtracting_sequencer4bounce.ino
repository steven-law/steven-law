/* Mit diesem Notensequencer können "Wahrscheinlichkeiten" für jede Note der ersten Oktave eingegeben werden.
    Nach dem Einschalten werden die aktuellen Potipositionen auf % "Wahrscheinlichkeit" umgerechnet und in einem
    array hinterlegt. Es wird die Note mit der höchsten Wahrscheinlichkeit ermittelt.
    Nach Eingang eines ersten Clocksignals soll die Note mit der höchsten WHK gespielt werden.
    Danach werden der Note 10% wahrscheinlichkeit abgezogen und der nächste maximalwert ermittelt.
    Nach einem neuelichen Clocksignal wird der gerade ermittelte maximalwert ausgegeben, 10%WHK der gerade
    gespielten Note abgezogen und der nächste Maximalwert ermittelt.

    Eine Note wird mitunter solange gespielt (dabei weiterhin ein Abzug von 10& WHK bei jedem Clocksignal),
    bis es eine andere Note gibt deren WHK mittlerweile höher ist.
    Wenn die WHK für eine Note unter 10 fällt wird die Position des jeweiligen AnalogInputs abgefragt
    und als neue WHK herangezogen.
    Optional:
 *  **Wenn alle Noten ein ähnliches WHK Level erreicht haben kann mittels random die WHK für die Notenfolge variiert werden.

    With this Sequencer you can set different "probabilitys" for each note in the first octave.
    After powering on, the actual potivalues for notes get scaled to 0-100% "probability" and stored in an array. The Note with
    the highest "prob" will be determined. After a clockin signal this note gets outputted on the dacOutput, the "prob" for this
    note gets decreased by a "-amount%" from 50% to 1%, so shorter or longer repetitions are possible. The next highest "prob" gets
    determined, the next clockinput sets the new value to the dac, this value gets decreased by "-amount%" and so on.

    As the sequencer decreases one value with every run, you can set somehow how many times a notes gets played within one sequence.
    As the sequencer rereads one analogIn as when its under 10, there should be( not tested yet) a slightly changing sequence with notes
    that are over 11% prob. Notes under 10% should never be played, as there is always a note with a higher prob.
    The same note can be played for a longer period as long as it´s "prob" is higher then the others.
    If a value gets under 10&, the analogIn is read, the value gets scaled to the new corresponding prob of that note.
    opt:
    1)If all notes have more or less the same prob, a random amount can be activated to switch between notes
      (as i switched from "-10points" prob decrease to "-amount% (that can be varied!) i think it´s not usefull anymore.
      also there is randomControl over Octaves, so i think it´s really not necessary. Maybe someone wants to implement this feature.)
    2) a byte lock function like the turing machine. As this sequencer somehow works like a programmable touring machine, it still
       would be cool to lock the last n bytes.
    2a) and/ or a reset input, to read all note analogIn values and start from there


*/
#include <Bounce2.h>
#include <MyMCP4725.h>
MyMCP4725 DAC(0x60); //I2C-Adress = 0x60
#define clockPin 2
#define gateOut 3
#define resetPin 4
Bounce2::Button clockState = Bounce2::Button();
int gateState = LOW;
long myTimer = 0;
long myTimeout = 100;
int resetState = LOW;
const int analogReadArraysize = 12;
int analogReadArray[analogReadArraysize];             // erstelle ein Array mit 4 Werten, hier werden die analogreads der einzelnen noten für den durchlauf gespeichert
int readPins[analogReadArraysize] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
float outPut[5] [12] = {
  {0, 0.08333, 0.16667, 0.25, 0.33333, 0.41667, 0.5, 0.58333, 0.66667, 0.75, 0.83333, 0.91667},
  {1, 1.08333, 1.16667, 1.25, 1.33333, 1.41667, 1.5, 1.58333, 1.66667, 1.75, 1.83333, 1.91667},
  {2, 2.08333, 2.16667, 2.25, 2.33333, 2.41667, 2.5, 2.58333, 2.66667, 2.75, 2.83333, 2.91667},
  {3, 3.08333, 3.16667, 3.25, 3.33333, 3.41667, 3.5, 3.58333, 3.66667, 3.75, 3.83333, 3.91667},
  {4, 4.08333, 4.16667, 4.25, 4.33333, 4.41667, 4.5, 4.58333, 4.66667, 4.75, 4.83333, 4.91667}
}; //Ausgangsspannungen für den DAC entsprechend den Noten C, C#, D, D# im 1V/Oktave System (12Noten/1V + Oktave)
int maxValIndex = 0;
int maxVal;
//int clockState = 0;         // Variable to save tasterstate
//bool dacStatus = true;
bool dacStatus = false;


void setup() {
  Serial.begin(115200);
  if (DAC.isReady())
  {
    Serial.println("\nDAC in Ordnung!");
    dacStatus = true;
  }
  else Serial.println("DAC Fehler!");

  clockState.attach( clockPin, INPUT );
  clockState.interval(5); // interval in ms
  clockState.setPressedState(HIGH);
  pinMode(gateOut, OUTPUT);
  digitalWrite(gateOut, gateState);
  pinMode(resetPin, INPUT);
  for (int i = 0; i < analogReadArraysize; i++) {
    analogReadArray[i] = map(analogRead(readPins[i]), 0, 1023, 0, 100);
  }
}

void loop() {
  resetState = digitalRead(resetPin);

  clockState.update();

  maxVal = 0;
  digitalWrite(gateOut, LOW);
  if ( clockState.pressed() ) {
    digitalWrite(gateOut, HIGH);
    if (dacStatus)
    {
      for (int i = 0; i < analogReadArraysize; i++) {
        if (analogReadArray[i] > maxVal) {
          maxVal      = analogReadArray[i];
          maxValIndex = i;

        }
      }
    }

    DAC.setVoltageFast(outPut[random(0, (int)map(analogRead(A12), 0, 1023, 0, 4))] [maxValIndex]);
    analogReadArray[maxValIndex] = analogReadArray[maxValIndex] - (map(analogRead(A13), 0, 1023, 1, 14));
    //analogReadArray[maxValIndex] = analogReadArray[maxValIndex] - 8;

    if (analogReadArray[maxValIndex] <= 15) {
      analogReadArray[0] = map(analogRead(0), 0, 1023, 0, 100);
      analogReadArray[1] = map(analogRead(1), 0, 1023, 0, 100);
      analogReadArray[2] = map(analogRead(2), 0, 1023, 0, 100);
      analogReadArray[3] = map(analogRead(3), 0, 1023, 0, 100);
      analogReadArray[4] = map(analogRead(4), 0, 1023, 0, 100);
      analogReadArray[5] = map(analogRead(5), 0, 1023, 0, 100);
      analogReadArray[6] = map(analogRead(6), 0, 1023, 0, 100);
      analogReadArray[7] = map(analogRead(7), 0, 1023, 0, 100);
      analogReadArray[8] = map(analogRead(8), 0, 1023, 0, 100);
      analogReadArray[9] = map(analogRead(9), 0, 1023, 0, 100);
      analogReadArray[10] = map(analogRead(10), 0, 1023, 0, 100);
      analogReadArray[11] = map(analogRead(11), 0, 1023, 0, 100);
      //analogReadArray[maxValIndex] = map(analogRead(readPins[maxValIndex]), 0, 1023, 0, 100);
    }

    Serial.print(maxVal);
    Serial.print("--");
    Serial.print(maxValIndex);
    Serial.print("--");
    Serial.print((int)outPut);
    Serial.print("--");
    Serial.print(DAC.readRegister());
    Serial.print("--");
    Serial.print(gateState);
    Serial.print("--");
    Serial.println(resetState);

    Serial.print("-");
    Serial.print(analogReadArray[1]);
    Serial.print("-");
    Serial.print(analogReadArray[3]);
    Serial.print("----");

    Serial.print(analogReadArray[6]);
    Serial.print("-");
    Serial.print(analogReadArray[8]);
    Serial.print("-");
    Serial.println(analogReadArray[10]);

    Serial.print(analogReadArray[0]);
    Serial.print("-");
    Serial.print(analogReadArray[2]);
    Serial.print("-");
    Serial.print(analogReadArray[4]);
    Serial.print("-");
    Serial.print(analogReadArray[5]);
    Serial.print("-");
    Serial.print(analogReadArray[7]);
    Serial.print("-");
    Serial.print(analogReadArray[9]);
    Serial.print("-");
    Serial.println(analogReadArray[11]);
    Serial.println();
    //    gateState = !gateState;
    //    digitalWrite(gateOut, gateState); // WRITE THE NEW ledState

  }
  if (resetState == HIGH) {
    analogReadArray[0] = map(analogRead(0), 0, 1023, 0, 100);
    analogReadArray[1] = map(analogRead(1), 0, 1023, 0, 100);
    analogReadArray[2] = map(analogRead(2), 0, 1023, 0, 100);
    analogReadArray[3] = map(analogRead(3), 0, 1023, 0, 100);
    analogReadArray[4] = map(analogRead(4), 0, 1023, 0, 100);
    analogReadArray[5] = map(analogRead(5), 0, 1023, 0, 100);
    analogReadArray[6] = map(analogRead(6), 0, 1023, 0, 100);
    analogReadArray[7] = map(analogRead(7), 0, 1023, 0, 100);
    analogReadArray[8] = map(analogRead(8), 0, 1023, 0, 100);
    analogReadArray[9] = map(analogRead(9), 0, 1023, 0, 100);
    analogReadArray[10] = map(analogRead(10), 0, 1023, 0, 100);
    analogReadArray[11] = map(analogRead(11), 0, 1023, 0, 100);
  }

}
