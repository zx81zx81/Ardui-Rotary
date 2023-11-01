/* Copyright (c) 2023 zx81

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 

*/

#define PIN_DT      3
#define PIN_CK      7
#define PIN_LEFT    4
#define PIN_RIGHT   5

#define ROTARY_RELEASE_TIME 100
#define ROTARY_DROP_TIME     40
#define LOOP_MIN_TIME        10

//#define DEBUG

int first_time = 1;
int rotary_dir = 0;
int prev_rotary_dir = 0;
int next_rotary_dir = 0;
int rotary_counter = 0;
int seq_status = 0x00;
int prev_seq_status = 0x00;
int key_pressed_left  = 0;
int key_pressed_right = 0;

unsigned long current_time = 0;
unsigned long last_time    = 0;
unsigned long delta_time   = 0;
unsigned long rotary_time  = 0;

void  setup() {
# ifdef DEBUG
  Serial.begin(9600);
# endif
  pinMode( PIN_CK, INPUT);
  pinMode( PIN_DT, INPUT);
  pinMode( PIN_LEFT , OUTPUT);
  pinMode( PIN_RIGHT, OUTPUT);
}

void keys_update() 
{
  if (rotary_dir == 1) {
    if (! key_pressed_right) 
    {
      digitalWrite( PIN_RIGHT, HIGH);
      digitalWrite( PIN_LEFT , HIGH);
      delay(5);
      digitalWrite( PIN_RIGHT, LOW );
      digitalWrite( PIN_LEFT , HIGH);
# ifdef DEBUG
      Serial.println("Key right pressed");
# endif          
    }
    key_pressed_right = 1;
    key_pressed_left  = 0;

  } else 
  if (rotary_dir == -1) {
    if (! key_pressed_left) 
    {
      digitalWrite( PIN_RIGHT, HIGH);
      digitalWrite( PIN_LEFT , HIGH);
      delay(5);
      digitalWrite( PIN_RIGHT, HIGH);
      digitalWrite( PIN_LEFT , LOW );
# ifdef DEBUG
      Serial.println("Key left pressed");
# endif          
    }
    key_pressed_right = 0;
    key_pressed_left  = 1;
  } else 
  if (rotary_dir == 0) {
    if (key_pressed_left || key_pressed_right) {      
      digitalWrite( PIN_RIGHT, HIGH);
      digitalWrite( PIN_LEFT , HIGH);
# ifdef DEBUG
      Serial.println("Keys released");
# endif          
    }
    key_pressed_right = 0;
    key_pressed_left  = 0;
  }
}

void rotary_check()
{
  int pin_ck = digitalRead(PIN_CK);
    
  if (pin_ck == 1) seq_status |=  0x2;
  else
  if (pin_ck == 0) seq_status &= ~0x2;

  int pin_dt = digitalRead(PIN_DT); 

  if (pin_dt == 1) seq_status |=  0x1;
  else
  if (pin_dt == 0) seq_status &= ~0x1;
}

void  loop() 
{
  rotary_check();

  current_time = millis();
  delta_time   = current_time - last_time;
  
  if (first_time) {
    prev_seq_status = seq_status;
    rotary_dir = 0;
    prev_rotary_dir = 0;    
    first_time = 0;
    rotary_time = current_time;
    key_pressed_left  = 1;
    key_pressed_right = 1;

    keys_update();
  }

  if (delta_time > LOOP_MIN_TIME) 
  {
    last_time = millis();
      
    if ( prev_seq_status != seq_status) {
      // Check clock wise or counter clock wise
      int  check = ((prev_seq_status << 2) | seq_status);
      bool cw  = ((check == 0x2) || (check == 0x4) || (check == 0xB) || (check == 0xD));
      bool ccw = ((check == 0x1) || (check == 0x7) || (check == 0x8) || (check == 0xE));

      if (!cw && !ccw) {
        // one event has been missed !?
        prev_seq_status = seq_status;
# ifdef DEBUG        
        Serial.print("Status: skipped! ");
        Serial.print(check,HEX);
        Serial.println(delta_time);
# endif
        return;
      }

      if (cw) next_rotary_dir = 1;
      else if (ccw) next_rotary_dir = -1;
      else next_rotary_dir = 0;

      if (rotary_dir != next_rotary_dir) {
        if ((current_time - rotary_time) > ROTARY_DROP_TIME) {
          rotary_dir  = next_rotary_dir;
          rotary_time = current_time;

          keys_update();      
        }
        else {
# ifdef DEBUG
          Serial.print("Status: dropped! ");
          Serial.print(" dir=");
          Serial.print(rotary_dir);
          Serial.print(" next_dir=");
          Serial.print(next_rotary_dir);
          Serial.print(" ");
          Serial.println(current_time - rotary_time);
# endif          
          return;
        }
               
      } else {
        rotary_time = current_time;     
      }
          
# ifdef DEBUG
      Serial.print("Status: ");
      //Serial.print(prev_seq_status,HEX);
      //Serial.print(" ");
      //Serial.print(seq_status,HEX);
      //Serial.print(" ");
      Serial.print(check,HEX);
      Serial.print(" ");
      Serial.print(rotary_dir);
      Serial.print(" ");
      Serial.println(seq_status,HEX);  
# endif
    } else {
      if ((current_time - rotary_time) > ROTARY_RELEASE_TIME) {
        rotary_dir = 0;
        keys_update();      
      }
    }
                
    prev_seq_status = seq_status;
    prev_rotary_dir = rotary_dir;
  }
}
