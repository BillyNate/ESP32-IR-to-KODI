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
 */
#include "Arduino.h"
extern "C"
{
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/event_groups.h"
  #include "freertos/semphr.h"
  #include "freertos/ringbuf.h"
}

#include "ESP32_IR_Remote.h"

// Clock divisor (base clock is 80MHz)
#define CLK_DIV 100

// Number of clock ticks that represent 10us.  10 us = 1/100th msec.
#define TICK_10_US (80000000 / CLK_DIV / 100000)

static RingbufHandle_t ringBuf;

#define TAG "IR_TASK"

#define NEC_BITS          32
#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_BIT_MARK     560
#define NEC_ONE_SPACE   1690
#define NEC_ZERO_SPACE   560
#define NEC_RPT_SPACE   2250


void ESP32_IRrecv::dumpStatus(rmt_channel_t channel)
{
   bool loop_en;
   uint8_t div_cnt;
   uint8_t memNum;
   bool lowPowerMode;
   rmt_mem_owner_t owner;
   uint16_t idleThreshold;
   uint32_t status;
   rmt_source_clk_t srcClk;

   rmt_get_tx_loop_mode(channel, &loop_en);
   rmt_get_clk_div(channel, &div_cnt);
   rmt_get_mem_block_num(channel, &memNum);
   rmt_get_mem_pd(channel, &lowPowerMode);
   rmt_get_memory_owner(channel, &owner);
   rmt_get_rx_idle_thresh(channel, &idleThreshold);
   rmt_get_status(channel, &status);
   rmt_get_source_clk(channel, &srcClk);
   /*
   Serial.print("Status for RMT channel: ");
   Serial.println(channel);
   */
   /*
   Serial.println("- Loop enabled: %d", loop_en);
   Serial.println("- Clock divisor: %d", div_cnt);
   Serial.println("- Number of memory blocks: %d", memNum);
   Serial.println("- Low power mode: %d", lowPowerMode);
   Serial.println("- Memory owner: %s", owner==RMT_MEM_OWNER_TX?"TX":"RX");
   Serial.println("- Idle threshold: %d", idleThreshold);
   Serial.println("- Status: %d", status);
   Serial.println("- Source clock: %s", srcClk==RMT_BASECLK_APB?"APB (80MHz)":"1MHz");
   */
}

ESP32_IRrecv::ESP32_IRrecv(int recvpin, int port)
{
  if(recvpin >= GPIO_NUM_0 && recvpin<GPIO_NUM_MAX)
  {
    gpionum = recvpin;
  }
  else
  {
    gpionum = (int)GPIO_NUM_22;
  }
  
  if(port <= RMT_CHANNEL_0 && port<RMT_CHANNEL_MAX)
  {
    rmtport = port;
  }
  else
  {
    rmtport = (int)RMT_CHANNEL_0;
  }
}

ESP32_IRrecv::ESP32_IRrecv(int recvpin)
{
  ESP32_IRrecv(recvpin,(int)RMT_CHANNEL_0);
}

void ESP32_IRrecv::init()
{
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_RX;
  config.channel = (rmt_channel_t)rmtport;
  config.gpio_num = (gpio_num_t)gpionum;
  config.mem_block_num = 2;
  config.rx_config.filter_en = 1;
  config.rx_config.filter_ticks_thresh = 100; // 80000000/100 -> 800000 / 100 = 8000  = 125us
  config.rx_config.idle_threshold = TICK_10_US * 100 * 20;
  config.clk_div = CLK_DIV;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 5000, 0));

  rmt_get_ringbuf_handle(config.channel, &ringBuf);
  dumpStatus(config.channel);
  rmt_rx_start(config.channel, 1);

  return;
}

bool ESP32_IRrecv::isInRange(rmt_item32_t item, int lowDuration, int highDuration, int tolerance)
{
  uint32_t lowValue = item.duration0 * 10 / TICK_10_US;
  uint32_t highValue = item.duration1 * 10 / TICK_10_US;
  /*
  ESP_LOGI(TAG, "lowValue=%d, highValue=%d, lowDuration=%d, highDuration=%d",
    lowValue, highValue, lowDuration, highDuration);
  */
  if(lowValue < (lowDuration - tolerance) || lowValue > (lowDuration + tolerance) || (highValue != 0 && (highValue < (highDuration - tolerance) || highValue > (highDuration + tolerance))))
  {
    return false;
  }
  return true;
}

bool ESP32_IRrecv::IR_is0(rmt_item32_t item)
{
  return isInRange(item, NEC_BIT_MARK, NEC_BIT_MARK, 100);
}

bool ESP32_IRrecv::IR_is1(rmt_item32_t item)
{
  return isInRange(item, NEC_BIT_MARK, NEC_ONE_SPACE, 100);
}

unsigned long ESP32_IRrecv::decodeIR(rmt_item32_t *data, int numItems)
{
  if(!isInRange(data[0], NEC_HDR_MARK, NEC_HDR_SPACE, 200))
  {
    return 0;
  }
  int i;
  unsigned long totalAccum = 0;
  for(i=0; i<numItems; i++)
  {
    if(IR_is0(data[i]))
    {
      totalAccum = totalAccum << 1;
    }
    else if(IR_is1(data[i]))
    {
      totalAccum = (totalAccum << 1) | 0x01;
    }
  }
  return totalAccum;
}

unsigned long ESP32_IRrecv::readIR()
{
  size_t itemSize;
  unsigned long command = 0;

  rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive((RingbufHandle_t)ringBuf, (size_t *)&itemSize, (TickType_t)portMAX_DELAY);

  int numItems = itemSize / sizeof(rmt_item32_t);
  int i;
  rmt_item32_t *p = item;
  for(i=0; i<numItems; i++)
  {
    p++;
  }
  command = decodeIR(item, numItems);
  vRingbufferReturnItem(ringBuf, (void*)item);
  
  return command;
}
