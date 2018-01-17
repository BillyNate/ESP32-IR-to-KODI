#ifndef COMMANDSTORE_H_
#define COMMANDSTORE_H_

#include <cstring>
#include <string>

extern "C"
{
  #include "esp_log.h"
  #include "nvs_flash.h"
  #include "nvs.h"
}

#define LOG_TAG "CMD STORE"

class Commandstore
{
  protected:
    nvs_handle stringstorage;
    nvs_handle cmdstorage;
    nvs_handle keystorage;
    
    void removeCommand(int8_t commandID);

    unsigned long findIRCode(int8_t commandID, uint8_t startAt=0, uint8_t *foundAt=NULL);
    
    uint8_t getKeyNumber();
    void getKeys(unsigned long *ircodes);
    void addKey(unsigned long ircode);
    void removeKey(unsigned long ircode);
    int8_t indexOfKey(unsigned long ircode);

    int8_t getCommandIDNumber();
    int8_t getCommandID(unsigned long ircode);
    void setCommandID(unsigned long ircode, int8_t commandID);
    void removeCommandID(unsigned long ircode);
    int8_t findCommandID(std::string kodicommand);
    
    std::string getKodiString(int8_t commandID);
    void setKodiString(int8_t commandID, std::string kodicommand);
    void removeKodiString(int8_t commandID);
    
    void logNVSError(int nvserror, bool logIfOK=true);
    
  public:
    int begin(char* stringsns="strings", char* commandsns="commands", char* keysns="keys");
    std::string getCommand(unsigned long ircode);
    void setCommand(unsigned long ircode, std::string kodicommand);
    void removeCommand(unsigned long ircode);
    void getCommands(unsigned long *ircodes);
    uint8_t length();
    void eraseAll();
    void printNVSDebug();
};

#endif /* COMMANDSTORE_H_ */
