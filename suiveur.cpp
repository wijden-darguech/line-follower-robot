#include <Arduino.h>

// ---------------- MOTOR & SENSOR DEFINITIONS -----------------
#define leftMotor1 5
#define leftMotor2 18
#define rightMotor1 16
#define rightMotor2 17
#define sensor1 32
#define sensor2 33
#define sensor3 35
#define sensor4 34
#define sensor5 25
#define sensor6 26
#define button 14
#define led 21

#define LEFT_MOTOR1_CH 0
#define LEFT_MOTOR2_CH 1
#define RIGHT_MOTOR1_CH 2
#define RIGHT_MOTOR2_CH 3
#define PWM_FREQ 1000
#define PWM_RES 8 

// ---------------- GLOBAL VARIABLES ---------------------------
const int sensors[6] = {sensor1,sensor2,sensor3,sensor4,sensor5,sensor6};
int threshvalue;
int rawSensorValues[6];
int binarySensor[6]; b
const int weights[6] = {-5,-3,-1,1,3,5};

float Kp=50.0;
float Ki=0.0;
float Kd=0.5;
int x=0;//variable de compteur de fois que le mode veut changer 
float t1=950;
float t2=5650;

float error;
float lastError;
float D, I=0.0;
float PIDvalue;

unsigned long lastTime = 0;
unsigned long lastLineSeenTime = 0;

#define baseSpeed 150
#define maxIntegral 50
#define lostLineTimeout 200

// ---------------- HYSTERESIS SETTINGS ------------------------
#define HYSTERESIS_MARGIN 300
int mode = 0;              // 0 = detecting black line, 1 = detecting white line
int donecalibrate = 0;

// ---------------- FUNCTION PROTOTYPES ------------------------
void calculatePID(int activeCount, int weightedSum);
void updateMotorSpeeds();
void readSensors(int *array);
void calibrateSensors();
void waitForButtonPress();
int min(int* T);
int max(int* T);
void moveMotors(int leftMotorSpeed, int rightMotorSpeed);

// =============================================================
//                           SETUP
// =============================================================
void setup() {
  Serial.begin(115200);

  // Setup PWM
  ledcSetup(LEFT_MOTOR1_CH, PWM_FREQ, PWM_RES);
  ledcSetup(LEFT_MOTOR2_CH, PWM_FREQ, PWM_RES);
  ledcSetup(RIGHT_MOTOR1_CH, PWM_FREQ, PWM_RES);
  ledcSetup(RIGHT_MOTOR2_CH, PWM_FREQ, PWM_RES);

  ledcAttachPin(leftMotor1, LEFT_MOTOR1_CH);
  ledcAttachPin(leftMotor2, LEFT_MOTOR2_CH);
  ledcAttachPin(rightMotor1, RIGHT_MOTOR1_CH);
  ledcAttachPin(rightMotor2, RIGHT_MOTOR2_CH);

  pinMode(sensor1,INPUT);
  pinMode(sensor2,INPUT);
  pinMode(sensor3,INPUT);
  pinMode(sensor4,INPUT);
  pinMode(sensor5,INPUT);
  pinMode(sensor6,INPUT);

  pinMode(button,INPUT_PULLUP);
  pinMode(led,OUTPUT);

  calibrateSensors();
  delay(1000);
  lastTime = millis();
}


void loop() {
  if(donecalibrate == 0) return;
  unsigned long t=millis()//compteur 

  unsigned long now = millis();
  float dt = (now-lastTime) / 1000.0;
  if (dt <= 0) dt = 0.0001;

  readSensors(rawSensorValues);

  int activeCount = 0;
  int weightedSum = 0;
  //fonction priorité 
if (t>t1)&&(t<t2){
  int n=edamfunction;
  if(n==1){
    getpriority()
  }
}



  // ------------------ READ LINE ACCORDING TO MODE --------------
  if(mode == 0) {
    // BLACK LINE = 1
    for(int i=0; i<6; i++){
      binarySensor[i] = rawSensorValues[i] > threshvalue ? 1 : 0;
      if(binarySensor[i]) {
        activeCount++;
        weightedSum += weights[i];
      }
    }
  }
  else {
    // WHITE LINE = 1
    for(int i=0; i<6; i++){
      binarySensor[i] = rawSensorValues[i] < threshvalue ? 1 : 0;
      if(binarySensor[i]) {
        activeCount++;
        weightedSum += weights[i];
      }
    }
  }

  // ------------------- LOST LINE HANDLING -----------------------
  if (activeCount > 0) lastLineSeenTime = now;

  if(activeCount == 0){
    error = (lastError > 0) ? 50 : -50;

    if(now - lastLineSeenTime > lostLineTimeout){
      moveMotors(100, 100);
      lastError = error;
      lastTime = now;
      return;
    }
  }

 
  calculatePID(activeCount, weightedSum);
  updateMotorSpeeds();
  
  lastTime = now;
}


