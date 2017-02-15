/***********************************************************************/
/*  Supported Microcontroller:RX62T                                    */
/*  File:                   kit12_rx62t.c                              */
/*  File Contents:          MCU Car Trace Basic Program(RX62T version) */
/*  Version number:         Ver.1.00                                   */
/*  Date:                   2013.09.01                                 */
/*  Copyright:              Renesas Micom Car Rally Secretariat        */
/***********************************************************************/
/*
This program supports the following boards:
* RMC-RX62T board
* Sensor board Ver. 5
* Motor drive board Ver. 5
*/

/*======================================*/
/* Include                              */
/*======================================*/
#include "iodefine.h"

/*======================================*/
/* Symbol definitions                   */
/*======================================*/

/* Constant settings */
#define PWM_CYCLE       24575           /* Motor PWM period (16ms)     */
#define SERVO_CENTER    2300            /* Servo center value          */
#define HANDLE_STEP     13              /* 1 degree value              */

/* Masked value settings X:masked (disabled) O:not masked (enabled) */
#define MASK2_2         0x66            /* X O O X  X O O X            */
#define MASK2_0         0x60            /* X O O X  X X X X            */
#define MASK0_2         0x06            /* X X X X  X O O X            */
#define MASK3_3         0xe7            /* O O O X  X O O O            */
#define MASK0_3         0x07            /* X X X X  X O O O            */
#define MASK3_0         0xe0            /* O O O X  X X X X            */
#define MASK4_0         0xf0            /* O O O O  X X X X            */
#define MASK0_4         0x0f            /* X X X X  O O O O            */
#define MASK4_4         0xff            /* O O O O  O O O O            */
#define MASK1_1         0x18            /* X X X O  O X X X            */
/* Masked value for PID ************************************************/
#define MASK4         0x80            /* O X X X  X X X X              */
#define MASK3         0x40            /* X O X X  X X X X              */
#define MASK2         0x20            /* X X O X  X X X X              */
#define MASK1         0x10            /* X X X O  X X X X              */
#define MASK_1        0x08            /* X X X X  O X X X              */
#define MASK_2        0x04            /* X X X X  X 0 X X              */
#define MASK_3        0x02            /* X X X X  X X O X              */
#define MASK_4        0x01            /* X X X X  X X X O              */
/* Masked value for PID ************************************************/
#define MASKe0        0xe0            /* O O O X  X X X X              */
#define MASK70        0x70            /* X O O O  X X X X              */
#define MASK38        0x38            /* X X O O  O X X X              */
#define MASK1c        0x1c            /* X X X O  O O X X              */
#define MASK0e        0x0e            /* X X X X  O O O X              */
#define MASK07        0x07            /* X X X X  X 0 O O              */
#define MASKc0        0xc0            /* O O X X  X X X X              */
#define MASK60        0x60            /* X O O X  X X X X              */
#define MASK30        0x30            /* X X O O  X X X X              */
#define MASK18        0x18            /* X X X O  O X X X              */
#define MASK0c        0x0c            /* X X X X  O O X X              */
#define MASK06        0x06            /* X X X X  X 0 O X              */
#define MASK03        0x03            /* X X X X  X X O O              */


/*======================================*/
/* Prototype declarations               */
/*======================================*/
void init(void);
void timer( unsigned long timer_set );
unsigned char sensor_inp( unsigned char mask );
unsigned char startbar_get( void );
int check_crossline( void );
int check_rightline( void );
int check_leftline( void );
unsigned char dipsw_get( void );
unsigned char buttonsw_get( void );
unsigned char pushsw_get( void );
void led_out_m( unsigned char led );
void led_out( unsigned char led );
void motor( int accele_l, int accele_r );
void handle( int angle );
int getSensorError(void);
void calculatePID(void);
unsigned char Mask( unsigned char sensor, unsigned char mask );
unsigned char getSensorValue();
/*======================================*/
/* Global variable declarations         */
/*======================================*/
unsigned long   cnt0;
unsigned long   cnt1;
int             pattern;
int right = 0;
int left =0;
int error = 0;
int increamentSpeed = 0;

