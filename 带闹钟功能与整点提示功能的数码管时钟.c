#include <reg52.h>
sbit dula=P2^0;
sbit wela=P2^1;//宏定义P2.0/P2.1引脚，控制断码和位码
sbit KEY1=P3^0;	//hour十位  
sbit KEY2=P3^1;	//hour个位
sbit KEY3=P3^2;	//minute十位
sbit KEY4=P3^3;	//minute个位
sbit KEY5=P3^4;	//sec十位
sbit KEY6=P3^5;	//sec个位
sbit KEY7=P3^6;
sbit KEY8=P3^7;
sbit SPK=P1^2;
#define dateport P0 //宏定义P0口，用于数码管数据
unsigned char code dofly_DuanMa[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
		                  	         0x77,0x7c,0x39,0x5e,0x79,0x71};
		                  	         //定义断码的选择数组
unsigned char code dofly_WeiMa[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
									 //定义位码的选择数组
unsigned char Buffer[10];//Buffer为缓冲区
// long int counter=0;
long int sec=0;
int minute=0;
int hour=0;
long int timer1; //闹钟计时
bit UpdateTimeFlag;
void delay(int time1);//延时函数
void display();//数码管刷新函数
void Init_Timer0(void);//定时器0配置
void Init_INT0(void);//中断0配置
void Init_Timer1(void);//定时器1配置
void Init_INT1(void);//中断1配置
void Init_Timer2(void);//定时器2配置
void Init_INT2(void);//中断2配置
unsigned int KeyScan(void);//键盘扫描程序
/*
主程序*/
void main()
{	
	Init_Timer0();
	Init_INT0();
	Init_Timer1();
	Init_INT1();
	Init_Timer2();
	while(1){
		/*防止sec、minute、hour超出合理范围*/
		if(sec>=60||sec<0) sec=0; 

		if(minute>=60||minute<0) minute=0;

		if(hour>=25||hour<0) hour=0;

		/*UpdateTimeFlag标注位来刷新Buffer区*/
		if(UpdateTimeFlag=1){
			UpdateTimeFlag=0;
			Buffer[0]=dofly_DuanMa[hour/10%10];
			Buffer[1]=dofly_DuanMa[hour%10];
			Buffer[2]=0x80;
			Buffer[3]=dofly_DuanMa[minute/10%10];
			Buffer[4]=dofly_DuanMa[minute%10];
			Buffer[5]=0x80;
			Buffer[6]=dofly_DuanMa[sec/10%10];
			Buffer[7]=dofly_DuanMa[sec%10];
		}
	}
}
/*
数码管显示程序*/
void display(){
	int i;	
	for(i=0;i<8;i++)
	{	


		dateport=dofly_WeiMa[i];
		wela=1;
		wela=0;

		dateport=Buffer[i];
		dula=1;
		dula=0;

		dateport=0;
		dula=1;
		dula=0;
		
	}
}
/*
定时器0中断服务程序*/
void Timer0_isr(void) interrupt 1 
{
 static int num=0;
 static int i=0;
 TH0=(65536-500)/256;		  //5ms刷新一次
 TL0=(65536-500)%256;
 display();       // 中断刷新数码管
 i++;
 num++;
 if(num>=10)
 {
 	num=0;
 	UpdateTimeFlag=1;
 }
 /*1s钟时 sec++ 记满60 minute++ 记满60 hour++ 记满24 清零*/
 if(i>=2000){
		i=0;
		sec++;
 		if(sec>=59)
 		{
 			sec=0;
 			minute++;
 			if(minute>=59)
 			{
				minute=0;
	 			hour++;
	 			if(hour>=24)
	 			{
		 			hour=0;
	 			} 			
 			}
 		}
	 }
}
/*
定时器1中断服务程序*/
void Timer1_isr(void) interrupt 3
{	
	 int KeyRes;
	 TH1=(65536-5000)/256;		  //5ms刷新一次
	 TL1=(65536-5000)%256;
	 KeyRes=KeyScan();
	 switch(KeyRes)
	 {
	 case 1: sec++;break;
	 case 2: sec--;break;
	 case 3: minute++;break;
     case 4: minute--;break;
	 case 5: hour++;break;
	 case 6: hour--;break;
	 case 7: timer1=sec+minute*60+hour*3600;break;
	 case 8: timer1=0;break;
	 case 9: break;		
	 default:break;
	 }

}
/*定时器2中断服务程序*/
void Timer2_isr(void) interrupt 5
{	
	TF2=0;
	if(minute==0&&sec==0)
	{
		int i;
		for(i=0;i<300;i++){
				SPK=!SPK;
				delay(1);
			}
	}
	if(timer1==sec+minute*60+hour*3600&&timer1!=0)
	{
		int i;
		for(i=0;i<300;i++){
				SPK=!SPK;
				delay(1);
			}
	}
	
}
/*
定时器0配置程序*/
void Init_Timer0(void)
{
 TMOD |= 0x01;	
 TH0=(65536-500)/256;		  //5ms刷新一次
 TL0=(65536-500)%256;
 EA=1;           
 ET0=1;           
 TR0=1;          
}
/*
定时器0配置程序*/
void Init_INT0(void)
{
  EA=1;          //开中断
  EX0=1;         
  IT0=1;        
}
/*
定时器1配置程序*/
void Init_Timer1(void)
{
 TMOD |= 0x01;	
 TH1=(65536-5000)/256;		  //5ms刷新一次
 TL1=(65536-5000)%256;
 EA=1;           
 ET1=1;           
 TR1=1;          
}
/*
定时器1配置程序*/
void Init_INT1(void)
{
  EA=1;          //开中断
  EX1=1;         
  IT1=1;        
}
/*
定时器2配置程序*/
void Init_Timer2(void)
{
 RCAP2H=0x10;
 RCAP2L=0x00;
 TH2=RCAP2H;
 TL2=RCAP2L;
 EA=1;           
 ET2=1;           
 TR2=1;          
}

/*
键盘扫描程序*/
unsigned int KeyScan(void)	//键盘扫描程序
{	
	int i;
	if(KEY6==0){
		for(i=0;i<100;i++);
			if(KEY6==0){
				while(KEY6==0)display();
					return 1; 
			}
	}
	if(KEY5==0){
		for(i=0;i<100;i++);
			if(KEY5==0){
				while(KEY5==0)display();
					return 2; 
			}
	}
	if(KEY4==0){
		for(i=0;i<100;i++);
			if(KEY4==0){
				while(KEY4==0)display();
					return 3; 
			}
	}
	if(KEY3==0){
		for(i=0;i<100;i++);
			if(KEY3==0){
				while(KEY3==0)display();
					return 4; 
			}
	}
	if(KEY2==0){
		for(i=0;i<100;i++);
			if(KEY2==0){
				while(KEY2==0)display();
					return 5; 
			}
	}
	if(KEY1==0){
		for(i=0;i<100;i++);
			if(KEY1==0){
				while(KEY1==0)display();
					return 6; 
			}
	}
	if(KEY7==0){
		for(i=0;i<100;i++);
			if(KEY7==0){
				while(KEY7==0)display();
					return 7;
			}
	}
	if(KEY8==0){
		for(i=0;i<100;i++);
			if(KEY8==0){
				while(KEY8==0)display();
					return 8;
			}
	}
	return 9;	//如果没有键盘按下 返回8
}
void delay(int time1)
{
	int i;
	int j;
	for (i = 0; i < time1; ++i)
	{
		for (j= 0; i < time1; ++i)
		{
		
		}
	}
}