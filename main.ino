#include <util/atomic.h>

#define L1 7
#define L2 6
#define L3 5
#define L4 4
#define L5 3

#define B1 8
#define B2 9
#define B3 10

#define BS 11

#define POT A0 // to set rating

enum LEDRecallState {
  MacOS,
  Windows,
  Linux,
  Invalid, // Invalid when appState != Recall
};

enum AppState {
  Modify,
  Recall,
  Start,
  TModify,
  TRecall,
  TStart
};


void setup()
{
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(L5, OUTPUT);
  
  pinMode(B1, INPUT);
  pinMode(B2, INPUT);
  pinMode(B3, INPUT);
  pinMode(BS, INPUT);

  pinMode(POT, INPUT);

  Serial.begin(9600);
}

AppState appState = Start;
LEDRecallState lrs = Invalid; // invalid since appState != Recall

AppState pastAppState = Start;
LEDRecallState pastLrs = Invalid;

// volatile int ratings[3] = {5, 2, 4};
int rating_1 = 5;
int rating_2 = 2;
int rating_3 = 4;

void loop()
{
  int curTime = millis();
  int prevTime = 0;

  if (pastAppState != appState) {
    Serial.print("App state has been changed to: ");
    Serial.println(appState);
    pastAppState = appState;

    // we don't finish transition to appState.Modify until user selects an OS to rate
    if (appState == TModify || appState == Start) reset();
  }

  if (digitalRead(B1) == HIGH) {
    buttonPress(MacOS);
    prevTime = curTime;
  }
  else if (digitalRead(B2) == HIGH) {
    buttonPress(Windows);
    prevTime = curTime;
  }
  else if (digitalRead(B3) == HIGH) {
    buttonPress(Linux);
    prevTime = curTime;
  }
  else if (digitalRead(BS) == HIGH) {
    // toggle
    if (appState != Modify && appState != TModify) {
      appState = TModify;
      lrs = Invalid; // lrs is set after an OS is selected; then app will transition from appState.TModify to appState.Modify
      Serial.println("Switched to appState.TModify mode");

      delay(200); // delay after toggle to prevent rapid switches
    }
    else {
      appState = TStart;
      reset();
      Serial.println("Switched to appState.TStart mode");

      delay(200); // delay after toggle to prevent rapid switches
      return;
    }
  }

  if (lrs != pastLrs) {
    Serial.print("LED recall state changed to: ");
    if (lrs == 0) Serial.println("MacOS (value 0)");
    if (lrs == 1) Serial.println("Windows (value 1)");
    if (lrs == 2) Serial.println("Linux (value 2)");
    if (lrs == 3) Serial.println("Invalid (value 3)");
    pastLrs = lrs;
  }
  
  if (appState == Recall) return;
  else if (appState == TRecall) {
    int safe_rating_1;
    int safe_rating_2;
    int safe_rating_3;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      safe_rating_1 = rating_1;
      safe_rating_2 = rating_2;
      safe_rating_3 = rating_3;
    }
    if (lrs == MacOS) {
      reset();
      delay(100);
      Serial.print("Displaying rating for MacOS as: ");
      Serial.println(safe_rating_1);
      animateTo(lrsFromRating(safe_rating_1));
    } else if(lrs == Linux) {
      reset();
      delay(100);
      Serial.print("Displaying rating for Linux as: ");
      Serial.println(safe_rating_2);
      animateTo(lrsFromRating(safe_rating_2));
    } else if (lrs == Windows) {
      reset();
      delay(100);
      Serial.print("Displaying rating for Windows as: ");
      Serial.println(safe_rating_3);
      animateTo(lrsFromRating(safe_rating_3));
    }
    appState = Recall; // finish transition
  }
  
  if (appState == Modify) {
    int cur_rating = lrsFromRating((analogRead(POT) - 40) / 240 + 1);
    Serial.println(cur_rating);

    int past_rating;
    if (lrs == MacOS) {
      past_rating = rating_1;
    } else if (lrs == Windows) {
      past_rating = rating_2;
    } else if (lrs == Linux) {
      past_rating = rating_3;
    }

    if (past_rating != cur_rating) {
      Serial.print("animate to: ");
      Serial.println(cur_rating);
      Serial.print("animate from: ");
      Serial.println(past_rating);
      animate(cur_rating, past_rating, true); // to, from
    }
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      if (lrs == MacOS) {
        rating_1 = cur_rating;
      } else if (lrs == Windows) {
        rating_2 = cur_rating;
      } else if (lrs == Linux) {
        rating_3 = cur_rating;
      }
    }
  }
}

bool isTransitioning() {
  return (int) appState > 2;
}

void buttonPress(LEDRecallState param_lrs) {
  lrs = param_lrs;
  if (appState != Modify && appState != TModify) appState = TRecall;
  else if (appState == TModify) {
    appState = Modify;
    Serial.print("Switched to appState.Modify mode, selected the following to modify: ");
    Serial.println(lrs);
  }
}

int lrsFromRating(int rating) {
  return (rating - 1) + L5;
}

void animateTo(int n) {
  for (int i = L1; i >= n; --i) {
  	digitalWrite(i, HIGH);
    // Serial.println(1000 / (L1 - n));
    delay((700 / (L1 - n)) + 200);
  }
}

void animate(int to, int from, bool fast) {
  int time;
  if (fast) time = 100;
  else time = (700 / (from - to)) + 200;

  if (to < from) {
    for (int i = from; i >= to; --i) {
      digitalWrite(i, HIGH);
      // Serial.println(1000 / (from - to));
      delay(time);
    }
  } else if (to > from) {
    for (int i = from; i <= to; ++i) {
      digitalWrite(i, LOW);
      // Serial.println(1000 / (from - to));
      delay(time);
    }
  }
}

void reset() {
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);
  digitalWrite(L4, LOW);
  digitalWrite(L5, LOW);
}