/*======================================*/
/* PID Calculation Variables            */
/*======================================*/
int pidOut = 0;
int currentError = 0;
double previousError = 0;
double pidP = 20;
double pidI = 0;
double pidD = 0;
double IMemory = 0.0;
int direction = 0;
int initialSpeed = 60;
int turnTimeout = 300;
int IBound = 5;
int DBound =5;
double integral = 0.0;
int maxSpeed = 60;
int doNotCareTimeout = 100;
double leftSpeed =0.0;
double rightSpeed=0.0;
/***********************************************************************/
/* Main program                                                        */
/***********************************************************************/
void main(void)
{
    /* Initialize MCU functions */
    init();

    /* Initialize micom car state */
	led_out(0x3);
	while( !pushsw_get()){}
	led_out(0x0);
	
    while( 1 ) {
		if(direction == 0){
			if(check_rightline() || check_crossline()){
				led_out(0x1);
				motor(maxSpeed,maxSpeed);
				direction = 2;
				timer(100);
			}
			else if (check_leftline() || check_crossline()){
				led_out(0x2);
				direction = 3;
				motor(maxSpeed,maxSpeed);
				timer(100);
			}
		}
		else if (direction != 0){
			if(sensor_inp(MASK4_4) == 0x00){
				led_out(0x3);
				timer(100);
				if(direction == 2){
					motor( maxSpeed, maxSpeed/5);
					while (sensor_inp(MASK1_1) == 0x00){}
					motor( maxSpeed/5, maxSpeed);
					timer(100);
					direction = 0; 
					led_out(0x0);
				}
				else if(direction == 3){
					motor( maxSpeed/5, maxSpeed);
					while (sensor_inp(MASK1_1) == 0x00){}
					motor( maxSpeed, maxSpeed/5);
					timer(100);
					direction = 0; 
					led_out(0x0);
				}
			}
			if(check_rightline()){
				led_out(0x3);
				direction = 0;
				timer(50);
				motor(0,0);
				motor( maxSpeed, -maxSpeed);
				timer(turnTimeout);
				led_out(0x0);
			
			}
			else if (check_leftline()){
				led_out(0x3);
				direction = 0;
				timer(50);
				motor(0,0);
				motor( -maxSpeed, maxSpeed);
				timer(turnTimeout);
				led_out(0x0);			
			}
		}
		currentError = getSensorError();
		calculatePID();
		leftSpeed=initialSpeed + pidOut;
		rightSpeed=initialSpeed - pidOut;
		
		if ( leftSpeed > 100){leftSpeed = maxSpeed;}
		else if ( leftSpeed < -100){leftSpeed =-1*maxSpeed;}
		
		if ( rightSpeed <-100){rightSpeed = -1*maxSpeed;}
		else if ( rightSpeed >100){rightSpeed =maxSpeed;}
			
		motor( leftSpeed, rightSpeed);
    }
}

