//#include <LiquidCrystal.h>   //basic arduino library for LCD display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"  //refers to the second tab pitch definitions used in melody generator
LiquidCrystal_I2C lcd(0x27, 16, 2);
//configures wiring for LCD
int redlight = 12;        //pin assignment for red LED
int blackbutton = 11;     //pin assignment for black (select) button
int yellowbutton = 10;    //pin assignment for yellow (enter) button
int foiltouch = 13;       //pin assignment for aluminum foil that, when touched by tweezers, registers a failed surgery
int foilstate = 0;        //variable that registers whether tweezers are currently touching foil.  1 is not touching, 0 is touching
int holster = 2;          //pin assignment for the metal holster for the tweezers, breaking this connection initiates timer for surgeries
int holsterdone = 6;     //pin assignment that turns analog pin 5 into a digital pin for reading a completed surgery
int holsterstate = 1;     //variable that registers whether tweezers are currently touching holster. 1 is not touching, 0 is touching
int blackstate = 1;       //variable that shows whether black button is pressed.  1 is not, 0 is pressed
int yellowstate = 1;      //variable that shows whether yellow button is pressed.  1 is not, 0 is pressed
int playernumber = 1;     //variable that registers whether a one or two player game has been selected by user
int winningscore = 1000;  //variable that determines how many $ must be earned to win game
int onescore = 0;         //variable that holds the amount of $ earned by player one
int twoscore = 0;         //variable that holds the amount of $ earned by player two
long firstdoc;            //variable used in the random generator that determines the doctor name assigned to player one
long seconddoc;           //variable used in the random generator that determines the doctor name assigned to player two
int playerturn;           //variable used to determine which player's turn is next
String firstname;         //variable that holds the randomly generated doctor name for player one
String secondname;        //variable that holds the randomly generated doctor name for player two
long randomsurgery;       //variable used in the random number generator used to determine which surgery is next
String surgery;
int procedurecost;    //variable that holds the amount of $ randomly generated for a procedure
int tweezerstate;     //variable that is used to determine whether tweezers are touching the holster
int timer;            //holds the amount of seconds left during a surgery
int t;                //used to govern a loop in the timer function
int x;                //used to govern a loop in the timer function
int payment;          //the payment for the surgery adjusted for how many seconds remain on the timer
int success = 0;      //registers whether the player was successful in surgery in order to update their score
int countertone;      //used in countdown sequence during surgery to advance musical tone up the scale during surgery
int randomlefty = 0;  //used in the random generator for bonus lefty surgery
int lefty = 0;        //registers whether the surgery is a bonus lefty surgery worth extra
int leftyfreq = 2;

void setup() {
  pinMode(redlight, OUTPUT);  //pin configurations
  pinMode(blackbutton, INPUT);
  pinMode(yellowbutton, INPUT);
  pinMode(foiltouch, INPUT);
  pinMode(holster, INPUT);
  pinMode(holsterdone, INPUT);
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  pinMode(14, INPUT);
  Serial.begin(9600);
}

void loop() {
  lighton();
  welcome();
  welcome_melody();
  lightoff();
  delay(2000);
  playerselect();
  delay(2000);
  scoreset();
  delay(2000);
  leftyrarity();
  delay(2000);
  nameassign();
  delay(4000);
  mainloop();
  gameover();
  delay(200);
}

void inicializarPantalla(int espaciadoInicial, int fila) {
  Wire.begin();
  lcd.backlight();
  lcd.begin(16, 2);
  lcd.setCursor(espaciadoInicial, fila);
}

void limpiarLcdYSetearCursor(int espaciadoInicial, int fila) {
  lcd.clear();
  lcd.setCursor(espaciadoInicial, fila);
}


void mainloop() {  //MAINLOOP includes functions to: show which player's turn, select and display a surgery and $ value,
                   //detect the tweezers lifting, time the surgery, determine points earned by player, display running score total
  playerturn = 1;
  onescore = 0;
  twoscore = 0;
  while ((onescore < winningscore) && (twoscore < winningscore)) {
    showturn();
    delay(3000);

    randomoperation();
    delay(4000);
    leftybonus();
    tweezerready();
    delay(200);
    timedsurgery();
    delay(2000);

    scoredisplay();
    delay(4000);

    if ((playernumber == 2) && (playerturn == 1)) {
      playerturn = 2;
    } else {
      playerturn = 1;
    }
  }

  Wire.begin();
  lcd.backlight();
  lcd.begin(16, 2);
}

