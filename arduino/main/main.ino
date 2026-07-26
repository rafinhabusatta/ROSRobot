#include <RoboCore_Vespa.h>
#include <WiFi.h>
//#include <WiFiUdp.h>
#include "UltrasonicSensor.h"
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32_multi_array.h>
#include "BluetoothSerial.h"
//#include "Utils.h"

// Macro de checagem de erro (Evita o Kernel Panic)
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){ while(1){delay(100);} } }

// Variável para guardar o comando recebido
char comando;

#define OBSTACLE_DISTANCE 20
#define WHEEL_RADIUS 0.0315
#define WHEEL_BASE 0.115
VespaMotors motors;

// --- Configurações da Rede ---
char ssid[] = "TP-Link_4211"; //VIVOFIBRA-WIFI6-23D1
char password[] = "23651050"; //j9ECC2NW9WxQnwx

//WiFiUDP udp;
//char incoming[255];

// --- Configurações do ROS ---
rcl_publisher_t publisher;
std_msgs__msg__Int32MultiArray msg;
int32_t sensor_data[3];   // array com os 3 valores do sonar

rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rclc_executor_t executor;

// --- Configurações sensores ---
const int sonarATrig = 5;
const int sonarBTrig = 19;
const int sonarCTrig = 21;
const int sonarAEcho = 18;
const int sonarBEcho = 23;
const int sonarCEcho = 22;

const int encoderLeftCH1 = 36;
const int encoderLeftCH2 = 39;
const int encoderRightCH1 = 16;
const int encoderRightCH2 = 17;

UltrasonicSensor ultrasonicFront(sonarATrig, sonarAEcho);
UltrasonicSensor ultrasonicLeft(sonarCTrig, sonarCEcho);
UltrasonicSensor ultrasonicRight(sonarBTrig, sonarBEcho);

int obstacleFront;
int obstacleLeft;
int obstacleRight;

struct Encoder {
  volatile int currentStateCH2;
  volatile int previousStateCH2;
  volatile bool rotationDirection;

  volatile unsigned long count1;
  volatile unsigned long count2;

  float rps;
  float wheelOmega;   // rad/s da roda
};

Encoder encoderLeft;
Encoder encoderRight;

const int NUMERO_CONTADORES = 2;
const int NUMERO_LEITURAS = 2;
const int NUMERO_DENTES = 10;

const int teethNumber = 10;
unsigned long timeBefore = 0;
const long TIME = 1000;

struct RobotState {
    float linearVelocity;    // m/s
    float angularVelocity;   // rad/s

    float x;
    float y;
    float theta;
};

void setup() {
  delay(3000);
  pinMode(sonarATrig,OUTPUT);
  pinMode(sonarBTrig,OUTPUT);
  pinMode(sonarCTrig,OUTPUT);

  pinMode(sonarAEcho,INPUT);
  pinMode(sonarBEcho,INPUT);
  pinMode(sonarCEcho,INPUT);

  pinMode(encoderLeftCH1, INPUT);
  pinMode(encoderLeftCH2, INPUT);
  pinMode(encoderRightCH1, INPUT);
  pinMode(encoderRightCH2, INPUT);

  Serial.begin(9600);
  Serial.println("\nSMARS System Initializing...");
  Serial.println("\nConnected to WiFi");    
  Serial.print("Board IP: ");
  Serial.println(WiFi.localIP());
  //udp.begin(4210);

  //Inicializa as interrupcoes com os pinos configurados para chamar as funcoes  
  //"contador_pulso2" e "contador_pulso1" respectivamente a cada mudanca de estado das portas
  attachInterrupt(digitalPinToInterrupt(encoderLeftCH2), counter_pulse2Left, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderRightCH1), counter_pulse1Right, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderRightCH2), counter_pulse2Right, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderLeftCH1), counter_pulse1Left, CHANGE);

  //set_microros_transports();
  set_microros_wifi_transports(ssid, password, "192.168.0.106", 8888);

  // 4. A MÁGICA DO PING: Espera o agente conectar antes de continuar!
  while (rmw_uros_ping_agent(100, 1) != RMW_RET_OK) {
    delay(100);
  }

  delay(2000);

  // Inicializa suporte micro-ROS
  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Cria nó
  RCCHECK(rclc_node_init_default(&node, "ultrassonic_node", "", &support));

  // Cria publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
    "sensors_dist"));

  // Configura mensagem
  msg.data.capacity = 3;
  msg.data.size = 3;
  msg.data.data = sensor_data;
}

