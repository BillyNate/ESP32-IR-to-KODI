#include "commandstore.h"

using namespace std;

int Commandstore::begin(char* stringsns, char* commandsns, char* keysns)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  
  err = nvs_flash_init();
  if(err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "Error in nvs_flash_init: %i", err);
  }
  
  err = nvs_open(stringsns, NVS_READWRITE, &stringstorage);
  if(err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "Error in nvs_open: %i", err);
  }
  
  err = nvs_open(commandsns, NVS_READWRITE, &cmdstorage);
  if(err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "Error in nvs_open: %i", err);
  }

  err = nvs_open(keysns, NVS_READWRITE, &keystorage);
  if(err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "Error in nvs_open: %i", err);
  }
}

void Commandstore::eraseAll()
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  
  err = nvs_erase_all(stringstorage);
  logNVSError(err, false);
  err = nvs_commit(stringstorage);
  logNVSError(err, false);
  if(cmdstorage != stringstorage)
  {
    err = nvs_erase_all(cmdstorage);
    logNVSError(err, false);
    err = nvs_commit(cmdstorage);
    logNVSError(err, false);
  }
  if(keystorage != stringstorage && keystorage != cmdstorage)
  {
    err = nvs_erase_all(keystorage);
    logNVSError(err, false);
    err = nvs_commit(keystorage);
    logNVSError(err, false);
  }
}

void Commandstore::printNVSDebug()
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  uint8_t keyLength = getKeyNumber();
  unsigned long keys[keyLength];
  int8_t commandID;
  string kodicommand;
  size_t strlength;
  uint8_t i;

  getKeys(keys);

  for(i=0; i<keyLength; i++)
  {
    commandID = getCommandID(keys[i]);
    kodicommand = getKodiString(commandID);
    
    ESP_LOGE(LOG_TAG, "IRCode %02X points to commandID %i containing the Kodi commandstring \"%s\"", keys[i], commandID, kodicommand);
  }

  // TODO: Add check if no other commandID / Kodi commandstring exists!
}

void Commandstore::setCommand(unsigned long ircode, string kodicommand)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  int8_t newCommandID = findCommandID(kodicommand);
  int8_t oldCommandID = getCommandID(ircode);
  unsigned long otherIRCode = 0x00000000;
  uint8_t otherIRCodePos = 0;
  int8_t keyIndex;

  if(oldCommandID >= 0) // Check if any other IRcode still uses the current kodicommand
  {
    otherIRCode = findIRCode(oldCommandID, 0, &otherIRCodePos);
    if(otherIRCode == ircode) // That's the code we're currently handling, not another XD
    {
      keyIndex = indexOfKey(ircode);
      otherIRCode = findIRCode(oldCommandID, otherIRCodePos + 1);
    }
  }

  ESP_LOGE(LOG_TAG, "newCommandID: %i, oldCommandID: %i, otherIRCode: 0x%08X", newCommandID, oldCommandID, otherIRCode);

  if(otherIRCode == 0x00000000 && newCommandID < 0) // No other IRcode uses the old kodicommand && the new kodi command does not yet exist in storage
  {
    ESP_LOGE(LOG_TAG, "Setter %i", 1);
    // Update existing kodicommand!
    if(oldCommandID >= 0)
    {
      setKodiString(oldCommandID, kodicommand);
    }
    else
    {
      newCommandID = getCommandIDNumber(); // Length of existing array is ID of new item
      
      char cmdnr[3];
      itoa(newCommandID, cmdnr, 10);
      ESP_LOGE(LOG_TAG, "Set ircode (0x%08X) to  newCommandID: %s", ircode, cmdnr);
      
      setKodiString(newCommandID, kodicommand);
      setCommandID(ircode, newCommandID);
    }
  }
  else if(otherIRCode == 0x00000000) // No other IRcode uses the old kodicommand && the new kodicommand does already exist in storage
  {
    ESP_LOGE(LOG_TAG, "Setter %i", 2);
    // Update ircode to point to existing new kodicommand
    // Loop over all KodiStrings, slicing out the old kodicommand
    // Loop over all IRCodes, adjusting the commandID where necessary
    
    setCommandID(ircode, newCommandID);
    if(oldCommandID >= 0)
    {
      removeCommand(oldCommandID);
    }
  }
  else if(newCommandID == -1) // The old kodicommand is used by another IRcode && the new kodicommand does not yet exist in storage
  {
    ESP_LOGE(LOG_TAG, "Setter %i", 3);
    // No need to remove the old kodicommand
    // Add the new kodicommand to the storage
    
    newCommandID = getCommandIDNumber(); // Length of existing array is ID of new item
    setKodiString(newCommandID, kodicommand);
    setCommandID(ircode, newCommandID);
  }
  else // The old kodicommand is used by another IRcode && the new kodicommand does already exist in storage
  {
    ESP_LOGE(LOG_TAG, "Setter %i", 4);
    // No need to remove the old kodicommand
    // Set the IRCode to point to the correct commandID

    setCommandID(ircode, newCommandID);
  }

  if(indexOfKey(ircode) < 0)
  {
    addKey(ircode);
  }
}

