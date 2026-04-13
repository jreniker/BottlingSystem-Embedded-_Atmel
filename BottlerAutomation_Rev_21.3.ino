/* 
 *  Bottler automation
 * -------------------- 
 *
 *  The purpose of this code is to automate a bottling process.  When manually performed, an operator will:
 *  
 *  Place bottles under nozzles.  Positioning is typically aided by the use of a "gate" which is a stop at the end of bottling row.
 *  Lower nozzles by toggling a switch.
 *  Start pump by toggling a switch.  Bottler uses overflow type filling nozzles which ensure a consistent fill level.  Gate can be opened while bottles are filling 
 *  Stop pump by toggling a swtich when all bottles full
 *  Raise nozzles by toggling a switch
 *  Move filled bottles out of the way (i.e. clear the bottles) by pushing them further down the filling track
 *  Close gate
 *  
 *  Automating the above process is feasible using either an array of sensors to validate each step (i.e. sensing bottles, fill level, nozzle position)
 *  This method of automation requires a complex array of sensors as well as associated coding.
 *  An alternative method of automation involves timing various steps.  This is not as precises and each filling cycle will require more time than the above due to
 *  a necessary buffer time to ensure each step is allowed to complete.  The latter technique is used to avoid expensive industrial automation tools and skills.
 *  For this system potentiometers are used to adjust various times which are then used as "waits" within the code between various operations.  Relays are used to 
 *  switch power for various operations such as raise nozzles, turn on pump, etc.  In the end, the system is controlled by turning on outlets for periods of time.
 *  
 *  Arduino code (also known as a sketch) consists of three sections
 *     Variable declarations.  Variables must be setup before they can be used.
 *     Arduino setup.  Arduino connections, ports, and pins have to be setup is input, output, etc.
 *     The code loop.  This is the logic that will be executed.
 *  
 *  This code is downloaded to the Arduino UNO R3 via the Arduino IDE which is available at:     https://www.arduino.cc/
 *  To perform the download of code:
 *    Obtain the IDE and install onto a PC or Mac
 *    Connect the Arduino board to your computer using a USB cable.  Confirm the board is properly connected by selecting "Tools" from the menu bar, then "Get Board Info"
 *    Open the file containing the code or type into the Arduino IDE
 *    Load the code into the Arduino by selecting the arrow in a circle pointing to the right which is located in the top-left corner of the window.  
 *_________________________________________________________________________________________________________________________________________________________________________
 *
 *   "Cycle Time" is the amount of time it takes the nozzles to lower and seat.  This is controllable via the exhaust/speed controls on the air solenoid.  Adjust 
 *   potentiometer to match the physical operation of the machine.  
 *   
 *   "Pump Time" is the time the pump is on.  The overflow nozzles should control the fill level within the bottle, but there is variability due to product consistency,
 *   turbulance within the bottle, pump quality, etc.  This should be adjusted so that the operator can see product coming out of the return port on every nozzle.
 *   
 *   "Deflate Time" is the amount of time the bottles relax after the pump is turned off, but before the nozzles are raised.  1.0sec appears to deliver good accuracy.
 *   Too long and product siphons from the nozzle via the return lines.  Too short and the bottles don't relax and normalize.  Find the best compromise for your product
 *   and bottle size.  Larger bottles will likely require more Deflate Time, while smaller ones will need less.  Again, 1.0sec appears to deliver good accuracy compromise
 *   for a variety of bottles but is tunable to allow greater precision to the operator.
 *_________________________________________________________________________________________________________________________________________________________________________  
 *
 *   This code was developed by, and is sole property of, Josh Reniker.  
 *   Initial version October 3, 2019
 *   Rewrite developed by Josh Reniker February 19, 2020.
 *   Rewrite #2 developed by Josh Reniker March 1, 2020
 *   Cleaning cycle added 4/10/2020
 *   Cleaning cycle rewritten 5/5/2020
 *   Cleaning cycle rewritten and press button added 4/13/2021
 *   Timing revisions Feb 2022 and again in May 2022
 *   Added LED's August 2022
 *   Revise gate logic August 2022
 *   
 *   Revision 21.3   <<<---THIS IS A VERY STABLE CODE VERSION!
 *   
 *   Code revision displayed in boot splash
 *_________________________________________________________________________________________________________________________________________________________________________
 *    
*/
 