void readSensors(int *array) {
  for(int i=0; i<6; i++)
    array[i] = analogRead(sensors[i]);
}

void calibrateSensors(){
  int sumblack[6]={0};
  int sumwhite[6]={0};
  const int samples=50;
  Serial.println("place robot on the line , calibrating black ...");
  waitForButtonPress();
  digitalWrite(led, HIGH);
  for (int j=0 ; j<samples; j++){
    for (int i=0;i<6 ; i++){
      sumblack[i]+=analogRead(sensors[i]);
    }
    delay(50);
  }
  digitalWrite(led, LOW);
  delay(2000);
  Serial.println("place robot off the line , calibrating white ...");
  waitForButtonPress();
  digitalWrite(led, HIGH);
  for (int j=0 ; j<samples; j++){
    for (int i=0;i<6 ; i++){
      sumwhite[i]+=analogRead(sensors[i]);
    }
    delay(50);
  }
  digitalWrite(led, LOW);                       
  for (int i=0;i<6 ; i++){
    sumblack[i]=sumblack[i]/samples;
    sumwhite[i]= sumwhite[i]/samples;}
  int minblack=min(sumblack);
  int maxwhite=max(sumwhite);
  threshvalue= (minblack + maxwhite)/2;
  Serial.println(minblack);
  Serial.println(maxwhite);
  Serial.println("Calibration done!");
  waitForButtonPress();
  donecalibrate=1;
}


int min(int* T) {
  int m = T[0];
  for(int i=0;i<6;i++)
    if(T[i] <= m) m = T[i];
  return m;
}

int max(int* T) {
  int M = T[0];
  for(int i=0;i<6;i++)
    if(T[i] >= M) M = T[i];
  return M;
}

void waitForButtonPress(){
  while(digitalRead(button)==HIGH) delay(5);
  delay(50); 
  while(digitalRead(button)==LOW) delay(5);
  delay(50); 
}


void calculatePID(int activeCount, int weightedSum) {
  unsigned long now = millis();
  float dt = (now-lastTime) / 1000.0;
  if(dt <= 0) dt = 0.0001;

  if(activeCount > 0)
    error = (float)weightedSum / activeCount;
  
  I += error * dt;
  I = constrain(I, -maxIntegral, maxIntegral);

  D = (error - lastError) / dt;
  D = constrain(D, -100, 100);

  PIDvalue = Kp*error + Ki*I + Kd*D;
  PIDvalue = constrain(PIDvalue, -255, 255);

  lastError = error;
}


void updateMotorSpeeds() {
  // Corrected: left slower, right faster for positive error (turn right)
  int leftSpeed = constrain(baseSpeed - PIDvalue, -255, 255);
  int rightSpeed = constrain(baseSpeed + PIDvalue, -255, 255);
  moveMotors(leftSpeed, rightSpeed);
}

void moveMotors(int leftMotorSpeed, int rightMotorSpeed){
  leftMotorSpeed  = constrain(leftMotorSpeed , -255,255);
  rightMotorSpeed = constrain(rightMotorSpeed, -255,255);

  // LEFT MOTOR
  if(leftMotorSpeed >= 0){
    ledcWrite(LEFT_MOTOR1_CH, leftMotorSpeed);
    ledcWrite(LEFT_MOTOR2_CH, 0);
  } else {
    ledcWrite(LEFT_MOTOR1_CH, 0);
    ledcWrite(LEFT_MOTOR2_CH, -leftMotorSpeed);
  }

  // RIGHT MOTOR
  if(rightMotorSpeed >= 0){
    ledcWrite(RIGHT_MOTOR1_CH, rightMotorSpeed);
    ledcWrite(RIGHT_MOTOR2_CH, 0);
  } else {
    ledcWrite(RIGHT_MOTOR1_CH, 0);
    ledcWrite(RIGHT_MOTOR2_CH, -rightMotorSpeed);
  }
}