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
   Para saber mais detalhes do funcionamento, leia o arquivo README.md nesse mesmo repositório.

*/

// IMPORTAÇÃO DE BIBLIOTECAS PARA O PROJETO
#include <PS4Controller.h>
#include <ESP32Servo.h> 


Servo DirectionServo;  // create servo object to control a servo

//CONFIGURAÇÕES E DEFINIÇÕES DE IO
static const int PinServo = 33; // Definiçao do IO para o servo motor
const int StartStop = 13; // Partida e desligamento do motor [PS4.Options()]
const int RightArrow = 2; // Seta para Direita [PS4.Right()]
const int LeftArrow = 4; // Seta para Esquerda [PS4.Left()]
const int Buzzer = 22; // Porta para usar o Buzzer de diferentes formas [PS4.R3()]
const int PositionLight = 5; // Porta utilizada para o Farolete ou Luzes de posição. [PS4.Cross()]
const int HeadLight = 18; // Porta utilizada para o Farol. [PS4.Cross()]
const int MileLight = 19; // Porta utilizada para o Farol de milha. [PS4.Triangle()]
const int BreakLight = 23; // Porta utilizada para o Freio. [PS4.Square()]
const int ReverseLight = 21; // Porta utilizada para Luz de Ré. [PS4.L2()]
const int ReverseMove = 12; // Porta utilizada para alterar na ponte H Frente/Ré (IO 34/35). 
const int ForwardMove = 14; // Porta utilizada para alterar na ponte H Frente/Ré (IO 34/35).  
const int MovePWM = 27; // Porta usada para controlore de PWM no avanço e ré 

/*******  VARIÁVEIS AUXILIARES   **********/
//CONTROLADORES DE STATUS
bool Truck_ON= false; // Inicializa em falso a variável que diz se o caminhao está ligado
bool PositionLightState = false; // Variáveis para rastrear o estado dos faroletes
bool HeadLightState = false; // Variáveis para rastrear o estado dos farois
bool leftArrowState = false; // Variáveis para rastrear o estado das setas
bool rightArrowState = false; // Variáveis para rastrear o estado das setas
bool alertArrowState = false; // Variáveis para rastrear o estado do alerta
bool mileLightState = false; // Variáveis para rastrear o estado do Farold e Milha
bool ForwardState = false; // Variáveis para rastrear se o caminhao esta andando pra frente
bool ReverseState = false; // Variáveis para rastrear se o caminhao esta andando pra trás

//FLAG DE CONTROLE
bool OptionsFlag = false; // inicializa a variável que indica se o botao OPTIONS foi pressionado
bool RightArrowFlag = false; // Variável para controle do botao pressionado
bool LeftArrowFlag = false; // Variável para controle do botao pressionado
bool CrossFlag = false; // Variável para controle do botao pressionado
bool CircleFlag = false; // Variável para controle do botao pressionado
bool AlertFlag = false; // Variável para controle do botao pressionado
bool TriangleFlag = false; // Variável para controle do botao pressionado

// Variáveis para controlar o intervalo de piscagem das setas
unsigned long lastBlinkTimeLeft = 0; // Guarda o tempo do milli() que esta ligada.
unsigned long lastBlinkTimeRight = 0; // Guarda o tempo do milli() que esta ligada.
unsigned long lastBlinkTimeAlert = 0; // Guarda o tempo do milli() que esta ligada.
const unsigned long blinkInterval = 350; // Intervalo para o pisca da seta

// Variáveis de controle de movimento
int ReverseAcc = 0; // Valor para aceleracao do reverso
int ForwardAcc = 0; // Valor para aceleracao para frente
int DirecionalX = 0; // Variável utilizada para controle de direçao usando Joystick analógico

// Variavel de controle para ruido sonoro da ré
unsigned long lastNoiseTimeReverse = 0; // Guarda o tempo do milli() que esta ligada.

