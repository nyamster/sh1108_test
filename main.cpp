#include <iostream>
#include <errno.h>
#include <wiringPiSPI.h>
#include  <wiringPi.h>
#include <unistd.h>
#include "def.h"

#define RESET 0
#define DATA_CMD 2
using namespace std;

#define uint unsigned int
#define uchar unsigned char

//Resolution
uchar	 SIZE;
#define Max_Column	128
#define Max_Row		160

// channel is the wiringPi name for the chip select (or chip enable) pin.
// Set this to 0 or 1, depending on how it's connected.
static const int CHANNEL = 0;

unsigned char Contrast=0xff;

void Write_Instruction(uint8_t byte)
{
    digitalWrite(DATA_CMD, LOW);
    wiringPiSPIDataRW(CHANNEL, &byte, 1);
}

void Write_Data(uint8_t byte)
{
    digitalWrite(DATA_CMD, HIGH);
    wiringPiSPIDataRW(CHANNEL, &byte, 1);
}



void init_program()
{

pinMode (RESET, OUTPUT);
pinMode (DATA_CMD, OUTPUT);
digitalWrite(RESET, HIGH);
delay(100);
digitalWrite(RESET, LOW);
delay(100);
digitalWrite(RESET, HIGH);
delay(500);

 	Write_Instruction(0xae);//Display OFF(sleep mode)
 	Write_Instruction(0x81);//Set Contrast Control
 	Write_Instruction(Contrast);
 	Write_Instruction(0xa0);//Set Segment Re-map
 	Write_Instruction(0xa4);//Set Entire Display OFF/ON
 	Write_Instruction(0xa6);//Set Normal/Reverse Display
 	Write_Instruction(0xa9);//Display Resolution Control
 	Write_Instruction(0x02);
 	Write_Instruction(0xad);//DC-DC Control Mode Set
 	Write_Instruction(0x80);
 	Write_Instruction(0xc0);//Set COM Output Scan Direction
 	Write_Instruction(0xd5);//Set Display Clock Divide Ratio/Oscillator Frequency
 	Write_Instruction(0x40);
 	Write_Instruction(0xd9);//Dis-charge/Pre-charge Period Mode Set
 	Write_Instruction(0x2f);
 	Write_Instruction(0xdb);//Set VCOMH Deselect Level
 	Write_Instruction(0x3f);
 	Write_Instruction(0x20);//Page addressing mode
 	Write_Instruction(0xdc);//VSEGM Deselect Level Mode Set
 	Write_Instruction(0x35);
 	Write_Instruction(0x30);//Set Discharge VSL Level
 	Write_Instruction(0xaf);//Set Display ON//Clear_screen();
}

void Set_Contrast_Control_Register(unsigned char mod)
{
    Write_Instruction(0x81);
	Write_Instruction(mod);
	return;
}


// Set page address 0~15
void Set_Page_Address(unsigned char add)
{
    Write_Instruction(0xb0);
    Write_Instruction(add);
	return;
}

void Set_Column_Address(unsigned char add)
{ 	  add=add+16;
    Write_Instruction((0x10|(add>>4)));
	Write_Instruction((0x0f&add));
	return;
}



void Set_Pos(unsigned char x, unsigned char y) 
{ 
 	  x=x+16;
	Write_Instruction(0xb0);
	Write_Instruction(y);
	Write_Instruction(((x&0xf0)>>4)|0x10);
	Write_Instruction((x&0x0f)); 
} 

void ClearScreen(void)
{
    unsigned char i,j;
	for(i=0;i<20;i++)
	{
	Set_Page_Address(i);
    Set_Column_Address(0x00);
        for(j=0;j<128;j++)
		{
		    Write_Data(0x00);
		}
	}
    return;
}

// ShowChar   size: 16/8 
void ShowChar(unsigned char x,unsigned char y,unsigned char chr)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';			
		if(x>Max_Column-1){x=0;if(SIZE==16)y+=2;else y+=1;}
		if(SIZE ==16)
			{
			Set_Pos(x,y);	
			for(i=0;i<8;i++)
			Write_Data(F8X16[c*16+i]);
			Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			Write_Data(F8X16[c*16+i+8]);
			}
			else {	
				Set_Pos(x,y);
				for(i=0;i<6;i++)
				Write_Data(F6x8[c][i]);
				
			}
}

unsigned int pow(unsigned char m,unsigned char n)
{
	unsigned int result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//Show  number	 		  
void ShowNum(unsigned char x,unsigned char y,unsigned int num,unsigned char len,unsigned char size2)
{         	
	unsigned char t,temp;
	unsigned char enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				ShowChar(x+(size2/2)*t,y,' ');
				continue;
			}else enshow=1; 
		 	 
		}
	 	ShowChar(x+(size2/2)*t,y,temp+'0'); 
	}
} 
//ShowString
void ShowString(unsigned char x,unsigned char y,unsigned char *chr)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		ShowChar(x,y,chr[j]);
			x+=8;
		if(x>120){x=0; if(SIZE==16)y+=2;else y+=1;}
			j++;
	}
}
//ShowCHinese
void ShowCHinese(unsigned char x,unsigned char y,unsigned char no)
{      			    
	unsigned char t;
	Set_Pos(x,y);	
    for(t=0;t<16;t++)
		{
				Write_Data(Hzk[32*no+t]);
			
        }	
		Set_Pos(x,y+1);	
    for(t=16;t<32;t++)
		{	
				Write_Data(Hzk[32*no+t]);
			
        }					
}

void DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
	    {      
	    	Write_Data(BMP[j++]);	    	
	    }
	}
} 


void Show(unsigned char d1,unsigned char d2)
{
    unsigned char i,j;
	for(i=0;i<20;i++)
	{
	Set_Page_Address(i);
    Set_Column_Address(0x00);
        for(j=0;j<64;j++)
		{
		    Write_Data(d1);
			Write_Data(d2);
		}
	}
    return;
}


void Show_line_dot()
{
   Show(0x55,0xaa);
 	delay(1000);
   Show(0xaa,0x55);
 	delay(1000);

   Show(0x55,0x55);
 	delay(1000);
   Show(0xaa,0xaa);
 	delay(1000);

   Show(0xff,0x00);
 	delay(1000);
   Show(0x00,0xff);
 	delay(1000);

    return;
}

int main()
{
   int fd, result;
   unsigned char buffer[100];

   cout << "Initializing" << endl ;

   // Configure the interface.
   // CHANNEL insicates chip select,
   // 500000 indicates bus speed.
   wiringPiSetup () ;
   fd = wiringPiSPISetup(CHANNEL, 500000);

   cout << "Init result: " << fd << endl;

   init_program();
//Set_Contrast_Control_Register(0x0);

   ClearScreen();

		ShowCHinese(8,0,0);
		ShowCHinese(24,0,1);
		ShowCHinese(40,0,2);
		ShowCHinese(56,0,3);
		ShowCHinese(8,2,4);
		ShowCHinese(24,2,5);  
		ShowCHinese(40,2,6); 
 		ShowCHinese(56,2,7); 

   SIZE=16;
   ShowString(4,4,(unsigned char*)"Slava velikomy Xi");
   SIZE=8;
//   ShowString(0,7,(unsigned char*)"2021/12/30");  
   SIZE=8;
//   ShowString(0,9,(unsigned char*)"Hollw!EASTRISING!"); 
		delay(1000);
//	    Write_Instruction(0xa7);  //Set  inverse  display 	 
	Set_Contrast_Control_Register(0x0);
		delay(5000);
	Set_Contrast_Control_Register(0xff);
//	    Write_Instruction(0xa6);  //Set normal display 
}

