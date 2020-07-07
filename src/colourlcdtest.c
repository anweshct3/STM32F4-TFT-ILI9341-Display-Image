#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/usart.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


void msdelay(const int);
void send_data(int val[]);
void hex_to_arr(unsigned int);
void Write_Command(unsigned int);
void Write_Data_Byte(unsigned int);
void Write_Data_Word(unsigned int);
void Write_Command_Data(unsigned int, unsigned int);
void Lcd_Init(void);
void LCD_clear(void);
void SetXY(unsigned int, unsigned int, unsigned int, unsigned int);
void Paint(unsigned int);
void usart_print_num(unsigned int);
void usart_print_bin(int[]);


//#define DEBUG 1

static void gpio_setup(void)
{

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_180MHZ]);		//PLL multiplier for increasing the system clock/oscillator (?) speed
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_USART2);
	
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);  //Default inbuilt led
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);


	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0); //Pin rd -  B0
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1); //Pin wr -  B1
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5); //Pin rs -  B5
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6); //Pin cs -  B6
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7); //Pin rst - B7

	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);  //Pin 0 - C0
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);  //Pin 1 - C1
	
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2);  //Pin 2 - C2
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);  //Pin 3 - C3
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);  //Pin 4 - C4
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);  //Pin 5 - C5

	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6); //Pin 6 - C6
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);  //Pin 7 - C7
}

static void usart_setup(void)
{
	/* Setup USART2 parameters. */
	usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART2);
}


#define rd_off 		gpio_set(GPIOB, GPIO0)
#define rd_on 		gpio_clear(GPIOB, GPIO0)
#define wr_off 		gpio_set(GPIOB, GPIO1)
#define wr_on 		gpio_clear(GPIOB, GPIO1)
#define rs_on 		gpio_set(GPIOB, GPIO5)
#define rs_off 		gpio_clear(GPIOB, GPIO5)
#define cs_off 		gpio_set(GPIOB, GPIO6)
#define cs_on 		gpio_clear(GPIOB, GPIO6)
#define rst_off 	gpio_set(GPIOB, GPIO7)
#define rst_on 		gpio_clear(GPIOB, GPIO7)


#define X_CONST 240
#define Y_CONST 320


void msdelay(int x)
{
	int j;
	const int z=30000*x;	//technically 45000

	for (j = 0; j < z ; j++)	
		__asm__("nop");
	
}


void usart_print_num(unsigned int x)
{
	
	int dig;
	int len = floor(log10(x));
	
	while(len>=0)
	{
		dig=x/((int)pow(10,len));
		usart_send_blocking(USART2, dig + '0');
		x=x- dig*(int)pow(10,len);
		len--;

	}
	usart_send_blocking(USART2, '\t');
}

void usart_print_bin(int arr[])
{
	
	int c=7;
	int val;
	while(c>=0)
	{
		val=arr[c];
		usart_send_blocking(USART2, val + '0');
		c--;
	}

	usart_send_blocking(USART2, '\t');
	
}




void hex_to_arr(unsigned int x)
{
	int c=0;
	int arr[8];
	while(c<8)
	{
		arr[c]=x&1;
		x=x>>1;
		c++;
	}
	send_data(arr);
}
		


void Write_Command(unsigned int c)
{
	cs_on;
    rs_off;
    rd_off;
    wr_off;

    #ifdef DEBUG
    usart_send_blocking(USART2, 'c');
	usart_send_blocking(USART2, ' ');
	usart_print_num(c);
	#endif

	hex_to_arr(c);
	
	wr_on;
	wr_off;
	cs_off;
	
}

void Write_Data_Word(unsigned int c)
{
	cs_on;
    rs_on;
    rd_off;
    wr_off;

    #ifdef DEBUG
    usart_send_blocking(USART2, 'd');
	usart_send_blocking(USART2, '1');
	usart_send_blocking(USART2, ' ');
	usart_print_num(c>>8);
	#endif

	hex_to_arr(c>>8);
	wr_on;
	wr_off;	


	#ifdef DEBUG
	usart_send_blocking(USART2, 'd');
	usart_send_blocking(USART2, '2');
	usart_send_blocking(USART2, ' ');
	usart_print_num(c);	
	#endif

	hex_to_arr(c);
	wr_on;
	wr_off;
	cs_off;
	
}