/*
 * Here is a sample for Serial Monitor troubleshooting syntax
 * Serial.println(duration);
 * Serial.println("     ");
*/

/*
 *  First some variable declarations.  In most cases the value assigned to the variable indicates the pin on the Arduino.  Values such as "INPUT" "OUTPUT" "HIGH" and "LOW"
 *  are meaningful to the Arduino.
*/

// We have to load some libraries to use the fancy serial LCD.  Do this first...
//#include <SoftwareSerial.h>
//#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

// Analog declarations.  This defines the wiring diagram!!!
int CyclePotPin = 0;
int PumpPotPin  = 1;
int DeflatePotPin = 2;

// Digital declarations.  This defines the wiring diagram!!!
int StartAutoSw = 2;
int GateTogSw = 4;
int CleanSw = 7;
int Relay1Nozzles = 8;
int Relay2Pump = 12;
int Relay3Gate = 13;
int StartLED = 6;
int GateLED = 5;
int CleanLED = 10;



// Floating point vals for the wait times - give precision to the adjustments.  Approx 40s=1gal, 6.5s=16oz
float CycleTime = 3.0;
float PumpTime = 40.0;    
float DeflateTime = 1.0;

// Variables to save data one time.
float CyclePotVal = 0;         // variable to store the value coming from the Cycle time potentiometer
float PumpPotVal = 0;          // variable to store the value coming from the Pump time potentiometer
float DeflatePotVal = 0;       // variable to store the value coming from the Deflation time potentiometer

// Variables to save data in a reusable manner
int COUNTER = 0;             // variable for counting bottles
int BOTTLES = 0;
int SIC = 0;
int STATE = 0; 
int STATE2 = 0;
int GCC = 0;
int GCSTATE = 0;
int GCSTATE2 = 0;
int GTRACK = 0;
int CLEAN = 0;
int CSTATE = 0;
int CSTATE2 = 0;
int CLEANIT = 0;

void setup() {
  // Setup pins and variables
  pinMode(CyclePotPin, INPUT);
  pinMode(PumpPotPin, INPUT);
  pinMode(DeflatePotPin, INPUT);
  pinMode(StartAutoSw, INPUT_PULLUP);
  pinMode(CleanSw, INPUT_PULLUP);
  pinMode(GateTogSw, INPUT_PULLUP);
  pinMode(Relay1Nozzles, OUTPUT);
  pinMode(Relay2Pump, OUTPUT);
  pinMode(Relay3Gate, OUTPUT);
  pinMode(StartLED,OUTPUT);
  pinMode(GateLED,OUTPUT);
  pinMode(CleanLED,OUTPUT);
  
  // Setup serial communication specs
  Serial.begin(9600);
  
  // Setup the display 
  lcd.init();  //initialize the lcd
  lcd.backlight();                   //open the backlight 
  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print("                    "); // write this string on the top row
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("       Bottler      "); // pad string with spaces for centering
  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("                    "); // pad with spaces for centering
  lcd.setCursor ( 0, 3 );            // go to the fourth row
  lcd.print("Rev. 21.3           ");
  delay(100);

  // Nozzles UP, Pump OFF, and Gate OPEN.
  digitalWrite(StartLED, HIGH);
  delay(100);
  digitalWrite(GateLED, HIGH);
  delay(100);
  digitalWrite(CleanLED, HIGH);
  delay(100);
  digitalWrite(Relay1Nozzles, HIGH);
  delay(100);
  digitalWrite(Relay2Pump, LOW);
  delay(100);
  digitalWrite(Relay3Gate, LOW);
  delay(100);
  digitalWrite(StartLED, LOW);
  digitalWrite(GateLED, LOW);
  digitalWrite(CleanLED, LOW);    
  lcd.init();
  delay(2);
  
}