//CONFIGURAÇÕES 
void setup() {
  // Define Serial Begin com taxa de 115200
  Serial.begin(115200);

  //Definindo o pino para o servo
  DirectionServo.attach(PinServo);

  // DEFININDO AS PORTAS COMO SAIDA
  pinMode(StartStop, OUTPUT);  // Liga ou Desliga o Caminhão e suas funcionalidades
  pinMode(RightArrow, OUTPUT);  // Liga ou Desliga Seta para Direita
  pinMode(LeftArrow , OUTPUT);  // Liga ou Desliga Seta para Esquerda
  pinMode(Buzzer, OUTPUT);  // Buzzer - Buzina, Ré, Conexão... Diversas funcionalidades
  pinMode(PositionLight, OUTPUT);  // Liga ou Desliga Faroletes
  pinMode(HeadLight, OUTPUT);  // Liga ou Desliga Faroletes
  pinMode(MileLight, OUTPUT);  // Liga ou Desliga Farol de Milha
  pinMode(BreakLight, OUTPUT);  // Liga ou Desliga Freio
  pinMode(ReverseLight, OUTPUT);  // Liga Luz de Ré
  pinMode(ReverseMove, OUTPUT); // Aciona Ponte H para aceelerar a Ré
  pinMode(ForwardMove, OUTPUT);// Aciona Ponte H para aceelerar a Ré
  pinMode(MovePWM , OUTPUT);  // Porta utilizada para o PWM (Andar frente e Ré)

 // MAC ADDRESS do meu controle de PS4
  PS4.begin("00:e0:4c:bd:44:03");

  // Aguarda até que a conexão seja bem sucedida, caso contrário o código permanecerá no While.
  while (!PS4.isConnected()){
    Serial.println("Aguardando conexão....");
    delay(200);
  }

  // Emite um bip ao ligar o receptor 
  tone(Buzzer, 1000, 100);delay(100);
  tone(Buzzer, 1500, 100);delay(100);
  tone(Buzzer, 2000, 100);delay(100);
  tone(Buzzer, 2500, 100);delay(100);
  noTone(Buzzer);

  analogWrite(PinServo,90);
  delay(1000);
  DirectionServo.write(0);  delay(1000);
  DirectionServo.write(180);  delay(1000);
  DirectionServo.write(0);  delay(1000);
}