/***********************************************************************/
/* RX62T Initialization                                                */
/***********************************************************************/
void init(void)
{
    // System Clock
    SYSTEM.SCKCR.BIT.ICK = 0;               //12.288*8=98.304MHz
    SYSTEM.SCKCR.BIT.PCK = 1;               //12.288*4=49.152MHz

    // Port I/O Settings
    PORT1.DDR.BYTE = 0x03;                  //P10:LED2 in motor drive board

    PORT2.DR.BYTE  = 0x08;
    PORT2.DDR.BYTE = 0x1b;                  //P24:SDCARD_CLK(o)
                                            //P23:SDCARD_DI(o)
                                            //P22:SDCARD_DO(i)
                                            //CN:P21-P20
    PORT3.DR.BYTE  = 0x01;
    PORT3.DDR.BYTE = 0x0f;                  //CN:P33-P31
                                            //P30:SDCARD_CS(o)
    //PORT4:input                           //sensor input
    //PORT5:input
    //PORT6:input

    PORT7.DDR.BYTE = 0x7e;                  //P76:LED3 in motor drive board
                                            //P75:forward reverse signal(right motor)
                                            //P74:forward reverse signal(left motor)
                                            //P73:PWM(right motor)
                                            //P72:PWM(left motor)
                                            //P71:PWM(servo motor)
                                            //P70:Push-button in motor drive board
    PORT8.DDR.BYTE = 0x07;                  //CN:P82-P80
    PORT9.DDR.BYTE = 0x7f;                  //CN:P96-P90
    PORTA.DR.BYTE  = 0x0f;                  //CN:PA5-PA4
                                            //PA3:LED3(o)
                                            //PA2:LED2(o)
                                            //PA1:LED1(o)
                                            //PA0:LED0(o)
    PORTA.DDR.BYTE = 0x3f;                  //CN:PA5-PA0
    PORTB.DDR.BYTE = 0xff;                  //CN:PB7-PB0
    PORTD.DDR.BYTE = 0x0f;                  //PD7:TRST#(i)
                                            //PD5:TDI(i)
                                            //PD4:TCK(i)
                                            //PD3:TDO(o)
                                            //CN:PD2-PD0
    PORTE.DDR.BYTE = 0x1b;                  //PE5:SW(i)
                                            //CN:PE4-PE0

    // Compare match timer
    MSTP_CMT0 = 0;                          //CMT Release module stop state
    MSTP_CMT2 = 0;                          //CMT Release module stop state

    ICU.IPR[0x04].BYTE  = 0x0f;             //CMT0_CMI0 Priority of interrupts
    ICU.IER[0x03].BIT.IEN4 = 1;             //CMT0_CMI0 Permission for interrupt
    CMT.CMSTR0.WORD     = 0x0000;           //CMT0,CMT1 Stop counting
    CMT0.CMCR.WORD      = 0x00C3;           //PCLK/512
    CMT0.CMCNT          = 0;
    CMT0.CMCOR          = 96;               //1ms/(1/(49.152MHz/512))
    CMT.CMSTR0.WORD     = 0x0003;           //CMT0,CMT1 Start counting

    // MTU3_3 MTU3_4 PWM mode synchronized by RESET
    MSTP_MTU            = 0;                //Release module stop state
    MTU.TSTRA.BYTE      = 0x00;             //MTU Stop counting

    MTU3.TCR.BYTE   = 0x23;                 //ILCK/64(651.04ns)
    MTU3.TCNT = MTU4.TCNT = 0;              //MTU3,MTU4TCNT clear
    MTU3.TGRA = MTU3.TGRC = PWM_CYCLE;      //cycle(16ms)
    MTU3.TGRB = MTU3.TGRD = SERVO_CENTER;   //PWM(servo motor)
    MTU4.TGRA = MTU4.TGRC = 0;              //PWM(left motor)
    MTU4.TGRB = MTU4.TGRD = 0;              //PWM(right motor)
    MTU.TOCR1A.BYTE = 0x40;                 //Selection of output level
    MTU3.TMDR1.BYTE = 0x38;                 //TGRC,TGRD buffer function
                                            //PWM mode synchronized by RESET
    MTU4.TMDR1.BYTE = 0x00;                 //Set 0 to exclude MTU3 effects
    MTU.TOERA.BYTE  = 0xc7;                 //MTU3TGRB,MTU4TGRA,MTU4TGRB permission for output

    MTU.TSTRA.BYTE  = 0x40;                 //MTU0,MTU3 count function
}

/***********************************************************************/
/* Interrupt                                                           */
/***********************************************************************/
#pragma interrupt Excep_CMT0_CMI0(vect=28)
void Excep_CMT0_CMI0(void)
{
    cnt0++;
    cnt1++;
}

/***********************************************************************/
/* Timer unit                                                          */
/* Arguments: timer value, 1 = 1 ms                                    */
/***********************************************************************/
void timer( unsigned long timer_set )
{
    cnt0 = 0;
    while( cnt0 < timer_set );
}

