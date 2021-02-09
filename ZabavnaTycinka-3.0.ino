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
int POSUNUTIX = 0;
int POSUNUTIY = 1;

int kontrast=20;
//kvůli praktičtějšímu držení na mém joysticku jsou osy vyměněny
int axeXPin=A5;
int axeYPin=A4;
//počáteční rychlost počet dárků a level, vše si později hráč vybere
int rychlost=2;
int pocetDarku=3;
int level=1;
int mapSize=200;
int level3[] = {35, 0, 100, 80, 100, 20, 40, 140, 40, 20, 80, 60, 80, 20, 120, 100, 120, 20, 140, 60, 140, 20, 160, 60, 160, 20, 180, 80, 180, 40, 60, 80, 60, 100, 20, 180, 20, 100, 80, 180, 80, 120, 100, 160, 100, 120, 120, 140, 120, 140, 60, 160, 60, 140, 140, 160, 140, 160, 40, 180, 40, 160, 120, 180, 120, 160, 160, 180, 160, 40, 0, 40, 20, 80, 0, 80, 20, 20, 20, 20, 80, 60, 20, 60, 40, 100, 20, 100, 80, 180, 20, 180, 40, 160, 40, 160, 60, 80, 60, 80, 100, 120, 60, 120, 100, 180, 60, 180, 120, 20, 120, 20, 140, 80, 120, 80, 200, 100, 120, 100, 180, 120, 120, 120, 200, 160, 120, 160, 140, 140, 140, 140, 180, 180, 140, 180, 180, 20, 160, 20, 180, 160, 160, 160, 200};
String levely[]={"level 1","level 2","level 3"};
boolean barva = BLACK;
int narazeni = 0;

//String mapa = {
//{}
//	{40, 40, 200, 40, 0, 80, 160, 80, 40, }
//	{}
//};

const uint8_t PROGMEM darek[] =
{ B11101110,
	B00111000,
	B11111110,
	B10010010,
	B10010010,
	B10010010,
	B10010010,
	B11111110,
};

const uint8_t PROGMEM stromek[] =
{ B00000001, B10000000,
	B00000010, B01000000,
	B00000101, B00100000,
	B00001000, B01010000,
	B00010011, B10001000,
	B00100100, B00100100,
	B01111000, B00011110,
	B00000101, B00100000,
	B00001000, B00010000,
	B00010000, B11101000,
	B00101011, B00010100,
	B01000100, B10000010,
	B11111110, B01111111,
	B00000010, B01000000,
	B00000010, B01000000,
	B00000011, B11000000
};

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);


void mapaTri(int px, int py, boolean b){
	for (int lineNumber = 0; lineNumber	<= (level3[0]); lineNumber++ ){
		display.drawLine( (level3[ 4 * lineNumber + 1] + px), (level3[ 4 * lineNumber + 2] + py), (level3[ 4 * lineNumber + 3] + px), (level3[ 4 * lineNumber + 4] + py), b);
	}
}
void mapaDva(int px, int py, boolean b){
	int ypos;
	int xpos;
	for (int i = 0; i < 4; i++){
		ypos = ( i + 1 ) * 40 + py;
		xpos = ( ( i + 1 ) % 2 ) * 40 + px;
		display.drawLine(xpos, ypos, xpos + 160, ypos, b);
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
	uint8_t objekt_body[pD][2];
	int skore = 0;
	int posunutiX = - ((mapSize - display.height() )/ 2 - 20);
	int posunutiY = - ((mapSize - display.width() )/ 2 + 15);
	for (uint8_t f = 0; f < pD; f++) {
		objekt_body[f][POSUNUTIX] = random(0, 192);
		objekt_body[f][POSUNUTIY] = random(0, 192);
	}
	while (skore < 25) {
		barva = WHITE;
		//Napise skore
		display.setTextSize(1);
		display.setCursor(2, 2);
		display.setTextColor(barva);
		display.print(skore);
		display.drawRect(0, 0, display.width(), display.height(),	barva);
		display.drawRect(posunutiX, posunutiY, 201, 201, barva);
		if (level == 2){
			mapaTri(posunutiX, posunutiY, barva);
		}
		else if(level == 1){
			mapaDva(posunutiX, posunutiY, barva);
		}
		display.drawBitmap(display.width()/2 - 8, display.height()/2 - 8, stromek, 16, 16, barva);
		for (uint8_t f = 0; f < pD; f++) {
			display.drawBitmap(objekt_body[f][POSUNUTIX] + posunutiX, objekt_body[f][POSUNUTIY] + posunutiY, darek, 8, 8, barva);
		}
		posunutiX += (analogRead(A5) - 524) * (-r) / 300;
		posunutiY += (analogRead(A4) - 524) * (r) / 300;
		if(narazeni > 0){
			posunutiX = - ((mapSize - display.height() )/ 2 - 20);
			posunutiY = - ((mapSize - display.width() )/ 2 + 15);
			skore = skore - 1;
		}
		for (uint8_t f = 0; f < pD; f++) {
			if((display.height()/2 + 8 > objekt_body[f][POSUNUTIY] + posunutiY) && (display.height()/2 - 16 < objekt_body[f][POSUNUTIY] + posunutiY) && (display.width()/2 + 8 > objekt_body[f][POSUNUTIX] + posunutiX) && (display.width()/2 - 16 < objekt_body[f][POSUNUTIX] + posunutiX)){	
				objekt_body[f][POSUNUTIX] = random(0, 192);
				objekt_body[f][POSUNUTIY] = random(0, 192);
				skore = skore + 1;
			}
		}
		narazeni = 0;
		barva = BLACK;
		display.setCursor(2, 2);
		display.setTextColor(barva);
		display.print(skore);
		display.drawRect(0, 0, display.width(), display.height(), barva);
		display.drawRect(posunutiX, posunutiY, 201, 201, barva);
		if (level == 2){
			mapaTri(posunutiX, posunutiY, barva);
		}
		else if(level == 1){
			mapaDva(posunutiX, posunutiY, barva);
		}
		
		for(int i = 0; i<16; i++){
			for(int j = 0; j<16; j++){
				narazeni = narazeni + display.getPixel(display.width()/2 - 8 + i, display.height()/2 - 8 + j);
			}
		}
		for (uint8_t f = 0; f < pD; f++) {
			display.drawBitmap(objekt_body[f][POSUNUTIX] + posunutiX, objekt_body[f][POSUNUTIY] + posunutiY, darek, 8, 8, barva);
		}
		display.drawBitmap(display.width()/2 - 8, display.height()/2 - 8, stromek, 16, 16, barva);
		
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
	hra(rychlost, pocetDarku, level);
}

