#include <Servo.h>

#define TRIGGER_PINL  A3  // Atribui o pino A3 ao Pino Trigger(responsável pela faixa de alcance) do sensor da esquerda
#define ECHO_PINL     A0  // Atribui o pino A0 ao Pino do Echo(responsável por ler o resultado do Trigger)do sensor da esquerda
#define TRIGGER_PINF  A4  // Atribui o pino A4 ao Pino Trigger(responsável pela faixa de alcance) do sensor da frente
#define ECHO_PINF     A1  // Atribui o pino A1 ao Pino do Echo(responsável por ler o resultado do Trigger)do sensor da frente
#define TRIGGER_PINR  A5  // Atribui o pino A5 ao Pino Trigger(responsável pela faixa de alcance) do sensor da esquerda
#define ECHO_PINR     A2  // Atribui o pino A2 ao Pino do Echo(responsável por ler o resultado do Trigger)do sensor da direita
 
//Define todas as conexões com o L298N
#define enA 13
#define in1 12
#define in2 11
#define in3 7
#define in4 6
#define enB 5
#define servoPin 2
 
class Motor{ 
  int enablePin;
  int paraTraz;
  int paraFrente; 
 
  public:
  
  Motor(int ENPin,int dPin1,int dPin2){ //Método para definir os pinos do motor
    enablePin = ENPin;
    paraTraz = dPin1;
    paraFrente = dPin2;
  };
 
  //Método para acionar o motor 0 ~ 255 dirigindo para frente. -1 ~ -255 dirigindo para trás
  void Drive(int speed){ // Atribui ao Motor a velocidade ex: leftMotor.Drive(255);
    if(speed>=0){
      digitalWrite(paraTraz, LOW);
      digitalWrite(paraFrente, HIGH);
    }else{
      digitalWrite(paraTraz, HIGH);
      digitalWrite(paraFrente, LOW);
      speed = - speed;
    }
    analogWrite(enablePin, speed); 
  }
};
 
Motor rightMotor = Motor(enA, in1, in2); // Motor(13, 12, 11);
Motor leftMotor = Motor(enB, in3, in4); // Motor(5, 6, 7);
Servo myservo; // criar objeto servo para controlar um servo
 
