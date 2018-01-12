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
 */

#ifndef ESP32_IR_REMOTE_H_
#define ESP32_IR_REMOTE_H_

extern "C"
{
  #include <stdint.h>
  #include "esp32-hal.h"
  #include "esp_intr.h"
  #include "driver/gpio.h"
  #include "driver/rmt.h"
  #include "driver/periph_ctrl.h"
  #include "freertos/semphr.h"
  #include "soc/rmt_struct.h"
}

class ESP32_IRrecv
{
  public:
    ESP32_IRrecv(int recvpin);
    ESP32_IRrecv(int recvpin, int port);
    void init();
    unsigned long readIR();

  private:
    int gpionum;
    int rmtport;
    void dumpStatus(rmt_channel_t channel);
    bool isInRange(rmt_item32_t item, int lowDuration, int highDuration, int tolerance);
    bool IR_is0(rmt_item32_t item);
    bool IR_is1(rmt_item32_t item);
    int  compare(unsigned int oldval,  unsigned int newval);
    unsigned long decodeIR(rmt_item32_t *data, int numItems);
    
};

#endif /* ESP32_IR_REMOTE_H_ */