/***********************************************************************/
/* Sensor state detection                                              */
/* Arguments:       masked values                                      */
/* Return values:   sensor value                                       */
/***********************************************************************/
unsigned char sensor_inp( unsigned char mask )
{
    unsigned char sensor;

    sensor  = ~PORT4.PORT.BYTE;

    sensor &= mask;

    return sensor;
}

/***********************************************************************/
/* Sensor state detection without masks                                */
/* Return values:   sensor value                                       */
/***********************************************************************/
unsigned char getSensorValue()
{
    unsigned char sensor;

    sensor  = ~PORT4.PORT.BYTE;
    return sensor;
}
/***********************************************************************/
/* mask the sensor value                                               */
/* Arguments:       sensor value values                                */
/* Arguments:       masked values                                      */
/* Return values:   sensor value with mask                             */
/***********************************************************************/
unsigned char Mask( unsigned char sensor, unsigned char mask )
{
    sensor &= mask;
    return sensor;
}



/***********************************************************************/
/* Read start bar detection sensor                                     */
/* Return values: Sensor value, ON (bar present):1,                    */
/*                              OFF (no bar present):0                 */
/***********************************************************************/
unsigned char startbar_get( void )
{
    unsigned char b;

    b  = ~PORT4.PORT.BIT.B0 & 0x01;     /* Read start bar signal       */

    return  b;
}

/***********************************************************************/
/* Cross line detection processing                                     */
/* Return values: 0: no cross line, 1: cross line                      */
/***********************************************************************/
int check_crossline( void )
{
    unsigned char b;
	right = 0;
	left = 0;
    b = sensor_inp(MASK3_3);
    if( b==0xe7 ) {
        return 1;
    }
    else return 0;
}

/***********************************************************************/
/* Right half line detection processing                                */
/* Return values: 0: not detected, 1: detected                         */
/***********************************************************************/
int check_rightline( void )
{
    unsigned char b;
    b = sensor_inp(MASK4_4);
    if( b==0x1f || b==0x0f) {
		return 1;
    }
	return 0;
}

/***********************************************************************/
/* Left half line detection processing                                 */
/* Return values: 0: not detected, 1: detected                         */
/***********************************************************************/
int check_leftline( void )
{
    unsigned char b;
    b = sensor_inp(MASK4_4);
	if( b==0xf8 || b==0xf0) {
		return 1;
    }
 	return 0;
}

/***********************************************************************/
/* DIP switch value read                                               */
/* Return values: Switch value, 0 to 15                                */
/***********************************************************************/
unsigned char dipsw_get( void )
{
    unsigned char sw,d0,d1,d2,d3;

    d0 = ( PORT6.PORT.BIT.B3 & 0x01 );  /* P63~P60 read                */
    d1 = ( PORT6.PORT.BIT.B2 & 0x01 ) << 1;
    d2 = ( PORT6.PORT.BIT.B1 & 0x01 ) << 2;
    d3 = ( PORT6.PORT.BIT.B0 & 0x01 ) << 3;
    sw = d0 | d1 | d2 | d3;

    return  sw;
}

/***********************************************************************/
/* Push-button in MCU board value read                                 */
/* Return values: Switch value, ON: 1, OFF: 0                          */
/***********************************************************************/
unsigned char buttonsw_get( void )
{
    unsigned char sw;

    sw = ~PORTE.PORT.BIT.B5 & 0x01;     /* Read ports with switches    */

    return  sw;
}

/***********************************************************************/
/* Push-button in motor drive board value read                         */
/* Return values: Switch value, ON: 1, OFF: 0                          */
/***********************************************************************/
unsigned char pushsw_get( void )
{
    unsigned char sw;

    sw  = ~PORT7.PORT.BIT.B0 & 0x01;    /* Read ports with switches    */

    return  sw;
}

/***********************************************************************/
/* LED control in MCU board                                            */
/* Arguments: Switch value, LED0: bit 0, LED1: bit 1. 0: dark, 1: lit  */
/*                                                                     */
/***********************************************************************/
void led_out_m( unsigned char led )
{
    led = ~led;
    PORTA.DR.BYTE = led & 0x0f;
}

