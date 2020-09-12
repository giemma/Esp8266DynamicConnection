#include "String.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "Portal.h"

//costruttore
Portal::Portal(){
	server = new ESP8266WebServer(80);
   

}

bool Portal::connectToLocal()
{
  triedConnectedToLocal=true;
  //try to connect to saved wifi
  WiFi.begin(ssid, password);
  unsigned tryes=0;
  while (WiFi.status() != WL_CONNECTED )
  {
    //if(millis()>timeout+2000)
    if(tryes > 10)
    {
      //se dopo 10 secondi non si è ancora connesso allora ritorno false
      Serial.println(" connection timeout");
      break;
    }
    tryes++;
    delay(600);
    Serial.print(".");
  }
  Serial.print(" .");
  return WiFi.status() == WL_CONNECTED;
}

void Portal::initialize ()
{	
	Serial.println();
	Serial.println();
	Serial.println("Portal initializing...");
  
  Serial.println("Loading credentials...");
  loadCredentials();
 
	WiFi.softAP(apSsid, apPassword ); 
	myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	delay(1000);
	
	server->on("/", std::bind(&Portal::showIndexPage, this));
	server->on("/fwlink", std::bind(&Portal::showIndexPage, this));
	server->on("/configure", std::bind(&Portal::showConfigurePage, this));
	server->on("/save", std::bind(&Portal::showSavePage, this));
	server->on("/info", std::bind(&Portal::showInfoPage, this));
  server->on("/serverInfo", std::bind(&Portal::showServerInfoPage, this));
	//server.on("/", [&]() { showInfoPage();});
	server->onNotFound (std::bind(&Portal::showNotFoundPage, this));
	
	server->begin();  
	delay(2000);
	Serial.println("Portal initialized!");
  
  if(credentialsFound==true){
    Serial.println("Connecting to saved network...");
    connectedToLocal = connectToLocal();
    if(connectedToLocal==true){
      Serial.println(" connected");

      for(int i=0;i<100;i++){
        delay(100);
        sendBroadcastPacket();   
      }
      

    }else{
      Serial.println(" NOT connected. Please configure the network");
    }
  }
}

void Portal::handleClient ()
{
	server->handleClient();
}

void Portal::showIndexPage ()
{
	Serial.println("/");
	
	String body=String("<a href=\"/configure\">Configure</a> | <a href=\"/serverInfo\">serverInfo</a> | <a href=\"/info\">Info</a> ");
	String page=GetHtmlTemplate(body);
	
	server->sendHeader("Content-Length", String(page.length()));
	server->send(200, "text/html", page);
}

void Portal::showConfigurePage ()
{
	Serial.println("/configure");
	loadCredentials();

  if(triedConnectedToLocal==false)
  {
  //  Serial.println("Provo aconnettermi");
    connectedToLocal = connectToLocal();
  //  Serial.println(connectedToLocal);
  }
  
  
	int numberOfNetworks = WiFi.scanNetworks();
	String html = String();
 
    if(connectedToLocal){
      html+= "<div><label  style='color:green;'>Connected to " + (String(ssid)) +". IP:"+ WiFi.localIP().toString() +"</label> </div>";
    }else{
      html+= "<div><label style='color:red;'>Connection failed for " + (String(ssid)) +"</label></div>";
    }
    
    if(credentialsFound){
      html+= "<div><label  style='color:green;'>Credentials found</label> </div>";
    }else{
      html+= "<div><label style='color:red;'>Credentials NOT found</label> </div>";
    }
     html+="<FORM action=\"/save\" method=\"post\">";
     html+= "<div><label>Name (maxlength 30): </label> <input type='text' name='dname' value='" + (String(dName)) +"' maxlength='30' /></div>";
     html+= "<div><label>Password: </label> <input type='text' name='pwd' value='" + (String(password)) +"' maxlength='30' /></div>";
     html+="<div><label>Select a network:</label>";
     html+="<select name='ssid' >";

     String networks[numberOfNetworks];
     for(int i =0; i<numberOfNetworks; i++){
        bool networkFound=false;   
        String network = WiFi.SSID(i);

        //remove duplicates
        for(int j=0;j<=i;j++){
          if(networks[j]==network){
            networkFound=true;
          }
        }

        if(networkFound==false){
            networks[i]=network;
        }else{
            networks[i]="";
        }
     }
     
     for(int i =0; i<numberOfNetworks; i++){
        if(networks[i] == "")
        {
          continue;
        }
        html+= "<option value='";
        html+= networks[i];              
        html+= "'";
        if(networks[i]== ssid){
            html += " selected ";
          }        
        html+= " >";
        html+= networks[i];
        
        //html += " (";
        //html += WiFi.RSSI(i);
        //html += ")";
        
        html+= "</option>";
     }
     html+= "</select></div>";
     html+= "<div><input type='submit' value='Submit' ></div>";
     html+= "</form>";

  
	String page=GetHtmlTemplate(html);

  Serial.println(page);
  
	server->sendHeader("Content-Length", String(page.length()));
	server->send(200, "text/html", page);
}

