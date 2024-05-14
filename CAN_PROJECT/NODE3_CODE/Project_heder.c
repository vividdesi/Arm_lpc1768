#include<lpc17xx.h>
#include<stdio.h>

#define p LPC_SC
#define n LPC_PINCON
#define ad LPC_ADC
#define u LPC_UART0
#define c LPC_CAN1
#define p1 LPC_GPIO1
#define p0 LPC_GPIO0
#define p2 LPC_GPIO2
#define t LPC_TIM0
#define cf LPC_CANAF
#define cfr LPC_CANAF_RAM
#define g LPC_GPIOINT
#define PW LPC_PWM1

void pll()
{
	p->SCS=(1<<5);
	while ((p->SCS& (1<<6))==0);
	p->CLKSRCSEL=(1<<0);
	p->PLL0CON=(1<<0);
	p->PLL0CFG=(14<<0);
	p->PLL0FEED=0XAA;
	p->PLL0FEED=0X55;
	p->CCLKCFG=(5<<0);
	while ((p->PLL0STAT&(1<<26))==0);
	p->PLL0CON|=(1<<1);
	p->PLL0FEED=0XAA;
	p->PLL0FEED=0X55;		                                                                 
}

void adc_init()
{
    p->PCONP|=(1<<12); //powering up
	n->PINSEL1 =(1<<18);//channel ad0.2	
}

unsigned int sense_data()
{
	unsigned int y;
	ad->ADCR|=(1<<2)|(5<<8)|(1<<21)|(1<<24);//2->channel ad0.2,clk->5,pdn-powerup,24->start
	while ((ad->ADDR2&(1U<<31))==0);//polling method for data done bit
	y =(ad->ADDR2&(0XFFF<<4))>>4;//data moved to y
	return y;
}

void uart_init()
{
	p->PCONP |=(1<<3);	
	n->PINSEL0 |= (1<<4)|(1<<6);
	u->LCR	= (1<<7)|(3<<0);
	u->DLL	= 97;
	u->DLM	= 0;
	u->LCR&=~(1<<7);
}

void hyper_terminal(char *str)
{
	unsigned int i;
	for(i =0;str[i]!='\0';i++)
	{
		while((u->LSR&(1<<5))!=(1<<5));
		u->THR = str[i];
	}			
}

void can_init()
{
	n->PINSEL0|= (1<<0)|(1<<2);
	p->PCONP |= (1<<13);
	c->MOD = 1;
	c->BTR= 0x0007000E;
	c->MOD = 0;
}

void can_tx(int dlc,int data,int rtr,int id)
{
	while((c->SR&(1<<2))!=(1<<2));
	c->TID1 = id;
	c->TDA1 = data;
	c->TDB1 = 0;
	c->TFI1 = (dlc<<16)|(rtr<<30) ;
	c->CMR = (1<<0)|(1<<5);
	while((c->SR&(1<<3))!=(1<<3));
	c->CMR = 0;
}

 void delay_ms1(unsigned int i)
{
	t->PR = 14999;
	t->MCR =(7<<0);	  
	t->MR0 = i;
	t->TCR = (1<<0);
	while((t->IR&(1<<0))!=(1<<0));
	t->IR = (1<<0);
	t->TC = 0x00;
} 	

void led_blink()
{
int i;
for(i=0;i<5;i++)
{
	p1->FIOSET=(1<<29);
	delay_ms1(500);
	p1->FIOCLR=(1<<29);
	delay_ms1(500);
}
}

void delay_ms2(unsigned int i1)
{
int i,j;
for(i=0;i<i1;i++)
for(j=0;j<2000;j++);
}

void buzzer()
{
p0->FIOSET=(1<<19);
delay_ms2(1000);
p0->FIOCLR=(1<<19);
delay_ms2(1000);
}

void DELAY()
{
	int i,j;
	for(i=0;i<100;i++)
	{
	  for(j=0;j<500;j++);
	}
}

void step_motor(int x)
{
	int i;
	for(i=0;i<(x/7.2);i++)
	{
		p2->FIOSET=(1<<10);
		DELAY();
		
		p2->FIOCLR=(1<<10);
		p2->FIOSET=(1<<11);
		DELAY();
		
		p2->FIOCLR=(1<<11);
		p2->FIOSET=(1<<12);
		DELAY();
		
		p2->FIOCLR=(1<<12);
		p2->FIOSET=(1<<13);
		DELAY();

		p2->FIOCLR=(1<<13);
	}
}

/*void dc_pwm(int x)
{	
    n->PINSEL4=(1<<2);
    PW->TCR=(1<<0);//0->enables pwn timer and pwm prescaler counter,2->
	PW->PR=0X0;
	PW->MCR=(1<<1);
	PW->MR0=100;
	PW->MR2=x;
	PW->LER=(1<<0)|(1<<2); //loading new match values
	PW->PCR=(1<<10); //enabling pwm output
} */

void dc_motor( )
{
  p0->FIOSET|=(1<<21);
  p0->FIOCLR|=(1<<22);
  delay_ms2(7000);
  p0->FIOCLR|=(1<<21);
  p0->FIOCLR|=(1<<22);
  delay_ms2(7000);
}

void init_pll()
{
    NVIC_EnableIRQ(EINT3_IRQn);
	p0->FIODIR|=(3<<21)|(1<<19);//P0.19,21,22
    p0->FIOCLR|=(3<<21)|(1<<19);	
	p1->FIODIR|=(1<<29);	//P1.29
    p1->FIOCLR|=(1<<29);
	p2->FIODIR|=(0xf<<10);//p2.10--13	 
	p2->FIOCLR|=(0xf<<10);
	 
	pll();
	uart_init();
	can_init();
	adc_init();
}



