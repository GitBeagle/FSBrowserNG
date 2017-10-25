/*
 Name: myUserConfigExample.ino
 Modified: 10/24/17
 Author: John

 Based on
 Name:		UserConfigExample.ino
 Created:	9/24/2017 10:39:12 AM
 Author:	Lee
 
 Modified to include configuration data in userConfig.json and userConfig.html
 Also integrated the user.html file in the callbacks.

 See the readme.md file in the data directory for more information
 
*/


#include <ESP8266WiFi.h>
#include "FS.h"
#include <WiFiClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FSWebServerLib.h"
#include <Hash.h>


// userConfig variables
int mqttServer[4];
String mqttUser;
String mqttPass;
int noVals;
int id[5];
int inCount;
int outCount;
String prefix;
String outSuffix[5];
String inSuffix[5];
String outTop[5];
String inTop[5];

byte node; // node number

void  callbackREST(AsyncWebServerRequest *request)
{
  
  DEBUGLOG("# args = %d\n", request->args());
  DEBUGLOG("method = %d\n", request->method());
  DEBUGLOG("url = %s\n", request->url().c_str());
  DEBUGLOG("content type = %s\n", request->contentType().c_str());
  DEBUGLOG("# params = %d\n", request->params());
  //its possible to test the url and do different things, 
  //test your rest URL
  if (request->url() == "/rest/userconfig")
  {
   
    //contruct and send and desired repsonse
    // Get user config data
    myUserConfig();
    String values = "mqttUser|"+ mqttUser +"|input\n";
    values += "mqttPass|"+ mqttPass +"|input\n";
    values += "mqttIP0|"+ (String)mqttServer[0] +"|input\n";
    values += "mqttIP1|"+ (String)mqttServer[1] +"|input\n";
    values += "mqttIP2|"+ (String)mqttServer[2] +"|input\n";
    values += "mqttIP3|"+ (String)mqttServer[3] +"|input\n";
    values += "noVals|"+ (String)noVals +"|input\n";
    values += "id0|"+ (String)id[0] +"|input\n";
    values += "id1|"+ (String)id[1] +"|input\n";
    values += "id2|"+ (String)id[2] +"|input\n";
    values += "id3|"+ (String)id[3] +"|input\n";
    values += "id4|"+ (String)id[4] +"|input\n";
    values += "prefix|"+ prefix +"|input\n";
    values += "inCount|"+ (String)inCount +"|input\n";
    values += "inSuffix0|"+ inSuffix[0] +"|input\n";
    values += "inSuffix1|"+ inSuffix[1] +"|input\n";
    values += "inSuffix2|"+ inSuffix[2] +"|input\n";
    values += "inSuffix3|"+ inSuffix[3] +"|input\n";
    values += "inSuffix4|"+ inSuffix[4] +"|input\n";
    values += "outCount|"+ (String)outCount +"|input\n";
    values += "outSuffix0|"+ outSuffix[0] +"|input\n";
    values += "outSuffix1|"+ outSuffix[1] +"|input\n";
    values += "outSuffix2|"+ outSuffix[2] +"|input\n";
    values += "outSuffix3|"+ outSuffix[3] +"|input\n";
    values += "outSuffix4|"+ outSuffix[4] +"|input\n";
    request->send(200, "text/plain", values);
    DEBUGLOG("Values = %s\n", values.c_str());
    values = "";

  } else if (request->url() == "/rest/user") 
  {
    String data = "";
    ESPHTTPServer.load_user_config("user1", data);
    String values = "user1|"+ data +"|input\n";

    ESPHTTPServer.load_user_config("user2", data);
    values += "user2|" + data + "|input\n";

    ESPHTTPServer.load_user_config("user3", data);
    values += "user3|" + data + "|input\n";
    request->send(200, "text/plain", values);
    DEBUGLOG("Values = %s\n", values.c_str());
    values = "";
  }
  else 
  { 
    //its possible to test the url and do different things, 
    String values = "message:Hello world! \nurl:" + request->url() + "\n";
    request->send(200, "text/plain", values);
    values = "";
  }
}

void  callbackPOST(AsyncWebServerRequest *request)
{
  DEBUGLOG("post # args = %d\n", request->args());
  DEBUGLOG("url = %s\n", request->url().c_str());
  //its possible to test the url and do different things, 
  if (request->url() == "/post/userconfig" || request->url() == "/post/user")
  {
    String target = "/";

    for (uint8_t i = 0; i < request->args(); i++) {
      DEBUGLOG("Arg %d: %s\r\n", i, request->arg(i).c_str());
      Serial.print(request->argName(i));  // comment these print stements after testing
      Serial.print(" : ");
      Serial.println(ESPHTTPServer.urldecode(request->arg(i)));

      //check for post redirect
      if (request->argName(i) == "afterpost")
      {
        DEBUGLOG("afterpost found %s\n", request->arg(i).c_str());
        target = ESPHTTPServer.urldecode(request->arg(i));
      }
      else  //or savedata in Json File
      {
        ESPHTTPServer.save_user_config(request->argName(i), request->arg(i));
        DEBUGLOG("saving argument: %s\n", request->arg(i).c_str());
      }
    }        

    request->redirect(target);

  }
  else
  {
    String values = "message:Hello world! \nurl:" + request->url() + "\n";
    request->send(200, "text/plain", values);
    values = "";

  }
}
/******************************************************************************
 * Setup
 *******************************************************************************/