void welcome() {  //brief splash opening screen with name of game
  inicializarPantalla(2, 0);

  lcd.print("Bienvenido a");
  lcd.setCursor(1, 1);
  lcd.print("Opera a Bender!");
}

void welcome_melody() {  //theme music that plays during splash screen
  int melody[] = {
    NOTE_D4, NOTE_CS4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_FS4, NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_D4, NOTE_E4, NOTE_A4  //melody can be altered here calling different pitches defined in tab 2 (pitches.h)
  };
  int noteDurations[] = {
    8, 8, 4, 8, 8, 4, 8, 8, 4, 8, 8, 4, 8, 8, 8, 8, 1
  };
  for (int thisNote = 0; thisNote < 17; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(3, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(3);
  }
}

void success_melody() {  //theme music that plays after successful surgery
  int melody[] = {
    NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, 0, NOTE_C4, NOTE_F4, 0,  //melody can be altered here calling different pitches defined in tab 2 (pitches.h)
  };
  int noteDurations[] = {
    8, 8, 8, 4, 16, 8, 2, 8
  };
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(3, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(3);
  }
}

void failure_melody() {  //theme music that plays after failed surgery
  int melody[] = {
    NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4,  //melody can be altered here calling different pitches defined in tab 2 (pitches.h)
  };
  int noteDurations[] = {
    4, 4, 4, 4, 1
  };
  for (int thisNote = 0; thisNote < 5; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(3, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(3);
  }
}

void lighton() {  //simple function to turn red LED on
  digitalWrite(redlight, HIGH);
}

void lightoff() {  //simple function to turn red LED off
  digitalWrite(redlight, LOW);
}

void playerselect() {  //procedure for choosing a one player or two player game using black button to change selection and yellow button to enter selection
  yellowstate = 1;
  blackstate = 1;
  limpiarLcdYSetearCursor(0, 0);

  lcd.print("Elige cantidad");
  lcd.setCursor(0, 1);
  lcd.print("de Jugadores");
  delay(3000);
  limpiarLcdYSetearCursor(0, 0);
  lcd.print("1 Jugador");
  int toggled = 0;  //toggle variable keeps track of how many times the black button has been pushed, determining number of players selected
  while (yellowstate == 1) {
    lightoff();
    yellowstate = digitalRead(yellowbutton);
    blackstate = digitalRead(blackbutton);
    if ((blackstate == 0) && (toggled == 0)) {
      toggled = 1;
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print("2 Jugadores");
      delay(300);
      blackstate = 1;
    }
    if ((blackstate == 0) && (toggled == 1)) {
      toggled = 0;
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print("1 Jugador");
      delay(300);
      blackstate = 1;
    }
  }

  lighton();
  delay(400);
  limpiarLcdYSetearCursor(2, 0);

  if (toggled == 0) {
    lcd.print("Cantidad de");
    lcd.setCursor(1, 1);
    lcd.print(" Jugadores: 1");
    playernumber = 1;
  } else {
    lcd.print("Cantidad de");
    lcd.setCursor(1, 1);
    lcd.print(" Jugadores: 2");
    playernumber = 2;
  }
  yellowstate = 1;
  lightoff();
}

void scoreset() {  //procedure for choosing the number of dollars needed to win the game
  yellowstate = 1;
  blackstate = 1;
  winningscore = 1000;
  limpiarLcdYSetearCursor(0, 0);

  lcd.print("Objetivo: $1000");
  lcd.setCursor(0, 1);
  lcd.print("Elige o cambia");
  int toggled = 0;  //toggle variable keeps track of how many times the black button has been pushed, determining cash value selected

  while (yellowstate == 1) {
    lightoff();
    yellowstate = digitalRead(yellowbutton);
    blackstate = digitalRead(blackbutton);
    if ((blackstate == 0) && (toggled == 0)) {
      toggled = 1;
      lcd.setCursor(0, 0);
      lcd.print("Objetivo: $1500 ");
      delay(300);
      blackstate = 1;
    }
    if ((blackstate == 0) && (toggled == 1)) {
      toggled = 2;
      lcd.setCursor(0, 0);
      lcd.print("Objetivo: $2000 ");
      delay(300);
      blackstate = 1;
    }
    if ((blackstate == 0) && (toggled == 2)) {
      toggled = 0;
      lcd.setCursor(0, 0);
      lcd.print("Objetivo: $1000   ");
      delay(300);
      blackstate = 1;
    }
  }

  lighton();
  delay(400);
  limpiarLcdYSetearCursor(0, 0);

  if (toggled == 0) {
    lcd.print("Necesitas $1000");
    lcd.setCursor(0, 1);
    lcd.print("Para ganar");
    winningscore = 1000;
  }

  if (toggled == 1) {
    lcd.print("Necesitas $1500");
    lcd.setCursor(0, 1);
    lcd.print("Para ganar");
    winningscore = 1500;
  }
  if (toggled == 2) {
    lcd.print("Necesitas $2000");
    lcd.setCursor(0, 1);
    lcd.print("Para ganar");
    winningscore = 2000;
  }
  yellowstate = 1;
  lightoff();
}

void leftyrarity() {  //procedure for choosing how frequently the 'lefty bonus' surgeries occur
  yellowstate = 1;
  blackstate = 1;
  leftyfreq = 2;
  limpiarLcdYSetearCursor(0, 0);


  lcd.print("Izq.: Normal");
  lcd.setCursor(0, 1);
  lcd.print("Elige o cambia");
  int toggled = 0;  //toggle variable keeps track of how many times the black button has been pushed, determining lefty frequency

  while (yellowstate == 1) {
    lightoff();
    yellowstate = digitalRead(yellowbutton);
    blackstate = digitalRead(blackbutton);
    if ((blackstate == 0) && (toggled == 0)) {
      toggled = 1;
      limpiarLcdYSetearCursor(2, 0);
      lcd.print("Izq.: Poco");
      lcd.setCursor(0, 1);
      lcd.print("Elige o cambia");
      delay(300);
      blackstate = 1;
    }
    if ((blackstate == 0) && (toggled == 1)) {
      toggled = 2;
      limpiarLcdYSetearCursor(2, 0);
      lcd.print("Izq.: Off");
      lcd.setCursor(0, 1);
      lcd.print("Elige o cambia");
      delay(300);
      blackstate = 1;
    }
    if ((blackstate == 0) && (toggled == 2)) {
      toggled = 0;
      limpiarLcdYSetearCursor(2, 0);
      lcd.print("Izq.: Normal");
      lcd.setCursor(0, 1);
      lcd.print("Elige o cambia");
      delay(300);
      blackstate = 1;
    }
  }

  lighton();
  delay(400);
  limpiarLcdYSetearCursor(0, 0);
  ;

  if (toggled == 0) {
    lcd.print(" Bonus con Izq.:");
    lcd.setCursor(0, 1);
    lcd.print("  Normal (50%)  ");
    leftyfreq = 2;
  }

  if (toggled == 1) {
    lcd.print(" Bonus con Izq.:");
    lcd.setCursor(0, 1);
    lcd.print("   Poco (33%)   ");
    leftyfreq = 3;
  }
  if (toggled == 2) {
    lcd.print(" Bonus con Izq.:");
    lcd.setCursor(0, 1);
    lcd.print("   Off (0%)   ");
    leftyfreq = 0;
  }
  yellowstate = 1;
  lightoff();
}
void setNameFirstPlayer(String name) {
  lcd.setCursor(0, 0);
  firstname = name;
  lcd.print("1: " + firstname);
}

void setNameSecondPlayer(String name) {
  lcd.setCursor(0, 1);
  secondname = name;
  lcd.print("2: " + secondname);
}

void nameassign() {  //randomly generates doctor names for each player from a list of seven possibilities.
  firstdoc = 0;
  seconddoc = 0;
  while (firstdoc == seconddoc) {
    randomSeed(analogRead(0));              //seeds the random number with the unpredictable noise on unconnected pin A0; seems to help with true randomness
    seconddoc = (random(1)) + (random(2));  //another attempt to achieve greater randomness- arduino initiates the exact same 'random' list of numbers, so the first
    firstdoc = random(4);                   //random number generated on startup always seems to be the same
  }
  limpiarLcdYSetearCursor(0, 0);

  switch (firstdoc) {
    case 0:
      setNameFirstPlayer("Fry");
      break;
    case 1:
      setNameFirstPlayer("Leela");
      break;
    case 2:
      setNameFirstPlayer("Dr. Zoidberg");
      break;
    case 3:
      setNameFirstPlayer("Prof. Hubert");
      break;
  }

  if (playernumber == 2) {
    switch (seconddoc) {
      case 0:
        setNameSecondPlayer("Fry");
        break;
      case 1:
        setNameSecondPlayer("Leela");
        break;
      case 2:
        setNameSecondPlayer("Dr. Zoidberg");
        break;
      case 3:
        setNameSecondPlayer("Prof. Hubert");
        break;
    }
  }
}

void aPrepararse(String nombre) {
  limpiarLcdYSetearCursor(0, 0);
  lcd.print(nombre + ", ");
  lcd.setCursor(0, 1);
  lcd.print("A prepararse!");
}

void showturn() {  //part of the main game loop- it shows the doctor name of whose turn is next.
  if (playerturn == 1) {
    aPrepararse(firstname);
  }
  if (playerturn == 2) {
    aPrepararse(secondname);
  }
}

void setItem(String itemName) {
  limpiarLcdYSetearCursor(0, 0);
  lcd.print(itemName);
  randomSeed(analogRead(0));
  procedurecost = (((random(3) + 1)) * 100) + (((random(9) + 1)) * 10);
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("Hasta $");
  lcd.setCursor(8, 1);
  lcd.print(procedurecost);
}

void randomoperation() {      //part of the main game loop. randomly chooses a procedure and a maximum $ value to be earned, which decreases as time elapses
  randomSeed(analogRead(0));  //the possible dollar ranges of each procedure can be set independently, if game board construction results in some surgeries being more
  randomsurgery = random(5);  //difficult than others
  switch (randomsurgery) {
    case 0:
      surgery = "Cerveza";
      break;
    case 1:
      surgery = "Tuerca";
      break;
    case 2:
      surgery = "Pistola";
      break;
    case 3:
      surgery = "Dinero";
      break;
    case 4:
      surgery = "Habano";
      break;
  }
  setItem(surgery);
}

void tweezerready() {                   //checks whether tweezers are in the holster, and if not, displays a message to return the tweezers.
  tweezerstate = digitalRead(holster);  //if tweezers are already in the holster or are returned, displays a message to pull tweezers from holster to begin a timed surgery.
  while (tweezerstate != 0) {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("Apoya la pinza");
    lcd.setCursor(0, 1);
    lcd.print("para continuar...");
    tweezerstate = digitalRead(holster);
    delay(300);
  }
  tweezerstate = 0;
  while (tweezerstate == 0) {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print(" Levantar pinza");
    lcd.setCursor(0, 1);
    lcd.print(" para iniciar!");
    tweezerstate = digitalRead(holster);
    delay(300);
  }
  tweezerstate = 1;
}

void leftybonus() {  //this function randomly triggers a 'lefty bonus' surgery, worth an additional $150

  lefty = 0;
  randomlefty = 0;
  randomlefty = random(leftyfreq);
  if (randomlefty == 1) {
    lefty = 1;
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("Bonus mano izq.");
    lcd.setCursor(0, 1);
    lcd.print("Usar otra mano!");
    delay(3000);
  }
}

void timedsurgery() {  //this function displays the current surgery on the top half of the LCD during surgery
  countertone = 80;
  timer = 19;  //this countdown timer gives 20 seconds for surgery and also polls both the foil bed and holster every microsecond to see if the tweezers touch

  tweezerstate = 1;
  foilstate = 1;
  delay(1000);
  tweezerstate = 1;
  foilstate = 1;
  for (t = 0; t < 19; t++) {
    while ((tweezerstate == 1) && (foilstate == 1) && (timer > -1)) {
      for (x = 0; x < 1000; x++) {
        tweezerstate = digitalRead(holsterdone);
        foilstate = digitalRead(foiltouch);
        delay(1);
        if ((tweezerstate == 0) || (foilstate == 0)) {
          lighton();
          break;
        }
      }
      limpiarLcdYSetearCursor(2, 0);
      lcd.print(surgery);
      lcd.setCursor(7, 1);
      lcd.print(timer);
      Serial.println("Aca llego");
      tone(3, (countertone), 1000);
      timer = timer - 1;
      countertone = countertone + 45;
      if (timer < 1) {
        break;
      }
    }
  }

  Serial.println("Aca llego2");
  if (tweezerstate == 0) {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("  Fue todo,");
    lcd.setCursor(0, 1);
    lcd.print("  un exito!");
    success = 1;
    success_melody();
    delay(3000);
    limpiarLcdYSetearCursor(0, 0);
    lcd.print("Devolver objeto");
    lcd.setCursor(0, 1);
    lcd.print("  al contenedor");
    delay(4000);

  } else {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("    Operacion");
    lcd.setCursor(0, 1);
    lcd.print("    fallida!");
    success = 0;
    failure_melody();
  }
}

void scoredisplay() {
  lightoff();
  payment = procedurecost - ((procedurecost / 20)) * ((18 - timer));
  if (payment < 100) {
    payment = 100;
  }
  if ((playerturn == 1) && (success == 1)) {
    onescore = onescore + payment;
  }
  if ((playerturn == 2) && (success == 1)) {
    twoscore = twoscore + payment;
  }
  if (((playerturn == 1) && (success == 1) && (lefty == 1))) {
    onescore = onescore + 150;
    limpiarLcdYSetearCursor(0, 0);

    lcd.print(" Bonus");
    lcd.setCursor(0, 1);
    lcd.print("  Completado!");
    success_melody();
    delay(3000);
  }
  if (((playerturn == 2) && (success == 1) && (lefty == 1))) {
    twoscore = twoscore + 150;
    limpiarLcdYSetearCursor(0, 0);

    lcd.print(" Bonus");
    lcd.setCursor(0, 1);
    lcd.print(" Completado!");
    success_melody();
    delay(3000);
  }
  if (playernumber == 2) {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("Jugador 1: $");
    lcd.setCursor(12, 0);
    lcd.print(onescore);
    lcd.setCursor(0, 1);
    lcd.print("Jugador 2: $");
    lcd.setCursor(12, 1);
    lcd.print(twoscore);

  } else {
    limpiarLcdYSetearCursor(0, 0);
    lcd.print("Jugador 1: $");
    lcd.setCursor(12, 0);
    lcd.print(onescore);
  }
}

void gameover() {  //when the winning threshold is achieved, show a congratulatory message if a one player game, who the winner was in a two player game,
  limpiarLcdYSetearCursor(0, 0);
  if (playernumber == 2) {
    lcd.print("Felicitaciones!");
    delay(3000);
    limpiarLcdYSetearCursor(0, 0);
    if (onescore > twoscore) {
      lcd.print(firstname+"," );
      lcd.setCursor(0, 1);
      lcd.print(" Ganaste!!");
      success_melody();
    } else {
      lcd.print(secondname+"," );
      lcd.setCursor(0, 1);
      lcd.print(" Ganaste!!");
      success_melody();
    }
  } else {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("Felicitaciones,");
    lcd.setCursor(0, 1);
    lcd.print(" Bender vive!");
    success_melody();
  }
  delay(4000);



  if (playernumber == 2) {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("Jugador 1: $");
    lcd.setCursor(12, 0);
    lcd.print(onescore);
    lcd.setCursor(0, 1);
    lcd.print("Jugador 2: $");
    lcd.setCursor(12, 1);
    lcd.print(twoscore);

  } else {
    limpiarLcdYSetearCursor(0, 0);

    lcd.print("Jugador 1: $");
    lcd.setCursor(12, 0);
    lcd.print(onescore);
  }
  delay(7000);
  yellowstate = 1;
  limpiarLcdYSetearCursor(0, 0);
  while (yellowstate == 1) {
    lcd.print("Enter para");
    lcd.setCursor(0, 1);
    lcd.print(" jugar de nuevo     ");
    yellowstate = digitalRead(yellowbutton);
    delay(50);
  }
  yellowstate = 1;
}