void loop() {

/**************************************************************************************************
          LÓGICA PARA PARTIDA E DESLIGAMENTO DO MOTOR 
- Algumas funções só funcionam após o botão de partida ser acionado. 
***************************************************************************************************/
 // Partida (Options) - Tudo no código fica OFF se TRUCK_ON = False
    if (PS4.Options()) {
      // Verifica se o botão OPTIONS não estava pressionado anteriormente
      if (!OptionsFlag) {
        Truck_ON = !Truck_ON;
        Serial.println("Partida Toggled");
        OptionsFlag = true;
        digitalWrite(StartStop, Truck_ON ? HIGH : LOW); // Define a saída física conforme o estado atual de Truck_ON
      }
    } else {OptionsFlag = false;}

/**************************************************************************************************
          LÓGICA PARA CONTROLE DE DIREÇAO USANDO SERVO MOTOR 
- Funcionamento indenpendete de qualquer outro circuito do veiculo
***************************************************************************************************/
  // DirectionServo
  if(PS4.LStickX())
  {

    int var = PS4.LStickX();
    Serial.printf("Analog %d\n", var);

    if (abs(var) < 10)
    {
        DirectionServo.write(100);
    }
    else{
      int novoAngulo = map(var, -128, 127, 0, 200);
      DirectionServo.write(novoAngulo);
      Serial.printf("Novo Angulo %d\n", novoAngulo);
    }
          
  }

/**************************************************************************************************
          LÓGICA PARA ACIONAMENTO ACELERAR FRENTE
***************************************************************************************************/
    if(PS4.R2() && Truck_ON && !ReverseState){
        /*-------------- EXCLUSIVIDADE PARA USO DA PONTE H  --------------------*/
        int y = PS4.R2Value(); // Recebe valor analógico do gatilho (R2) do controle para usar como aceleração na ponte H
        ForwardAcc = map(y,0,255,100,255); // Aumenta a curva do acelerador, ganhando mais precisao.
        digitalWrite(ReverseMove, LOW); // Aciona Ponte H para aceelerar a Ré
        digitalWrite(ForwardMove, HIGH); // Aciona Ponte H para aceelerar a Ré
        analogWrite(MovePWM,ForwardAcc); //Valor passado para ponte H por PWM
        /*----------------------------------------------------------------------*/
        Serial.print("Vamos pra frente");
        Serial.println(ForwardAcc );
        ForwardState = true; // Flag global para sinalizar que o caminhao esta em movimento pra frente
    }else{
      ForwardAcc=0;
      ForwardState = false;
      if(!ReverseState){
          digitalWrite(ReverseMove, LOW); // Aciona Ponte H para aceelerar a Ré
          digitalWrite(ForwardMove, LOW); // Aciona Ponte H para aceelerar a Ré
      }
    }


/**************************************************************************************************
          LÓGICA PARA ACIONAMENTO ACIONAR A RÉ
- Luz de ré acionada diretamente quando o botão L2 é acionado.
***************************************************************************************************/
  if (PS4.L2() && Truck_ON && !ForwardState) {
      int x = PS4.L2Value(); // Recebe valor analógico do gatilho (L2) do controle para usar como aceleração na ponte H
      digitalWrite(ReverseLight,HIGH); // Liga Luz de Ré
      ReverseAcc = map(x,0,255,100,255); // Aumenta a curva do acelerador, ganhando mais precisao.
      /*-------------- EXCLUSIVIDADE PARA USO DA PONTE H  --------------------*/
      digitalWrite(ReverseMove, HIGH); // Aciona Ponte H para aceelerar a Ré
      digitalWrite(ForwardMove, LOW); // Aciona Ponte H para aceelerar a Ré
      analogWrite(MovePWM,ReverseAcc); //Valor passado para ponte H por PWM
      /*----------------------------------------------------------------------*/

      if(!ReverseState && ReverseAcc>120){ // Só vai ligar o sinal sonoro depois que o botao ja estiver pressionado acima de 50
        lastNoiseTimeReverse = millis();
        tone(Buzzer,1200,500); // Ja inicia o som assim que o led da ré é ligado
        ReverseState = true;
      }
      Serial.print("Vamos pra tras");
      Serial.println(ReverseAcc );
  }else{
          ReverseAcc=0;
          digitalWrite(ReverseLight,LOW);
          ReverseState = false; 
          if(!ForwardState){ // Verifica se nao existe comando pra ir pra frente, se nao houve dai ele desliga as saidas
            digitalWrite(ReverseMove, LOW); // Desliga Ponte H para aceelerar a Ré
            digitalWrite(ForwardMove, LOW); // Desliga  Ponte H para aceelerar a Ré
          }
          }//Quando o botao nao estiver pressionado ele deve se manter em 0

  if(ReverseState){
      ReverseBuzzer(); //Apito de ré
    }

/**************************************************************************************************
          LÓGICA PARA ACIONAMENTO DA BUZINA
- Buzina não funciona se a Ré estiver ligada para evitar interferencia no Buzzer          
***************************************************************************************************/

      // Tratativa para Buzina
      if (PS4.R3() && ReverseState){ // Buzina não irá funcionar quando a ré estiver ligada
          tone(Buzzer, 200, 50); // Frequencia de 200Hz e tempo de 50ms 
      }

/**************************************************************************************************
    LÓGICA PARA ACIONAMENTO DO FREIO
- Freio acionado diretamente, luz responde de imediato ao comando do botao.    
- Freio para imediatamente todos os movimentos de motores. 
***************************************************************************************************/
      // Tratativa para Freio
      if (PS4.Square()){
          digitalWrite(BreakLight, HIGH);
          digitalWrite(ReverseMove, LOW); // Obriga o motor a parar de rodar
          digitalWrite(ForwardMove, LOW); // Obriga o motor a parar de rodar
      }
      else{digitalWrite(BreakLight, LOW);}


/**************************************************************************************************
    LÓGICA PARA ACIONAMENTO DO FAROLETE
- O mesmo botão liga e desliga o farolete
***************************************************************************************************/
      if (PS4.Cross()){
        if(!CrossFlag){
          PositionLightState = !PositionLightState ;
          digitalWrite(PositionLight, PositionLightState ? HIGH : LOW); // Define a saída física conforme o estado atual de PositionLightState
          CrossFlag = true;
          Serial.println("Farolete");
        }
      }else{CrossFlag=false;}


/**************************************************************************************************
    LÓGICA PARA ACIONAMENTO DO FAROL 
- Farol liga somente se o caminhao estiver ligado   
- Farol liga somente se o farolete estiver ligado

***************************************************************************************************/
      if (PS4.Circle() && PositionLightState && Truck_ON){
        if(!CircleFlag){
          HeadLightState = !HeadLightState ;
          digitalWrite(HeadLight, HeadLightState ? HIGH : LOW); // Define a saída física conforme o estado atual de PositionLightState
          CircleFlag = true;
          Serial.println("Farol");
        }
      }else{
        CircleFlag=false;
        if(PositionLightState==false || Truck_ON == false) {digitalWrite(HeadLight, LOW);HeadLightState=false;} //Desliga o farol se o farolete ou o caminhao estiver desligado
        }

/**************************************************************************************************
    LÓGICA PARA ACIONAMENTO DO FAROL DE MILHA 
- Farol de milha liga somente se o caminhao estiver ligado   
- Farol de milha  liga somente se o farolete estiver ligado
***************************************************************************************************/
      if (PS4.Triangle() && PositionLightState && Truck_ON){
        if(!TriangleFlag){
          mileLightState = !mileLightState ;
          digitalWrite(MileLight, mileLightState ? HIGH : LOW); // Define a saída física conforme o estado atual de PositionLightState
          TriangleFlag = true;
          Serial.println("Farol Milha");
        }
      }else{
        TriangleFlag=false;
        if(PositionLightState==false || Truck_ON == false) {digitalWrite(MileLight, LOW);mileLightState=false;} //Desliga o farol se o farolete ou o caminhao estiver desligado
        }

/**************************************************************************************************
    LÓGICA DE FUNCIONAMENTO PARA AS LUZES DE DIREÇÃO - SETAS DIREITA, ESQUERDA E ALERTA
- Seta liga somente se o caminhao estiver ligado  
- Alerta funciona em qualquer situção
***************************************************************************************************/
    // Seta para Direita (Right)
    // Funciona apenas após a partida 
    if (PS4.Right() && Truck_ON  && alertArrowState == false) {
        if (RightArrowFlag == false) { // Verifica se a lógica já foi executada (flag está em falso)
          lastBlinkTimeRight = millis(); // Captura o tempo atual para utilizar no tempo de piscar
          rightArrowState = !rightArrowState ; // Liga ou desliga a seta; o mesmo botão pode ligar ou desligar a seta
          if(rightArrowState==false) digitalWrite(RightArrow, LOW); // Condição para garantir que, ao pressionar para desligar, o LED seja apagado e não permaneça ligado
          leftArrowState = false; // Quando ligar a seta do lado direito, a seta do lado esquerdo deve parar imediatamente
          digitalWrite(LeftArrow, LOW); // Desliga a porta da seta do lado oposto
          RightArrowFlag = true; // Sinaliza que esse botão foi pressionado e só voltará ao estado de falso quando for solto (borda de subida)
          Serial.println(" Seta para Direita!");
        }
    }
    // Essa condição só irá acontecer quando o botão estiver solto, portanto ela ajuda no controle das repetições quando o botão se mantém pressionado.
    else{
      RightArrowFlag = false;
      if(Truck_ON==false && alertArrowState ==false){digitalWrite(RightArrow, LOW);rightArrowState = false;} //Desliga a seta se o caminhao for desligado
      } // Reset da flag quando o botão é solto (borda de descida)


    // Seta para Esquerda (Left)
    // Funciona apenas após a partida
    if (PS4.Left() && Truck_ON &&  alertArrowState == false) {
        
        if (!LeftArrowFlag) {// Verifica se a lógica já foi executada (flag está em falso)
            lastBlinkTimeLeft = millis();// Captura o tempo atual para utilizar no tempo de piscar
            leftArrowState = !leftArrowState;// Liga ou desliga a seta; o mesmo botão pode ligar ou desligar a seta
            if (leftArrowState == false) {digitalWrite(LeftArrow, LOW);}// Condição para garantir que, ao pressionar para desligar, o LED seja apagado e não permaneça ligado
            rightArrowState = false;// Quando ligar a seta do lado esquerdo, a seta do lado direito deve parar imediatamente
            digitalWrite(RightArrow, LOW); // Desliga a porta da seta do lado oposto
            LeftArrowFlag = true; // Rising edge
            Serial.println(" Seta para Esquerda!");
        }
    }
    // Essa condição só irá acontecer quando o botão estiver solto, portanto ela ajuda no controle das repetições quando o botão se mantém pressionado.
    else {
      LeftArrowFlag = false;
      if(Truck_ON==false && alertArrowState ==false){digitalWrite(LeftArrow, LOW);leftArrowState = false;} //Desliga a seta se o caminhao for desligado
      } // Reset da flag quando o botão é solto (borda de descida)

    // Verifica se o botão da seta para a direita ou para a esquerda está pressionado
    if (rightArrowState || leftArrowState) {
        if (rightArrowState) {rightArrowBlinkFunction();}// Se a variável de indicaçao de seta para a direita está ligada, chama a função de piscar para a seta direita
        if (leftArrowState) {leftArrowBlinkFunction();}// Se a variável de indicaçao de seta para a esquerda está ligada, chama a função de piscar para a seta esquerda
    }

    if (PS4.Up()) {
      if(!AlertFlag){
        lastBlinkTimeAlert = millis();// Captura o tempo atual para utilizar no tempo de piscar
        alertArrowState = !alertArrowState; // Troca o status do botao pressionado
        leftArrowState= false; // Desliga a seta quando alerta é ligado, alerta tem prioridade
        rightArrowState= false; // Desliga a seta quando alerta é ligado, alerta tem prioridade
        if(alertArrowState==false){digitalWrite(RightArrow, LOW);digitalWrite(LeftArrow, LOW);} //Garante de desligar os dois lados da seta.
        if(alertArrowState==true){digitalWrite(RightArrow, HIGH);digitalWrite(LeftArrow, HIGH);} //Garante de desligar os dois estarao sincronizados.
        AlertFlag = true; // Sinaliza que esse botão foi pressionado e só voltará ao estado de falso quando for solto (borda de subida) 
        Serial.println(alertArrowState );
      }
    }else{AlertFlag = false;}

    // Liga os dois lados da seta ao mesmo tempo, fazendo o efeito esperado para o ALERTA
    if ( alertArrowState){
        blinkAlert();
    }


} //FIM DO VOID LOOP

