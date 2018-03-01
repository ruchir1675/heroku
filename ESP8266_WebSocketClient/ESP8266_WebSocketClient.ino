
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>


const char* ssid     = "xxxxxxx"; //enter your ssid/ wi-fi(case sensitiv) router name - 2.4 Ghz only
const char* password = "xxxxxxxx";     // enter ssid password (case sensitiv)
char host[] = "alexatestpi.herokuapp.com"; //enter your Heroku domain name like "espiot.herokuapp.com" 
 // enter the distance in inches between you ultrasonic sensor and garage door

int duration, distance;
const int relayPin1 = 16;
const int relayPin2 = 5; 
const int relayPin3 = 4;
const int relayPin4 = 14;
int pingCount = 0;
int port = 80;
char path[] = "/ws"; 
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

DynamicJsonBuffer jsonBuffer;
String currState, oldState, message;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) { 


    switch(type) {
        case WStype_DISCONNECTED:
            //USE_SERIAL.printf("[WSc] Disconnected!\n");
           Serial.println("Disconnected! ");
            break;
            
        case WStype_CONNECTED:
            {
             Serial.println("Connected! ");
          // send message to server when Connected
            webSocket.sendTXT("Connected");
            }
            break;
            
        case WStype_TEXT:
            Serial.println("Got data");
            //Serial.println("looping...");
            
            
            
            
            processWebScoketRequest((char*)payload);

            break;
            
        case WStype_BIN:

            hexdump(payload, length);
            Serial.print("Got bin");
            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
    pinMode(relayPin1, OUTPUT);
    pinMode(relayPin2, OUTPUT);
    pinMode(relayPin3, OUTPUT);
    pinMode(relayPin4, OUTPUT);

    
      for(uint8_t t = 4; t > 0; t--) {
          delay(1000);
      }
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    
    //Serial.println(ssid);
    WiFiMulti.addAP(ssid, password);

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
        delay(1000);
    }
    Serial.println("Connected to wi-fi");
    webSocket.begin(host, port, path);
    webSocket.onEvent(webSocketEvent);

}

void loop() {

    webSocket.loop();
        delay(100);
  // make sure after every 40 seconds send a ping to Heroku
  //so it does not terminate the websocket connection
  //This is to keep the conncetion alive between ESP and Heroku
  if (pingCount > 20) {
    pingCount = 0;
    webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
  }
  else {
    pingCount += 1;
  }
}


void processWebScoketRequest(String data){
            String jsonResponse = "{\"version\": \"1.0\",\"sessionAttributes\": {},\"response\": {\"outputSpeech\": {\"type\": \"PlainText\",\"text\": \"<text>\"},\"shouldEndSession\": true}}";
            JsonObject& req = jsonBuffer.parseObject(data);

            String instance = req["instance"];
            String state = req["state"];
            String query = req["query"];
            String message = "{\"event\": \"OK\"}";
            
            Serial.println("Data2-->"+data);
            Serial.println("State-->" + state);

            if(query == "?"){ //if command then execute
              Serial.println("Recieved query!");
                 if(currState=="open"){
                      message = "On";
                    }else{
                      message = "Off";
                    }
                   jsonResponse.replace("<text>", "Bed room light " + instance + " is " + message );
                   webSocket.sendTXT(jsonResponse);
                   
            }else if(query == "cmd"){ //if query check state
              Serial.println("Recieved command!");
            if(instance == "bed"){
                    if(state == "on"){
                            digitalWrite(relayPin1, HIGH);
                            message = "turning   on";
                            Serial.println(instance);
                          }else if(state == "off"){
                            message = "turning   off";
                            digitalWrite(relayPin1, LOW);
                            Serial.println(instance);
                        }
                        
            } 
           
            if(instance == "hall"){
              if(state == "on"){
                            digitalWrite(relayPin2, HIGH);
                            message = "turning   on";
                            Serial.println(instance);
                          }else if(state == "off"){
                            message = "turning   off";
                            digitalWrite(relayPin2, LOW);
                            Serial.println(instance);
                        }
            }
           
            if(instance == "kitchen"){
              if(state == "on"){
                            digitalWrite(relayPin3, HIGH);
                            message = "turning   on";
                            Serial.println(instance);
                          }else if(state == "off"){
                            message = "turning   off";
                            digitalWrite(relayPin3, LOW);
                            Serial.println(instance);
                        }
            }
         
            if(instance == "bath"){
              if(state == "on"){
                            digitalWrite(relayPin4, HIGH);
                            message = "turning   on";
                            Serial.println(instance);
                          }else if(state == "off"){
                            message = "turning   off";
                            digitalWrite(relayPin4, LOW);
                            Serial.println(instance);
                        }
            }
            if(instance == "all"){
              if(state == "on"){
                            digitalWrite(relayPin1, HIGH);
                            digitalWrite(relayPin2, HIGH);
                            digitalWrite(relayPin3, HIGH);
                            digitalWrite(relayPin4, HIGH);
                            message = "turning   on";
                            Serial.println(instance);
                          }else if(state == "off"){
                            message = "turning   off";
                            digitalWrite(relayPin4, LOW);
                            digitalWrite(relayPin4, LOW);
                            digitalWrite(relayPin4, LOW); 
                            digitalWrite(relayPin4, LOW);
                              
                            Serial.println(instance);
                        }
            }
            
                  jsonResponse.replace("<text>",  instance + " room light   is now " + message );
                  webSocket.sendTXT(jsonResponse);
                  }

            
            else{//can not recognized the command
                    Serial.println("Command is not recognized!");
                   jsonResponse.replace("<text>", "Command is not recognized by room light in Alexa skill");
                   webSocket.sendTXT(jsonResponse);
            }
            Serial.print("Sending response back");
            Serial.println(jsonResponse);
                  // send message to server
                  webSocket.sendTXT(jsonResponse);
}



