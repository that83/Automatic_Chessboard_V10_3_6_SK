// ***********************************************
// Original Project by Greg06 visit: https://www.instructables.com/Automated-Chessboard/
// This is adapted for Linear Hall Sensor, instead Reed Switch or Unipolar Hall Sensor
//            AUTOMATIC CHESSBOARD
//    
// ***********************************************
// Search here and on Global the word "Simulide" for find and make changes on the code, if you use on Real or on Simulide
// ******************************  INCLUDING FILES
// Changes from Original
// Pins changed
// Use Arduino Mega (Memory Problems with Nano)
// Use Analog Hall Sensor (Solve White eating Problem)
// Test your own Linear Hall for define the parameters.
// added a Matrix Led 8x8 MAX7219 (For better check the Sensors)
// White Pieces one polarization - Black Pieces Opposite Polarization
// added a H-Bridge L298 Added (For change Electromagnet polarization to move White/Black)
// 
// Version 7.1 Only add PWM to Electromagnet
// Version 9.2.2 December 2023 Only Change MUX for use with Library and Microsteps
// Version 9.2.3 December 2023 Add Serial.print for debug motor part. Change place for Set polarization of Coil
// Version 9.2.4 change Variable definition for step_number
// Version 9.2.5 March 2024 -- Change mux files & columns -- see Sensor_Mux_Test_SK_V9_2_5
// Version 9.2.6  - change record sensor - Eliminate diagonal moves. 
// Sensors only works when the Player Turn's end. (Previous versions works all time)
// Record Sensors Positions only if Move is Valid (Previous versions change the position when Invalid move was made, and game can't continue)
// White Eating situation: 2 sensors change, adapted.
//
// V9.2.7.1 With Diagonal Moves. (Diagonal Factor returns)
// Version 9.2.7.2 
// En Passant Eating & White Castle implemented
//  Black Castle coordinates fixed
//  
// V10 June 2024
// Change Void Motor()
// For new Lay Out Motors
// Change Home. Greg put on E7, I put on H7 for easy test that place
//
// V10_1 August 2024
// Fixed Bishop moves, for c8f5 have to make diagonal movement: AH81 but it makes HA81 so go other place on the board 
//
// V10_3 October 2024 
// Have to mix with V10_2 changes
// Add a Funtion with a Parameter for Fix the distance error if the Trolley not put the piece on the center of the Square
// If that distance error can't fix with the Parameter, then another new function is created for set the position of the pieces by hand on the board after a bad move.
// This is for not start the game again if something go wrong. 
// 
// V10_3_1 
// Add a Button for replace the White Button when want to FIX the board
//  
// V10_3_2 Erase the extra Button for FIX and Use Black Button 
// Delete/reduce some delay() and serial prints for have more velocity on the game
//
// V10_3_4 Try to solve the Fix position Function
//  
// V10_3_5 Sorry, changes commented in Spanish ;)
// Luego que mueva Black, hago el extra movimiento sin verificacion previa (Poner FIXUP_POSITION =0; si no se necesita el movimiento extra) 
// El Movimiento Extra se puede necesitar porque el Electroiman, aunque este centrado debajo del casillero, deje a la pieza unos mm desfasada 
// (Y no pudo corregirse ese desfase, ni con aumentando el PWM del Electroiman, ni acercando el acrilico fisicamente)
// 
// Se agrega la inicializacion de las matrices - by Ethan Khoo (EK) October 2024
// Se agregan 2 Variables en Global (Global V8) para contar el numero de piezas en Micromax y en los Sensores
//
// Have 2 Issues
// Issue 1: Multiple Detections on the First Press (False Positives)
// Issue 2: No Detection on the First Press
// Both Solved by EK
//
// V10_3_6 again comments in spanish
// Cuando la pieza negra va a comer la blanca, el trolley parece:
// It would "forget" to move to where the white piece is at and just start the eating function where it was located.
//
// Detected Limitations not implemented yet ************
// 
// Promotion: Only to Queen  
// Case Game Over (Times up and no ends)
//
// **** for SK microswitch Lay Out ***
#include "HC4067.h" // V9 for easy mannage and understanding of the Mux
#include "global_V8.h"  // October 2024
#include "Micro_Max_V2.h" // V10.3.5 make b[] global for can acces it from here. b[] is the place where micromax save the board
#include <Wire.h>
#include "LedControl.h" // Added for Matrix Led based on MAX7219
#include <LiquidCrystal_I2C.h> // Old Library, replaced for a new one
LiquidCrystal_I2C lcd(0x27,16,2); // Old Library, replaced for a new one
// #include <LiquidCrystal_AIP31068_I2C.h> // V10.3.1 Change the Lib LiquidCrystal_I2C for this one 
// LiquidCrystal_AIP31068_I2C lcd( 0x27, 16, 2 ); // V10.3.1 Change the Lib LiquidCrystal_I2C for this one // See the I2C Adress 0x27, maybe your hardware have other adress
LedControl lc=LedControl(51,52,53,1); // For A Mega DIN=51 , CLK=52, CS=53, 1 Matrix Led

// ****************************************  SETUP
void setup() {
  Serial.begin(9600);
  Serial.println("Automated_Chessboard_V10_3_6_SK.ino Version");

  //  Electromagnet
  	pinMode(IN1, OUTPUT);
  	pinMode(IN2, OUTPUT);	
	pinMode (MAGNET, OUTPUT);

  //  Motor
  pinMode (MOTOR_WHITE_STEP, OUTPUT);
  pinMode (MOTOR_WHITE_DIR, OUTPUT);
  pinMode (MOTOR_BLACK_STEP, OUTPUT);
  pinMode (MOTOR_BLACK_DIR, OUTPUT);

  //  Multiplexer V9 
   pinMode (Mux_Out, INPUT_PULLUP);

// Matrix Led
  lc.shutdown(0,false);   // Turn on matriz Led
  lc.setIntensity(0,4);   // Set Intensity of Led
  lc.clearDisplay(0);     // Turn off leds

    // Initialize the hall sensor memory array to 0 - EK // V10.3.5
    hall_sensor_status_memory[8][8] = 0;
     
    // Initialize the hall sensor array to 0 - EK       // V10.3.5
    hall_sensor_status[8][8] = 0;
    
    // Initialize the hall sensor array to 0 - EK       // V10.3.5
    hall_value[8][8] = 0;

  //  Set the hall initial status
    Read_Sensor (mux1);
    Read_Sensor (mux2);
    Read_Sensor (mux3);
    Read_Sensor (mux4);
    delay(100);
    record_sensors();
    delay(300);
    hall_display();
    delay(300);

   //  MicroMax
  lastH[0] = 0;

  //  LCD
  lcd.init();

  //  Countdown
  timer = millis();

  //  Arcade button - Limit Switch
  pinMode (BUTTON_WHITE_SWITCH_MOTOR_WHITE, INPUT_PULLUP);
  pinMode (BUTTON_BLACK_SWITCH_MOTOR_BLACK, INPUT_PULLUP);
  
  

  lcd_display();
 
}

