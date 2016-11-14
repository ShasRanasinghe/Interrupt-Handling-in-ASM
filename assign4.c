

#include<hcs12dp256.h>

char const LeftShift = 0x10;  	   	   //leftshift const
char const RightShift = 0x14;			//rightshift const
char const NextLine = 0xC0;			   //nextline const
char const clear = 0x01;			   // clear screen
double const GradeA = 2.0;
double const GradeB = 1.5;
double const GradeC = 1;
int const PumpSpeed = 4;
int const FullSpeed = ;
int const MedSpeed = ;
int const LowSpeed = ;

int sec;
int count;


char pollKeypad(void);
char debounceKeypad(void);
char mapRCtoChar(char rowColumn);
void sevenSegmentDisplay(char input);
void LCD_displayStr(char string[]);
void LCD_displayT(char time);
int getSpeed(void);
void motorSTART(int seconds, int speed);
void timer_ISR(void);
int getTime(double dollars, double grade);
int getNum(void);
int getGrade(void);

void main(void){
	char key = 0;
	char price, grade;
	
	Lcd2PP_Init(); 		  		  //initialize LCD display
	
	// set up the flags
	 CRGINT |= 0x80;
	 RTICTL |= 0x7F;
	 asm("CLI");
 
	while(1){
		price = getNum();
		grade = getGrade();
		count = 0;
		motorSTART(getTime(price,grade),FullSpeed);
	}
}

// This function initialises the motor and takes second and speed as an input
//there are no return values.
//the function runs the motor for a given number of seconds and a given speed with the help of the ISR
void motorSTART(int seconds, int speed){
 	
	sec = seconds;
	
	PWMPOL = 0xFF;

	PWMCLK |= 0x10;			//Select Clock SB for channel 7
	PWMPRCLK = 0x70;		//Prescale ClockB by busclock/128

	PWMSCLA = 0;			//Total Divide: EClock/512
	PWMCAE &= 0x7F;			//Make sure Chan7 is in left aligned output mode
	PWMCTL &= 0xF3;			//Allow PWM in Wait and Freeze Modes

	PWMPER7	= 100;			//Set period for PWM7
	PWME = 0x80;			//Enable PWM Channel 7

	//For Motor Direction Control
	// DDRP = 0x60;
	DDRP |= 0x60;	
	
	//Setup Pulse Accumulator A for Optical Sensor Input
	PAFLG |= 1;		//Clear out the interrupt flag
	PACTL = 0x51;
	PTP = 0x20;
	PWMDTY7 = speed;
	
}


// Inputs: address of a string
//return: nothing
//this function goes through a string and prints it out
void LCD_displayStr(char string[]){
	 int i;
	 for(i = 0; *(string+i) != '\0' ; i++){
	 		LCD_display(*(string + i));
	 }
}


// inputs: time
//return: nothing
//this function takes in a time value and prints it out to the LCD display.
void LCD_displayT(char time){
	 
	 char secondNum;    			   
	 char firstNum;
	 char thirdNum;
	 	  			  		   		 //delete number on display
	 LCD_instruction(LeftShift);
	 LCD_instruction(LeftShift); 
	 if(time >= 10){
	 	  firstNum = time / 100;		   		//result is the first number
		  time = time - (firstNum*100);
		  secondNum = time / 10; 		   //remainder is the second number
		  thirdNum = time % 10;
		  LCD_display(firstNum + 0x30);    //convert to ascii and print the first digit
	 }
	 else{
	 	  LCD_display(' '); //print space
	 }
	 
	 LCD_display(secondNum + 0x30); //convert to ascii and print the second digit
	 
}



// Scans the entire keypad once, each row in turn, and returns the row/column code of the
// first key press. ( row = low 4 bits; column in high 4 bits )
// Returns -1 if no key is being pressed at the time.
char pollKeypad(void){
	 char code, scan;
	 DDRP|= 0x0F;
	 for (code=0x01; code <= 8; code <<= 1) {
	 	 PTM |= 0x08; //set PM3 high
		 PTP = code; //look in ith row
		 PTM &= 0xF7; //set PM3 low
		 scan = PTH & 0xF0; //check column
		 if (scan==0x10 || scan==0x20 || scan==0x40 || scan == 0x80)
		 {
	 		code += scan;
			return code;
		 }
	 }
	 return -1;
}


