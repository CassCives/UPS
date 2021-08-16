#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Servo.h>


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
//pines del 8 al 12 rfid
//el sensor de temperatura MLX90614 esta en los pines 20(sda) y 21(scl) pero no hace falta asignarlo
#define sensorObstaculo 13 //asignacion pin detector de obstaculos Lm393
#define pulsador 42 //solo de test


const int intervaloVerificacion = 2000; //asignacion tiempo de demora si hay verificacon fallida del rfid
bool puedeEntrar = false; //variable verificacion entrada rfid
bool saleAlguien = false; //variable salida
bool temperaturaValida = false; //variable temperatura
Servo rociador; //objeto del tipo servo para poder manejarlo
int angulo1 = 180; //variables para guardar la posicion del servo
int angulo2 = 0;


//declaracion de funciones
//declaracion de funciones
void enviarDatos(){
  //enviar datos a base de datos 
  }

bool verificacionRfid(){
  //verificar credencial con la base de datos
  return true;
  }

bool medicionTemperatura(){
  Serial.print("Ambiente = ");
  Serial.print(mlx.readAmbientTempC());
  Serial.print("ºC\tObjeto = ");
  Serial.print(mlx.readObjectTempC());
  Serial.println("c");
  delay(500);
  if(mlx.readObjectTempC()<38){
      return true;
  }
  return false;
}

bool deteccionObstaculo(){//cuando el sensor de obstaculo detecta se pone en low y cuando no hay nada en high
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
void setup() {
  //asignacion pin modes
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(pulsador, INPUT);
  rociador.attach(servo); //se relaciona la variable del servo con su pin
  //cerradura
  //laser
  pinMode(buzzer, OUTPUT);
  //rfid
  mlx.begin(); //iniciacion sensor de temperatura
  Serial.begin(9600);
}

void loop() {
  puedeEntrar = verificacionRfid();
  saleAlguien = deteccionObstaculo();
  temperaturaValida = medicionTemperatura();
  
 // if(digitalRead(pulsador)==HIGH){
    if(temperaturaValida){
      digitalWrite(ledVerde, HIGH);
      delay(500);
      rociarAlcohol();
      delay(500);
      //abrir cerradura
      delay(3000);
      digitalWrite(ledVerde, LOW);
    }
    else if(!temperaturaValida){
      digitalWrite(ledRojo, HIGH);
      delay(2000);
      digitalWrite(ledRojo, LOW);
    }
    if(saleAlguien){
      Serial.println("Abrir puerta salida");
    }
    else{
      Serial.println("No sale nadie");
    }
  //}
}