// *****************************************  LOOP
void loop() {

  switch (sequence) {
    case start:
      lcd_display();
      if (button(WHITE) == true) {  // HvsH Mode
        game_mode = HvsH;
        sequence = player_white;
      }
      else if (button(BLACK) == true) {  // HvsC Mode
        game_mode = HvsC;
        sequence = calibration;
      }
      break;

    case calibration:
      lcd_display();
      calibrate();
      sequence = player_white;
      break;

    case player_white:
      if (millis() - timer > 995) {  // Display the white player clock
        countdown();
        lcd_display();
      }
      
      if (button(WHITE) == true) {  // White player end turn
        detect_movement();
        player_displacement();
        if (game_mode == HvsH) {
          AI_HvsH();  // Chekc is movement is valid
          if (no_valid_move == false)
          {
            //  Set the new status of the hall sensors
            // Grabo valores en la Memoria
            record_sensors();
            delay(300);

              new_turn_countdown = true;
              sequence = player_black;
          } 
          else lcd_display();
        }
        else if(game_mode == HvsC) { 
          AI_HvsC();
		      if (no_valid_move == false){
            //  Set the new status of the hall sensors
            // Grabo valores en la Memoria
            record_sensors();
            delay(300);
            // Muestra los valores
            hall_display();
            delay(300);
            
            sequence = player_black;
          }
          else {
          lcd_display();
          // Fue a not valid move - se corrigio a mano el tablero - puede continuar el juego
          new_turn_countdown = true; // V10.3.5
          sequence = player_black;    // V10.3.5
          }
        }
        break;

      case player_black:
        //  Game mode HvsH
        if (game_mode == HvsH) {  // Display the black player clock
          if (millis() - timer > 995) {
            countdown();
            lcd_display();
          }
          
          if (button(BLACK) == true) {  // Black human player end turn
            detect_movement();
            player_displacement();
            AI_HvsH();  // Chekc is movement is valid
            if (no_valid_move == false)
              {

            //  Set the new status of the hall sensors
            // Grabo valores en la Memoria
            record_sensors();
            delay(300);

             new_turn_countdown = true;
             sequence = player_white;
              }
            else {
              lcd_display();
              // Fue a not valid move - se corrigio a mano el tablero - puede continuar el juego
              new_turn_countdown = true; // V10.3.5
              sequence = player_white;    // V10.3.5
              }
          }
        }
        //  Game mode HvsC
        else if(game_mode == HvsC) { 
          lcd_display();
          black_player_movement();  //  Move the black chess piece
          delay(25);
          detect_movement();
          
          // Verifico si la cantidad de piezas en el tablero es correcta, pues puede ser que un sensor no la haya leido // V10.3.5
          if (Pieces_on_board() == false ){  // Cuento la cantidad de Piezas en el Tablero, si no son las esperadas=false. 
              
                Set_The_Board();                  // Si es false -> solucion manual
            }
          
          //  Set the new status of the hall sensors
          // Grabo valores en la Memoria - Si viene de Set the Board, los vuelve a grabar - Ver de Optimizar
            record_sensors();
            delay(300);
          sequence = player_white;
        }
        break;
      } // Close White Player end turn?
  } // close sequence
} // close loop

// ***************************************  SWITCH
boolean button(byte type) {

  if (type == WHITE && digitalRead(BUTTON_WHITE_SWITCH_MOTOR_WHITE) != HIGH) {
    delay(100);
    return true;
  }
  if (type == BLACK && digitalRead(BUTTON_BLACK_SWITCH_MOTOR_BLACK) != HIGH) {
    delay(100);
    return true;
  }
  return false;
}

// ************************************  CALIBRATE
void calibrate() {

  //  Slow displacements up to touch the limit switches
  while (digitalRead(BUTTON_WHITE_SWITCH_MOTOR_WHITE) == HIGH) motor(A_H, SPEED_SLOW, calibrate_speed);  // H_A RB // A_H SK // If in Lay Out, Switch_white is near H1 --> A_H // If in Lay Out, Switch_white is near A1 --> H_A
  while (digitalRead(BUTTON_BLACK_SWITCH_MOTOR_BLACK) == HIGH) motor(F1_F8, SPEED_SLOW, calibrate_speed); // If in Lay Out, Switch_black is near H8 --> F1_F8 // If in Lay Out, Switch_black is near A8 --> F8_F1
  delay(100);

  //  Rapid displacements up to the Black start position 
  motor(F8_F1, SPEED_FAST, TROLLEY_START_POSITION_X);                                                     // Remember to Swap the previous
  motor(H_A, SPEED_FAST, TROLLEY_START_POSITION_Y);       // A_H RB // H_A SK                             // Remember to Swap the previous
  delay(100);
}

