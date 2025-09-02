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
  Start
};

enum _TransitionalAppState {
  TModify,
  TRecall,
  TStart,
  Junk
};

volatile int ratings[3] = {5, 2, 4};

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

void loop()
{
  if (pastAppState != appState) {
    Serial.print("App state: ");
    Serial.println(appState);
    pastAppState = appState;
  }

  _TransitionalAppState tt = Junk;
  if (digitalRead(B1) == HIGH) {
    lrs = MacOS;
    if (appState != Modify) tt = TRecall;
  } else if (digitalRead(B2) == HIGH) {
  	lrs = Windows;
    if (appState != Modify) tt = TRecall;
  } else if (digitalRead(B3) == HIGH) {
  	lrs = Linux;
    if (appState != Modify) tt = TRecall;
  } else if (digitalRead(BS) == HIGH) {
    // toggle
    if (appState != Modify) {
      tt = TModify;
      Serial.println("hiii");
    }
    else appState = Start;
    Serial.println("hihii");
  }

  if (lrs != pastLrs) {
    Serial.print("lrs: ");
    Serial.println(lrs);
    pastLrs = lrs;
  }
  
  if (appState == Recall && tt != TRecall && tt != TModify) return;
  else if (tt == TRecall) {
    appState = (int) tt; // enums map to numbers starting with 0 index so we're safe

    if (lrs == MacOS) {
      reset();
      delay(100);
      animateTo(L5);
    } else if(lrs == Linux) {
      reset();
      delay(100);
      animateTo(L4);
    } else if (lrs == Windows) {
      reset();
      delay(100);
      animateTo(L2);
    }
  }

  
  // while you're modifying, do
  if (appState == Modify) {
    Serial.println("byeeee");
    
    int cur_rating = lrsFromRating(analogRead(POT) * (5.0 / 1023.0));

    int past_rating = ratings[lrs];

    Serial.print("animate to:");
    Serial.println(past_rating);
    Serial.print("animate from:");
    Serial.println(cur_rating);

    animate(past_rating, cur_rating); // to, from
    
    // persist rating
    ratings[lrs] = cur_rating;
  }

  // when turning on modify mode, wait until user picks which OS to rate
  if (tt == TModify) {
    reset();
    delay(100);
    Serial.println("HIHIHI");
    appState = Modify; // finish transition
    lrs = Invalid; // led user pick which OS to rate afterwards
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

void animate(int to, int from) {
  for (int i = from; i >= to; --i) {
  	digitalWrite(i, HIGH);
    // Serial.println(1000 / (from - to));
    delay((700 / (from - to)) + 200);
  }
}

void reset() {
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);
  digitalWrite(L4, LOW);
  digitalWrite(L5, LOW);
}
