// Written by KKQuark
// LED pattern flashing controller
// NOTE: all times are in milliseconds
class Flasher {
  public:
    // possible modes for a flasher to be in
    enum flashMode {
      modeStopped = 0,
      modePaused,
      modeRunning
    };

    // create a flasher with just an on-time and an off-time
    Flasher(unsigned int pinNo, unsigned long onTime, unsigned long offTime = 0L, bool startOn = true) :
          m_pinNo(pinNo),
          m_onTime(onTime),
          m_offTime((offTime == 0L) ? onTime : offTime),
          m_onOffList(NULL),
          m_listLength(0),
          m_listPos(0),
          m_startOn(startOn),
          m_on(!m_startOn),
          m_mode(modeRunning),
          m_switchTime(millis()),
          m_nextFlasher(NULL) {
      pinMode(m_pinNo, OUTPUT);
      switchLED();
      attachFlasher(this);
    }

    // create a flasher with a list of on/off times (for more complicated flashing patterns)
    Flasher(unsigned int pinNo, unsigned long* onOffList, unsigned int count, bool startOn = true) :
          m_pinNo(pinNo),
          m_onTime(0),
          m_offTime(0),
          m_onOffList(onOffList),
          m_listLength(count),
          m_listPos(0),
          m_startOn(startOn),
          m_on(!m_startOn),
          m_mode(modeRunning),
          m_switchTime(millis()),
          m_nextFlasher(NULL) {
      pinMode(m_pinNo, OUTPUT);
      switchLED();
      attachFlasher(this);
    }

    // pause a flasher exactly where it is in its cycle
    void pause() {
      if (m_mode == modeRunning) {
        m_mode = modePaused;
        m_switchTime -= millis();
      }
    }

    // resume a paused flasher exactly where it left off in its cycle
    void resume() {
      if (m_mode == modePaused) {
        m_mode = modeRunning;
        m_switchTime += millis();
      }
    }

    // stop a flasher (also turns it off)
    void stop() {
      if (m_mode != modeStopped) {
        m_mode = modeStopped;
        digitalWrite(m_pinNo, LOW);
      }
    }

    // restarts a stopped flasher at the beginning of its cycle (will also resume a paused flasher)
    void start() {
      if (m_mode == modeStopped) {
        m_mode = modeRunning;
        m_listPos = 0;
        m_on = !m_startOn;
        m_switchTime = millis();
        switchLED();
      } else if (m_mode == modePaused) {
        resume();
      }
    }

    // resets a flasher's values with a simple on-time/off-time pattern (sets the mode to running)
    void setFLash(unsigned long onTime, unsigned long offTime = 0L, bool startOn = true) {
      m_onOffList = NULL;
      m_listLength = 0;
      m_listPos = 0;
      m_onTime = onTime;
      m_offTime = offTime ? onTime : offTime;
      m_startOn = startOn;
      m_on = !m_startOn;
      m_mode = modeRunning;
      switchLED();
    }

    // resets a flasher's values with a flashing pattern
    void setFLash(unsigned long* onOffList, unsigned int listLength, bool startOn = true) {
      m_onTime = 0;
      m_offTime = 0;
      m_onOffList = onOffList;
      m_listLength = listLength;
      m_listPos = 0;
      m_startOn = startOn;
      m_on = !m_startOn;
      m_mode = modeRunning;
      switchLED();
    }

    static void loop() {
      if (FlasherList) {
        unsigned long t = millis();
        Flasher* next = FlasherList;
        while (next) {
          next->flashLoop(t);
          next = next->m_nextFlasher;
        }
      }
    }

  private:
    void flashLoop(unsigned long t) {
      // see if the flasher is currently active
      if (m_mode != modeRunning) return;
      // if the flasher has reached its switch time, switch it
      if (t >= m_switchTime) {
        switchLED();
      }
    }

    void switchLED() {
      // see if the flasher is currently active
      if (m_mode != modeRunning) return;
      // calclate the new state of the LED (just flip it)
      m_on = !m_on;
      if (m_on) {
        // if the LED is supposed to be on now, turn it on and adjust the time for the next flip
        digitalWrite(m_pinNo, HIGH);
        // if the on time is not specified, assume we are using a time list
        if (m_onTime) {
          m_switchTime += m_onTime;
          return;
        }
      } else {
        // the LED is supposed to go off now, so turn it off and adjust the time
        digitalWrite(m_pinNo, LOW);
        if (m_onTime) {
          m_switchTime += m_offTime;
          return;
        }
      }
      // if we got to here, m_onTime is zero, so the timer is controlled by a list of on/off times
      if (m_listLength) {
        // if the time list index is too large, reset it to 0
        if (m_listPos >= m_listLength) {
          m_listPos = 0;
        }
        // adjust the time for the next LED state flip
        m_switchTime += m_onOffList[m_listPos++];
      }
    }
    
    static void attachFlasher(Flasher* pFlasher) {
      if (pFlasher) {
        pFlasher->m_nextFlasher = FlasherList;
        FlasherList = pFlasher;
      }
    }

    unsigned int m_pinNo;
    // onTime/offTime is the simple version of a flasher
    unsigned long m_onTime;
    unsigned long m_offTime;
    // if onTime is zero, the more versatile on/off list is used for LED timing
    unsigned long* m_onOffList;
    unsigned int m_listLength;
    unsigned int m_listPos;
    // what state the LED starts in (first time or after a stop/start cycle)
    bool m_startOn;
    // the current state of the LED
    bool m_on;
    // the current mode of the flasher, can be stopped, paused, or running
    flashMode m_mode;
    // time of the next LED state change (if paused, this will be a delta value until resume() is called)
    unsigned long m_switchTime;
    // link to next flasher in the linked list of all flashers
    Flasher* m_nextFlasher;
    
    static Flasher* FlasherList;
};

Flasher* Flasher::FlasherList = NULL;
