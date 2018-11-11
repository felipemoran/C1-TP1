 /* libtp.H: déclaration de prototype pour LCD et décodage de clavier (4*3) */
/* tp microcontroleur 2000  MF/BS/AD   */

/***************************************/
/* automate de transition clavier      */
/* valeur de retour : char             */
/*     - code brute de la touche       */
/*     - si pas de touche : 0xff       */    
/***************************************/
extern char detect_touche(void);

/***************************************/
/* fonction de decodage clavier        */
/* retourne char 0-9 (acsii)'*', '#'   */
/***************************************/
extern char decode_touche(char code_brute);


/****************************************************************************/

  /**** lecture de l'etat du LCD  ****/
extern void  LCD_stat  (void);

  /**** Attendre de pouvoir realiser une transaction sur le LCD ****/  
extern void  LCD_wait  (void);

  /***** permet d'envoyer des codes de configuration *****/
  /***** voir support de DOC du LCD pour les configs *****/
extern void  LCD_conf  (unsigned char);

  /***** positionner le curseur a une position x,y *****
  /***** pour afficheur 1 ligne 16 caracteres:
		8 premiers caracteres (de gauche) sont vues comme ligne 0
		8 derniers caracteres (de droite) sont vues comme ligne 1
   *****   x = 0 ou 1 numero de ligne
   *****	 y < 8 numero de colonne
   ***** apres execution, le curseur se positionne à (x,y)*****/
extern void  LCD_goto  (char,char);

  /**** fonction d'initialisation de l'afficheur ****/
extern void  init_lcd  (void);

  /** affiche une chaine de caracteres a la position courante sur le LCD**/
extern void  print_lcd (char *str);

extern void LCD_1write(unsigned char);