void Write_Data_Byte(unsigned int c)
{
	cs_on;
    rs_on;
    rd_off;
    wr_off;

	#ifdef DEBUG
	usart_send_blocking(USART2, 'd');
	usart_send_blocking(USART2, '2');
	usart_send_blocking(USART2, ' ');
	usart_print_num(c);	
	#endif

	hex_to_arr(c);
	wr_on;
	wr_off;
	cs_off;
	
}

void Write_Command_Data(unsigned int cmd, unsigned int dat)
{
	Write_Command(cmd);
	Write_Data_Word(dat);
}

void Lcd_Init()
{
	rst_on;
    msdelay(20);	
	rst_off;
	msdelay(150);
	//rst_on;
	//msdelay(15);

	Write_Command(0x3A);
	Write_Data_Byte(0x55);
	Write_Command(0x11);
	Write_Command(0x29);
	Write_Command(0x36);
	Write_Data_Byte(0x28);


}


void SetXY(unsigned int x0,unsigned int x1,unsigned int y0,unsigned int y1)
{

	Write_Command(0x2A);
	Write_Data_Word(x0);
	Write_Data_Word(x1);
	Write_Command(0x2B);
	Write_Data_Word(y0);
	Write_Data_Word(y1);
	Write_Command(0x2C); 
}



void LCD_clear()
{
    unsigned int i,j;
	SetXY(0,319,0,239);
	#if 1
	for(i=0;i<320;i++)
	{
	    for(j=0;j<240;j++)
		{    
          	Write_Data_Word(0x0000);
		}
	}
	#endif
}

void Paint(unsigned int color)
{
	int i,j,k=1;
	SetXY(0,319,0,239);
	unsigned char *ptr;
	ptr=(unsigned char *)0x20046;
	uint8_t x;
	uint16_t y;

    for(i=0;i<320;i++)
	{
		for (j=0;j<240;j++)
	   	{
	   		#if 1
	   		x=*(ptr+k);
	   		y=(uint16_t)(x * 0x100);
	   		x= *(ptr + (k-1) );
	   		y+=(uint16_t)x;
	   		k+=2;
	   		#endif
         	Write_Data_Word(y);
         	//msdelay(3);
         	
	    }

	 }	
}


void send_data(int val[])
{

	#ifdef DEBUG
	usart_send_blocking(USART2, 'b');
	usart_print_bin(val);
	#endif

	if(val[0])
	gpio_set(GPIOC, GPIO0);
	else
	gpio_clear(GPIOC, GPIO0);


	if(val[1])
	gpio_set(GPIOC, GPIO1);
	else
	gpio_clear(GPIOC, GPIO1);


	if(val[2])
	gpio_set(GPIOC, GPIO2);
	else
	gpio_clear(GPIOC, GPIO2);


	if(val[3])
	gpio_set(GPIOC, GPIO3);
	else
	gpio_clear(GPIOC, GPIO3);


	if(val[4])
	gpio_set(GPIOC, GPIO4);
	else
	gpio_clear(GPIOC, GPIO4);


	if(val[5])
	gpio_set(GPIOC, GPIO5);
	else
	gpio_clear(GPIOC, GPIO5);


	if(val[6])
	gpio_set(GPIOC, GPIO6);
	else
	gpio_clear(GPIOC, GPIO6);


	if(val[7])
	gpio_set(GPIOC, GPIO7);
	else
	gpio_clear(GPIOC, GPIO7);

}




int main(void)
{
	
	gpio_setup();
	usart_setup();
	Lcd_Init();
  	//LCD_clear();
	Paint(0xf1f0);
	//msdelay(2000);
	#if 0
	unsigned char *ptr;
	ptr=(unsigned char *)0x20036;
	uint8_t x;
	uint16_t y;
	x=*(ptr+1);
	y=(uint16_t)(x * 0x100);
	x=*ptr;
	y+=(uint16_t)x;

	#endif

	

	while(1)
	{
		gpio_set(GPIOA, GPIO5);
		msdelay(500);
		gpio_clear(GPIOA, GPIO5);
		msdelay(500);

	}

	return 0;
}