std::string Commandstore::getCommand(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  int8_t commandID = getCommandID(ircode);
  string kodicommand;
  if(commandID >= 0)
  {
    kodicommand = getKodiString(commandID);
  }
  return kodicommand;
}

void Commandstore::removeCommand(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  
  int8_t commandID = getCommandID(ircode);
  uint8_t otherIRCodePos = 0;
  unsigned long otherIRCode = findIRCode(commandID, 0, &otherIRCodePos);

  if(otherIRCode == ircode) // Keep on seaching if the first search returns the IRCode we're already about to remove
  {
    otherIRCode = findIRCode(commandID, otherIRCodePos + 1);
  }

  ESP_LOGE(LOG_TAG, "otherIRCode: 0x%08X", otherIRCode);
  
  if(otherIRCode == 0x00000000) // No other IRCode uses this command, remove it from storage
  {
    removeCommand(commandID); // This will slice out the command from the stringstorage
  }
  
  removeCommandID(ircode);
  removeKey(ircode);
}

void Commandstore::removeCommand(int8_t commandID)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  int8_t commandIDLength = getCommandIDNumber();
  int8_t i;
  string tmpKodicommand;
  size_t kodicommandLength;
  uint8_t keyLength = getKeyNumber();
  unsigned long keys[keyLength];
  int8_t tmpCommandID;
  
  for(i=commandID; i<commandIDLength-1; i++) // Move all Kodi strings >= commandID one place back
  {
    tmpKodicommand = getKodiString(i + 1);
    setKodiString(i, tmpKodicommand);
  }
  removeKodiString(commandIDLength - 1); // Remove last Kodi string

  getKeys(keys);
  
  for(i=0; i<keyLength; i++) // Correct all commandID values of the IRCodes
  {
    tmpCommandID = getCommandID(keys[i]);
    
    if(tmpCommandID >= commandID)
    {
      setCommandID(keys[i], tmpCommandID - 1);
    }
  }
}

uint8_t Commandstore::length()
{
  return getKeyNumber();
}

void Commandstore::getCommands(unsigned long *ircodes)
{
  getKeys(ircodes);
}

std::string Commandstore::getKodiString(int8_t commandID)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  char cmdnr[3];
  size_t strlength;

  itoa(commandID, cmdnr, 10);
  
  err = nvs_get_str(stringstorage, cmdnr, NULL, &strlength);
  
  if(err != ESP_OK)
  {
    logNVSError(err);
    return string();
  }
  
  char command[strlength];
  
  err = nvs_get_str(stringstorage, cmdnr, command, &strlength);
  logNVSError(err, false);

  return string(command);
}

void Commandstore::setKodiString(int8_t commandID, string kodicommand)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  /*
   * This function only sets the kodicommand at the commandID position,
   * it does not check nor append.
   */
  esp_err_t err;
  char cmdnr[3];
  
  itoa(commandID, cmdnr, 10);

  err = nvs_set_str(stringstorage, cmdnr, kodicommand.c_str());
  logNVSError(err, false);
  err = nvs_commit(stringstorage);
  logNVSError(err, false);
}

void Commandstore::removeKodiString(int8_t commandID)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  /*
   * This function only removes the key/value from the stringstorage
   * it does no checks, and does not remove the appropriate ircode either.
   * Best used in conjuction with removeKey and removeCommandID!
   */
  esp_err_t err;
  char cmdnr[3];
  
  itoa(commandID, cmdnr, 10);
  
  err = nvs_erase_key(stringstorage, cmdnr);
  logNVSError(err, false);
}

int8_t Commandstore::findCommandID(string kodicommand)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  int8_t commandLength = getCommandIDNumber();
  char cmdnr[3];
  size_t strlength;
  
  for(int i=0; i<commandLength; i++)
  {
    itoa(i, cmdnr, 10);
    err = nvs_get_str(stringstorage, cmdnr, NULL, &strlength);

    if(err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
      logNVSError(err);
      break;
    }
    
    if(err != ESP_ERR_NVS_NOT_FOUND)
    {
      char *command = (char*) malloc(strlength * sizeof(char));
      
      err = nvs_get_str(stringstorage, cmdnr, command, &strlength);
      logNVSError(err, false);
      
      if(kodicommand.compare(command) == 0)
      {
        return i;
      }
      free(command);
    }
  }
  
  return -1;
}

int8_t Commandstore::getCommandIDNumber()
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  uint8_t keyslength = getKeyNumber();
  unsigned long keys[keyslength];
  int8_t commandLength = -1;
  int8_t commandTMP;
  getKeys(keys);

  for(int i=0; i<keyslength; i++)
  {
    commandTMP = getCommandID(keys[i]);
    if(commandTMP > commandLength)
    {
      commandLength = commandTMP;
    }
  }
  return commandLength + 1;
}

