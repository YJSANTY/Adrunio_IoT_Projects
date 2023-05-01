#include "painlessMesh.h"
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>


struct {
  int temp_1 = 5;
  int humi_1 = 5;
  int ldr_1  = 5;
  int soil_1 = 5;
  int temp_2 = 5;
  int humi_2 = 5;
  int ldr_2  = 5;
  int soil_2 = 5;
}alpha ;


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


SoftwareSerial NodeMCU(D2,D3);


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Received";
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
//  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  char kal =msg[0];
  if(kal== 't')
  {
    alpha.temp_1 = msg.toInt();
  }
  else if(kal=='l')
  {
    alpha.ldr_1 = msg.toInt();
  }
  else if(kal=='s')
  {
    alpha.soil_1 = (msg).toInt();
  }  
  else if(kal=='h')
  {
    alpha.humi_1 = (msg).toInt();
  }
  else if(kal=='e')
  {
    alpha.temp_2 = (msg).toInt();
  }
  else if(kal=='d')
  {
    alpha.ldr_2 = (msg).toInt();
  }
  else if(kal=='o')
  {
    alpha.soil_2 = (msg).toInt();
  }
  else if(kal=='u')
  {
    alpha.humi_2 = (msg).toInt();
  }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  
  Serial.begin(115200);
  NodeMCU.begin(4800);

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  
  Serial.println(alpha.temp_1);
  Serial.println(alpha.humi_1);
  Serial.println(alpha.ldr_1);
  Serial.println(alpha.soil_1);
  Serial.println(alpha.temp_2);
  Serial.println(alpha.humi_2);
  Serial.println(alpha.ldr_2);
  Serial.println(alpha.soil_2);

//  Serial.println((alpha.temp_1).toInt());
//  Serial.println((alpha.humi_1).toInt());
//  Serial.println((alpha.ldr_1).toInt());
//  Serial.println((alpha.soil_1).toInt());
//  Serial.println((alpha.temp_2).toInt());
//  Serial.println((alpha.humi_2).toInt());
//  Serial.println((alpha.ldr_2).toInt());
//  Serial.println((alpha.soil_2).toInt());
  
  NodeMCU.write("h"+alpha.temp_1);
  NodeMCU.write(alpha.humi_1);
  NodeMCU.write(alpha.ldr_1);
  NodeMCU.write(alpha.soil_1);
  NodeMCU.write(alpha.temp_2);
  NodeMCU.write(alpha.humi_2);
  NodeMCU.write(alpha.ldr_2);
  NodeMCU.write(alpha.soil_2);
  mesh.update();
  
  
}
