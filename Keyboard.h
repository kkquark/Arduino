#ifndef KEYBOARD_H
#define KEYBOARD_H  1
/**********************************************
/ In the keyboard hardware setup, put diodes on
/ the outPins to help the rollover work better.
/ Note however that there are still problems
/ with keyboards that don't have diodes on every
/ switch, so beware of ghost keys when pressing 3
/ or more keys at the same time
/
/ to use:
/   #include "Keyboard.h"
/   create a Keyboard object, using this constructor:
/     Keyboard(int noOutPins, unsigned int* outPins, int noInPins, unsigned int* inPins,
/             const char* keyMap, long debounceTime = DEBOUNCE_TIME, long repeatDelay = REPEAT_DELAY, long repeatTime = REPEAT_TIME,
/             char shiftKey = '\0', const char* shiftKeyMap = NULL, int shiftLED = -1)
/     e.g.
/         Keyboard kb(4, outPins, 4, inPins, "123A456B789C*0#D");
/   somewhere in your loop() function, call the keyboard loop() function, e.g.
/     void loop() {
/         kb.loop();
/         ...other loop processing, including:
/         key = kb.getKey();
/         if (key) {
/             ...process the key
/         }
/     }
/ **********************************************/

// choose how you want the shift key to operate
// no shift key functionality
#define SHIFT_OFF   0
// press and release shift key, and only the next keypress will be shifted (immediate 2nd shift releases it with no shift performed)
#define SHIFT_ONE   1
// press and release shift key, and all keypresses will be shifted until next shift key releases lock
#define SHIFT_LOCK  2
// press and hold shift key, all keypresses made during shift-hold will be shifted until shift key is released
#define SHIFT_HOLD  3
// set SHIFT_KEY functionality here to one of the settings above
#define SHIFT_KEY  SHIFT_ONE

// set the size of the key buffer (it actually holds one less than this, and holds both presses and releases)
// if two or more keys are held down simultaneously, there values will repeat intermingled, e.g. "abababababcabcabcabcabc"
#define KEY_ROLLOVER  9

// define default values for how the keys behave (debouncing and repeating -- all times in ms)
#define DEBOUNCE_TIME 20
#define REPEAT_DELAY  300
#define REPEAT_TIME   20

// different types of actions performed with a key (kaRepeat is holding the key down long enough for it to begin repeating)
enum KeyAction {
  kaPress,
  kaRelease,
  kaRepeat
};

// these are the modes of the state machine that processes key presses
enum KeyMode {
  kmReleased,
  kmPressDebounce,
  kmPressed,
  kmRepeating,
  kmReleaseDebounce
};

// this is a structure for holding all the details of a key action (which key, was it shifted, what happened)
class Action {
  public:
    int       m_index;
    bool      m_shift;
    KeyAction m_keyAction;
};

// each key has its own unique state, and that information is stored here (what mode is the key in -- debouncing, open, repeating -- and when is the next state change/event)
class KeyState {
  public:
    KeyState() : m_mode(kmReleased), m_nextEvent(0) {}

    KeyMode       m_mode;
    unsigned long m_nextEvent;
};

// this is a wrap-around buffer holding actions the user has taken with the keyboard
class ActionBuffer {
  public:
    ActionBuffer() : m_in(0), m_out(0) {}
  
    bool put(Action a) {m_buffer[m_in] = a; m_in = (m_in + 1) % KEY_ROLLOVER; if (m_in != m_out) return true; m_out = (m_out + 1) % KEY_ROLLOVER; return false;}
    bool get(Action& a) {if (m_in == m_out) return false; a = m_buffer[m_out]; m_out = (m_out +1) % KEY_ROLLOVER; return true;}
    void clear() {m_in = m_out = 0;}

  private:
    Action  m_buffer[KEY_ROLLOVER];
    int     m_in;
    int     m_out;
};