/**************************************************************************************************************
                      ESCOPO DE FUNÇÕES - LUZES DE DIREÇÃO - SETAS
**************************************************************************************************************/
/*------------------------------------ ALERTA ----------------------------------------------------------*/
void blinkAlert() {
  // Verifica se o tempo decorrido até o momento é maior ou igual ao intervalo de piscar definido
  if ((millis() - lastBlinkTimeAlert) >= blinkInterval) {
    // Inverte o estado das saídas das setas
    digitalWrite(RightArrow, !digitalRead(RightArrow));
    digitalWrite(LeftArrow, !digitalRead(LeftArrow));
    // Emite sinal sonoro da seta
    tone(Buzzer,50,30);
    // Atualiza o tempo da última piscada para o tempo atual
    lastBlinkTimeAlert = millis();
  }
}

/*------------------------------------ SETA DIREITA ----------------------------------------------------------*/

// Função para controlar o piscar do LED da seta direita com base no intervalo definido no cabeçalho do projeto.
void rightArrowBlinkFunction() {
  // Inicializa a variável para verificar se o LED da seta está ligado ou desligado neste momento
  bool statePin = false;

  // Verifica se o tempo decorrido até o momento é maior ou igual ao valor pré-definido no cabeçalho
  if ((millis() - lastBlinkTimeRight) >= blinkInterval) {
    // Lê o estado atual do pino de saída da seta para garantir que o estado será trocado
    statePin = digitalRead(RightArrow);
    // Inverte o valor da saída; se o LED estiver ligado, ele vai para desligado e vice-versa
    digitalWrite(RightArrow, !statePin);
    // Emite sinal sonoro da seta
    tone(Buzzer,50,30);
    // Atualiza o tempo da última piscada para o tempo atual
    lastBlinkTimeRight = millis();
  }
}

