#include <Arduino_FreeRTOS.h>
#include <Servo.h>

#define SONAR2_TRIGGER 2
#define SONAR2_ECHO 3
#define SONAR_TRIGGER 7
#define SONAR_ECHO 5
#define LED_RED 12
#define BUZZER 13
#define MOTOR_IN1 9  
#define MOTOR_IN2 10  
#define MOTOR_ENA 11
#define SERVO_PIN 4

#define DISTANCIA_PERIGOSA 8
#define DISTANCIA_RAZOAVEL_MIN 15
#define DISTANCIA_RAZOAVEL_MAX 30

Servo meuServo;
volatile int distanciaEsq = 0;
volatile int distanciaDir = 0;

void medirDistanciaTask(void *pvParameters);
void controlarMotorTask(void *pvParameters);
void controlarServoTask(void *pvParameters);
void controlarBuzzerLEDTask(void *pvParameters);

void setup() {
  Serial.begin(9600);
  while (!Serial);  
  Serial.println(F("Iniciando sistema..."));

  pinMode(SONAR_TRIGGER, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);
  pinMode(SONAR2_TRIGGER, OUTPUT);
  pinMode(SONAR2_ECHO, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);

  meuServo.attach(SERVO_PIN);
  meuServo.write(90);
  Serial.println(F("Servo inicializado."));

  TaskHandle_t xHandle1 = NULL, xHandle2 = NULL, xHandle3 = NULL, xHandle4 = NULL;

  xTaskCreate(medirDistanciaTask, "Medir", 128, NULL, 2, &xHandle1);
  xTaskCreate(controlarMotorTask, "Motor", 64, NULL, 1, &xHandle2);
  xTaskCreate(controlarServoTask, "Servo", 64, NULL, 1, &xHandle3);
  xTaskCreate(controlarBuzzerLEDTask, "Buzzer", 64, NULL, 1, &xHandle4);

  if (xHandle1 == NULL || xHandle2 == NULL || xHandle3 == NULL || xHandle4 == NULL) {
    Serial.println(F("Erro ao criar tarefas! Memória insuficiente."));
    while (1);
  }

  Serial.println(F("Tarefas criadas com sucesso!"));
  vTaskStartScheduler();
}

void loop() {}

int medirDistancia(int triggerPin, int echoPin, int sensor) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  if (duracao == 0) {
    Serial.print(F("Falha no sensor "));
    Serial.println(sensor == 1 ? F("esquerdo!") : F("direito!"));
    return -1;
  }

  int dist = duracao / 58.2;
  if (dist > 400 || dist <= 0) return -1;

  Serial.print(sensor == 1 ? F("Esquerdo: ") : F("Direito: "));
  Serial.println(dist);
  return dist;
}

void medirDistanciaTask(void *pvParameters) {
  while (1) {
    int leituraEsq = medirDistancia(SONAR_TRIGGER, SONAR_ECHO, 0);
    int leituraDir = medirDistancia(SONAR2_TRIGGER, SONAR2_ECHO, 1);

    if (leituraEsq != -1 && leituraDir != -1) {
      taskENTER_CRITICAL();
      distanciaEsq = leituraEsq;
      distanciaDir = leituraDir;
      taskEXIT_CRITICAL();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void controlarMotorTask(void *pvParameters) {
  while (1) {
    taskENTER_CRITICAL();
    int distEsq = distanciaEsq;
    int distDir = distanciaDir;
    taskEXIT_CRITICAL();

    int distanciaMinima = min(distEsq, distDir);

    if (distanciaMinima < DISTANCIA_PERIGOSA) {
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, HIGH);
      analogWrite(MOTOR_ENA, 255); 
      Serial.println(F("Motor: PERIGO! Invertendo direção."));
    } else if (distanciaMinima >= DISTANCIA_RAZOAVEL_MIN && distanciaMinima < DISTANCIA_RAZOAVEL_MAX) {
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      analogWrite(MOTOR_ENA, 84); 
      Serial.println(F("Motor: Distância razoável. Velocidade média."));
    } else if (distanciaMinima >= DISTANCIA_RAZOAVEL_MAX) {
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      analogWrite(MOTOR_ENA, 124); 
      Serial.println(F("Motor: Distância segura. Velocidade máxima."));
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void controlarServoTask(void *pvParameters) {
  while (1) {
    taskENTER_CRITICAL();
    int distEsq = distanciaEsq;
    int distDir = distanciaDir;
    taskEXIT_CRITICAL();

    int diferenca = abs(distEsq - distDir);

    if (distEsq >= DISTANCIA_PERIGOSA && distDir >= DISTANCIA_PERIGOSA) {
      if(diferenca <= 6 && distEsq <= DISTANCIA_RAZOAVEL_MAX && distDir <= DISTANCIA_RAZOAVEL_MAX && distEsq > DISTANCIA_RAZOAVEL_MIN && distDir > DISTANCIA_RAZOAVEL_MIN){
        meuServo.write(135);
        Serial.println(F("Obstáculo central! Desviando para a direita."));
      }
      else{
        if (distEsq >= DISTANCIA_RAZOAVEL_MIN && distEsq <= DISTANCIA_RAZOAVEL_MAX) {
          meuServo.write(135); 
        } else if (distDir >= DISTANCIA_RAZOAVEL_MIN && distDir <= DISTANCIA_RAZOAVEL_MAX) {
          meuServo.write(45); 
        } else {
          meuServo.write(90); 
        }
      }
    } else {
      meuServo.write(90);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void controlarBuzzerLEDTask(void *pvParameters) {
  while (1) {
    taskENTER_CRITICAL();
    int distEsq = distanciaEsq;
    int distDir = distanciaDir;
    taskEXIT_CRITICAL();

    int distanciaMinima = min(distEsq, distDir);

    if (distanciaMinima < DISTANCIA_PERIGOSA) {
      tone(BUZZER, 392);
      digitalWrite(LED_RED, HIGH);
    } else {
      noTone(BUZZER);
      digitalWrite(LED_RED, LOW);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}