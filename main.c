#include "lpc111x.h"
#include "at24c32.h"
#include "ds1307.h"
#include "i2c.h"
#include "serial.h"
DS1307Date TheDate;
void delay(int dly)
{
    while(dly--);
}
void WakeInterrupt()
{
    GPIO0IC = BIT8;  // clear the pending interrupt for GPIO0_8 
    ICPR = BIT8;    // clear the interrupt
    STARTRSRP0CLR = BIT8; // clear start up signal
    
  // Turn on clock for UART
    SYSAHBCLKCTRL |= BIT12;
    SYSAHBCLKCTRL |= BIT5; // enable I2C	
    GPIO0DATA ^= BIT7; // Blink the LED
}
void lp_ports()
{
    // turn off all pull-up resistors (except for reset)
    IOCON_PIO2_6 &= ~BIT4;
    IOCON_PIO2_0 &= ~BIT4;
    IOCON_PIO0_1 &= ~BIT4;
    IOCON_PIO1_8 &= ~BIT4;
    IOCON_PIO0_2 &= ~BIT4;    
    IOCON_PIO2_7 &= ~BIT4;
    IOCON_PIO2_8 &= ~BIT4;
    IOCON_PIO2_1 &= ~BIT4;
    IOCON_PIO0_3 &= ~BIT4;
    IOCON_PIO0_4 &= ~BIT4;
    IOCON_PIO0_5 &= ~BIT4;
    IOCON_PIO1_9 &= ~BIT4;
    IOCON_PIO3_4 &= ~BIT4;
    IOCON_PIO2_4 &= ~BIT4;
    IOCON_PIO2_5 &= ~BIT4;
    IOCON_PIO3_5 &= ~BIT4;
    IOCON_PIO0_6 &= ~BIT4;
    IOCON_PIO0_7 &= ~BIT4;
    IOCON_PIO2_9 &= ~BIT4;
    IOCON_PIO2_10 &= ~BIT4;
    IOCON_PIO2_2 &= ~BIT4;
    IOCON_PIO0_8 &= ~BIT4;
    IOCON_PIO0_9 &= ~BIT4;
    IOCON_SWCLK_PIO0_10 &= ~BIT4;
    IOCON_PIO1_10 &= ~BIT4;
    IOCON_PIO2_11 &= ~BIT4;
    IOCON_R_PIO0_11 &= ~BIT4;
    IOCON_R_PIO1_0 &= ~BIT4;
    IOCON_R_PIO1_1 &= ~BIT4;
    IOCON_R_PIO1_2 &= ~BIT4;
    IOCON_PIO3_0 &= ~BIT4;
    IOCON_PIO3_1 &= ~BIT4;
    IOCON_PIO2_3 &= ~BIT4;
    IOCON_SWDIO_PIO1_3 &= ~BIT4;
    IOCON_PIO1_4 &= ~BIT4;
    IOCON_PIO1_11 &= ~BIT4;
    IOCON_PIO3_2 &= ~BIT4;
    IOCON_PIO1_5 &= ~BIT4;
    IOCON_PIO1_6 &= ~BIT4;
    IOCON_PIO1_7 &= ~BIT4;
    IOCON_PIO3_3 &= ~BIT4;
    GPIO0DIR = 0xfff;
    GPIO0DATA = 0;
 /*   GPIO1DIR = 0xfff;
    GPIO0DATA = 0;
    GPIO2DIR = 0xfff;
    GPIO2DATA = 0;
  */
    
}
void enter_lpmode()
{
    // Measured current drops to 67 microamps during deep sleep 
    PDSLEEPCFG = 0x00018ff; // disable BOD and WDT during deep sleep (made a HUGE difference)
    SYSAHBCLKCTRL &= ~BIT12;  // Turn off clock for UART        
    SYSAHBCLKCTRL &= ~(BIT5); // disable I2C	
    MAINCLKSEL = 0; // Use IRC as main clock
    MAINCLKUEN = 1; // Inform core of clock update
    sleep();        // enter deep sleep
}
void leave_lpmode()
{
    MAINCLKSEL = 3; // Use PLL as main clock
    MAINCLKUEN = 1; // Inform core of clock update
    SYSAHBCLKCTRL |= BIT12; // Turn on clock for UART
    SYSAHBCLKCTRL |= BIT5;  // enable I2C	
}
int main()
{
    
    //lp_ports();
    // Configure PIO0_7 as an output for an LED
    IOCON_PIO0_7 = 0; // Turn off pull-up resistor
    GPIO0DIR |= BIT7; // make PIO_7 an output
    GPIO0DATA = 0;    // output low to begin with
    
    // initialize the peripherals
    DS1307Init();
    AT24C32Init();
    initUART();
    enable_interrupts();
    // set up for deep sleep mode
    GPIO0DIR &= ~BIT8;  // make bit 8 an input
    PDAWAKECFG &=~BIT7; // want PLL powered when coming out of deep sleep
    GPIO0IC = BIT8;     // clear the pending interrupt for GPIO0_8 
    ICPR = BIT8;    // clear any pending interrupts in NVIC
    STARTRSRP0CLR = BIT8; // clear start up signal
    ISER = BIT8;    // enable PIO0 interrupts in NVIC
    GPIO0IE |= BIT8;      // enable interrupts on GPIO0_8     
    STARTERP0 |= BIT8;    // enable start signal on GPIO0_8
    SCR |= BIT2;          // enable deep sleep
    

    // Additional power trimming
    SYSAHBCLKCTRL &= ~(BIT13 | BIT11); // turn off ADC and SSP0 to save power
    SYSAHBCLKCTRL &= ~(BIT16); // disable IOCON as we are finished configuring i/o
    // set an initial date
    TheDate.Second = 0;
    TheDate.Minute = 0x59;
    TheDate.Hour = 0x23;
    TheDate.Day = 2;
    TheDate.Date = 0x22;
    TheDate.Month = 3;
    TheDate.Year = 0x16;
    DS1307SetDate(&TheDate);
    while(1)
    {
       
        DS1307GetDate(&TheDate);
        printByte(TheDate.Second);printString("\r\n");  // output second count over uart
        UART_TXDrain();                               // Wait for TX to drain (and interrupts stop)                
        enter_lpmode();
        leave_lpmode();
    }
}