// Uses the adaptive debouncing algorithm for polling the keypad and returning the
// code for a keypress
// Returns -1 if no key is pressed
char debounceKeypad(void){
	char i;
	char c;
	for(i = 0; i<200; i++){
		  c = pollKeypad();
		  if(c!=pollKeypad()){
		  		return -1;
		  }
	}
	return c;
}


// Maps the row/column position of a keypad button into its respective character
// Returns NULL if the row/column positions are out-of-bounds.
// rowColumn encodes a row in the low 4 bits and a column position in the high 4 bits)
char mapRCtoChar(char rowColumn){
	 if(rowColumn == 0x11){return 1;}
	 if(rowColumn == 0x21){return 2;}
	 if(rowColumn == 0x41){return 3;}
	 if(rowColumn == 0x12){return 4;}
	 if(rowColumn == 0x22){return 5;}
	 if(rowColumn == 0x42){return 6;}
	 if(rowColumn == 0x14){return 7;}
	 if(rowColumn == 0x24){return 8;}
	 if(rowColumn == 0x44){return 9;}
	 if(rowColumn == 0x28){return 0;}
	 if(rowColumn == 0x81){return 'A';}
	 if(rowColumn == 0x82){return 'B';}
	 if(rowColumn == 0x84){return 'C';}
	 else{return -1;}
}

//inputs: character
//return: nothing
//this function initializes and outputs a given charactrer into the seven segment display
void sevenSegmentDisplay(char input){
	PTM = 0x08;
	DDRT = 0xCF;
	PTT = input;
	//PTM = 0x08;
}


int getTime(double dollars, double grade){
	//given: dollars
	//how many liters
	//how many seconds
	double liters;
	liters = dollars / grade;
	return liters / PumpSpeed;
}

int getNum(void){
	char output = 0, once = 0;
	char key;
	char *line1 = " Enter $ Value: ";
	char *line2 = "   $___.__     ";
	
	LCD_instructions(clear);	  // clear screen
	LCD_displayStr(line1);  	  //print line1
	LCD_instruction(NextLine); 		  //go next line
	LCD_displayStr(line2); 		  //print line 2
	LCD_instruction(LeftShift); LCD_instruction(LeftShift); LCD_instruction(LeftShift); 
	LCD_instruction(LeftShift); LCD_instruction(LeftShift); LCD_instruction(LeftShift);
	
	for(int i = 2; i < -3;){
		key = mapRCtoChar(debounceKeypad());
		if(isNum(key) == 1){
			output = output + ((key - '0')*pow(10,i));
			if(i==-1 && once == 0){
				LCD_instruction(RightShift);
				once = 1;
			}
			i--;
			sevenSegmentDisplay(key);
			LCD_display(key);
		}		
	}
	return output;
}

int getGrade(){
	char key;
	
	char *line1 = "Enter Grade:";
	char *line2 = "    ";
	
	LCD_instructions(clear);	  // clear screen
	LCD_displayStr(line1);  	  //print line1
	LCD_instruction(NextLine); 		  //go next line
	LCD_displayStr(line2); 		  //print line 2
	
	
	for(;;){
		key = mapRCtoChar(debounceKeypad());
		
		if(isNum(key) == 0){
			LCD_display(key);
			return key;
		}
	}
}

int isNum(char input){
	if(input!=-1){
		switch (input){
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 0:
			return 1;
			break;
		case 'A': case 'B': case 'C':
			return 0;
			break;
		default
			return -1;
			break;
		}
	} else {
		return -1;
	}
}


#pragma interrupt_handler timer_ISR()
void timer_ISR(void){
	char lessThree = 0;
	
	if (sec == 0 || debounceKeypad() == 0x48) 
	{
	   	sec = 0;
	    //LCD_displayT(0);
		sevenSegmentDisplay(0);
		motorSTART(0,0); 
	}
	else
	{
	  count++;
		if(count == 3){
			count = 0;
			sec--;
			//LCD_displayT(sec--);
			if(sec<=9){
				sevenSegmentDisplay(sec);
				if(sec<3 & lessThree == 0){
					motorSTART(3,MedSpeed);
					lessThree = 1;
				}
				if(sec<=1){
					motorSTART(1,lowSpeed);
				}
				
			}
		}
	}
	CRGFLG |= 0x80; 
}