/***********************************************************************/
/* LED control in motor drive board                                    */
/* Arguments: Switch value, LED0: bit 0, LED1: bit 1. 0: dark, 1: lit  */
/* Example: 0x3 -> LED1: ON, LED0: ON, 0x2 -> LED1: ON, LED0: OFF      */
/***********************************************************************/
void led_out( unsigned char led )
{
    led = ~led;
    PORT7.DR.BIT.B6 = led & 0x01;
    PORT1.DR.BIT.B0 = ( led >> 1 ) & 0x01;
}

/***********************************************************************/
/* Motor speed control                                                 */
/* Arguments:   Left motor: -100 to 100, Right motor: -100 to 100      */
/*        Here, 0 is stopped, 100 is forward, and -100 is reverse.     */
/* Return value:    None                                               */
/***********************************************************************/
void motor( int accele_l, int accele_r )
{	
	/*
	if(pidOut < 1 && pidOut > -1){
		accele_l = 100;
		accele_r = 100;
	}
	if(accele_l != 0 && accele_r != 0){
		if(currentError == 0) increamentSpeed ++;
		//else if(pidOut < 5 && pidOut > -5) increamentSpeed ++;
		else increamentSpeed = 0;
		if(pidOut < 1 && pidOut > -1){
			accele_l += 20;
			accele_r += 20;
			IMemory = 0;
		}
		else if(currentError > -11 && currentError < 11){
			accele_l += 10;
			accele_r += 10;
			IMemory = 0;
		}
		else if (pidOut < 5 && pidOut> -5){
			accele_l += 10;
			accele_r += 10;
		}
		else if (pidOut < 10 && pidOut> -10){
			accele_l += 5;
			accele_r += 5;
		}
	}*/
	//accele_l += increamentSpeed;
	//accele_r += increamentSpeed;
	if (direction != 0){
		accele_l -= 10;
		accele_r -= 10;
	}
	accele_l=-accele_l;
	if(accele_l > 100)  accele_l = 100;
	if(accele_l < -100) accele_l = -100;
	if(accele_r > 100)  accele_r = 100;
	if(accele_r < -100) accele_r = -100;
    /* Left Motor Control */
    if( accele_l >= 0 ) {
        PORT7.DR.BYTE &= 0xef;
        MTU4.TGRC = (long)( PWM_CYCLE - 1 ) * accele_l / 100;
    } else {
        PORT7.DR.BYTE |= 0x10;
        MTU4.TGRC = (long)( PWM_CYCLE - 1 ) * ( -accele_l ) / 100;
    }

    /* Right Motor Control */
    if( accele_r >= 0 ) {
        PORT7.DR.BYTE &= 0xdf;
        MTU4.TGRD = (long)( PWM_CYCLE - 1 ) * accele_r / 100;
    } else {
        PORT7.DR.BYTE |= 0x20;
        MTU4.TGRD = (long)( PWM_CYCLE - 1 ) * ( -accele_r ) / 100;
    }
}
/***********************************************************************/
/* Servo steering operation                                            */
/* Arguments:   servo operation angle: -90 to 90                       */
/*              -90: 90-degree turn to left, 0: straight,              */
/*               90: 90-degree turn to right                           */
/***********************************************************************/
void handle( int angle )
{
    /* When the servo move from left to right in reverse, replace "-" with "+". */
    MTU3.TGRD = SERVO_CENTER - angle * HANDLE_STEP;
}

