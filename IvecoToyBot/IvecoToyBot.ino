/*
   IVECO TOY BOT - CONTROLE REMOTO VIA PS4

   Autor: Eng. Renato Augusto

   OBJETIVO: Este programa transforma um caminhão de brinquedo da Iveco em um veículo de controle remoto. 

   Github: https://github.com/renatoaugustii/IvecoToyBot

   ** CONTROLE DE VERSÕES **
   Versão: 1.0 - Por Renato Augusto
   Data: 21/01/2023
   Belo Horizonte - MG / Brasil

   Para dúvidas ou sugestões, entre em contato:
   renato.augusto.correa@outlook.com

*/

// IMPORTAÇÃO DE BIBLIOTECAS PARA O PROJETO
#include <PS4Controller.h>

//CONFIGURAÇÕES E DEFINIÇÕES DE IO
const int StartStop = 13; // Partida e desligamento do motor [PS4.Options()]
const int RightArrow = 2; // Seta para Direita [PS4.Right()]
const int LeftArrow = 4; // Seta para Esquerda [PS4.Left()]
const int Buzzer = 22; // Porta para usar o Buzzer de diferentes formas [PS4.R3()]

//VARIÁVEIS AUXILIARES 
bool Truck_ON= false; // Inicializa em falso a variável que diz se o caminhao está ligado
bool optionsPressed = false; // inicializa a variável que indica se o botao OPTIONS foi pressionado
bool leftArrowState = false; // Variáveis para rastrear o estado das setas
bool rightArrowState = false; // Variáveis para rastrear o estado das setas

// Variáveis para controlar o intervalo de piscagem das setas
unsigned long lastBlinkTimeLeft = 0;
unsigned long lastBlinkTimeRight = 0;
const unsigned long blinkInterval = 350; // Intervalo de 350ms


//CONFIGURAÇÕES 
void setup() {
  // Define Serial Begin com taxa de 115200
  Serial.begin(115200);

  // DEFININDO AS PORTAS COMO SAIDA
  pinMode(StartStop, OUTPUT);  // Liga ou Desliga o Caminhão e suas funcionalidades
  pinMode(RightArrow, OUTPUT);  // Liga ou Desliga Seta para Direita
  pinMode(LeftArrow , OUTPUT);  // Liga ou Desliga Seta para Esquerda
  pinMode(Buzzer, OUTPUT);  // Buzzer - Buzina, Ré, Conexão... Diversas funcionalidades

 // MAC ADDRESS do meu controle de PS4
  PS4.begin("00:e0:4c:bd:44:03");

  // Aguarda até que a conexão seja bem sucedida, caso contrário o código permanecerá no While.
  while (!PS4.isConnected()){
    Serial.println("Aguardando conexão....");
    delay(200);
  }

  // Emite um bip ao ligar o receptor -  Usado para saber que esta tudo certo entre o controle e o ESP-32
  tone(Buzzer, 700, 100);
  delay(100);
  tone(Buzzer, 1300, 100);
  delay(500);
  tone(Buzzer, 700, 100);
  delay(100);
  tone(Buzzer, 1300, 100);
  delay(100);
  noTone(Buzzer);
}

void loop() {


 // Partida (Options) - Tudo no código fica OFF se TRUCK_ON = False
    if (PS4.Options()) {
      if (!optionsPressed) {
        Truck_ON = !Truck_ON;
        toggleState(StartStop);
        Serial.println("Partida Toggled");
        optionsPressed = true;
        digitalWrite(StartStop, Truck_ON ? HIGH : LOW); // Define a saída física conforme o estado atual de Truck_ON
      }
    } else {
      optionsPressed = false;
    }

    // Código dependente do Truck_ON
    if (Truck_ON){
       
      // Tratativa para Buzina
      if (PS4.R3()){
          tone(Buzzer, 200, 50); // Frequencia de 200Hz e tempo de 50ms 
      }

// Seta para Direita (Right)
    if (PS4.Right()) {
      if (millis() - lastBlinkTimeRight >= blinkInterval) {
        toggleArrows(RightArrow, rightArrowState);
        lastBlinkTimeRight = millis();
      }
    }

    // Seta para Esquerda (Left)
    if (PS4.Left()) {
      if (millis() - lastBlinkTimeLeft >= blinkInterval) {
        toggleArrows(LeftArrow, leftArrowState);
        lastBlinkTimeLeft = millis();
      }
    }
    }

 delay(50);

}

// Função para alternar o estado da saída
void toggleArrows(int pin, bool &state) {
  state = !state;
  digitalWrite(pin, state ? HIGH : LOW);
}


// Função para alternar o estado da saída
void toggleState(int pin) {
  if (digitalRead(pin) == HIGH) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
}
