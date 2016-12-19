#include <msp430x552x.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>



void Transmit(uint32_t duty_control_1);//This function transmitts to putty
void Transmit_xbee(uint32_t duty_control_1); //This function transmitts data wirelessly to flight controller
void printString(char * string);
void Recieve(uint32_t *duty_control_1);
void initilize_Serial_MonitorUART(void);
void initilizeXbeeUART(void);


#define FLAG_NONE 0x00000000
#define FLAG_Set_Data_Recieved_From_Terminal 0x00000001
volatile uint8_t flag=FLAG_NONE;


int main(void) {


	uint16_t k= 0;
	uint32_t duty_control_1=0;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	//initilize UART1 peripheral (Xbee)
	initilizeXbeeUART();
	//initilize UART0 peripheral (Serial Monitor)
	initilize_Serial_MonitorUART();

	while(1){

		if(flag==FLAG_Set_Data_Recieved_From_Terminal)
		{
				while ((UCA1STAT & 0b00000001) !=0){};
				duty_control_1= UCA1RXBUF;//Recieve Keyboard ASCII
				Transmit(duty_control_1);//Transmit Keyboard ASCII to Serial Monitor
				Transmit_xbee(duty_control_1);//Transmit to Flight Controller

				if (duty_control_1 =='\r')
				{
				  printString("\r\n");
				}
				flag=FLAG_NONE;
	      }

		 _enable_interrupts();      // interrupts enabled

	}

	return 0;
}
void initilize_Serial_MonitorUART(void)
{
	//Configure the ports 3.3 and 3.4
	P4SEL = BIT4+BIT5; // P3.4,5 = USCI_A0 TXD/RXD

	//Rx(1)= P4.5
	//TX(1)= P4.4
		//0 Reset
		UCA1CTL1 |= UCSWRST;
		//1 control0
		UCA1CTL0 = 0b00000000;
		//2 control1
		UCA1CTL1 |= UCSSEL_2;
		//3 Buad Rate 0
		UCA1BR0 =  6;
		//4 Buad Rate 1
		UCA1BR1 = 0;
		//5 Modulation control
		UCA1MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,
		//6 status Register
		//7 Recieve Buffer
		//8 Transmit Buffer
		//9 Transmit Controler Register
		UCA1IRTCTL &= 0b00000000;
		//10 Recieve Control Register
		UCA1IRRCTL  &= 0b00000000;
		//Enable UART
		UCA1CTL1 &= ~UCSWRST;
		//11 Interrrupt Enable/Disable Register
		UCA1IE |= UCRXIE;               		// Enable USCI_A1 RX interrupt


	}
void initilizeXbeeUART(void)
{
	P3SEL = BIT3+BIT4; // P3.4,5 = USCI_A0 TXD/RXD
	//Rx(1)= P3.4
	//TX(1)= P3.3
		//0 Reset
		UCA0CTL1 |= UCSWRST;
		//1 control0
		UCA0CTL0 = 0b00000000;
		//2 control1
		UCA0CTL1 |= UCSSEL_2; // SMCLK
		//3 Buad Rate 0
		UCA0BR0 =  6;
		//4 Buad Rate 1
		UCA0BR1 = 0;
		//5 Modulation control
		UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,
		//6 status Register
		//7 Recieve Buffer
		//8 Transmit Buffer
		//9 Transmit Controler Register
		UCA0IRTCTL &= 0b00000000;
		//10 Recieve Control Register
		UCA0IRRCTL  &= 0b00000000;
		//11 Interrrupt Enable/Disable Register
		UCA0IE &= 0b00000000;

		//Enable UART
		UCA0CTL1 &= ~UCSWRST;
	}
//Note: Recieve will connect PA0
void Recieve(uint32_t *duty_control_1)//Recieve information from user input from keyboard
{
	while ((UCA1STAT & 0b00000001) !=0){};//waits until the user types something on the keyboard because we need to check there is something to read from

	*duty_control_1=UCA1RXBUF;//input from keyboard deteted at port A, pin A0
}
//Note: Transmit will connect to PA1
void Transmit(uint32_t duty_control_1)//This function transmitts to putty
{
	while((UCA1STAT & 0b00000001) !=0)//check the UART flag register to see if any previous transmission has ended bevause we need to check there is something to send
	{
		//keep looping until the transmission register is empty
	}
	UCA1TXBUF=duty_control_1; //sends the value sent to putty via UART0 port A, pin A1
}
//Note: The Transmit_Xbee will have its pin connected to PB0 to Din pin of the xbee module. The Dout (recieve ) pin is not really used  but can be left conncted (PB1 to Dout)
void Transmit_xbee(uint32_t duty_control_1)//This function transmitts data wirelessly to flight controller
{
	while((UCA0STAT & 0b00000001) !=0)//check the UART flag register to see if any previous transmission has ended bevause we need to check there is something to send
		{
			//keep looping until the transmission register is empty
		}
	UCA0TXBUF=duty_control_1; //transmit the value sent via UART1 port B, pin B1 to Din pin of the of xbee
	}
void printString(char * string)//Help with printing on putty
{
	while(*string)
	{
		Transmit(*(string)++);
	}
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{


	int8_t read=0;
	while ((UCA1STAT & 0b00000001) !=0);
	read=UCA1RXBUF;
	flag=FLAG_Set_Data_Recieved_From_Terminal;

}