// ****************************************  MOTOR V10 //See Excel file -> https://docs.google.com/spreadsheets/d/1Lql2zc5xnVrvUsg8civ4FzFImBMNn56j/edit?usp=sharing&ouid=111301687926868500189&rtpof=true&sd=true
//
void motor(byte direction, int speed, float distance) { 
// for example: motor(F1_F8, SPEED_FAST, 1); 
Last_Direction= direction; // V10.3
 int step_number = 0;  
// Set Step Number -> Calcul the distance

  if (distance == calibrate_speed) step_number = 1*microsteps ; // V10 borre el 4 y puse 1 para mayor presicion
 
 else if (direction == AH_18 || direction == HA_81 || direction == AH_81 || direction == HA_18) step_number = distance * SQUARE_SIZE * DIAGONALFACTOR;  
 else step_number = distance * SQUARE_SIZE; 


  //  Direction of the motor rotation // V10 -> See the Excel

    if (direction == H_A || direction == F1_F8 || direction == HA_18 ) {
    digitalWrite(MOTOR_WHITE_DIR, HIGH);
   
    }
    else {
      digitalWrite(MOTOR_WHITE_DIR, LOW);
       
    }
    if (direction == A_H|| direction == F1_F8 || direction == AH_18) {
      digitalWrite(MOTOR_BLACK_DIR, HIGH);
       
     }
    else {
      digitalWrite(MOTOR_BLACK_DIR, LOW);
        
     }


//  Active the motors // V10 -> See the Excel
  for (int x = 0; x < step_number; x++){
    if (direction == AH_18 || direction == HA_81 ){
      digitalWrite(MOTOR_WHITE_STEP, LOW); 
    }
    else {
      digitalWrite(MOTOR_WHITE_STEP, HIGH);   
    }
    if (direction == AH_81 || direction == HA_18 ) {
      digitalWrite(MOTOR_BLACK_STEP, LOW);  
     }
    else {
      digitalWrite(MOTOR_BLACK_STEP, HIGH); 
    }
    delayMicroseconds(speed);
    digitalWrite(MOTOR_WHITE_STEP, LOW);  
    digitalWrite(MOTOR_BLACK_STEP, LOW); 
    delayMicroseconds(speed);
  }
  
}

// *******************************  ELECTROMAGNET H-Bridge 
void electromagnet(boolean state) 
{
  Serial.print("Electroiman ");
  Serial.println(state ? "Encendido" : "Apagado");
  



  if (state == true)  // For this Test
  {
   for (int pwmValue = 0; pwmValue <= pwmMax; pwmValue++) { // Agrege incremento suave, en lugar de ir directo al valor para ver si ayuda a evitar que las piezas salgan de su posicion al encender el EM
            analogWrite(MAGNET, pwmValue);
            delay(incrementDelay);  // Espera antes de incrementar el PWM
        }
    //delay(holdTime);  // Mantén el PWM al máximo durante el tiempo especificado
        
  }
  else  
  {
    //delay(100);
    digitalWrite(MAGNET, LOW);
  }
}
// ***********************************  COUNTDONW
void countdown() {

  //  Set the time of the current player
  if (new_turn_countdown == true ) {
    new_turn_countdown = false;
    if (sequence == player_white) {
      second = second_white;
      minute = minute_white;
    }
    else if (sequence == player_black) {
      second = second_black;
      minute = minute_black;
    }
  }

  //  Countdown
  timer = millis();
  second = second - 1;
  if (second < 1) {
    second = 60;
    minute = minute - 1;
  }

  //  Record the white player time
  if (sequence == player_white) {
    second_white = second;
    minute_white = minute;
  }
  //  Record the black player time
  else if (sequence == player_black) {
    second_black = second;
    minute_black = minute;
  }
}

