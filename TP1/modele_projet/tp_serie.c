/************************************************************************
 **	Programme exemple pour le TP serie		       **
 **                                                                    **
 ************************************************************************/
 
#include <reg592.h>                /* registres specifiques au 592 */
#include <stdio.h>  
#include "libtp2.h" 

sbit P1_7_Tx=0x97;
sbit P1_6_Rx=0x96;

xdata unsigned int barreg _at_ 0xfd00; 

/*****************************************/
/* declaration des variables et tableaux */
/*****************************************/

code int TAB[10]={0x001,0x003,0x007,0x00f,0x01f, 0x03f,0x07f,0x0ff, 0x1ff,0x3ff};
bit FLAG=0;
unsigned int i,j, compt,indice;

bit transmit=0;
bit Tx=1,Rx=1;
char etat_Tx=0;
unsigned char logic_count_Tx;
char nb_bits;
unsigned char Var_Tx; /* Tampon de transmission */

unsigned char logic_count_Rx;
char nb_bitR,etat_Rx=0;
bit received_data=0;
unsigned char Var_Rx,Var_loc;



unsigned char touche;
char buff[8];

/*****************************************/
/*  routines automates de TX RX		 */
/*****************************************/













void affiche(){
	barreg=TAB[j];
	j=(j+1)%10;
}

bit tempo2(){
		
		compt++;

		if (compt==10000){
			compt=0;
			return 1;
		}
		else return 0;
}
void Motif(void){

		FLAG=tempo2();
		if (FLAG==1) {
			
			affiche();
			

		}
		
}

/*****************************************/
/*  Interruption du timer0		 */
/*****************************************/

timer0() interrupt 1 {
	P1_7_Tx=Tx;
	Rx=P1_6_Rx;
	Emission();
	Reception();		
}

/***************************************/
/* main program                        */
/***************************************/


void main (void)  {  /* debut du programme apres le startup  */
		  

		  TH0=TL0=0;
		  TMOD=TMOD |2; /* timer 0 en autoload 8 bits */
		  TR0=1; /* lance le timer 0*/
		  ET0=1; /* autorise l'IT du timer 0 */
		  EA=1; /* autorise les ITs  */
  	
	transmit=0;  
	received_data=0;
	init_lcd();

  while (1) {  
      
			touche=detect_touche();

			if(transmit==0) {
		  	    if(touche!=(char)0xff){
				     Var_Tx=decode_touche(touche);
				     transmit=1;
		  	    }
		    }


		/*	if(received_data==1){
				received_data=0;
				
				sprintf(buff,"%u\n", (unsigned int) Var_Rx);
				LCD_goto(0,0);
				print_lcd(buff);
	      		
				printf("touche : %d\n",(unsigned int)Var_Rx );
	      		
	      	}*/
    
			
		  Motif();
  }  		
} 