int8_t Commandstore::getCommandID(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  char hex[16];
  int8_t id = -1;

  utoa(ircode, hex, 16);
  err = nvs_get_i8(cmdstorage, hex, &id);
  if(err == ESP_ERR_NVS_NOT_FOUND)
  {
    return -1;
  }
  logNVSError(err, false);

  return id;
}

void Commandstore::setCommandID(unsigned long ircode, int8_t commandID)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  char hex[16];
  
  utoa(ircode, hex, 16);
  err = nvs_set_i8(cmdstorage, hex, commandID);
  logNVSError(err, false);
  err = nvs_commit(cmdstorage);
  logNVSError(err, false);
}

void Commandstore::removeCommandID(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  /*
   * This function only removes the key/value from the cmdstorage
   * it does no checks, and does not remove the appropriate kodi command either.
   * Best used in conjuction with removeKey and removeKodiString!
   */
  esp_err_t err;
  char hex[16];
  
  utoa(ircode, hex, 16);
  err = nvs_erase_key(cmdstorage, hex);
  logNVSError(err, false);
  err = nvs_commit(cmdstorage);
  logNVSError(err, false);
}

uint8_t Commandstore::getKeyNumber()
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  size_t keyssize = 0;
  
  err = nvs_get_blob(keystorage, "keys", NULL, &keyssize);
  if(err == ESP_ERR_NVS_NOT_FOUND)
  {
    return 0;
  }
  logNVSError(err, false);
  return (keyssize / sizeof(unsigned long));
}

void Commandstore::getKeys(unsigned long *ircodes)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  size_t keyssize = 0;
  
  err = nvs_get_blob(keystorage, "keys", NULL, &keyssize);
  if(err == ESP_ERR_NVS_NOT_FOUND)
  {
    return;
  }
  logNVSError(err, false);
  
  err = nvs_get_blob(keystorage, "keys", ircodes, &keyssize);
  if(err == ESP_ERR_NVS_NOT_FOUND)
  {
    return;
  }
  logNVSError(err, false);
}

void Commandstore::addKey(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  esp_err_t err;
  uint8_t keyslength = getKeyNumber();
  ESP_LOGE(LOG_TAG, "Number of keys already known: %i", keyslength);
  unsigned long keys[keyslength + 1];
  getKeys(keys);
  keys[keyslength] = ircode;
  
  err = nvs_set_blob(keystorage, "keys", keys, (keyslength + 1) * sizeof(unsigned long));
  logNVSError(err, false);
  err = nvs_commit(keystorage);
  logNVSError(err, false);
}

void Commandstore::removeKey(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  uint8_t keyslength = getKeyNumber();
  unsigned long keys[keyslength];
  int8_t index = indexOfKey(ircode);
  
  if(index >= 0)
  {
    esp_err_t err;
    
    getKeys(keys);
    memmove(&keys[index], &keys[index+1], (keyslength - index - 1) * sizeof(unsigned long));
    err = nvs_set_blob(keystorage, "keys", keys, (keyslength - 1) * sizeof(unsigned long));
    logNVSError(err, false);
    err = nvs_commit(keystorage);
    logNVSError(err, false);
  }
  // ADD error here?
}

int8_t Commandstore::indexOfKey(unsigned long ircode)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  uint8_t keyslength = getKeyNumber();
  unsigned long keys[keyslength];
  getKeys(keys);

  for(int8_t i=0; i<keyslength; i++)
  {
    if(memcmp(&ircode, &keys[i], sizeof(unsigned long)) == 0)
    {
      return i;
    }
  }
  return -1;
}

unsigned long Commandstore::findIRCode(int8_t commandID, uint8_t startAt, uint8_t *foundAt)
{
  ESP_LOGE(LOG_TAG, "Calling function %s", __FUNCTION__);
  uint8_t keyslength = getKeyNumber();
  unsigned long keys[keyslength];
  int8_t otherCommandID;
  getKeys(keys);

  for(int i=startAt; i<keyslength; i++)
  {
    otherCommandID = getCommandID(keys[i]);
    if(commandID == otherCommandID)
    {
      if(foundAt != NULL)
      {
        *foundAt = i;
      }
      return keys[i];
    }
  }

  return 0;
}

void Commandstore::logNVSError(esp_err_t nvserror, bool logIfOK)
{
  switch(nvserror)
  {
    case ESP_OK:
      if(logIfOK)
      {
        ESP_LOGE(LOG_TAG, "NVS error: %s", "OK");
      }
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      ESP_LOGE(LOG_TAG, "NVS error: %s", "NVS_NOT_FOUND");
      break;
    case ESP_ERR_NVS_INVALID_HANDLE:
      ESP_LOGE(LOG_TAG, "NVS error: %s", "INVALID_HANDLE");
      break;
    case ESP_ERR_NVS_INVALID_NAME:
      ESP_LOGE(LOG_TAG, "NVS error: %s", "INVALID_NAME");
      break;
    case ESP_ERR_NVS_INVALID_LENGTH:
      ESP_LOGE(LOG_TAG, "NVS error: %s", "INVALID_LENGTH");
      break;
    default:
      ESP_LOGE(LOG_TAG, "Unknown NVS error");
  }
}