/***********************************************************************/
/***********************************************************************/
int getSensorError(void){
	unsigned char sensorValue = getSensorValue();
	
	if(Mask(sensorValue, MASKe0) == 0xe0){
		return -5;
	}
	else if(Mask(sensorValue, MASK70 == 0x70)){
		return -3;
	}
	else if(Mask(sensorValue, MASK38 == 0x38)){
		return -1;
	}
	else if(Mask(sensorValue, MASK1c) == 0x1c){
		return 1;
	}
	else if(Mask(sensorValue, MASK0e) == 0x0e){
		return 3;
	}
	else if(Mask(sensorValue, MASK07) == 0x07){
		return 5;
	}
	else if(Mask(sensorValue, MASKc0) == 0xc0){
		return -6;
	}
	else if(Mask(sensorValue, MASK60) == 0x60){
		return -4;
	}
	else if(Mask(sensorValue, MASK30) == 0x30){
		return -2;
	}
	else if(Mask(sensorValue, MASK0c) == 0x0c){
		return 2;
	}else if(Mask(sensorValue, MASK0c) == 0x06){
		return 4;
	}
	else if(Mask(sensorValue, MASK03) == 0x03){
		return 6;
	}
	else if(Mask(sensorValue, MASK4) == 0x80){
		return -7;
	}
	else if(Mask(sensorValue, MASK3) == 0x40){
		return -5;
	}
	else if(Mask(sensorValue, MASK2) == 0x20){
		return -3;
	}
	else if(Mask(sensorValue, MASK1) == 0x10){
		return -1;
	}
	else if(Mask(sensorValue, MASK_1) == 0x08){
		return 1;
	}
	else if(Mask(sensorValue, MASK_2) == 0x04){
		return 3;
	}
	else if(Mask(sensorValue, MASK_3) == 0x0c){
		return 5;
	}
	else if(Mask(sensorValue, MASK_4) == 0x01){
		return 7;
	}
	else {
		return 0;
	}
	
/*	
	double totalError = 0;
	switch (sensor_inp(MASK4_4))
	{
//////////////////////////////////////////////			3'l� durumlar
		case 0xD0:
			totalError = -5 ;
			break;
			
		case 0x70:
			totalError = -3 ;
			break;
			
		case 0x38:
			totalError = -1;
			break;
			
		case 0x1C:
			totalError = 1 ;
			break;
			
		case 0x0D:
			totalError = 3 ;
			break;
			
		case 0x07:
			totalError = 5 ;
			break;
//////////////////////////////////////////////			2'li durumlar
		case 0xC0:
			totalError = -6 ;
			break;
			
		case 0x60:
			totalError = -4 ;
			break;
			
		case 0x30:
			totalError = -2;
			break;
			
		case 0x0C:
			totalError = 2 ;
			break;
			
		case 0x06:
			totalError = 4 ;
			break;
			
		case 0x03:
			totalError = 6 ;
			break;
//////////////////////////////////////////////			1'li durumlar
		case 0x80:
			totalError = -7 ;
			break;
			
		case 0x40:
			totalError = -5 ;
			break;
			
		case 0x20:
			totalError = -3 ;
			break;
			
		case 0x10:
			totalError = -1 ;
			break;
			
		case 0x08:
			totalError = 1;
			break;
			
		case 0x04:
			totalError = 3 ;
			break;
			
		case 0x02:
			totalError = 5 ;
			break;
			
		case 0x01:
			totalError = 7 ;
			break;
			
		default:
			totalError = 0 ;		//// di�er durumlarda hata olu�mas�n 
			break;
	}		
	
	return totalError;*/

}

void calculatePID(void){
	IMemory += pidI * currentError;
	
/*	if(IMemory > IBound) IMemory = IBound;
	else if(IMemory < -IBound) IMemory =-IBound;
*/	
	integral = pidD * (currentError - previousError);
	
/*	if(integral > DBound) integral = DBound;
	else if(integral < -DBound) integral =-DBound;
*/	
	pidOut = pidP * currentError + IMemory +integral;
	//if(pidOut > 75) pidOut = 75;
	//else if(pidOut < -75) pidOut =-75;
	previousError = currentError;
	
}
/*int pidOut = 0;
int currentError = 0;
int previousError = 0;
double pidP = 0.0;
double pidI = 0.0;
double pidD = 0.0;
double IMemory = 0.0;*/
/***********************************************************************/
/* end of file                                                         */
/***********************************************************************/