void setup() {
  
	// WiFi is started inside library
	SPIFFS.begin(); // Not really needed, checked inside library and started if needed
	ESPHTTPServer.begin(&SPIFFS);
	/* add setup code here */

  //set optional callbacks
  ESPHTTPServer.setRESTCallback(callbackREST);
  ESPHTTPServer.setPOSTCallback(callbackPOST);
  
  // Get node address
  IPAddress myIP = WiFi.localIP();
  node = myIP[3];  // node number is the last octet of the IP address
  DEBUGLOG("Node = %d\n", node);
    
  // Get user config data
  myUserConfig();

}

void loop() {
	/* add main program code here */

	// DO NOT REMOVE. Attend OTA update from Arduino IDE
	ESPHTTPServer.handle();

}

void myUserConfig() {

  // Get all the data from the userConfig.json file
  ESPHTTPServer.load_user_config("mqttIP0", mqttServer[0]);
  ESPHTTPServer.load_user_config("mqttIP1", mqttServer[1]);
  ESPHTTPServer.load_user_config("mqttIP2", mqttServer[2]);
  ESPHTTPServer.load_user_config("mqttIP3", mqttServer[3]);
  ESPHTTPServer.load_user_config("mqttUser", mqttUser);
  ESPHTTPServer.load_user_config("mqttPass", mqttPass);
  ESPHTTPServer.load_user_config("noVals", noVals);
  ESPHTTPServer.load_user_config("id0", id[0]);
  ESPHTTPServer.load_user_config("id1", id[1]);
  ESPHTTPServer.load_user_config("id2", id[2]);
  ESPHTTPServer.load_user_config("id3", id[3]);
  ESPHTTPServer.load_user_config("id4", id[4]);
  ESPHTTPServer.load_user_config("inCount", inCount);
  ESPHTTPServer.load_user_config("outCount", outCount);
  ESPHTTPServer.load_user_config("prefix",  prefix);
  ESPHTTPServer.load_user_config("outSuffix0", outSuffix[0]);
  ESPHTTPServer.load_user_config("outSuffix1", outSuffix[1]);
  ESPHTTPServer.load_user_config("outSuffix2", outSuffix[2]);
  ESPHTTPServer.load_user_config("outSuffix3", outSuffix[3]);
  ESPHTTPServer.load_user_config("outSuffix4", outSuffix[4]);
  ESPHTTPServer.load_user_config("inSuffix0", inSuffix[0]);
  ESPHTTPServer.load_user_config("inSuffix1", inSuffix[1]);
  ESPHTTPServer.load_user_config("inSuffix2", inSuffix[2]);
  ESPHTTPServer.load_user_config("inSuffix3", inSuffix[3]);
  ESPHTTPServer.load_user_config("inSuffix4", inSuffix[4]);

  // Build the MQTT server IP address
  IPAddress mqttIP(mqttServer[0], mqttServer[1], mqttServer[2], mqttServer[3]);

  // Optional output of the user configuration data
  DEBUGLOG("userConfig Output:");
  DEBUGLOG("mqttServer = (%d, %d, %d, %d)\n", mqttServer[0], mqttServer[1], mqttServer[2], mqttServer[3]);
  DEBUGLOG("mqttUser = %s\n", mqttUser.c_str());
  DEBUGLOG("mqttPass = %s\n", mqttPass.c_str());
  DEBUGLOG("noVals = %d\n", noVals);
  DEBUGLOG("id[0] = %d\n", id[0]);
  DEBUGLOG("id[4] = %d\n", id[4]);
  DEBUGLOG("inCount = %d\n", inCount);
  DEBUGLOG("outCount = %d\n", outCount);
  DEBUGLOG("noVals = %d\n", noVals);
  DEBUGLOG("outSuffix[0] = %s\n", outSuffix[0].c_str());
  DEBUGLOG("outSuffix[1] = %s\n", outSuffix[1].c_str());
  DEBUGLOG("outSuffix[2] = %s\n", outSuffix[2].c_str());
  DEBUGLOG("outSuffix[3] = %s\n", outSuffix[3].c_str());
  DEBUGLOG("outSuffix[4] = %s\n", outSuffix[4].c_str());
  DEBUGLOG("inSuffix[0] = %s\n", inSuffix[0].c_str());
  DEBUGLOG("inSuffix[1] = %s\n", inSuffix[1].c_str());
  DEBUGLOG("inSuffix[2] = %s\n", inSuffix[2].c_str());
  DEBUGLOG("inSuffix[3] = %s\n", inSuffix[3].c_str());
  DEBUGLOG("inSuffix[4] = %s\n", inSuffix[4].c_str()); 

  // Build MQTT topics
  // My topics consist of a prefix/node number/suffix where the node number is the 
  // last octet of the ip address.  I use static IP addresses for all my nodes.
  
  char buf[20];
  for (byte i=0;i<5;i++) {
    sprintf(buf,"%s%s%d%s%s",prefix.c_str(),"/",node,"/",outSuffix[i].c_str());
    //Serial.println(buf);
    outTop[i] = buf;
    DEBUGLOG("outTop[%d] = %s\n", i, outTop[i].c_str());
    if (inSuffix[i] == "time" || inSuffix[i] == "file"){
      sprintf(buf,"%s%s%s",prefix.c_str(),"/",inSuffix[i].c_str());
      inTop[i] = buf;
      DEBUGLOG("inTop[%d] = %s\n", i, inTop[i].c_str());      
    } else {
      sprintf(buf,"%s%s%d%s%s",prefix.c_str(),"/",node,"/",inSuffix[i].c_str());
      inTop[i] = buf;
      DEBUGLOG("inTop[%d] = %s\n", i, inTop[i].c_str());
    }
  }
}