void Portal::showSavePage ()
{
	Serial.println("/save");

  String html = String("");
  html+= "<div><label  style='color:green;'>Credentials found</label> </div>";
  html+= "<div><a href=\"/configure?"+ String(millis()) + "\">Return to configure</a> Credentials found</label> </div>";
	if (server->hasArg("dname") && server->hasArg("pwd") && server->hasArg("ssid")  ) { //se è stato inviato un valore
    
    server->arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
    server->arg("pwd").toCharArray(password, sizeof(password) - 1);
    server->arg("dname").toCharArray(dName, sizeof(dName) - 1);

    saveCredentials();
    triedConnectedToLocal=false;
/*
    server.arg("dName").toCharArray(dName, sizeof(dName) - 1);
    server.arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
    server.arg("pwd").toCharArray(password, sizeof(password) - 1);
*/  
    Serial.println(ssid);
    Serial.println(password);
    Serial.println(dName);
    
  }else{
    showConfigurePage();
    return;
  }
    
	String page=GetHtmlTemplate(html);
	server->sendHeader("Content-Length", String(page.length()));
	server->send(200, "text/html", page);
}

void Portal::showInfoPage ()
{
	Serial.println("/info");
	
	String info ="Info..";
	
	String html=GetHtmlTemplate(info);
	server->sendHeader("Content-Length", String(html.length()));
	server->send(200, "text/html", html);
}

void Portal::sendBroadcastPacket(){
  if(connectedToLocal==false){
    Serial.println("I can't send packets because I'm not connected :(");
    return;
  }

  Serial.println("Sending packet...");
  UDP.begin(localUdpPort);
  UDP.beginPacket("255.255.255.255", localUdpPort);
  UDP.write("Hello");
  UDP.endPacket();
  delay(10);
  Serial.print(" sent!");
}

void Portal::showServerInfoPage ()
{
  Serial.println("/ServerInfo");

  String info =String("Sending packets sent to 255.255.255.255 on ");
  info+= localUdpPort ;
  info += " port...";
  
  sendBroadcastPacket();

  Serial.print(info);

  
  
  Serial.println("sent!");
    
  String html=GetHtmlTemplate(info);
  server->sendHeader("Content-Length", String(html.length()));
  server->send(200, "text/html", html);
}

void Portal::showNotFoundPage ()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server->uri();
	message += "\nMethod: ";
	message += ( server->method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server->args();
	message += "\n";

	for ( uint8_t i = 0; i < server->args(); i++ ) {
		message += " " + server->argName ( i ) + ": " + server->arg ( i ) + "\n";
	}
	
	server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server->sendHeader("Pragma", "no-cache");
	server->sendHeader("Expires", "-1");
	server->sendHeader("Content-Length", String(message.length()));
	server->send ( 404, "text/plain", message );
}

String Portal::GetHtmlTemplate(String content){
  String result = String( "<!DOCTYPE HTML>")+  
    "<html>"+
    "<head>"+
    "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"+
    "<title>Device configuration</title>"+
    "<style>"+
    "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""+
	"a{background-color: gray;margin:5px;}"+
    "</style>" +
    "</head>" +
    "<body>" +
    
    "<h1>Device configuration</h1>" +

    content +
        
    "</body>" +
    "</html>";

  return result;
}

void Portal::loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0+sizeof(ssid), password);
  EEPROM.get(0+sizeof(ssid) + sizeof(password), dName);
  char ok[2+1];
  EEPROM.get(0+sizeof(ssid)+sizeof(password) + sizeof(dName), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    credentialsFound=false;
  }else{
    credentialsFound=true;
  }
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(dName);
}


void Portal::saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0+sizeof(ssid), password);
  EEPROM.put(0+sizeof(ssid) + sizeof(password), dName);
  char ok[2+1] = "OK";
  EEPROM.put(0+sizeof(ssid)+sizeof(password) + sizeof(dName), ok);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Credentials saved");
}
