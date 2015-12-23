/*
 * Arduino Baseball
 * Author: David Monnerat (www.davidmonnerat.com)
 * 12/23/2015
 * Rev 0.1
 * 
 * This was my first custom use of the Arduino kit. I told my spn that I would take him to the 
 * baseball field after school to hit some balls. That was before we stepped outside in to the rain. 
 * Instead of giving up, I built this baseball game with the Arduino microcontroller, and LCD, and some LED lights. The 
 * "pitch" is the LED lights moving to green. Push the button to swing. Weighted average for hit vs foul vs triple vs 
 * double vs out. Keeps score, changes sides. Of course, he figured it out and mostly hits home runs...
 *
 * To Do:
 * Stop the game after 9 innings (or extra innings if there is a tie)
 * Smooth out the having to hit the button after a hit, or indicate it better 
 */

#include <LiquidCrystal.h>

/* Layout of the pins */
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int buttonPin = 13; //the button
int buttonState = 0; //the state of the button

/*
 * Baseball Variables
 */
int inning = 1; //what inning is it
int homeScore = 0; //home team score
int visitorScore = 0; //visiting team score
int outs = 0; //how many outs
int strikes = 0; //how many strikes
int balls = 0; //how many balls
boolean atBat = false; //false == visitor
boolean firstBase = false;
boolean secondBase = false;
boolean thirdBase = false;

void setup() {
  // put your setup code here, to run once:

  lcd.begin(16, 2); //initialize the LCD
  pinMode(buttonPin, INPUT); //set up the button input

  /* Set all the LED pins to OUTPUT */
  for (int x = 7; x < 11; x++)
  {
    pinMode(x, OUTPUT);
  }

  // start a new game
  resetGame();
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW)
  {
    //the button is not pressed
  }
  else
  {
    showScore();
    showStatus();
    batterUp();
  }
}

/* The batting sequence.
 *  Batter up!
 *  Ready...
 *  (LED from top to bottom, turning on, then off when the next one comes on)
 *  Wait for batter to swing.
 *
 *  Process hit.
 *  Less than 150ms response time = Home Run
 *  More than 1s = strike
 *  Otherwise, weighted random number sequence to determine foul, out, single, double, triple.
 *
 *  Advance bases on hit.
 */
void batterUp()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Batter up!");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready...");

  for (int x = 10; x > 6; x--)
  {
    delay(random(500, 1500));
    digitalWrite(x, HIGH);
    digitalWrite(x + 1, LOW);
  }

  unsigned long startTime = millis();
  unsigned long interval = 0;

  lcd.setCursor(8, 0);
  lcd.print("SWING!");

  while (true)
  {
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW)
    {
      //button not pressed
    }
    else
    {
      //button pressed
      digitalWrite(7, LOW);
      unsigned long endTime = millis();
      interval = endTime - startTime;
      break;
    }
  }

  //process hit
  if (interval > 1000) {
    //strike
    strikes++;
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("STRIKE ");
    lcd.setCursor(10, 0);
    lcd.print(strikes);
    lcd.setCursor(12, 0);
    lcd.print("!");
    delay(2000);

    if (strikes >= 3)
    {
      lcd.setCursor(6, 1);
      lcd.print("OUT!");
      outs++;
    }
  }
  else if (interval < 150) {
    for (int x = 0; x < 5; x++)
    {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("!!!HOME RUN!!!");
      delay(500);
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("!!!HOME RUN!!!");
      delay(500);
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("!!!HOME RUN!!!");
      delay(500);
    }
    advanceBases(4);

  }
  else
  {
    //not a home run and not a late swing
    // 0 = foul ball
    // 1 2 = out
    // 3 4 = single
    // 5 = double
    // 6 = triple
    // 7 8 = out
    // 9 = single
    int hit = random(0, 9);

    switch (hit)
    {
      case 0:
        //foul baul
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("FOUL BALL!");

        if (strikes < 2)
        {
          strikes++;
        }

        lcd.setCursor(3, 1);
        lcd.print("STRIKE ");
        lcd.setCursor(10, 1);
        lcd.print(strikes);
        lcd.setCursor(11, 1);
        lcd.print("!");

        break;
      case 1:
      case 2:
      case 7:
      case 8:
        outs++;
        strikes = 0;
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("OUT!");
        lcd.setCursor(5, 1);
        lcd.print(outs);
        lcd.setCursor(7, 1);
        if (outs < 2)
        {
          lcd.print("Out.");
        }
        else
        {
          lcd.print("Outs.");
        }
        break;
      case 3:
      case 4:
      case 9:
        //single
        strikes = 0;
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("HIT!");
        lcd.setCursor(4, 1);
        lcd.print("SINGLE!");
        advanceBases(1);
        break;
      case 5:
        //double
        strikes = 0;
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("HIT!");
        lcd.setCursor(4, 1);
        lcd.print("DOUBLE!");
        advanceBases(2);
        break;
      case 6:
        //triple
        strikes = 0;
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("HIT!");
        lcd.setCursor(4, 1);
        lcd.print("TRIPLE!");
        advanceBases(3);
        break;
    }

    delay(2000);

  }

  if (outs >= 3)
  {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Change Sides!");
    //change sides
    atBat = !atBat;
    if (!atBat)
    {
      //if we're back to vistor, increment inning
      inning++;
    }
    strikes = 0;
    outs = 0;
    emptyBases();
  }
}

