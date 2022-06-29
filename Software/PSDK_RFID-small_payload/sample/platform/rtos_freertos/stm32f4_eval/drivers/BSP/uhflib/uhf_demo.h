/**
 * @file uhf_demo.h
 * @date 2021.8.25
 * @brief uhf lib demo
 * @author firestaradmin
 * @ps fuxxxxk RFID, this is forth time I write the fuxked program. by the way, this version of dataSheet of 
 * the new module is like a shit.
 * @ps2 I am really really really a genius.
*/



#ifndef _UHF_DEMO_H_
#define _UHF_DEMO_H_

/* C lib */
#include <stdio.h>

/* uhf lib */
#include "uhflibV4.h"


/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"

/* UHF_Module status */
typedef enum UHF_SWITCH
{
    UHF_SWITCH_OFF = 0,
    UHF_SWITCH_ON
}uhf_switch_t;



extern uhf_dev_t rfid_dev;
extern char MsgToMSDK_message[500];
extern char MsgToMSDK_log[255];
extern char rfid_findingStatus_boolStr[6];
extern SemaphoreHandle_t MsgToMSDK_mutex;
extern SemaphoreHandle_t RfidDev_mutex;
extern SemaphoreHandle_t logStr_mutex;
extern uhf_switch_t uhfSwitch;
extern bool uhf_switch_trigged;
extern uint8_t label_trig_threshold;




/** 
 * @brief the callBack func for writing frame data to module
 * @return the Length of data be transfered successfully
 */
uint8_t uhf_write_data_cb(uint8_t *data, uint8_t dataLen);

/** 
 * @brief the delay func for uhf lib. if used in RTOS, you need call OS_delay func in it.
 * @param timeToDelay delay times in Ms.
 */
void uhf_system_delay_cb(uint32_t timeToDelay);

/**
 * @brief uhf_task.
 */
void uhf_task_demo(void *parameter);

/**
 * @brief uhf_timer_task  main body.
 */
void uhf_timer_task_demo(void *parameter);

void uhf_turn_on();
void uhf_turn_off();

#endif // !_UHF_DEMO_H_
