#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7	Serial clock out (SCLK)
// pin 6	Serial data out (DIN)
// pin 5	Data/Command select (D/C)
// pin 4	LCD chip select (CS)
// pin 3	LCD reset (RST)
//zapojeni - zábavná tyčka
// pin A4	osa X (svisla)
// pin A5	osa Y (vodorovna)
// pin 13	switch
//kontrast nastavit zde
int kontrast=23;
//kvůli praktičtějšímu držení na mém joysticku jsou osy vyměněny
int axeXPin=A5;
int axeYPin=A4;
//počáteční rychlost počet dárků a level, vše si později hráč vybere
int rychlost=2;
int pocetDarku=3;
int level=1;
String levely[]={"level 1","level 2","level 3"};
boolean barva = BLACK;

//String mapa = {
//{}
//  {40, 40, 200, 40, 0, 80, 160, 80, 40, }
//  {}
//};

const uint8_t PROGMEM ctverec[] =
{ B11101110,
  B00111000,
  B11111110,
  B10010010,
  B10010010,
  B10010010,
  B10010010,
  B11111110,
};
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);


void mapaDva(int px, int py, boolean b){
  int ypos;
  int xpos;
  for (int i = 0; i < 2; i++){
    for (int j = 0; j < 2; j++){
      ypos = i + j + 40+py;
      xpos = (2-i)%2*40+py;
      display.drawLine(xpos, ypos, xpos + 160, ypos, b);
    }
  }
}
void vypisPopis(String (popis)){
	display.setTextSize(1);
	display.setTextColor(BLACK);
	display.setCursor(0, 0);
	display.print("vyber ");
	display.println(popis);
}
//funkce pro vyber menu
int menuCislo (String popis, int cislo, int limit_1, int limit_2){
	display.clearDisplay();
	vypisPopis(popis);
	//bude zvěčovat/zmenčovat číslo dokud nepohnu joystickem doprava(switch mi nefungoval)
	while(analogRead(axeXPin)<700){
		//smaže číslo
		display.setTextColor(WHITE);
		display.setCursor(35, 20);
		display.println(cislo);
		//zkontroluje změnu(větší)
		if(analogRead(axeYPin)>700){
			if(cislo<limit_2){
				cislo+=1;
			}
		}
		//(menší)
		else if(analogRead(axeYPin)<300){
			if(cislo>limit_1){
				cislo-=1;
			}
		}
		//(stejné)
		else{}
		//napíše číslo a počká
		display.setTextSize(1);
		display.setTextColor(BLACK);
		display.setCursor(35, 20);
		display.println(cislo);
		display.display();
		delay(300);
	}
	//na konci smaže display a vrátí nastavenou hodnotu
	display.clearDisplay();
	display.display();
	return cislo;
}

int menuVyber (String popis, int pocet){
	//u této funkce pracuji přímo s levely, protože pole popisů jednotlivých levelů nešlo nahrát
	display.clearDisplay();
	vypisPopis(popis);
	//nastavim cislo na 99, abych se mohl pohybovat v menu nahorů i dolů bez nutnosti řešit záporné číslo
	int cislo=99;
	//bude vybírat číslo dokud nepohnu joystickem doprava (switch mi nefungoval)
	while(analogRead(axeXPin)<700){
		//na zacatku znovu vypíše popis aby stačilo na konci display jen vyčistit
		vypisPopis(popis);
		//podle joysticku zvětší/zmenší číslo a tím změní jeho zbytek po dělení poctem f
		if(analogRead(axeYPin)>700){
			cislo-=1;
		}
		else if(analogRead(axeYPin)<300){
			cislo+=1;
		}
		else{}
		//vypíše každý level, ten právě vybraný inverzně
		for(int i=0; i<pocet; i++){
			//v závislosti na i nastaví výšku ve které bude psát
			display.setCursor(0, 10*(i+1));
			display.setTextSize(1);
			// v závislosti na i nastaví inverzi a na konci nastaví opět normální text
			if (i==cislo%pocet){
				display.setTextColor(WHITE, BLACK);
			}
			else{
				display.setTextColor(BLACK, WHITE);
			}
			display.println(levely[i]);
			display.setTextColor(BLACK, WHITE);
		}
		display.display();
		display.clearDisplay();
		delay(300);
	}
	display.clearDisplay();
	display.display();
	//na konci vrátí level
	return cislo%pocet;
}

//void tiskHra(){
//}

void hra(int r, int pD, int l){
  int skore = 0;
  int posunutiX = 0;
  int posunutiY = 0;
  while (skore < 25) {
    barva = WHITE;
    //Napise skore
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.setTextColor(barva);
    display.print(skore);
    mapaDva(0 , posunutiY, barva);

    delay(100*r);
    if (analogRead(axeXPin)>300){
      skore+=1;
      posunutiX += (analogRead(A5) - 524) * r / 300;
      posunutiY += (analogRead(A4) - 524) * (-r) / 300;
    }

    barva = BLACK;
    display.setCursor(2, 2);
    display.setTextColor(barva);
    display.print(skore);
    display.display();
  }
}
void setup(){
	Serial.begin(9600);
	display.begin();
	display.setContrast(kontrast);
	display.clearDisplay();
}
void loop(){
	rychlost=menuCislo("rychlost", 2, 1, 5);
	delay(500);
	pocetDarku=menuCislo("pocet darku", 1, 3, 10);
	delay(500);
	level=menuVyber("level", 3);
	delay(500);
	hra(rychlost, pocetDarku, menuVyber);
}
