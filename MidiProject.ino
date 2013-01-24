#include <ooPinChangeInt.h>

#include <MIDI.h>
#include <AdaEncoder.h>

#include <pt.h>

struct input {
  const unsigned char pin;
  const unsigned char midi;
  const unsigned char debounce;
  unsigned short val, state;
  unsigned long timestamp;

  struct pt pt;
};

//  PIN MIDI DEBOUNCE
struct input direct_buttons[] = {
  { 
    A4,  6, 10    }
  ,
  { 
    A5,  7, 10    }
  ,
  { 
    6,  0, 10     }
  ,
  { 
    7,  1, 10     }
};
struct input muxed_buttons[] = {
  { 
    0,  2, 10    }
  ,
  { 
    1,  3, 10    }
  ,
  { 
    2,  4, 10    }
  ,
  { 
    3,  5, 10    }
};

// no debounce
struct input muxed_pots[] = {
  {
    6,  0, 0    }
  ,
  {
    7,  1, 0    }
  ,
  {
    8,  2, 0    }
  ,
  {
    9,  3, 0    }
  ,
  {
    10, 4, 0    }
  ,
  {
    11, 5, 0    }
  ,
  {
    12, 6, 0    }
  ,
  {
    13, 7, 0    }
  ,
  {
    14, 8, 0    }

};


void set_channel(int channel) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(i + 2, bitRead(channel, i));
  }
}
AdaEncoder encoderA = AdaEncoder('a', 8, 9);
AdaEncoder encoderB = AdaEncoder('b', 10, 11);

void setup () {
  MIDI.begin(1);
  Serial.begin(115200);

  // enable internal pull-up resistors
  for (int i = 0; i < (sizeof(direct_buttons) / sizeof(struct input)); i++) {
    pinMode(direct_buttons[i].pin, INPUT);
    digitalWrite(direct_buttons[i].pin, HIGH);
  }
  pinMode(A0, INPUT);

  pinMode(6, INPUT); 
  digitalWrite(6, HIGH);
  pinMode(7, INPUT); 
  digitalWrite(7, HIGH);
  pinMode(A4, INPUT); 
  digitalWrite(A4, HIGH);
  pinMode(A5, INPUT); 
  digitalWrite(A5, HIGH);
  // set mux channels to output mode
  for (int i = 0; i < 4; i++) {
    pinMode(i + 2, OUTPUT);
  }


}

bool firstrun = false;
PT_THREAD(handle_direct_input(struct input *btn)) {
  PT_BEGIN(&btn->pt);
  btn->val = digitalRead(btn->pin);
  if (btn->val != btn->state) {
    PT_WAIT_UNTIL(&btn->pt, (millis() - btn->timestamp) > btn->debounce);
    if (digitalRead(btn->pin) == btn->val) {
      if (btn->val == LOW) {
        MIDI.sendNoteOn(btn->midi, 127, 1);
      } 
      else {
        MIDI.sendNoteOff(btn->midi, 127, 1);
      }
      btn->state = btn->val;
    }
    btn->timestamp = millis();
  }

  PT_END(&btn->pt);
}

PT_THREAD(handle_muxed_button(struct input *btn)) {
  PT_BEGIN(&btn->pt);

  set_channel(btn->pin);
  digitalWrite(A0, HIGH);
  btn->val = digitalRead(A0);
  if (btn->val != btn->state) {
    digitalWrite(A0, LOW);
    PT_WAIT_UNTIL(&btn->pt, (millis() - btn->timestamp) > btn->debounce);
    digitalWrite(A0, HIGH);
    if (digitalRead(A0) == btn->val) {
      if (btn->val == LOW) {
        MIDI.sendNoteOn(btn->midi, 127, 1);
      } 
      else {
        MIDI.sendNoteOff(btn->midi, 127, 1);
      }
      btn->state = btn->val;
    }
    btn->timestamp = millis();

  }
  digitalWrite(A0, LOW);
  PT_END(&btn->pt);
}

void handle_muxed_pot(struct input *pot) {
  set_channel(pot->pin);

  analogRead(A0); // the ADC capacitors ake some time to fill up, so we do 2 readings.
  /* the arduino ADC gives 10 bit readings (0v = 0, 5v = 1 << 10). MIDI takes 14 bit readings.
   * this code converts a 10 bit reading to a 14 bit reading
   */
  int val = analogRead(A0);

  val = val < 8 ? 0 : val > 1018 ? 1023 : val;
  val = map(val,0,1023,0,127);

  if (val != pot->val) {
    MIDI.sendControlChange(pot->midi, val, 1); // send last 7 bits
    pot->val = val;
  }


}



void handle_encoders() {
  AdaEncoder *enc = NULL;
  enc = AdaEncoder::genie();

  if (enc != NULL) {
    int clicks = enc->query();
    MIDI.sendControlChange(enc->getID() + 20, clicks, 1);
  }
}

void loop () {

  for (int i = 0; i < (sizeof(muxed_buttons) / sizeof(struct input)); i++) {
    handle_muxed_button(&muxed_buttons[i]);
  }
  for (int i = 0; i < (sizeof(direct_buttons) / sizeof(struct input)); i++) {
    handle_direct_input(&direct_buttons[i]);
  }  
  for (int i = 0; i < (sizeof(muxed_pots) / sizeof(struct input)); i++) {
    handle_muxed_pot(&muxed_pots[i]);
  }
  //MIDI.sendControlChange(5, map(analogRead(A1), 0, 1023, 0, 127), 1);
  //handle_muxed_pot(&muxed_pots[0]);
  handle_encoders();


}