/* Handles base running and adjusting the score */
void advanceBases(int bases)
{

  strikes = 0; //any time there is a hit, reset strikes

  //otherwise, advance runners
  switch (bases)
  {
    case 4:
      {
        //home run
        int scoreToAdd = 1;
        if (firstBase)
        {
          scoreToAdd++;
        }
        if (secondBase)
        {
          scoreToAdd++;
        }
        if (thirdBase)
        {
          scoreToAdd++;
        }
        addScore(scoreToAdd);
        emptyBases();

      }
      break;
    case 1: //single;
      if (basesLoaded()) //only play someone scores with a single is if the bases are loaded
      {
        addScore(1); //bases stay loaded, just add one
      }
      else if (!firstBase) //if no one is on first, then just get a guy on first and leave other bases alone
      {
        firstBase = true;
      }
      else if (firstBase && !secondBase) //force to second, leave third base as-is
      {
        firstBase = true;
        secondBase = true;
      }
      else if (firstBase && secondBase) //bases arent loaded, so third base is empty, load the bases
      {
        firstBase = true;
        secondBase = true;
        thirdBase = true;
      }
      break;
    case 2: //double
      if (basesLoaded())
      {
        addScore(2); //2 runs score, 1st base left empty
        firstBase = false;
      }
      else if (firstBase && secondBase)
      {
        addScore(1);
        firstBase = false;
        secondBase = true;
        thirdBase = true;
      }
      else if (secondBase && thirdBase)
      {
        addScore(1);
        firstBase = false;
        secondBase = true;
        thirdBase = true;
      }
      else if (firstBase && thirdBase)
      {
        addScore(1);
        firstBase = false;
        secondBase = true;
        thirdBase = true;
      }
      else if (thirdBase && !secondBase && !firstBase)
      {
        firstBase = false;
        secondBase = true;
        thirdBase = true;
      }
      else if (!thirdBase && secondBase && !firstBase)
      {
        firstBase = false;
        secondBase = true;
        thirdBase = true;
      }
      else if (!thirdBase && !secondBase && firstBase)
      {
        firstBase = false;
        secondBase = true;
        thirdBase = true;
      }
      break;
    case 3: //triple
      if (basesLoaded())
      {
        addScore(3); //3 runs score, 1st & second base left empty
      }
      else
      {
        int scoreToAdd = 0; //same as home run except batter doesn't score
        if (firstBase)
        {
          scoreToAdd++;
        }
        if (secondBase)
        {
          scoreToAdd++;
        }
        if (thirdBase)
        {
          scoreToAdd++;
        }
        addScore(scoreToAdd);

      }
      //for any triple, bases are clear except for third

      firstBase = false;
      secondBase = false;
      thirdBase = true;
      break;

  }

}

/* Helper method to determine if the bases are loaded. */
boolean basesLoaded()
{
  return (firstBase && secondBase && thirdBase);
}


/* Add score based on who is at bat. */
void addScore(int scoreToAdd)
{
  if (atBat)
  {
    homeScore += scoreToAdd;
  }
  else
  {
    visitorScore += scoreToAdd;
  }
}

/* Show the welcome screen and flash the lights. */
void resetGame() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("\ Play Baseball!");

  resetLights();

  lcd.setCursor(0, 1);
  lcd.print("\ \ Push Button");
}

/* Flash the LED lights. */
void resetLights() {
  for (int x = 7; x < 11; x++)
  {
    digitalWrite(x, HIGH);
  }
  delay(1000);
  for (int x = 7; x < 11; x++)
  {
    digitalWrite(x, LOW);
  }
}

/* Show the score with an indicator for who is at bat. */
void showScore() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Visitor: ");
  lcd.setCursor(11, 0);
  lcd.print(visitorScore);

  lcd.setCursor(5, 1);
  lcd.print("Home: ");
  lcd.setCursor(11, 1);
  lcd.print(homeScore);

  if (atBat)
  {
    lcd.setCursor(0, 1);
  }
  else
  {
    lcd.setCursor(0, 0);
  }
  lcd.print("*");
  delay(2000);
}


/* Render the status of the game (Strikes, Balls, outs, inning, and status of the bases */
void showStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("S:");
  lcd.setCursor(2, 0);
  lcd.print(strikes);
  lcd.setCursor(4, 0);
  lcd.print("B:");
  lcd.setCursor(6, 0);
  lcd.print(balls);
  lcd.setCursor(8, 0);
  lcd.print("O:");
  lcd.setCursor(10, 0);
  lcd.print(outs);
  lcd.setCursor(12, 0);
  lcd.print("I:");
  lcd.setCursor(14, 0);
  lcd.print(inning);

  lcd.setCursor(0, 1);
  if (basesLoaded())
  {
    lcd.print("Based are loaded.");
  }

  if (firstBase)
  {
    lcd.setCursor(0, 1);
    lcd.print("1ST");
  }
  if (secondBase)
  {
    lcd.setCursor(4, 1);
    lcd.print("2ND");
  }
  if (thirdBase)
  {
    lcd.setCursor(8, 1);
    lcd.print("3RD");
  }

  delay(3000);

}

/* Clear all the bases. */
void emptyBases() {
  firstBase = false;
  secondBase = false;
  thirdBase = false;
}