void loop() {
  // Enter the main loop of the program.  There should never be an exit from this loop unless there is a crash or electronically induced reset due to noise, EMI, inductance, etc.
  while(1){
    delay(2);
    // Use "break;" to exit this loop
    CyclePotVal = analogRead(CyclePotPin);                   // Read potentiometer and map to ~5 seconds
    CycleTime = CyclePotVal / 340.0;
    CycleTime = 2.0 + round(CycleTime*2.0)/2.0;                // This limits precision to one decimal place (i.e. 0.1 second)
    //
    PumpPotVal = analogRead(PumpPotPin);                     // Read potentiometer and map to ~60 seconds
    PumpTime = PumpPotVal / 18.1;
    PumpTime = 3.5 + round(PumpTime*2.0)/2.0;                  // This limits precision to one decimal place (i.e. 0.1 second)      
    //
    DeflatePotVal = analogRead(DeflatePotPin);               // Read potentiometer and map to ~2 seconds
    DeflateTime = DeflatePotVal / 146.0;
    DeflateTime = 1.0 + round(DeflateTime*5.0)/5.0;          // This limits precision to one decimal place (i.e. 0.2 second)

    // Write out potentiometer/time data
    lcd.setCursor ( 0, 0 );                // Top row, left aligned
    lcd.print("Cycle time:    ");            // Print at cursor position
    lcd.print(CycleTime);

    lcd.setCursor ( 0, 1 );                // Second row, left aligned
    lcd.print("Pump times:    ");            // Print at cursor position
    lcd.print(PumpTime);

    lcd.setCursor ( 0, 2 );                // Third row, left aligned
    lcd.print("Deflate time:  ");            // Print at cursor position 
    lcd.print(DeflateTime);

    lcd.setCursor ( 0, 3 );                // Fourth row, left aligned
    lcd.print("Cycles: ");                 // Print at cursor position 
    lcd.print(COUNTER);
    lcd.print(" / ");
    lcd.print(BOTTLES);

//  Next, check for button presses. There are three buttons; "Cycle" which initiates the filling cycle, "Gate" which open/closes the gate, and "Clean" to initiate cleaning cycle

      //  Let's check for a cleaning cycle.  This cycle is initiated by pressing the red button. 
      CSTATE = digitalRead(CleanSw);
      if (CSTATE == LOW) {
         delay(5);
         CSTATE2 = digitalRead(CleanSw);
            if (CSTATE2 == LOW) {
              CLEANIT = 1;
              CLEAN = 30;
              digitalWrite(CleanLED, HIGH);
              delay(5);              
              lcd.init();
              delay(2);
            }
      }

    // If the Gate Toggle button is pushed, open/close the gate...
    GCC=1;
    GCSTATE = digitalRead(GateTogSw); 
       if (GCSTATE == LOW) {
         delay(5);
         GCSTATE2 = digitalRead(GateTogSw);
         if (GCSTATE2 == LOW) {
           GCC=0;
           digitalWrite(GateLED, HIGH);
           delay(2);
           lcd.init();
           delay(2);
         }
       }

    // If the Automation Start button is pushed, then start...
    SIC=1;
    STATE = digitalRead(StartAutoSw); 
       if (STATE == LOW) {
         delay(5);
         STATE2 = digitalRead(StartAutoSw);
         if (STATE2 == LOW) {
           SIC=0;
           digitalWrite(StartLED, HIGH);
           delay(2);
           lcd.init();
           delay(2);
         }
       }

//  Now perform the actions dictated by the buttons ##################################################

    // Toggle the gate if needed.
    if (GCC == 0) {
      GTRACK=digitalRead(Relay3Gate);
      delay(2);
      if (GTRACK == LOW)
         {
         digitalWrite(Relay3Gate, HIGH);
         }
      if (GTRACK == HIGH) 
         {
         digitalWrite(Relay3Gate, LOW);
         }
      lcd.init();  //initialize the lcd
      delay(2); 
      digitalWrite(GateLED, LOW);    
      delay(2);
      }
    
      // Start the cleaning cycle if that button was pressed      
      if (CLEANIT == 1) { 
             GCC=1; // Don't toggle the gate!
             // Lower nozzles
             digitalWrite(Relay1Nozzles, LOW);
             // Cycle pump for 2 minute
               while (CLEAN > 0) {
                 // Update screen
                 lcd.init();
                 lcd.setCursor ( 0, 0 );                  // Second row, left aligned
                 lcd.print("Cleaning...      ");            // Print at cursor position
                 lcd.setCursor ( 0, 1 );
                 lcd.print("Adjustable 1-5sec");            // Print at cursor position
                 CyclePotVal = analogRead(CyclePotPin);                   // Read potentiometer and map to ~5 seconds
                 CycleTime = CyclePotVal / 250.0;
                 CycleTime = 1.0 + round(CycleTime*2.0)/2.0;                // This limits precision to one decimal place (i.e. 0.1 second) 
                 lcd.setCursor ( 0, 2 );
                 lcd.print("Clean time:    ");            // Print at cursor position
                 lcd.print(CycleTime);
                 lcd.setCursor ( 0, 3 );                  // Second row, left aligned
                 lcd.print(CLEAN);                    // Print at cursor position 
                 delay(CycleTime*1000);                      // Wait the Cycle time                  
                 digitalWrite(Relay2Pump, HIGH);
                 delay(2000);
                 digitalWrite(Relay2Pump, LOW);
                 CLEAN=CLEAN - 1;
                 digitalWrite(GateLED, LOW);
              }
             // Wait for the bottles to deflate  
             delay(DeflateTime*1000);   
             // Raise nozzles
             digitalWrite(Relay1Nozzles, HIGH);
            lcd.init();
            delay(5);
            CLEANIT = 0;
           }  


    // If the start button was pressed, then let's fill some bottles!!!
    if (SIC==0) {
      // Clear the screen
      lcd.init();
      delay(5);
      
      // Lower Nozzles, update status on LCD, then wait for the cycle time to complete
      digitalWrite(Relay1Nozzles, LOW);
      lcd.setCursor ( 0, 0 );                     // First row, left aligned
      lcd.print("Lowering nozzles    ");          // Print at cursor position
      delay(CycleTime*1000);                      // Wait the Cycle time
      lcd.setCursor ( 0, 1 );                     // Second row, left aligned
      lcd.print("Nozzles lowered     ");          // Print at cursor position
            
      // Start pump, open gate, update status on LCD, then wait to fill
      lcd.setCursor ( 0, 2 );                     // Third row, left aligned
      lcd.print("Gate Opening        ");          
      digitalWrite(Relay3Gate, LOW);
      delay(5);
      lcd.setCursor ( 0, 3 );                     // Fourth row, left aligned
      lcd.print("Pump on  -  FILLING ");    
      digitalWrite(Relay2Pump, HIGH);
      delay(PumpTime*1000);

      // Clear the screen...
      lcd.init();
      delay(2);

      // Update status on LCD and stop the pump
      lcd.setCursor ( 0, 0 );                     // First row, left aligned
      lcd.print("Pump off, deflating ");
      digitalWrite(Relay2Pump, LOW);
      delay(DeflateTime*1000);   
              
      // Raise nozzles, update status on LCD, then wait for the cycle time to complete
      lcd.setCursor ( 0, 1 );                     // First row, left aligned
      lcd.print("Raising nozzles     ");          // Print at cursor position
      digitalWrite(StartLED, LOW);
      delay(10);
      digitalWrite(Relay1Nozzles, HIGH);
      delay(CycleTime*330);                      // Wait 1/3 Cycle time       
      lcd.setCursor ( 0, 2 );                     // Third row, left aligned
      lcd.print("Clear bottles       ");          // Print at cursor position
      delay(CycleTime*660);                      // Wait 2/3 Cycle time    

      // Clean up a few things just to be sure
      SIC = 0;
      STATE = 0; 
      STATE2 = 0;
      GCC = 0;
      GCSTATE = 0;
      GCSTATE2 = 0;

      // Track the work.
      COUNTER = COUNTER + 1;
      BOTTLES = COUNTER * 6;
      lcd.init();
      delay(5);
      break;
    }
    
  }
  
}
