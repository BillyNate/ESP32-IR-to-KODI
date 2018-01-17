#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-fpermissive"

/* Copyright (c) 2017 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 * 
 * Based on the Code from Neil Kolban: https://github.com/nkolban/esp32-snippets/blob/master/hardware/infra_red/receiver/rmt_receiver.c
 * Current code on https://github.com/pcbreflux/espressif/tree/master/esp32/arduino/sketchbook/ESP32_IR_Remote/ir_demo
 */
#include <numeric>
#include <WiFi.h>

#include "ESP32_IR_Remote.h"
#include "xbmcclient.h"
#include "commandstore.h"
#include "ESP32-IR-to-KODI.h"

// Somewhere there's a macro messing things up... Disable it!:
#undef connect(a,b,c)

const int RECV_PIN = 23; // pin on the ESP32

const char* ssid     = ""; // Your network SSID to connect to
const char* password = ""; // password of the SSID
const char* kodihost = ""; // IP address of your KODI machine
const unsigned int kodiport = 9777;
const char* url = "/";
const uint16_t wificlientport = 80;

ESP32_IRrecv irrecv(RECV_PIN, 3);
static unsigned long command = 0;

WiFiUDP udp;
CXBMCClient eventclient = CXBMCClient(udp, kodihost, kodiport);

WiFiServer wifiserver(80);

Commandstore cmdstore;

static bool listening = false;
static unsigned long caught = 0;

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  Serial.print("Starting WiFi...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.print(" WiFi connected!");
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Setting up IR...");
  pinMode(RECV_PIN, INPUT);
  irrecv.init();
  Serial.println(" Done!");

  Serial.print("Starting Kodi client...");
  eventclient.SendHELO("ESP32 Remote", ICON_PNG);
  Serial.println(" Complete");

  Serial.print("Initializing NVS...");
  cmdstore.begin();
  Serial.println(" Done!");
  
  Serial.print("Starting webserver...");
  wifiserver.begin();
  Serial.println(" Done!");

  Serial.print("Setting up tasks...");
  xTaskCreate(pingKodi, "pingKodi", 10000, NULL, 1, NULL);
  xTaskCreate(readIR,   "readIR",   10000, NULL, 1, NULL);
  xTaskCreate(serveWeb, "serveWeb", 10000, NULL, 1, NULL);
  Serial.println(" Done!");

  Serial.println("Setup complete!");
}

