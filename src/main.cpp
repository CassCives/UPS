#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Servo.h>
#include <MFRC522.h>
#include <Buzzer.h>
#include <TM1637.h>

//pines 0 y 1 wifi
#define ledRojo 2 //asignacion pin led rojo
#define ledVerde 3 //asignacion pin led verde
#define servo 4 //asignacion pin servomotor para rociar alcohol
#define cerradura 5 //asignacion relay conectado a la cerradura electrica
#define laser 6 //asignacion pin laser que indica visualmente la posicion del sensor de temperatura al usuario
//buzzer 7
#define sensorObstaculo 8 //asignacion pin detector de obstaculos Lm393
#define CLK 9 //9 y 10 displ
#define DIO 10
//pines rfid RST 5 sda 53 mosi 51 miso 50 sck 52
#define RST_PIN  5
#define SS_PIN  53
//el sensor de temperatura MLX90614 esta en los pines 20(sda) y 21(scl) pero no hace falta asignarlo


TM1637 tm(CLK,DIO);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Servo rociador; //objeto del tipo servo para poder manejarlo
Buzzer buzzer(7);// declaracion por libreria de buzzer
MFRC522 mfrc522(SS_PIN, RST_PIN); // Objeto mfrc522 enviando pines de slave select y reset
int angulo1 = 180; //variables para guardar la posicion del servo
int angulo2 = 0;
byte LecturaUID[4]; // Array para almacenar el UID leido
byte Usuario1[4]= {0xCA, 0x27, 0x71, 0x7f}; // NUMERO DEL USUARIO 1 
byte Usuario2[4]= {0x8C, 0xEB, 0xD4, 0x35}; // NUMERO DEL USUARIO 2 


//declaracion de funciones
//declaracion de funciones
void buzzerCorrecto(){
	buzzer.sound(NOTE_E4, 200);
	buzzer.sound(NOTE_AS4, 250);
}

void buzzerIncorrecto(){
  buzzer.sound(NOTE_AS4, 400);
  delay(200);
  buzzer.sound(NOTE_AS4, 400);
}

bool comparaUID(byte lectura[],byte usuario[])
{
  for (byte i=0; i < 4; i++){ //bucle recorre de a un byte por vez el UID
  if(lectura[i] != usuario[i]) //si byte de UID leido es distinto a usuario
    return false;
  }
  return true;
}

bool verificacionRfid(){
  if (!mfrc522.PICC_IsNewCardPresent()){ 
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()){
    buzzerIncorrecto();
    return false;
  }
  Serial.print("UID:");
  for (byte i = 0; i < 4; i++) { //bucle recorre de a un byte por vez el UID
    if (mfrc522.uid.uidByte[i] < 0x10){  //si el byte leido es menor a 0x10
      Serial.print(" 0"); //imprime espacio en blanco y numero cero
    }
    else{ 
      Serial.print(" "); //imprime un espacio en blanco
    }
    Serial.print(mfrc522.uid.uidByte[i], HEX); //imprime el byte del UID leido en hexadecimal
    LecturaUID[i]=mfrc522.uid.uidByte[i]; //almacena en array el byte del UID leido   
  }
  Serial.print("\nTemperatura = ");
  Serial.print(mlx.readObjectTempC());
  Serial.println("c");   
  if(comparaUID(LecturaUID, Usuario1)){ //comparaUID con Usuario1
    Serial.println("Verificado");
    return true;
  }
  else if(comparaUID(LecturaUID, Usuario2)){ //comparaUID con Usuario2
    Serial.println("Verificado");
    return true;
  }
  else{  
    Serial.println("No Verificado");
    digitalWrite(ledRojo, HIGH);
    buzzerIncorrecto();
    delay(1000);
    digitalWrite(ledRojo, LOW);
    return false;
  }
  mfrc522.PICC_HaltA(); // detiene comunicacion con tarjeta
}

bool deteccionObstaculo(){
  //cuando el sensor de obstaculo detecta se pone en low y cuando no hay nada en high
  if(digitalRead(sensorObstaculo)==LOW){
    return true;
  }
  return false;
}

void rociarAlcohol(){
  for(int i=0;i<=angulo1;i++)
  {
    rociador.write(i);
    delay(15);
  }
  for(int i=180;i>=angulo2;i--)
  {
    rociador.write(i);
    delay(15);
  }
}

//fin declaracion de funciones
//fin declaracion de funciones

void setup() {
  //asignacion pin modes
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  rociador.attach(servo); //se relaciona la variable del servo con su pin
  pinMode(cerradura, OUTPUT);
  //laser
  SPI.begin(); // inicializa bus SPI
  mfrc522.PCD_Init(); // inicializa modulo lector
  mlx.begin(); //iniciacion sensor de temperatura
  tm.init(); //inicio display
  tm.set(4); //brillo display
  Serial.begin(9600);
}

void loop() {
  if(deteccionObstaculo()){
    Serial.println("\nSale alguien");
    digitalWrite(cerradura,HIGH);//se abre la cerradura
    digitalWrite(ledRojo, HIGH);
    buzzerCorrecto();
    delay(3000);
    digitalWrite(cerradura,LOW);//se cierra la cerradura
    digitalWrite(ledRojo, LOW);
  }
  //Serial.print("Ambiente = ");
  //Serial.print(mlx.readAmbientTempC());
  //Serial.print("ºC\tObjeto = ");
  //Serial.print(mlx.readObjectTempC());
  //Serial.println("c");
  if(verificacionRfid()){
    if(mlx.readObjectTempC()<30){
      digitalWrite(ledVerde, HIGH);//se prende el led verde
      buzzerCorrecto();//suena el buzzer
      delay(500);
      tm.display(0,mlx.readObjectTempC()); //se muestra la temp
      rociarAlcohol();//se rocia alcohol
      digitalWrite(cerradura,HIGH);//se abre la cerradura
      delay(3000);
      digitalWrite(cerradura,LOW);//se cierra la cerradura
      digitalWrite(ledVerde, LOW);//se apaga el led verde
      tm.clearDisplay(); //se apaga display
    }
    else if(mlx.readObjectTempC()>30){
      digitalWrite(ledRojo, HIGH);//se prende el led rojo
      buzzerIncorrecto();//suena el buzzer
      tm.display(0,mlx.readObjectTempC());//se muestra la temp
      delay(2000);
      digitalWrite(ledRojo, LOW);//se apaga el led rojo
      tm.clearDisplay(); //se apaga display
    }
  }
}