// ***********************  BLACK PLAYER MOVEMENT
void black_player_movement() {
Serial.println("black_player_movement");// for debug
  //  Convert the AI characters in variables
  
  /*Serial.print("lastM[0]=");     //V9.2.6
  Serial.println(lastM[0]);   //V9.2.6
  Serial.print("lastM[0]- 'a'=");     //V9.2.6
  Serial.println(lastM[0]- 'a');   //V9.2.6

  Serial.print("lastM[1]=");     //V9.2.6
  Serial.println(lastM[1]);   //V9.2.6
  Serial.print("abs(lastM[1]-'1')=");  // Original -'8'-1   //V9.2.6
  Serial.println(abs(lastM[1]-'1'));   // Original -'8'-1  //V9.2.6

  Serial.print("lastM[2]=");     //V9.2.6
  Serial.println(lastM[2]);   //V9.2.6
  Serial.print("lastM[2]- 'a'=");     //V9.2.6
  Serial.println(lastM[2]- 'a');   //V9.2.6

  Serial.print("lastM[3]=");     //V9.2.6
  Serial.println(lastM[3]);   //V9.2.6
  Serial.print("abs(lastM[3]-'1')=");  // Original -'8'-1  // V9.2.6
  Serial.println(abs(lastM[3]-'1'));   // Original -'8'-1 //V9.2.6
*/

  int departure_coord_Letter = lastM[0] - 'a';
  int departure_coord_Number = abs(lastM[1]-'1');  // Original -'8'-1  // V9.2.6
  int arrival_coord_Letter = lastM[2] - 'a';
  int arrival_coord_Number = abs(lastM[3]-'1');	// Original -'8'-1	// V9.2.6
  byte displacement_X = 0;
  byte displacement_Y = 0;

  //  Trolley displacement to the starting position
  int convert_table [] = {0, 1, 2, 3, 4, 5, 6, 7, 0}; // V9.2.6
  byte white_capturing = 1;

  if (abs(hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]) == 1) white_capturing = 0; // v9.2.6
   for (byte i = abs(white_capturing); i < 2; i++) {
    if (i == 0) {
      displacement_X = abs(arrival_coord_Letter - trolley_coordinate_Letter);
      displacement_Y = abs(arrival_coord_Number - trolley_coordinate_Number);

  Serial.println("Trolley displacement to the starting position Case Eating");
 
  Serial.print("hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]="); // V9.2.6
  Serial.println(hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]); // V9.2.6
  Serial.print("trolley_coordinate_Letter=");     //V9.2.6
  Serial.println(trolley_coordinate_Letter);   //V9.2.6
  Serial.print("trolley_coordinate_Number=");     //V9.2.6
  Serial.println(trolley_coordinate_Number);   //V9.2.6

  Serial.print("departure_coord_Letter=");    //V9.2.6
  Serial.println(departure_coord_Letter);     //V9.2.6
  Serial.print("departure_coord_Number=");    //V9.2.6
  Serial.println(departure_coord_Number);     //V9.2.6
  
  Serial.print("arrival_coord_Letter=");      //V9.2.6
  Serial.println(arrival_coord_Letter);       //V9.2.6
  Serial.print("arrival_coord_Number=");      //V9.2.6
  Serial.println(arrival_coord_Number);       //V9.2.6
  
  Serial.print("displacement_X=");      //V9.2.6
  Serial.println(displacement_X);       //V9.2.6
  Serial.print("displacement_Y=");      //V9.2.6
  Serial.println(displacement_Y);       //V9.2.6
  if (arrival_coord_Letter > trolley_coordinate_Letter) motor(A_H, SPEED_FAST, displacement_X);           // V10.3.6
    else if (arrival_coord_Letter < trolley_coordinate_Letter) motor(H_A, SPEED_FAST, displacement_X);    // V10.3.6
    if (arrival_coord_Number > trolley_coordinate_Number) motor(F1_F8, SPEED_FAST, displacement_Y);       // V10.3.6
    else if (arrival_coord_Number < trolley_coordinate_Number) motor(F8_F1, SPEED_FAST, displacement_Y);  // V10.3.6
  Serial.println("Trolley moved to the starting position Case Eating");                                   // V10.3.6
  // update Trolley location
  trolley_coordinate_Letter=arrival_coord_Letter; // V10.3.6
  trolley_coordinate_Number=arrival_coord_Number; // V10.3.6
   }
    else if (i == 1) {
      displacement_X = abs(departure_coord_Letter - trolley_coordinate_Letter);
      displacement_Y = abs(departure_coord_Number - trolley_coordinate_Number);

  Serial.println("Trolley displacement to the starting position Case NO Eating");
  Serial.print("hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]="); // V9.2.6
  Serial.println(hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]); // V9.2.6

  Serial.print("trolley_coordinate_Letter=");     //V9.2.6
  Serial.println(trolley_coordinate_Letter);   //V9.2.6
  Serial.print("trolley_coordinate_Number=");     //V9.2.6
  Serial.println(trolley_coordinate_Number);   //V9.2.6

  Serial.print("departure_coord_Letter=");    //V9.2.6
  Serial.println(departure_coord_Letter);     //V9.2.6
  Serial.print("departure_coord_Number=");    //V9.2.6
  Serial.println(departure_coord_Number);     //V9.2.6
  
  Serial.print("arrival_coord_Letter=");      //V9.2.6
  Serial.println(arrival_coord_Letter);       //V9.2.6
  Serial.print("arrival_coord_Number=");      //V9.2.6
  Serial.println(arrival_coord_Number);       //V9.2.6
  
  Serial.print("displacement_X=");      //V9.2.6
  Serial.println(displacement_X);       //V9.2.6
  Serial.print("displacement_Y=");      //V9.2.6
  Serial.println(displacement_Y);       //V9.2.6

  if (departure_coord_Letter > trolley_coordinate_Letter) motor(A_H, SPEED_FAST, displacement_X);           // V10.3.6
    else if (departure_coord_Letter < trolley_coordinate_Letter) motor(H_A, SPEED_FAST, displacement_X);    // V10.3.6
    if (departure_coord_Number > trolley_coordinate_Number) motor(F1_F8, SPEED_FAST, displacement_Y);       // V10.3.6
    else if (departure_coord_Number < trolley_coordinate_Number) motor(F8_F1, SPEED_FAST, displacement_Y);  // V10.3.6
  
  Serial.println("Trolley moved to the starting position Case NO Eating");                                  // V10.3.6

	  }
      if (i == 0) {
    digitalWrite(IN1, LOW);   // LOW for white piece polarizarion
    digitalWrite(IN2, HIGH);  // HIGH for white piece polarizarion
      
  Serial.println("Piece White moving");// for debug

  Serial.print("trolley_coordinate_Letter=");     //V9.2.6
  Serial.println(trolley_coordinate_Letter);   //V9.2.6
  Serial.print("trolley_coordinate_Number=");     //V9.2.6
  Serial.println(trolley_coordinate_Number);   //V9.2.6

  Serial.print("departure_coord_Letter=");    //V9.2.6
  Serial.println(departure_coord_Letter);     //V9.2.6
  Serial.print("departure_coord_Number=");    //V9.2.6
  Serial.println(departure_coord_Number);     //V9.2.6
  
  Serial.print("arrival_coord_Letter=");      //V9.2.6
  Serial.println(arrival_coord_Letter);       //V9.2.6
  Serial.print("arrival_coord_Number=");      //V9.2.6
  Serial.println(arrival_coord_Number);       //V9.2.6
  

  Serial.print("hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]="); // V9.2.6
  Serial.println(hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]); // V9.2.6

  Serial.print("displacement_X=");      //V9.2.6
  Serial.println(displacement_X);       //V9.2.6
  Serial.print("displacement_Y=");      //V9.2.6
  Serial.println(displacement_Y);       //V9.2.6

   
      electromagnet(true);
     motor(F8_F1, SPEED_SLOW, 0.5);
      motor(H_A, SPEED_SLOW, arrival_coord_Letter + 0.5); // v9.2.7 Put the white piece out of the board
      electromagnet(false);
      motor(F1_F8, SPEED_FAST, 0.5);
      motor(A_H, SPEED_FAST, arrival_coord_Letter + 0.5); // v9.2.7 Return the trolley
      trolley_coordinate_Letter = arrival_coord_Letter; // May be can erase this line, defined before
      trolley_coordinate_Number = arrival_coord_Number; // May be can erase this line, defined before
      delay(100);               // V9.2.6
      detect_movement(); // V9.2.6
      delay(100);               // V9.2.7
      record_sensors();       // V9.2.7 Put on Memory white piece removed from the board
      delay(100);               // V9.2.7
    }
  }
  trolley_coordinate_Letter = arrival_coord_Letter;
  trolley_coordinate_Number = arrival_coord_Number;

  //  Move the Black chess piece to the arrival position
  displacement_X = abs(arrival_coord_Letter - departure_coord_Letter);
  displacement_Y = abs(arrival_coord_Number - departure_coord_Number);
   digitalWrite(IN1, HIGH);   // HIGH for Black piece polarizarion
    digitalWrite(IN2, LOW);  // LOW for Black piece polarizarion

  Serial.println("Piece Black moving");// SPK for debug
   
  Serial.print("trolley_coordinate_Letter=");     //V9.2.6
  Serial.println(trolley_coordinate_Letter);   //V9.2.6
  Serial.print("trolley_coordinate_Number=");     //V9.2.6
  Serial.println(trolley_coordinate_Number);   //V9.2.6

  Serial.print("departure_coord_Letter=");    //V9.2.6
  Serial.println(departure_coord_Letter);     //V9.2.6
  Serial.print("departure_coord_Number=");    //V9.2.6
  Serial.println(departure_coord_Number);     //V9.2.6
  
  Serial.print("arrival_coord_Letter=");      //V9.2.6
  Serial.println(arrival_coord_Letter);       //V9.2.6
  Serial.print("arrival_coord_Number=");      //V9.2.6
  Serial.println(arrival_coord_Number);       //V9.2.6
  

  Serial.print("hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]="); // V9.2.6
  Serial.println(hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter]); // V9.2.6

  Serial.print("displacement_X=");      //V9.2.6
  Serial.println(displacement_X);       //V9.2.6
  Serial.print("displacement_Y=");      //V9.2.6
  Serial.println(displacement_Y);       //V9.2.6


  electromagnet(true);
  //  Horse displacement - V9.2.3 // X= F1_F8  Y= A_H
  if (displacement_X == 1 && displacement_Y == 2 || displacement_X == 2 && displacement_Y == 1) {
    if (displacement_Y == 2) {
      if (departure_coord_Letter < arrival_coord_Letter) {
        motor(A_H, SPEED_SLOW, 0.5);
        if (departure_coord_Number < arrival_coord_Number) motor(F1_F8, SPEED_SLOW, 2);
        else motor(F8_F1, SPEED_SLOW, 2);
        motor(A_H, SPEED_SLOW, 0.5);
      }
      else if (departure_coord_Letter > arrival_coord_Letter) {
        motor(H_A, SPEED_SLOW, 0.5);
        if (departure_coord_Number < arrival_coord_Number) motor(F1_F8, SPEED_SLOW, 2);
        else motor(F8_F1, SPEED_SLOW, 2);
        motor(H_A, SPEED_SLOW, 0.5);
      }
    }
    else if (displacement_X == 2) {
      if (departure_coord_Number < arrival_coord_Number) {
        motor(F1_F8, SPEED_SLOW, 0.5);
        if (departure_coord_Letter < arrival_coord_Letter) motor(A_H, SPEED_SLOW, 2);
        else motor(H_A, SPEED_SLOW, 2);
        motor(F1_F8, SPEED_SLOW, 0.5);
      }
      else if (departure_coord_Number > arrival_coord_Number) {
        motor(F8_F1, SPEED_SLOW, 0.5);
        if (departure_coord_Letter < arrival_coord_Letter) motor(A_H, SPEED_SLOW, 2);
        else motor(H_A, SPEED_SLOW, 2);
        motor(F8_F1, SPEED_SLOW, 0.5);
      }
    }
  }
   //  Diagonal displacement - Bishop - Queen - *** V10_1
    else if (displacement_X == displacement_Y) {
    if (departure_coord_Letter > arrival_coord_Letter && departure_coord_Number > arrival_coord_Number) motor(HA_81, SPEED_SLOW, displacement_X);
    else if (departure_coord_Letter > arrival_coord_Letter && departure_coord_Number < arrival_coord_Number) motor(HA_18, SPEED_SLOW, displacement_X);
    else if (departure_coord_Letter < arrival_coord_Letter && departure_coord_Number > arrival_coord_Number) motor(AH_81, SPEED_SLOW, displacement_X);
    else if (departure_coord_Letter < arrival_coord_Letter && departure_coord_Number < arrival_coord_Number) motor(AH_18, SPEED_SLOW, displacement_X);
   
  }
  //  Black Kingside castling
  else if (departure_coord_Letter == 4 && departure_coord_Number == 7 && arrival_coord_Letter == 6 && arrival_coord_Number == 7) {  
  //  Kingside castling // v9.2.7.2
    motor(F8_F1, SPEED_SLOW, 0.5);
    motor(A_H, SPEED_SLOW, 2);
    electromagnet(false);
    motor(A_H, SPEED_FAST, 1);
    motor(F1_F8, SPEED_FAST, 0.5);
    electromagnet(true);
    motor(H_A, SPEED_SLOW, 2);
    electromagnet(false);
    motor(A_H, SPEED_FAST, 1);
    motor(F8_F1, SPEED_FAST, 0.5);
    electromagnet(true);
    motor(F1_F8, SPEED_SLOW, 0.5);
  }
  else if (departure_coord_Letter == 4 && departure_coord_Number == 7 && arrival_coord_Letter == 2 && arrival_coord_Number == 7) {  
    //  Queenside castling // v10.3.5
    // Start Queenside castling
    // from 1 to 2, works fine, so no need to change:
    motor(F8_F1, SPEED_SLOW, 0.5);
    motor(H_A, SPEED_SLOW, 2);
    electromagnet(false);
    // Then from 2 to 3, works fine, so not have to change this:
    motor(H_A, SPEED_FAST, 2);
    motor(F1_F8, SPEED_FAST, 0.5);
    electromagnet(true);
    motor(A_H, SPEED_SLOW, 3);
    // Now need an Extra movement for center the tower, so may add:
    motor(A_H, SPEED_SLOW,FIXUP_POSITION);  // New Line
    // continue the code:
    electromagnet(false);
    // Undo the extra movement
    motor(H_A, SPEED_SLOW,FIXUP_POSITION);  // New Line
    // now is on 3 and have to move back to position 2
    motor(H_A, SPEED_FAST, 1);
    motor(F8_F1, SPEED_FAST, 0.5);
    // here is the Issue, the Electromagnet not get the Piece
    // So may need an extra movement
    motor(F8_F1, SPEED_FAST,  FIXUP_POSITION ); // New line
    // Now move to the position 4
    electromagnet(true);
    motor(F1_F8, SPEED_SLOW,  FIXUP_POSITION ); // New line //  Undo the las Fixup
    motor(F1_F8, SPEED_SLOW, 0.5);
    motor(F1_F8, SPEED_SLOW,  FIXUP_POSITION ); // New Line  // More Fixup
    electromagnet(false);
    motor(F8_F1, SPEED_FAST,  FIXUP_POSITION ); // New Line // Undo last Fixup
    // end Queenside castling
  }
  //  Horizontal displacement
  else if (displacement_Y == 0) {
    if (departure_coord_Letter > arrival_coord_Letter) motor(H_A, SPEED_SLOW, displacement_X);
    else if (departure_coord_Letter < arrival_coord_Letter) motor(A_H, SPEED_SLOW, displacement_X);
  }
  //  Vertical displacement
  else if (displacement_X == 0) {
    if (departure_coord_Number > arrival_coord_Number) motor(F8_F1, SPEED_SLOW, displacement_Y);
    else if (departure_coord_Number < arrival_coord_Number) motor(F1_F8, SPEED_SLOW, displacement_Y);
  }
   Fix_Distance();// v10.3.5 extra last black movement, for center the piece on the square
  electromagnet(false);

  //  Upadte the hall sensors states with the Balck move
  hall_sensor_status_memory[convert_table[departure_coord_Number]][departure_coord_Letter] = 0; // Original 1 // V9.2.6
  hall_sensor_status_memory[convert_table[arrival_coord_Number]][arrival_coord_Letter] = 1; // Original 0 // V9.2.6
  hall_sensor_status[convert_table[departure_coord_Number]][departure_coord_Letter] = 0;  // Original 1 // V9.2.6
  hall_sensor_status[convert_table[arrival_coord_Number]][arrival_coord_Letter] = 1;  // Original 0 // V9.2.6
	
	
}