// Keyboard class -- represents the entire keyboard as an object, including all its states and modes and actions
class Keyboard {
  public:
    Keyboard(int noOutPins, unsigned int* outPins, int noInPins, unsigned int* inPins,
             const char* keyMap, long debounceTime = DEBOUNCE_TIME, long repeatDelay = REPEAT_DELAY, long repeatTime = REPEAT_TIME,
             char shiftKey = '\0', const char* shiftKeyMap = NULL, int shiftLED = -1) :
      m_noOutPins(noOutPins),
      m_noInPins(noInPins),
      m_outPins(outPins),
      m_inPins(inPins),
      m_keyState(NULL),
      m_shiftKey(shiftKey),
      m_shifted(false),
      m_keyMap(keyMap),
      m_shiftKeyMap(shiftKeyMap),
      m_shiftLED(shiftLED),
      m_debounceTime(debounceTime),
      m_repeatDelayTime(repeatDelay),
      m_repeatTime(repeatTime)
    {
      m_keyState = new KeyState[m_noOutPins * m_noInPins];
      enableKeyboardPins();
    }

    void  loop(unsigned long t = 0);
    char  getKey();
    bool  getAction(Action& action) {return m_actions.get(action);}
    int   actionToKey(Action a) {return (a.m_shift ? m_shiftKeyMap[a.m_index] : m_keyMap[a.m_index]);}

  private:
    // enable all the input and output pins as needed
    void enableKeyboardPins();
    // if there is a shift LED, set it according to the current state of the m_shifted property
    void setShiftLED() {if (m_shiftLED >= 0) m_shifted ? digitalWrite(m_shiftLED, HIGH) : digitalWrite(m_shiftLED, LOW);}
    // put an action into the action buffer
    bool putAction(Action action) {return m_actions.put(action);}
    
    int           m_noOutPins;
    int           m_noInPins;
    unsigned int* m_outPins;
    unsigned int* m_inPins;
    KeyState*     m_keyState;
    char          m_shiftKey;
    bool          m_shifted;
    const char*   m_keyMap;
    const char*   m_shiftKeyMap;
    int           m_shiftLED;
    long          m_debounceTime;
    long          m_repeatDelayTime;
    long          m_repeatTime;
    ActionBuffer  m_actions;
};

// enable the output and input pins that communicate with the keyboard
void Keyboard::enableKeyboardPins() {
  int inactive;

  // initialize the output pins to high 
  for (int i = 0; i < m_noOutPins; i++) {
    pinMode(m_outPins[i], OUTPUT);
    digitalWrite(m_outPins[i], HIGH);
  }
  // initialize the input pins with pullup resistors
  for (int i = 0; i < m_noInPins; i++) {
    if (m_inPins[i] == A6 || m_inPins[i] == A7) {
      // analog pins A6-A7 (on Nanos) need to have external pullups attached
      pinMode(m_inPins[i], INPUT);    // a resistor value of about 33k should be good
    } else {
      // configure digital pins (and other analog pins) for input with built-in pullups
      pinMode(m_inPins[i], INPUT_PULLUP);
    }
  }
  // initialize the state map for the current state of all the keys
  for (int i = 0; i < m_noOutPins; i++) {
    digitalWrite(m_outPins[i], LOW);
    for (int j = 0; j < m_noInPins; j++) {
      inactive = digitalRead(m_inPins[j]);
      m_keyState[i * m_noOutPins + j].m_mode = inactive ? kmReleased : kmPressed;
      m_keyState[i * m_noOutPins + j].m_nextEvent = 0;
    }
    digitalWrite(m_outPins[i], HIGH);
  }
  // initialize the shift LED pin and set the state of the LED (if there is one)
  if (m_shiftLED >= 0) {
    pinMode(m_shiftLED, OUTPUT);
    setShiftLED();
  }
}

