const int step_out = 3;
const int dir_out = 4;
const int disable_out = 5;
const int start_in = 2;
const int pot_in_an = 14;
const int pot_deadband = 20;
const unsigned long turns_to_do = 750ul;
const unsigned long steps_per_turn = 200ul;
const unsigned long steps_to_do = turns_to_do * steps_per_turn;
const unsigned int delay_lookup[] = { 12500u, 10000u, 8000u, 6300u, 5000u, 4000u, 3150u, 2500u, 2000u, 1600u, 1250u, 1000u, 800u, 800u };

unsigned long start_millis;
unsigned long steps_done;
unsigned long turns_done;
unsigned long current_micros;
unsigned long previous_micros;
int pot_center;
int pot_min;
int pot_max;
int pot_value;
int speed;
unsigned int step_delay;
unsigned int cycle_time;
bool forward;
bool can_turn;
bool allow_run;

void setup() { 
  // Start toggle.
  pinMode(start_in, INPUT_PULLUP);
  // Speed control potentiometer
  pinMode(pot_in_an, INPUT);
  // Stepper driver control
  pinMode(step_out, OUTPUT);
  pinMode(dir_out, OUTPUT);
  pinMode(disable_out, OUTPUT);
  digitalWrite(disable_out, LOW);
  // Initialise values
  pot_max = 678;
  pot_min = 370;
  start_millis = millis();
  speed = 0;
  allow_run = false;
  pot_center = 513;

  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
} 

void loop() { 
  if ((digitalRead(start_in) == LOW) && allow_run) {
    // Active running.
    digitalWrite(disable_out, LOW);
    // Check if the pot is requesting a speed, and scale it
    pot_value = analogRead(pot_in_an);
    // Continuously ensure we are not going out of bounds
    //pot_max = max(pot_value, pot_max);
    //pot_min = min(pot_value, pot_min);
    can_turn = true;
    if (pot_value > pot_center + pot_deadband) {
      forward = true;
      digitalWrite(dir_out, LOW);
      speed = constrain(map(pot_value, pot_center + pot_deadband, pot_max, 0, 13), 0, 13);
    } else if (pot_value < pot_center - pot_deadband) {
      forward = false;
      digitalWrite(dir_out, HIGH);
      speed = constrain(map(pot_value, pot_center - pot_deadband, pot_min, 0, 13), 0, 13);
    } else {
      can_turn = false;
    }
    step_delay = delay_lookup[speed];

    // Stops at the step counts
    if ((steps_done == steps_to_do) && forward) {
      can_turn = false;
    }
    if (steps_done == 0 && !forward) {
      can_turn = false;
    }

    // See where we are on the timing cycle.
    // Note: We assume that the cycle time is within unsigned int range
    current_micros = micros();
    if (current_micros >= previous_micros) {
      cycle_time = current_micros - previous_micros;
    } else {
      cycle_time = 4294967295 - previous_micros + current_micros;
    }

    if (can_turn) {
      delayMicroseconds(step_delay - cycle_time);
      digitalWrite(step_out, HIGH);
      delayMicroseconds(step_delay);
      digitalWrite(step_out, LOW);
      if (forward) {
        steps_done++;
      } else {
        steps_done--;
      }
    }
    previous_micros = micros();
  } else {
    // Reset the values for the counters and centering
    digitalWrite(disable_out, HIGH);
    steps_done = 0ul;
    // Get the min and max values for the speed control
    pot_value = analogRead(pot_in_an);
    //pot_max = max(pot_value, pot_max);
    //pot_min = min(pot_value, pot_min);
    // Set the center point. Note that this will be center when you enable the counter.
    //pot_center = pot_value;
    previous_micros = micros();
    allow_run = true;
  }
  // show the turns completed every second while running.
  // This could be in an interrupt, but meh
  if (millis() > start_millis + 1000ul) {
    turns_done = steps_done/steps_per_turn;
    Serial.print("turns = ");
    Serial.print(turns_done);
    Serial.print(" pot = ");
    Serial.print(pot_value);
    Serial.println();
    start_millis = start_millis + 1000ul;
  }
}