// **********************************  LCD DISPLAY
void lcd_display() {

   lcd.backlight(); // If Real this line not be comment // If Simulide put comment "//" at the beginig of the line

  if (no_valid_move == true) {
    lcd.setCursor(0, 0);
    lcd.print("  NO VALID MOVE  ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    delay(500);  // V10.2
    Set_The_Board();          // V10.3.5        // Si es false -> solucion manual
    no_valid_move = false;
    return;
  }

  switch (sequence) {
    case start_up:
      lcd.setCursor(0, 0);
      lcd.print("10_3_4 AUTOMATIC"); // V10.3.4 add the Number Version
      lcd.setCursor(0, 1);
      lcd.print("   CHESSBOARD   ");
      sequence = start;
      delay(800); // V10.3.2 reduce delay()
    case start:
      lcd.setCursor(0, 0);
      lcd.print(" PRESS W - HvsH ");  // V10.3.2
      lcd.setCursor(0, 1);
      lcd.print(" PRESS B - HvsC ");
      break;
    case calibration:
      lcd.setCursor(0, 0);
      lcd.print("  CALIBRATION   ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      break;
    case player_white:
      lcd.setCursor(0, 0);
      lcd.print("     WHITE      ");
      lcd.setCursor(0, 1);
      lcd.print("     " + String(minute) + " : " + String(second) + "     ");
      break;
    case player_black:
      lcd.setCursor(0, 0);
      lcd.print("     BLACK      ");
      lcd.setCursor(0, 1);
      lcd.print("     " + String(minute) + " : " + String(second) + "     ");
      break;
  }
}

// ************************  DETECT MOVEMENT  
void detect_movement() { // Read twice, because some times, no Detection on the First - EK V10.3.5

  // Reset Matrix, 
  hall_value[8][8] = 0; // Reset matrix with hall values (Measures) //V10.3.5
  hall_sensor_status[8][8] = 0; // Reset matrix with hall Status (1 or 0) //V10.3.5

  //  Record the hall switches status
 static bool first_run = true; // EK
 if(first_run){
      // Take an initial reading to stabilize the sensors  // V10.3.5
            Read_Sensor(mux1);
            Read_Sensor(mux2);
            Read_Sensor(mux3);
            Read_Sensor(mux4);
            delay(200); // Give some time for the system to stabilize
            first_run = false;  // Mark first run as complete
    }
  // Perform regular readings
    Read_Sensor (mux1);
    Read_Sensor (mux2);
    Read_Sensor (mux3);
    Read_Sensor (mux4);
    delay(100);


  // Comparo valores con la Memoria // la funcion compare_hall_status Define la posicion de arribo y de partida.
  compare_hall_status();
  delay(300);
  // Muestra los valores
    hall_display();
    delay(300);

}

void Read_Sensor (HC4067 &mux) 
{

  //  Read the hall sensor status
 
  multiplex = 0;
  mux.disable(); // Deshabilita todos los mux

// Activa el mux llamado como parámetro
  mux.enable();

// Establece las columnas y filas según el mux
  if (&mux == &mux1) {
    column = 0;
    row = 0;
  } else if (&mux == &mux2) {
    column = 0;
    row = 2;
  } else if (&mux == &mux3) {
    column = 0;
    row = 4;
  } else if (&mux == &mux4) {
    column = 0;
    row = 6;
  }
 // Lee los valores del sensor para el mux activado
  read_hall_values(mux);

  // Deshabilita el mux después de la lectura
  mux.disable();

  
}


void read_hall_values(HC4067 &mux) {
  for (int j = 0; j < 16; j++) {
    mux.setChannel(j);
    hallMeasure = analogRead(Mux_Out);
    delay(5);
    Record_hall_measure(); // Graba el valor del sensor activado
    column++;
    if (column > 7) {
      column = 0;
      row++;
    }
  }
}

void Record_hall_measure() {
  if (hallMeasure <= hall_min) {
    hall_sensor_status[row][7-column] = -1;    // revisa la polaridad de tus imanes. Aqui defini las blancas (-1)
    hall_value[row][7-column] = hallMeasure;
  } else if (hallMeasure > hall_max) {
    hall_sensor_status[row][7-column] = 1;   // revisa la polaridad de tus imanes. Aqui defini las negras (1)
    hall_value[row][7-column] = hallMeasure;
  } else {
    hall_sensor_status[row][7-column] = 0;    // revisa la polaridad de tus imanes. Aqui defini que no hay pieza
    hall_value[row][7-column] = hallMeasure;
  }
}
void compare_hall_status()
{
number_sensor_change=0;  // V9.2.7.2 

// first see all sensors changes

  for (byte i = 0; i < 8; i++) 
  {
    for (byte j = 0; j < 8; j++) 
    {
      if (abs(hall_sensor_status[i][j]) != abs(hall_sensor_status_memory[i][j])) // En valor Absoluto, indistinto B&N 
      { 
        number_sensor_change++;

      }
    }
  }
// deppend on the number of sensor changed, normal move, eat situation, castle situation
  switch (number_sensor_change){
 
  case 1:
    // 1 Cambio en valor absoluto. única opcion se quita 1 pieza del tablero -> situacion comer normal

    for (byte i = 0; i < 8; i++) 
    {
    for (byte j = 0; j < 8; j++)      
      {
        if (hall_sensor_status[i][j] != hall_sensor_status_memory[i][j]) 
      { 
        if (hall_sensor_status_memory[i][j] == 1) // 1=Black piece before, So White Eat a Black piece arrival position
         {
          hall_colone[1] = i;     //V9.2.6 // original [0]
          hall_line[1] = j;       //V9.2.6 // original [0]
         }
        if (hall_sensor_status_memory[i][j] == -1) // -1=White piece before, so the departure position. 
         {
          hall_colone[0] = i; //V9.2.6 // original [0]
          hall_line[0] = j;   // V9.2.6 // original [0] 
          }
      }
    }
  }
    break;

  case 2:
    // 2 cambios en valor absoluto. unica opcion-> una pieza se mueve del tablero a una casilla vacia-> mover pieza

    for (byte i = 0; i < 8; i++) 
    {
    for (byte j = 0; j < 8; j++)      
      {
        if (hall_sensor_status[i][j] != hall_sensor_status_memory[i][j]) 
      { 
        if (hall_sensor_status_memory[i][j] == 0) // 0=no Piece before, so arrival position
         {
          hall_colone[1] = i; // v9.2.5
          hall_line[1] = j;  
         }
        
        if (hall_sensor_status_memory[i][j] == -1) // -1=White piece before, so the departure position. 
         {
          hall_colone[0] = i; //V9.2.6 // original [0]
          hall_line[0] = j;   // V9.2.6 // original [0] 
          }
      }
    }
  }

    break;

  case 3:
    // 3 cambios en valor absoluto. unica opcion -> 1 pieza se quita del tablero y 1 pieza se mueve a casilla vacia-> Situacion Comer al Paso
    
   for (byte i = 0; i < 8; i++) 
    {
    for (byte j = 0; j < 8; j++)      
      {
        if (hall_sensor_status[i][j] != hall_sensor_status_memory[i][j]) 
      { 
        if (hall_sensor_status_memory[i][j] == 0) // 0=no Piece before, so arrival position
         {
          hall_colone[1] = i; // v9.2.5
          hall_line[1] = j;  
         }
        if (hall_sensor_status_memory[i][j] == -1) // -1=White piece before, so the departure position. 
         {
          hall_colone[0] = i; //V9.2.6 // original [0]
          hall_line[0] = j;   // V9.2.6 // original [0] 
          }
      }
    }
  }
    break;

  case 4:
    // 4 cambios en valor Absoluto. unica opcion -> 2 piezas se mueven a casillas vacias -> Situacion Enroque
    // Si Torre a1 se movio entonces enroque lado dama. Si no, Enroque lado Rey.
    // Siempre sale del Rey, e1. Puede ir al c1 (lado dama) o al g1(lado Rey)
    hall_colone[0] = 0; 
    hall_line[0] = 4; 

    if (hall_sensor_status[0][0] != hall_sensor_status_memory[0][0]) // verifico casilla a1
    {
          hall_colone[1] = 0; 
          hall_line[1] = 2;    
    }
    else
    {
          hall_colone[1] = 0; 
          hall_line[1] = 6;  
    }

    break;

  default:
    // Cualquier otro valor de cambio de sensores en valor absoluto(0, o mayor que 4), error
    break;

  }
 
}

void hall_display() {
  lc.clearDisplay(0);
 
  // hall value and matrix
  
  Serial.println("+ -    -   -   -  -   -   -   -   -+");
  for (int i = 0; i < 8; i++) {
    Serial.print(' ');
    Serial.print(i+1);
    Serial.print("| ");
    for (int j = 0; j < 8; j++) {
      Serial.print(hall_value[i][j]);
      
      Serial.print(" ");
    }
    Serial.println('|');
  }
  Serial.println("+ -    -   -   -  -   -   -   -   -+");
  Serial.println("     a   b   c   d   e   f   g   h");

 // *** hall status and Matrix
  Serial.println("+ - - - - - - - - -+");
  for (int i = 0; i < 8; i++) {
    Serial.print(' ');
    Serial.print(i+1);
    Serial.print("| ");
    for (int j = 0; j < 8; j++) {
      Serial.print(hall_sensor_status[i][j]);
      if (hall_sensor_status[i][j] == 1 || hall_sensor_status[i][j] == -1) {
        // *************************************************************************************
        lc.setLed(0, j, i, true); // You can rotate the Matrixled so lcd matrix could not folow the same logic as hall matrix  // SK = lc.setLed(0, j, i, true);  // EK = lc.setLed(0, 7-j, 7-i, true); // Ethan Change This Line for you Matrix Led Layout
        //*****************************************************************************************
      }
      Serial.print(" ");
    }
    Serial.println('|');
  }
  Serial.println("+ - - - - - - - - -+");
  Serial.println("  a  b c d e f g h");
}
void record_sensors() 
{
    for (byte i = 0; i < 8; i++) 
   {
      for (byte j = 0; j < 8; j++) 
      {
      hall_sensor_status_memory[i][j] = hall_sensor_status[i][j]; // v9.2.6 switched
    }
    }
}

// **************************  PLAYER DISPLACEMENT
void player_displacement() {

  //  Convert the hall sensors switches coordinates in characters
  char table1[] = {'1', '2', '3', '4', '5', '6', '7', '8'}; // v9.2.5
  char table2[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

  mov[0] = table2[hall_line[0]];  // For move e2 -> e4
 // Serial.print("mov[0]");  // added SPK for debug
 // Serial.println(mov[0]);    // added SPK for debug - Show letter e
  mov[1] = table1[hall_colone[0]];
// Serial.print("mov[1]");    // added SPK for debug
// Serial.println(mov[1]);    // added SPK for debug - Show number 2 (e2, previous move)
  mov[2] = table2[hall_line[1]];
// Serial.print("mov[2]");    // added SPK for debug
// Serial.println(mov[2]);    // added SPK for debug - Show letter e
  mov[3] = table1[hall_colone[1]];
// Serial.print("mov[3]");    // added SPK for debug
// Serial.println(mov[3]);    // added SPK for debug  - Show number 4 (e4, last move)
}

// **************************  Set the Board manualy if Bad Move and can't fix automatic
// V10.3.5
void Set_The_Board(){ // modificar para que resalte en la matriz led y en el serial los casilleros a corregir
  Serial.println("Set_The_Board called");
	// Place manualy the pieces on place as on the Last move, before the "Bad Move" appears
	lcd.setCursor(0, 0);
    lcd.print("PUT PIECES AS   ");
    lcd.setCursor(0, 1);
    lcd.print("EXPECTD-PRESS BL");
    Serial.println(" ");
	Serial.print("Place the pieces on place as show on the serial monitor");
	Serial.println(" ");
	Serial.print("When finished, press BLACK Button");
  serialBoard(); // *********** print the Board from Micro_Max
	 while (digitalRead(BUTTON_BLACK_SWITCH_MOTOR_BLACK) == HIGH ) {
      delay(100);
  } // Here wait the White button pressed for Continue
  Serial.println(" ");
	Serial.print("Button BLACK Pressed");
	lcd.setCursor(0, 0);
    lcd.print("BUTTON BLACK    ");
    lcd.setCursor(0, 1);
    lcd.print("PRESSED         ");
    Serial.println(" ");
    delay(100);
    //  Set the new status of the hall sensors
    
   // Reset Matrix, 
  hall_value[8][8] = 0; // Reset matrix with hall values (Measures) //V10.3.5
  hall_sensor_status[8][8] = 0; // Reset matrix with hall Status (1 or 0) //V10.3.5

  //  Read and Record the hall Sensors
 static bool first_run = true; // EK
 if(first_run){
      // Take an initial reading to stabilize the sensors  // V10.3.5
            Read_Sensor(mux1);
            Read_Sensor(mux2);
            Read_Sensor(mux3);
            Read_Sensor(mux4);
            delay(200); // Give some time for the system to stabilize
            first_run = false;  // Mark first run as complete
    }
    // Perform regular readings
          Read_Sensor (mux1);
          Read_Sensor (mux2);
          Read_Sensor (mux3);
          Read_Sensor (mux4);
          delay(100);

	hall_display();  // update Matrix Led

    // hall_sensor_status returned to the value seted by user
    Serial.println(" ");
	Serial.print("hall_sensor_status seted by the user to the expected value");
    lcd.setCursor(0, 0);
    lcd.print("BOARD SETED BY  ");
    lcd.setCursor(0, 1);
    lcd.print("   THE USER     ");
    Serial.println(" ");
    delay(100);
}

// *********** Fix the Distance ****************** V10.3
// Fix the Distance between the Trolley and the Piece if the Sensor not activated
// So, if the Piece is not on the center, trolley will move it
// Last_Direction is taken From Motor ()
// Directions {A_H,H_A,F1_F8,F8_F1,AH_18, HA_81, AH_81, HA_18, calibrate_speed}; From Global
// So 0=A_H, 1=H_A , ...

void Fix_Distance(){
  Serial.println("Fix_Distance called");
// /*
  Serial.print("Last direction was = ");
  Serial.print(Last_Direction);
  Serial.print(" = ");
  Serial.println(directionNames[Last_Direction]);
// */
  if (Last_Direction == A_H){ //0
    Undo_Last_Direction=H_A;
  }
  else if (Last_Direction == H_A){ //1
    Undo_Last_Direction=A_H;
  }
  else if(Last_Direction == F1_F8){ //2
    Undo_Last_Direction=F8_F1;
  }
  else if (Last_Direction == F8_F1){ //3
    Undo_Last_Direction=F1_F8;
  }
  else if (Last_Direction == AH_18){ //4
    Undo_Last_Direction=HA_81;
  }
  else if (Last_Direction == HA_81){ //5
    Undo_Last_Direction=AH_18;
  }
  else if (Last_Direction == AH_81){ //6
    Undo_Last_Direction=HA_18;
  }
  else if (Last_Direction == HA_18){ //7
    Undo_Last_Direction=AH_81;
  }
 /*
    Serial.println ("*** Automated try to correct the Poistion of the Piece ***");
    lcd.setCursor(0, 0);
    lcd.print("AUTOMATED TRY TO");
    lcd.setCursor(0, 1);
    lcd.print("   CORRECT IT   ");
    Serial.println(" ");
    delay(300);
 */
    electromagnet(true);
    Serial.println (" *** Move FIXUP_POSITION ***");
    motor(Last_Direction, SPEED_SLOW,FIXUP_POSITION); 
    electromagnet(false);
    Serial.println (" *** Undo Move FIXUP_POSITION ***");
    motor(Undo_Last_Direction, SPEED_SLOW, FIXUP_POSITION);

    /*
    Serial.println("The Piece has Moved ");
    lcd.setCursor(0, 0);
    lcd.print("PIECE MOVED     ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    Serial.println(" ");
    delay(300);
    */

}
// Funcion Cantidad de piezas en el Tablero.
// Agregar esa funcion aqui
// Asi se marcan en la Matriz led y en el serial las posiciones del tablero a verificar
// Es posible que falte verificar: los cambios en fix automatico y en manual, no deben afectar el last move.
// Just count the pieces on the board is on Micro_Max and is on the Hall Sensor
// In Micro_Max the Board is save on this array:
/* board is left part, center-pts table is right part, and dummy 

char b[] = {
  22, 19, 21, 23, 20, 21, 19, 22, 28, 21, 16, 13, 12, 13, 16, 21,
  18, 18, 18, 18, 18, 18, 18, 18, 22, 15, 10,  7,  6,  7, 10, 15,
  0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
  0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
  0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
  0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
  9,  9,  9,  9,  9,  9,  9,  9, 22, 15, 10,  7,  6,  7, 10, 15,
  14, 11, 13, 15, 12, 13, 11, 14, 28, 21, 16, 13, 12, 13, 16, 21, 0
};
*/

bool Pieces_on_board(){
Serial.println(" ");
Serial.println("Pieces_on_board Called");

pieces_on_micromax =0;          // reseteo el contador de piezas en Micromax
pieces_on_hall_status =0;      // reseteo el contador de piezas en el Sensor

// Cuento la cantidad de Piezas, segun Micromax
for (int i = 0; i < 127; i++) { // recorre todo b[] menos la dummy, la ultima posicion del array que tiene un 0
        
        if (i % 16 < 8) { // i%16= resto. Por ejemplo el resto de 10/16=10 por lo tanto ignorar las posiciones invisibles de b[].
            if (b[i] != 0) { // Si hay una pieza (valor distinto de 0)
                pieces_on_micromax++;
            }
        }
    }
// Cuento la Cantidad de Piezas, segun la lectura en los Hall Sensor

for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if(hall_sensor_status[i][j] != 0){ // Si hay una pieza (valor distinto de 0)
                pieces_on_hall_status++;
      }
    }
  }

if(pieces_on_micromax == pieces_on_hall_status)
  return true;
else
  return false;

}