/*------------------------------------ SETA ESQUERDA ----------------------------------------------------------*/
// Função para piscar led da seta baseado no tempo de pisca definido no cabeçalho desse projeto
void leftArrowBlinkFunction(){
  // Inicializa a variável que irá verificar se o LED da seta está ligado ou desligado nesse momento
  bool statePin =  false;
  // Se o tempo decorrido até o momento for maior ou igual ao valor pré-definido no cabeçalho é necessário trocar o 'estado' do LED
  if ((millis() - lastBlinkTimeLeft) >= blinkInterval)
  {
    // Lê o pino de saída da seta para garantir que o estado será trocado.
    statePin = digitalRead(LeftArrow);
    digitalWrite(LeftArrow, !statePin); // Aqui ele inverte o valor da saída, se o LED estiver ON ele vai para OFF e vice versa.
    // Emite sinal sonoro da seta
    tone(Buzzer,50,30);
    // Atualiza o tempo da última piscada para o tempo atual
    lastBlinkTimeLeft = millis();
  }
}

/**************************************************************************************************************
                      ESCOPO DE FUNÇÕES - SIRENE DE RÉ 
**************************************************************************************************************/
void ReverseBuzzer(){
  if(millis()-lastNoiseTimeReverse>= 1000)
  {
        tone(Buzzer,1200,500);
        lastNoiseTimeReverse = millis();
  }
}
