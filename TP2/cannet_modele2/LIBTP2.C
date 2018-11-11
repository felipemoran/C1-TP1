  /************************************************************************
 **	Programme exemple pour le TP micro contrôleur		       **
 **  La touche du clavier sera affichée sur la fenêtre SIO du PC
 ************************************************************************/

#include <reg592.h>      /* registres spécifiques au 552 */

/***************************************/
/* automate de transition clavier      */
/* valeur de retour : char                  */
/*     - code brute de la touche          */
/*     - si pas de touche : 0xff	 */    
/***************************************/

char etat_touche=0;
// registre ou se trouve le clavier
// 0 C3 C2 C1 L4 L3 L2 L1
xdata volatile unsigned char regtouche _at_ 0xfd08;
char detect_touche() {
register char lect_clav;
    regtouche=0x0f; /*mettre les lignes à 1*/
    lect_clav=regtouche; 
	lect_clav=regtouche; 
	lect_clav=regtouche;         

   if(etat_touche==0){
	 if(lect_clav!=0x0f) { 
	           regtouche= lect_clav |0xf0;
			   regtouche= lect_clav |0xf0;
			   regtouche= lect_clav |0xf0;
               etat_touche=1;
	           lect_clav=regtouche;
			   lect_clav=regtouche; 
			   lect_clav=regtouche;
               return lect_clav;
           } else{
               return 0xff;
         }
   }

   if(etat_touche==1){
	 if(lect_clav==0x0f) {
/* pas de touche on va a l'etat 0*/
	       etat_touche=0;                         
         }
   }
   return 0xff;
}


/***************************************/
/* fonction de decodage clavier        */
/***************************************/


/* table de translation code -> binaire*/
char tab_t[12]={0x57,0x6e,0x5e,0x3e,
		     0x6d,0x5d,0x3d,
		     0x6b,0x5b,0x3b,
		     0x67,     0x37};

char decode_touche(char code_t) {
char i=0;
    
  while((i<12)&&(tab_t[i]!=code_t)){  
      i++;
  }
  switch(i){
         case 12: i=0xff;break;     
         case 10: i='*';break;
         case 11: i='#';break;
         default:;		 /* code non valide */
  }
  return i; 
}

/**********************************************************************/
/**      FONCTIONS DU LCD                                             */
/**********************************************************************/
/**** adresses I/O pour le LCD               ******
 **** 4 adresses sur le plan memoire externe ******/
xdata unsigned char CTR_LCDW _at_ 0xfD02;
xdata unsigned char CTR_LCDR _at_ 0xfD06;
xdata unsigned char DATA_LCDW  _at_ 0xfD03;
xdata unsigned char DATA_LCDR _at_ 0xfD07;


/* declarations fonctions systeme */
/* ------------------------------ */

/**** lecture de l'etat du LCD  ****/
/**** si bit 7 du retour = 1 alors le LCD est busy ***
 ****		aucune operation n'est realisable : ***/
char LCD_stat()
{
	return(CTR_LCDR);
}
/*****************************************************************/
/**** Attendre de pouvoir realiser une transaction sur le LCD ****/
void LCD_wait() {
	 char i;
	 while((LCD_stat()&0x80)==0x80){ for(i=0;i<10;i++);}
}
/***** permet d'envoyer des codes de configuration *****/
/***** voir support de DOC du LCD pour les configs *****/
void LCD_conf(unsigned char cc)
{ 
	LCD_wait() ;
	CTR_LCDW=cc;
}

/***** positionner le curseur a une position x,y *****
/***** pour afficheur 1 ligne 16 caracteres:
		8 premiers caracteres (de gauche) sont vues comme ligne 0
		8 derniers caracteres (de droite) sont vues comme ligne 1
 *****   x = 0 ou 1 numero de ligne
 *****	 y < 8 numero de colonne
 ***** apres execution, le curseur se positionne à (x,y)*****/
void LCD_goto(char x,char y) {
	LCD_wait();
	LCD_conf(0x80+(y&0x0f)+((x&1)<<6));
}


/**** affiche un caractere a la position courante du curseur ****/
void LCD_1write(unsigned char cc)
{
	LCD_wait();
	DATA_LCDW=cc;
}

/** affiche une chaine de caracteres a la position courante sur le LCD**/
void print_lcd(unsigned char _TXT[]) {
	unsigned char uc=0;
	while((_TXT[uc])!=0) {
		LCD_1write(_TXT[uc++]);
		  }
}
/**** fonction d'initialisation de l'afficheur ****/
void init_lcd(void) {
 //   unsigned int ii;
	/* initialisation de l'afficheur */
	
	LCD_conf(0x38);
	LCD_conf(0x14);			
	LCD_conf(0x01);
	//LCD_conf(0x08);
	LCD_conf(0x06);

	LCD_conf(0x0c);
	LCD_conf(0x02);		
	LCD_conf(0x01);	
	//LCD_conf(0x0c);

//	LCD_conf(0x0d);
//	LCD_conf(0x0d);



	LCD_wait();
}