void serveWeb(void * parameter)
{
  WiFiClient serverclient;
  while(1)
  {
    serverclient = wifiserver.available();

    if(serverclient)                                // if you get a client,
    {
      Serial.println("New Client.");          // print a message out the serial port
      std::string currentLine = "";           // make a String to hold incoming data from the client
      std::string urlLine = "";
      bool respond = false;
      unsigned int page = 3;
      unsigned long startedLoopOn = 0;
      unsigned int index = 0;
      while(serverclient.connected())               // loop while the client's connected
      {
        respond = false;
        if(serverclient.available())                // if there's bytes to read from the client,
        {
          char c = serverclient.read();             // read a byte, then
          /*
          Serial.write(c);                    // print it out the serial monitor
          */
          if(c == '\n')                       // if the byte is a newline character
          {
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if(currentLine.find("GET /") != std::string::npos)
            {
              urlLine = currentLine;
            }
            
            if(currentLine.length() == 0)
            {
              respond = true;
            }
            else // if you got a newline, then clear currentLine:
            {
              currentLine = "";
            }
          }
          else if(c != '\r')  // if you got anything else but a carriage return character,
          {
            currentLine += c;      // add it to the end of the currentLine
          }

          if(respond)
          {
            if(urlLine.find("GET / ") != std::string::npos)
            {
              page = WEBPAGE_INDEX;
            }
            else if(urlLine.find("GET /getall ") != std::string::npos)
            {
              page = WEBPAGE_GETALL;
            }
            else if(urlLine.find("GET /listen ") != std::string::npos)
            {
              page = WEBPAGE_LISTEN;
            }
            else if(index = urlLine.find("GET /change-") != std::string::npos)
            {
              caught = strtoul(urlLine.substr(index + 11, urlLine.find(":") - index + 11).c_str(), NULL, 16);
              Serial.print("Change received: ");
              Serial.println(caught, HEX);
              page = WEBPAGE_CHANGE;
            }
            else if(index = urlLine.find("GET /del-") != std::string::npos)
            {
              caught = strtoul(urlLine.substr(index + 8, 8).c_str(), NULL, 16);
              Serial.print("Delete received: ");
              Serial.println(caught, HEX);
              page = WEBPAGE_DELETE;
            }
          }
          
          if(respond)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            serverclient.println("HTTP/1.1 200 OK");
            serverclient.print("Content-type: ");
            if(page != WEBPAGE_GETALL)
            {
              serverclient.print("text/html");
            }
            else
            {
              serverclient.println("application/json");
            }
            serverclient.println("; charset=UTF-8");
            serverclient.println();
            
            if(page == WEBPAGE_INDEX)
            {
              serverclient.println("<!doctype html><html><head><script src=\"https://billynate.github.io/ESP32-IR-to-KODI/configurator.js\"></script></head><body></body></html>");
            }
            else if(page == WEBPAGE_GETALL)
            {
              // TODO: Loop over all IR codes and their kodicommands, output this to the serverclient as JSON
              uint8_t commandsLength = cmdstore.length();
              unsigned long ircodes[commandsLength];
              std::string kodicommand;
              size_t kodicommandSize;
              cmdstore.getCommands(ircodes);
              serverclient.print("[");
              for(uint8_t i=0; i<commandsLength; i++)
              {
                if(i > 0)
                {
                  serverclient.print(",");
                }
                kodicommand = cmdstore.getCommand(ircodes[i]);
                serverclient.print("{\"ir\":\"");
                serverclient.print(ircodes[i], HEX);
                serverclient.print("\",\"cmd\":\"");
                Serial.println(kodicommand.c_str());
                serverclient.print(kodicommand.c_str());
                serverclient.print("\"}");
              }
              serverclient.print("]");
            }
            else if(page == WEBPAGE_LISTEN)
            {
              caught = 0x00000000;
              listening = true;
              startedLoopOn = millis();
              while(caught == 0x00000000 && millis() - startedLoopOn < 10 * 1000)
              {
                delay(50);
              }
              if(caught != 0x00000000)
              {
                std::string kodiCommand;
                size_t kodiCommandSize = 0;

                kodiCommand = cmdstore.getCommand(caught);

                if(kodiCommandSize == 0) // This command isn't added yet, so do it now
                {
                  kodiCommandSize = 5 * sizeof(char);
                  cmdstore.setCommand(caught, std::string("noop"));
                }
                
                serverclient.println(caught, HEX);
                caught = 0;
              }
            }
            else if(page == WEBPAGE_CHANGE)
            {
              size_t pos = urlLine.find(":") + 1;
              size_t len = urlLine.find("HTTP") - pos - 1;
              Serial.println(urlDecode(urlLine.substr(pos, len)).c_str());
              cmdstore.setCommand(caught, urlDecode(urlLine.substr(pos, len)));
            }
            else if(page == WEBPAGE_DELETE)
            {
              cmdstore.removeCommand(caught);
            }
            // The HTTP response ends with another blank line:
            serverclient.println();            
            // break out of the while loop:
            break;
          }
        }
      }
      // close the connection:
      serverclient.stop();
      Serial.println("Client Disconnected.");
    }
  }
}

void pingKodi(void * parameter)
{
  while(1)
  {
    delay(15*1000);
    Serial.println("Ping!");
    eventclient.SendPING();
  }
}

void readIR(void *parameter)
{
  std::string kodiCommand;
  
  while(1)
  {
    command = irrecv.readIR();
    if(listening && command != 0x00000000)
    {
      caught = command;
      listening = false;
    }
    else if(command != 0x00000000)
    {
      kodiCommand = cmdstore.getCommand(command);
      
      if(!kodiCommand.empty())
      {
        eventclient.SendACTION(kodiCommand.c_str(), ACTION_BUTTON);
      }
      else
      {
        Serial.print("Received unknown IR command: ");
        Serial.println(command, HEX);
      }
    }
    delay(10);
  }
}

void loop()
{
  delay(portMAX_DELAY);
}

std::string urlDecode(std::string str)
{
  /*
   * Function taken from https://stackoverflow.com/questions/154536/encode-decode-urls-in-c#answer-29962178
   */
  std::string ret;
  char ch;
  int i, ii, len = str.length();

  for(i=0; i<len; i++)
  {
    if(str[i] != '%')
    {
      if(str[i] == '+')
        ret += ' ';
      else
        ret += str[i];
    }
    else
    {
      sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
      ch = static_cast<char>(ii);
      ret += ch;
      i = i + 2;
    }
  }
  return ret;
}