// do internal processing of the keyboard state
void Keyboard::loop(unsigned long t) {
  // get the current time if not supplied
  if (t == 0) t = millis();
  Action  action;

  // scan all the keys and take the appropriate action
  for (int i = 0; i < m_noOutPins; i++) {
    // select the next row of keys
    digitalWrite(m_outPins[i], LOW);

    // scan through each key in the row and see what needs to be done
    for (int j = 0; j < m_noInPins; j++) {
      bool pressed;
      if (m_inPins[j] == A6 || m_inPins[j] == A7) {
        pressed = analogRead(m_inPins[j]) < 512;
      } else {
        pressed = !digitalRead(m_inPins[j]);
      }
      int index = action.m_index = i * m_noOutPins + j;
      action.m_shift = m_shifted;
      switch (m_keyState[index].m_mode) {
        case kmReleased:
          if (pressed) {
            m_keyState[index].m_mode = kmPressDebounce;
            m_keyState[index].m_nextEvent = t + m_debounceTime;
            // put a button press action in the action queue
            action.m_keyAction = kaPress;
            putAction(action);
          }
          break;
        case kmPressDebounce:
          if (t >= m_keyState[index].m_nextEvent) {
            m_keyState[index].m_mode = kmPressed;
            m_keyState[index].m_nextEvent += m_repeatDelayTime;
          }
          break;
        case kmPressed:
          if (pressed) {
            if (t >= m_keyState[index].m_nextEvent) {
              m_keyState[index].m_mode = kmRepeating;
              m_keyState[index].m_nextEvent += m_repeatTime;
              // put a button repeat action in the action queue
              action.m_keyAction = kaRepeat;
              putAction(action);
            }
          } else {
            m_keyState[index].m_mode = kmReleaseDebounce;
            m_keyState[index].m_nextEvent += m_debounceTime;
            // put a button release action in the action queue
            action.m_keyAction = kaRelease;
            putAction(action);
          }
          break;
        case kmRepeating:
          if (pressed) {
            if (t >= m_keyState[index].m_nextEvent) {
              m_keyState[index].m_nextEvent += m_repeatTime;
              // put a button repeat action in the action queue
              action.m_keyAction = kaRepeat;
              putAction(action);
            }
          } else {
            m_keyState[index].m_mode = kmReleaseDebounce;
            m_keyState[index].m_nextEvent += m_debounceTime;
            // put a button release action in the action queue
            action.m_keyAction = kaRelease;
            putAction(action);
          }
          break;
        case kmReleaseDebounce:
          if (t >= m_keyState[index].m_nextEvent) {
            m_keyState[index].m_mode = kmReleased;
            m_keyState[index].m_nextEvent = 0;
          }
          break;
      }
    }
    digitalWrite(m_outPins[i], HIGH);
  }
}

/*********************************************************
/ select how you want shift to work at top of file
/   no shift functionality
/   shift next keystroke only
/   shift key turns on and off shift_lock mode
/   shift all keystrokes as long as shift key is held down
/ *******************************************************/

// see if a key has been pressed or released, return character or 0 if none
char Keyboard::getKey() {
  // set the default return value
  char key = '\0';
  // check for a keyboard action
  Action action;
  if (getAction(action)) {
    // ignore key releases for this implementation (except for SHIFT_HOLD setting)
#if SHIFT_KEY == SHIFT_HOLD
    if (action.m_keyAction == kaRelease) {
      if (actionToKey(action) == m_shiftKey) {
        m_shifted = false;
        setShiftLED();
      }
    } else
#endif
    if (action.m_keyAction == kaPress || action.m_keyAction == kaRepeat) {
      key = actionToKey(action);
      // see if there is a shift key and if this is it
#if SHIFT_KEY == SHIFT_ONE
      if (key == m_shiftKey) {
        m_shifted = !m_shifted;
        key = '\0';
      } else {
        m_shifted = false;
      }
      setShiftLED();
#elif SHIFT_KEY == SHIFT_LOCK
      if (key == m_shiftKey) {
        m_shifted = !m_shifted;
        key = '\0';
      }
      setShiftLED();
#elif SHIFT_KEY == SHIFT_HOLD
      if (key == m_shiftKey) {
        m_shifted = true;
        key = '\0';
      }
      setShiftLED();
#endif
    }
  }
  return key;
}
#endif
