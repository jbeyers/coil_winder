const int step_out = 3;
const int dir_out = 4;
const int disable_out = 5;
const int start_in = 2;
const int pot_in_an = 14;
const int pot_deadband = 20;
const unsigned long turns_to_do = 750ul;
const unsigned long steps_per_turn = 200ul;
const unsigned long steps_to_do = turns_to_do * steps_to_do;

unsigned long start_millis;
int start_latch;
unsigned long steps_done;
unsigned long turns_done;
int pot_center;
int pot_min;
int pot_max;
int pot_value;
int speed;
int range;
unsigned int step_delay;
bool forward;
bool can_turn;

void setup() { 
  // Start toggle.
  pinMode(start_in, INPUT_PULLUP);
  // Speed control potentiometer
  pinMode(pot_in_an, INPUT);
  // Stepper driver control
  pinMode(step_out, OUTPUT);
  pinMode(dir_out, OUTPUT);
  pinMode(disable_out, OUTPUT);
  // Initialise values
  pot_max = 0;
  pot_min = 1024;
  start_millis = millis();

  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
} 

void loop() { 
  if (digitalRead(start_in) == LOW) {
    // Active running.
    digitalWrite(disable_out, LOW);
    // Check if the pot is requesting a speed, and scale it
    // TODO: Use the Map function to do this.
    pot_value = analogRead(pot_in_an);
    pot_max = max(pot_value, pot_max);
    pot_min = min(pot_value, pot_min);
    speed = pot_value - pot_center;
    if (abs(speed) > pot_deadband) {
        if ( speed < 0 ) {
          digitalWrite(dir_out, HIGH);
          forward = false;
          range = pot_center - pot_deadband - pot_min;
          speed = 300*(pot_center - pot_value - pot_deadband)/range;
        } else {
          digitalWrite(dir_out, LOW);
          forward = true;
          range = pot_max - pot_center - pot_deadband;
          speed = 300*(pot_value - pot_center - pot_deadband)/range;
        }
        speed = speed + 10;
        step_delay = 1600 + (60000/speed);
        can_turn = true;
        if ((steps_done >= steps_to_do) && forward) {
          can_turn = false;
        }
        if (steps_done == 0 && !forward) {
          can_turn = false;
        }
        if (can_turn) {
            digitalWrite(step_out, HIGH);
            delayMicroseconds(step_delay);
            digitalWrite(step_out, LOW);
            delayMicroseconds(step_delay);
            if (forward) {
              steps_done++;
            } else {
              steps_done--;
            }
        }
    }
    // show the turns completed every second while running.
    // This could be in an interrupt, but meh
    if (millis() > start_millis + 1000ul) {
      turns_done = steps_done/steps_per_turn;
      Serial.print("turns = ");
      Serial.print(turns_done);
      Serial.println();
      start_millis = start_millis + 1000ul;
    }
  } else {
    // Reset the values for the counters and centering
    digitalWrite(disable_out, HIGH);
    steps_done = 0ul;
    // Get the min and max values for the speed control
    pot_value = analogRead(pot_in_an);
    pot_max = max(pot_value, pot_max);
    pot_min = min(pot_value, pot_min);
    // Set the center point. Note that this will be center when you enable the counter.
    pot_center = pot_value;
  }
}

