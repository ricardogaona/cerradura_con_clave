#include <EEPROM.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define TAM     5
#define CERRAR  90
#define ABRIR   0

enum Puerta {
  CERRADO,
  ABIERTO
};

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

char option;

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LiquidCrystal_I2C lcd(0x27,16,2);
Servo servo;

char clave[5];

const int doorStatusAddr = 128;
const int passwordAddr = 256;
const int firstBootAddr = 512;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  servo.attach(11);

  limpiar_eeprom();

  if(primer_arranque()) {
    lcd.print("Nueva clave: ");
    guardar_clave();
    EEPROM.write(doorStatusAddr, ABIERTO);
    EEPROM.write(firstBootAddr, false);
  }

  if(is_open()){
    servo.write(ABRIR);
  } else {
    servo.write(CERRAR);
  }
  
  menu();

}

void loop() {
  // put your main code here, to run repeatedly:

  option = customKeypad.getKey();

  switch(option) {
    case '1':
      if(is_open()){
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("La puerta ya");
        lcd.setCursor(2, 1);
        lcd.print("esta abierta");
        delay(2000);
      }
      else
        abrir_puerta();
      menu();
      break;
    case '2':
      if(is_open())
        cerrar_puerta();
      else{
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("La puerta ya");
        lcd.setCursor(2, 1);
        lcd.print("esta cerrada");
        delay(2000);
      }
      menu();
      break;
    case '3':
      cambiar_clave();
      menu();
      break;
  }
}

/* Funcion de utileria para limpiar la eeprom */
void limpiar_eeprom(){
  for(int i = 0; i < 1024; ++i) {
    EEPROM[i] = 0;
  }
  EEPROM[firstBootAddr] = true;
}

/* Funcion que despliega el menu en el lcd */
void menu() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1-Abrir 2-Cerrar");
  lcd.setCursor(0,1);
  lcd.println("3-Cambiar clave ");
}

/* Verifica si se trata del primer arranque del sistema */
bool primer_arranque() {
  return EEPROM.read(firstBootAddr) == true;
}

/* Verifica si la puerta esta abierta */
bool is_open() {
  return EEPROM.read(doorStatusAddr) == ABIERTO;
}

/* Limpia la pantalla e imprime un nuevo mensaje */
void lcd_message(String message) {
  lcd.clear();
  lcd.print(message);
}

/* Lee la clave ingresada por el usuario */
void leer_clave(char clave[TAM]) {
  char key;
  lcd.setCursor(5, 1);
  for(int i = 0; i < TAM; ++i) {
    do {
      key = customKeypad.getKey();
    }while(!key);

    clave[i] = key;
    lcd.print(key);
  }
  delay(500);
}

/* Guarda en la memoria EEPROM la clave ingresada por el usuario */
void guardar_clave() {
  char clave[TAM];
  leer_clave(clave);
  EEPROM.put(passwordAddr, clave);
  lcd_message("Clave Guardada!");
  delay(1000);
}

/* Verifica si la clave ingresada por el usuario es la correcta */
bool verificar_clave() {
  char clave[TAM], clave_guardada[TAM];
  leer_clave(clave);
  EEPROM.get(passwordAddr, clave_guardada);

  bool ok = true;
  for(int i = 0; i < TAM; ++i) {
    if(clave[i] != clave_guardada[i])
      ok = false;
  }

  return ok;
}

/* Funcion que permite a un usuario cambiar su clave */
bool cambiar_clave() {
  char clave[TAM];
  bool ok = true;
  
  lcd_message("Ingresar clave:");
  if(verificar_clave()) {
    lcd_message("Nueva clave:");
    leer_clave(clave);
    EEPROM.put(passwordAddr, clave);
    lcd_message("Clave modificada");
  } else {
    lcd_message("Clave incorrecta!");
    ok = false;
  }

  delay(1000);

  return ok;
}

void cerrar_puerta() {
  lcd_message("Cerrando puerta");
  servo.write(CERRAR);
  delay(1000);
  EEPROM.write(doorStatusAddr, CERRADO);
  lcd_message("Puerta cerrada!");
  delay(1000);
}

void abrir_puerta() {
  lcd_message("Ingresar clave:");
  if(verificar_clave()){
    lcd_message("Abriendo puerta");
    servo.write(ABRIR);
    delay(1000);
    lcd_message("Puerta abierta!");
    EEPROM.write(doorStatusAddr, ABIERTO);
  } else {
    lcd_message("Clave incorrecta");
  }
  delay(1000); 
}
