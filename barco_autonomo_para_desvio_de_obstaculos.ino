#include <Arduino_FreeRTOS.h>
#include <Servo.h>

#define SONAR2_TRIGGER 1
#define SONAR2_ECHO 0
#define SONAR_TRIGGER 7
#define SONAR_ECHO 5
#define LED_RED 12
#define BUZZER 13
#define MOTOR_PIN1 9
#define MOTOR_PIN2 10
#define SERVO_PIN 4

#define DISTANCIA_PERIGOSA 99
#define DISTANCIA_RAZOAVEL_MIN 100
#define DISTANCIA_RAZOAVEL_MAX 150
#define NUM_LEITURAS 3

Servo meuServo;
volatile int distanciaEsq = 0;
volatile int distanciaDir = 0;

void medirDistanciaTask(void *pvParameters);
void controlarMotorTask(void *pvParameters);
void controlarServoTask(void *pvParameters);
void controlarBuzzerLEDTask(void *pvParameters);

void setup() {
  pinMode(SONAR_TRIGGER, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);
  pinMode(SONAR2_TRIGGER, OUTPUT);
  pinMode(SONAR2_ECHO, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);

  Serial.begin(9600);

  if (meuServo.attach(SERVO_PIN) == INVALID_SERVO) {
    Serial.println("Erro ao anexar o servo!");
  } else {
    meuServo.write(90);
  }

  xTaskCreate(medirDistanciaTask, "Medir Distância", 256, NULL, 2, NULL);
  xTaskCreate(controlarMotorTask, "Controlar Motor", 128, NULL, 1, NULL);
  xTaskCreate(controlarServoTask, "Controlar Servo", 128, NULL, 1, NULL);
  xTaskCreate(controlarBuzzerLEDTask, "Controlar Buzzer/LED", 128, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {}

void medirDistanciaTask(void *pvParameters) {
  while (1) {
    int leituraEsq = medirDistancia(SONAR2_TRIGGER, SONAR2_ECHO);
    int leituraDir = medirDistancia(SONAR_TRIGGER, SONAR_ECHO);

    if (leituraEsq != -1 && leituraDir != -1) {
      taskENTER_CRITICAL();
      distanciaEsq = leituraEsq;
      distanciaDir = leituraDir;
      taskEXIT_CRITICAL();
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

int medirDistancia(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  int dist = duracao / 58.2;

  if (dist > 400 || dist <= 0) {
    return -1;
  }

  return dist;
}

void controlarMotorTask(void *pvParameters) {
  while (1) {
    taskENTER_CRITICAL();
    int distEsq = distanciaEsq;
    int distDir = distanciaDir;
    taskEXIT_CRITICAL();

    int distanciaMinima = min(distEsq, distDir);

    if (distanciaMinima < DISTANCIA_PERIGOSA) {
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, HIGH);
      Serial.println("Motor: DISTÂNCIA PERIGOSA!");
    } else if (distanciaMinima >= DISTANCIA_RAZOAVEL_MIN && distanciaMinima <= DISTANCIA_RAZOAVEL_MAX) {
      digitalWrite(MOTOR_PIN1, HIGH);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("Motor: DISTÂNCIA RAZOÁVEL!");
    } else {
      digitalWrite(MOTOR_PIN1, HIGH);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("Motor: DISTÂNCIA SEGURA!");
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

    if (distEsq < DISTANCIA_PERIGOSA && distDir >= DISTANCIA_PERIGOSA) {
      meuServo.write(135); 
    } else if (distDir < DISTANCIA_PERIGOSA && distEsq >= DISTANCIA_PERIGOSA) {
      meuServo.write(45); 
    } else if (distEsq < DISTANCIA_PERIGOSA && distDir < DISTANCIA_PERIGOSA) {
      if (distEsq > distDir) {
        meuServo.write(45); 
      } else {
        meuServo.write(135);
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
    } else if (distanciaMinima >= DISTANCIA_RAZOAVEL_MIN && distanciaMinima <= DISTANCIA_RAZOAVEL_MAX) {
      noTone(BUZZER);
      digitalWrite(LED_RED, HIGH);
    } else {
      noTone(BUZZER);
      digitalWrite(LED_RED, LOW);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}