void motorInitiation(){
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  // Set initial direction and speed
  digitalWrite(enA, LOW);
  digitalWrite(enB, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}
 
//Variáveis --------------------------------------------------------------------------
// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;
bool ObstaculoAFrente = false;
int servoPos = 180;
  int erroEsquerda;
  int erroDireita;
 
enum Directions { IrParaFrente, VirarAEsquerda, VirarADireita, DarMeiaVolta,Parar, AjustarEsquerda, AjustarDireita};
 
Directions proximoPasso = IrParaFrente;
 
unsigned long tempo1;
unsigned long tempo2;
unsigned long largura_do_pulso;
float cm;
String caminhopercorrido;
 
//SETUP--------------------------------------------------------------------------
void setup() {
  //Os pinos dos Triggers vão dizer aos sensores para encontrar as faixas
  pinMode(TRIGGER_PINL, OUTPUT); // Atribuindo o modo do pino como de saida
  pinMode(TRIGGER_PINF, OUTPUT); // Atribuindo o modo do pino como de saida
  pinMode(TRIGGER_PINR, OUTPUT); // Atribuindo o modo do pino como de saida
  digitalWrite(TRIGGER_PINL, LOW); // Definindo a energia do pino como baixa 
  digitalWrite(TRIGGER_PINF, LOW); // Definindo a energia do pino como baixa 
  digitalWrite(TRIGGER_PINR, LOW); // Definindo a energia do pino como baixa 
 
  Serial.begin(9600); // Inicializando o console para exibição dos resultados - Na IDE do Arduino Em Ferramentas > Monitor Serial
  myservo.attach(servoPin);
  motorInitiation(); // Inicializa o Motor
  Directions proximoPasso = IrParaFrente;
}
 
void loop() {
  checkDirection();
  drive();
  Serial.println(caminhopercorrido);
  /*checkDirectionL();
  drive();
  checkDirectionR();
  drive();*/
}

void checkDirection(){
  checkDistance(TRIGGER_PINL, ECHO_PINL); // checa a distacia da esquerda
  checarObstaculo();
  checarParedes();
  if(ObstaculoAFrente == false){//se não tiver obstaculo a esquerda então
    proximoPasso = VirarAEsquerda; // Atribui  o valor da proximoPasso como a esquerda
    //myservo.write(180);
    delay(400);
    caminhopercorrido += "L";
    return;
  }
 checkDistance(TRIGGER_PINF, ECHO_PINF); // checa a distacia da frente
  checarObstaculo();
  checarParedes();
  if(ObstaculoAFrente == false){//se não tiver obstaculo a frente então
    proximoPasso = IrParaFrente; // Atribui  o valor da proximoPasso como a frente
    //myservo.write(90);
    delay(400);
    caminhopercorrido += "F";
    return;
  }
  checkDistance(TRIGGER_PINR, ECHO_PINR); // checa a distacia da direita
  checarObstaculo();
  checarParedes();
  if(ObstaculoAFrente == false){ //se não tiver obstaculo a direita então
    proximoPasso = VirarADireita; // Atribui  o valor da proximoPasso como a direita
    myservo.write(0);
    delay(400);
    caminhopercorrido += "R";
    return;
  }
  proximoPasso = DarMeiaVolta;
  caminhopercorrido += "B"; 
}

void checkDirectionL(){
  checkDistance(TRIGGER_PINL, ECHO_PINL); // checa a distacia da esquerda
  if(ObstaculoAFrente == true){//se tiver obstaculo a esquerda então
    proximoPasso = AjustarEsquerda; // Atribui  o valor da proximoPasso como AjustarEsquerda
    myservo.write(180);
    delay(400);
    return;
  }
}

void checkDirectionR(){
  checkDistance(TRIGGER_PINR, ECHO_PINR); // checa a distacia da direita
  if(ObstaculoAFrente == true){//se tiver obstaculo a direita então
    proximoPasso = AjustarDireita; // Atribui  o valor da proximoPasso como AjustarDireita
    myservo.write(0);
    delay(400);
    return;
  }
}

void checkDistance(int trig, int ech){
 
  // Mantem o pino do trigger alto por pelo menos 10 nós
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
 
  // Aguarda o pulso no pino echo
  while (digitalRead(ech) == 0 );
    // Mede quanto tempo o pino echo foi mantido alto (largura de pulso)
    // A Função micros() Retorna o número de microssegundos desde que a placa do Arduino começou a executar o programa atual.
    tempo1 = micros();
  //Fecha wi
  while (digitalRead(ech) == 1);
	tempo2 = micros();
	
  largura_do_pulso = tempo2 - tempo1;

  // As constantes são encontradas na datasheet e calculadas a partir da velocidade presumida do som no ar ao nível do mar (~ 340 m / s).
  cm = largura_do_pulso / 58.0; // Calcula distância em centímetros
  delay(60); // Espera pelo menos 60ms antes da próxima medição

}

void checarObstaculo(){
   if(cm<= 30){
  ObstaculoAFrente = true;
  }else{ 
  ObstaculoAFrente = false;
  }
}

void checarParedes(){
    if(cm<= 15){
      erroEsquerda = 255-cm*2;
      Serial.println(255-cm*2);
      erroDireita = 255-cm*2;
      Serial.println(255-cm*2);
  }
}

void drive(){
  switch (proximoPasso){
    case IrParaFrente:
      leftMotor.Drive(erroEsquerda);
      rightMotor.Drive(erroDireita);
      //Serial.println("Indo em Fente");
      delay(200);
      break;
    
    case VirarAEsquerda:
      leftMotor.Drive(-255);
      rightMotor.Drive(255);
      //Serial.println("Virando a Esquerda");
      delay(200); 
      break;
    
    case VirarADireita:
      leftMotor.Drive(255);
      rightMotor.Drive(-255);
      //Serial.println("Virando a Direita");
      delay(200);
      break;
    
    case DarMeiaVolta:
      leftMotor.Drive(255);
      rightMotor.Drive(-255);
      //Serial.println("Dando Meia Volta");
      delay(200);
      break;
     
    case Parar:
      leftMotor.Drive(0);
      rightMotor.Drive(0);
      //Serial.println("Parando");
      delay(50);
      break;

    case AjustarEsquerda:
      leftMotor.Drive(255);
      rightMotor.Drive(130);
      //Serial.println("Ajustando a Esquerda");
      delay(100);
      break;
    
    case AjustarDireita:
      leftMotor.Drive(130);
      rightMotor.Drive(255);
      delay(100);
      //Serial.println("Ajustando a Direita");
      break; 
    
    }
}