void loop() {
  obstacleFront = ultrasonicFront.getDistance();
  obstacleLeft = ultrasonicLeft.getDistance();
  obstacleRight = ultrasonicRight.getDistance();

  sensor_data[0] = ultrasonicLeft.getDistance();
  sensor_data[1] = ultrasonicFront.getDistance();
  sensor_data[2] = ultrasonicRight.getDistance();


  //Verifica a contagem de tempo e exibe as informacoes coletadas do motor
  if ((millis() - timeBefore) > TIME) { //A cada TIME ms

    //Calcula a media dos contadores
    int mediaEsq = (encoderLeft.count1 + encoderLeft.count2) / (NUMERO_CONTADORES); 
    int mediaDir = (encoderRight.count1 + encoderRight.count2) / (NUMERO_CONTADORES);

    encoderLeft.rps = mediaEsq / (float)(NUMERO_DENTES  * NUMERO_LEITURAS);
    encoderRight.rps = mediaDir / (float)(NUMERO_DENTES  * NUMERO_LEITURAS);

    //velocidade em rad/s de cada roda
    encoderLeft.wheelOmega = encoderLeft.rps * 2.0f * PI;
    encoderRight.wheelOmega = encoderRight.rps * 2.0f * PI;

    if (encoderLeft.rotationDirection)
      encoderLeft.wheelOmega *= -1.0f;
    
    if (!encoderRight.rotationDirection)
      encoderRight.wheelOmega *= -1.0f;

    float vLeft  = encoderLeft.wheelOmega * WHEEL_RADIUS;
    float vRight = encoderRight.wheelOmega * WHEEL_RADIUS;

    float linearVelocity  = (vLeft + vRight) / 2.0f;
    float angularVelocity = (vRight - vLeft) / WHEEL_BASE;

    //Zera os contadores e reinicia a contagem de tempo.
    encoderLeft.count1 = 0;
    encoderRight.count1 = 0;

    encoderLeft.count2 = 0;
    encoderRight.count2 = 0;

    timeBefore = millis();

  }


  // Publica no tópico
  rcl_publish(&publisher, &msg, NULL);

  delay(500); // publica a cada 0,5s

  // Xbox controller start
  /*int len = udp.parsePacket();
  if (len > 0) {
    udp.read(incoming, 255);
    incoming[len] = 0;
    String cmd = String(incoming);
    if (cmd == "FRENTE") motors.forward(100);
    else if (cmd == "TRAS") motors.backward(100);
    else if (cmd == "ESQUERDA") motors.turn(100,50);
    else if (cmd == "DIREITA") motors.turn(50,100);
    else if (cmd == "STOP") motors.stop();
  }*/
  // Xbox controller end
  //SonarTest(obstacleFront, obstacleLeft, obstacleRight);

  if (obstacleFront <= OBSTACLE_DISTANCE) {
    motors.stop();
    if (obstacleLeft > OBSTACLE_DISTANCE) {
      delay(1000);
      motors.turn(100,50);
      delay(1000);
    }
    delay(1000);
    motors.turn(50,100);
    delay(1000);
  }
  else {
    motors.forward(+80);
  }
  
}

//---------------------------------------------------------------------------------------
//Funcao de interrupcao
void counter_pulse2Left() {

  //Incrementa o contador
  encoderLeft.count2++;

  //Verifica o sentido de rotacao do motor
  encoderLeft.currentStateCH2 = digitalRead(encoderLeftCH2);
  if (encoderLeft.previousStateCH2 == LOW && encoderLeft.currentStateCH2 == HIGH) {
    encoderLeft.rotationDirection = digitalRead(encoderLeftCH1) == LOW;
  }
  encoderLeft.previousStateCH2 = encoderLeft.currentStateCH2;

}

void counter_pulse2Right() {

  //Incrementa o contador
  encoderRight.count2++;

  //Verifica o sentido de rotacao do motor
  encoderRight.currentStateCH2 = digitalRead(encoderRightCH2);
  if (encoderRight.previousStateCH2 == LOW && encoderRight.currentStateCH2 == HIGH) {
    encoderRight.rotationDirection = digitalRead(encoderRightCH1) == LOW;
  }
  encoderRight.previousStateCH2 = encoderRight.currentStateCH2;

}

//Funcao de interrupcao
void counter_pulse1Left() {

  //Incrementa o contador
  encoderLeft.count1++;

}

void counter_pulse1Right() {

  //Incrementa o contador
  encoderRight.count1++;
}




