#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Servo.h>
#include <MFRC522.h>


///cosas para decidir:
//funcion del buzzer:si hacerlo funcionar cuando da erroneo luego de probar varias veces o hacerlo cada vez que se abra la puerta
//angulo servo

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//pines 0 y 1 wifi
#define ledRojo 2 //asignacion pin led rojo
#define ledVerde 3 //asignacion pin led verde
#define servo 4 //asignacion pin servomotor para rociar alcohol
#define cerradura 5 //asignacion relay conectado a la cerradura electrica
#define laser 6 //asignacion pin laser que indica visualmente la posicion del sensor de temperatura al usuario
#define buzzer 7 //asignacion pin buzzer
#define sensorObstaculo 8 //asignacion pin detector de obstaculos Lm393
//pines rfid
#define RST_PIN  5// Pin de reset
#define SS_PIN  53// Pin de slave select
//pines del 50 al 53 rfid
//el sensor de temperatura MLX90614 esta en los pines 20(sda) y 21(scl) pero no hace falta asignarlo


const int intervaloVerificacion = 2000; //asignacion tiempo de demora si hay verificacon fallida del rfid
//bool puedeEntrar = false; //variable verificacion entrada rfid
//bool saleAlguien = false; //variable salida
//bool temperaturaValida = false; //variable temperatura
Servo rociador; //objeto del tipo servo para poder manejarlo
MFRC522 mfrc522(SS_PIN, RST_PIN); // Objeto mfrc522 enviando pines de slave select y reset
int angulo1 = 180; //variables para guardar la posicion del servo
int angulo2 = 0;
byte LecturaUID[4]; // Array para almacenar el UID leido
byte Usuario1[4]= {0xCA, 0x27, 0x71, 0x7f}; // NUMERO DEL USUARIO 1 
byte Usuario2[4]= {0x8C, 0xEB, 0xD4, 0x35}; // NUMERO DEL USUARIO 2 


//declaracion de funciones
//declaracion de funciones
bool comparaUID(byte lectura[],byte usuario[])
{
  for (byte i=0; i < 4; i++){    // bucle recorre de a un byte por vez el UID
  if(lectura[i] != usuario[i])        // si byte de UID leido es distinto a usuario
    return false;          // retorna falso
  }
  return true;           // si los 4 bytes coinciden retorna verdadero
}

bool verificacionRfid(){
  if (!mfrc522.PICC_IsNewCardPresent()){ 
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()){
    return false;
  }
  Serial.print("UID:");
  for (byte i = 0; i < 4; i++) { // bucle recorre de a un byte por vez el UID
    if (mfrc522.uid.uidByte[i] < 0x10){  // si el byte leido es menor a 0x10
      Serial.print(" 0"); // imprime espacio en blanco y numero cero
    }
    else{ 
      Serial.print(" ");  // imprime un espacio en blanco
    }
    Serial.print(mfrc522.uid.uidByte[i], HEX);    // imprime el byte del UID leido en hexadecimal
    LecturaUID[i]=mfrc522.uid.uidByte[i];     // almacena en array el byte del UID leido      
  }
  if(comparaUID(LecturaUID, Usuario1)){    // llama a funcion comparaUID con Usuario1
    Serial.println("\tVerificado");
    return true;
  }
  else if(comparaUID(LecturaUID, Usuario2)){    // llama a funcion comparaUID con Usuario2
    Serial.println("\tVerificado");
    return true;
  }
  else{  
    Serial.println("\tNo Verificado");
    return false;
  }
  mfrc522.PICC_HaltA();  // detiene comunicacion con tarjeta
}

bool deteccionObstaculo(){
  if(digitalRead(sensorObstaculo)==LOW){//cuando el sensor de obstaculo detecta se pone en low y cuando no hay nada en high
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
  //cerradura
  //laser
  pinMode(buzzer, OUTPUT);
  //rfid
  SPI.begin();        // inicializa bus SPI
  mfrc522.PCD_Init();     // inicializa modulo lector
  mlx.begin(); //iniciacion sensor de temperatura
  Serial.begin(9600);
}

void loop() {
  if(deteccionObstaculo()){
    Serial.println("Abrir puerta salida");
  }
  if(verificacionRfid()){
    if(mlx.readObjectTempC()<38){
      //suena el buzzer
      digitalWrite(ledVerde, HIGH);//se prende el led verde
      delay(500);
      //se muestr la temp
      rociarAlcohol();//se rocia alcohol
      //se abre la cerradura
      digitalWrite(ledVerde, LOW);//se apaga el led verde
    }
    else if(mlx.readObjectTempC()>38){
      //suena el buzzer
      digitalWrite(ledRojo, HIGH);
      //se muestra la temp
      delay(2000);
      digitalWrite(ledRojo, LOW);
    }
  }
  delay(5000);
}


//Serial.print("Ambiente = ");
//Serial.print(mlx.readAmbientTempC());
//Serial.print("ºC\tObjeto = ");
//Serial.print(mlx.readObjectTempC());
//Serial.println("c");