/**
 * @file uhf_demo.c
 * @date 2021.8.24
 * @brief uhf lib demo
 * @author firestaradmin
 * @ps fuxxxxk RFID, this is forth time I write the fuxked program. by the way, this version of dataSheet of 
 * the new module is like a shit.
 * @ps2 I am really really really a genius.
*/

#include "uhf_demo.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"

/* my lib */
#include "psdk_logger.h"
#include "stm32f4xx_hal.h"
#include "uart.h"

// EPC最大取值为32个word
#define MAX_EPC_LEN         (64)

uhf_dev_t rfid_dev;

uhf_switch_t uhfSwitch = UHF_SWITCH_OFF;
bool uhf_switch_trigged = false;

// now is not used
uint8_t label_trig_threshold = 3;

// "[switch],[trig epc],[log]"
char MsgToMSDK_message[500];

char MsgToMSDK_log[255];

SemaphoreHandle_t widgetmsg_mutex;

// "[switch],[trig epc],[log]"
SemaphoreHandle_t MsgToMSDK_mutex;

SemaphoreHandle_t RfidDev_mutex;
SemaphoreHandle_t logStr_mutex;

extern UART_HandleTypeDef s_uart4Handle;
extern char widget_message[];

/** 
 * @brief the callBack func for writing frame data to module
 * @return the Length of data be transfered successfully
 */
uint8_t uhf_write_data_cb(uint8_t *data, uint8_t dataLen) {
    // UartSendMultByte(UART4, data, dataLen);
    HAL_UART_Transmit(&s_uart4Handle, data, dataLen, 1000);
    return dataLen;
}

/** 
 * @brief the delay func for uhf lib. if used in RTOS, you need call OS_delay func in it.
 * @param timeToDelay delay times in Ms.
 */
void uhf_system_delay_cb(uint32_t timeToDelay) {
    Osal_TaskSleepMs(timeToDelay);
}




/**
 * @brief uhf_task.
 */
void uhf_task_demo(void* parameter) {

    /* 0. declare variable */
    int32_t ret;
    uint8_t bootloaderV[4] = {0};
    
    uhf_labelInfo_t labelInfo[10];
    rfid_dev.status = UHF_STATUS_UNINIT;
    float timestampForScanning1sCnt = 0;

    /* 1. init uhf lib */
    ret = uhf_init(&rfid_dev, uhf_write_data_cb, uhf_system_delay_cb);
	if(ret == 0){
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "Init RFID OK!\n");
		xSemaphoreGive(widgetmsg_mutex);
		PsdkLogger_UserLogInfo("%s\n", widget_message);
	}
	else{
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "Init RFID Error!\nPlease reboot machine.");
		PsdkLogger_UserLogInfo("%s\n", widget_message);
		xSemaphoreGive(widgetmsg_mutex);
		while(1){
			Osal_TaskSleepMs(300);
		}
	}
	Osal_TaskSleepMs(500);
	
    /* 2. get firmware version, test comminucation */
	while(uhf_get_bootloader_ver(bootloaderV) != 0){
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "get Module Bootloader Version Error!\nTry again ...");
		PsdkLogger_UserLogInfo("%s\n", widget_message);
		xSemaphoreGive(widgetmsg_mutex);
		Osal_TaskSleepMs(500);
	}

	xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
	snprintf(widget_message, 255, "uhf bootloader ver:%d.%d.%d.%d ", bootloaderV[0], bootloaderV[1], bootloaderV[2], bootloaderV[3]);
	PsdkLogger_UserLogInfo("%s\n", widget_message);
	xSemaphoreGive(widgetmsg_mutex);

	rfid_dev.status = UHF_STATUS_INITOK;
	Osal_TaskSleepMs(500);

    /* 3. make module into app environment */
    ret = uhf_run_in_app_env();
    if(ret == 0){
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "uhf run in app env ok.\n");
		xSemaphoreGive(widgetmsg_mutex);
		PsdkLogger_UserLogInfo("%s\n", widget_message);
	}
	else{
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "uhf run in app env failed! ret = %d", ret);
		PsdkLogger_UserLogInfo("%s\n", widget_message);
		xSemaphoreGive(widgetmsg_mutex);
		while(1){
			Osal_TaskSleepMs(300);
		}
	}
	


    /* 4. set module scanning power */
    ret = uhf_set_scanning_watt(3300);    // 33dbm
    if(ret == 0){
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "uhf set power watt ok.\n");
		xSemaphoreGive(widgetmsg_mutex);
		PsdkLogger_UserLogInfo("%s\n", widget_message);
	}
	else{
		xSemaphoreTake(widgetmsg_mutex, portMAX_DELAY);
		snprintf(widget_message, 255, "uhf set power watt failed! ret = %d", ret);
		PsdkLogger_UserLogInfo("%s\n", widget_message);
		xSemaphoreGive(widgetmsg_mutex);
		while(1){
			Osal_TaskSleepMs(300);
		}
	}

    rfid_dev.status = UHF_STATUS_IDLE;

    while(1){
        uint8_t foundLabelsCnt = 0;
        uint8_t acquiredLabelsCnt = 0;
        uint8_t mostTimesLabelIndex = 0;
        int len = 0;
        if(uhfSwitch == UHF_SWITCH_ON){ // switch on
            rfid_dev.status = UHF_STATUS_SCANNING;
            uhf_switch_trigged = false;

            foundLabelsCnt = uhf_start_scanning(2000);   // scanning 2000 Ms
            timestampForScanning1sCnt += 2;
            if(foundLabelsCnt > 0){ // found labels
                // 1. find the labels that are read most
                acquiredLabelsCnt = uhf_get_found_labels_info(labelInfo, 10);
                for (int i = 1; i < acquiredLabelsCnt; i++) {
                    if(labelInfo[i].readCount > labelInfo[mostTimesLabelIndex].readCount)
                        mostTimesLabelIndex = i;
                }

                // 2. send msg to msdk
                char epcStr[MAX_EPC_LEN] = {0};
                char tempStr[4];
                // xSemaphoreTake(RfidDev_mutex, portMAX_DELAY); 
                
                // 2.1 get epc id
                for(int i = 0; i < labelInfo[mostTimesLabelIndex].epc_len; i++){
                    snprintf(tempStr, 4, "%02x", labelInfo[mostTimesLabelIndex].p_epc_ID[i]);
                    strcat(epcStr, tempStr);
                }

                // 2.2 format str data and transfer
                // "[switch],[trig epc],[log]"
                // xSemaphoreTake(logStr_mutex, portMAX_DELAY);
                snprintf(MsgToMSDK_log, 250, "[%.2f] | find %d; maxTimes %d | rssi: %d dbm" ,
						timestampForScanning1sCnt, 
						foundLabelsCnt, 
						labelInfo[mostTimesLabelIndex].readCount, 
						labelInfo[mostTimesLabelIndex].rssi);
				
                len = snprintf(MsgToMSDK_message, 500, "%s,%s,%s", (rfid_dev.status == UHF_STATUS_SCANNING)? "true":"false", epcStr, MsgToMSDK_log);
                sprintf(MsgToMSDK_log, "");
                // xSemaphoreGive(logStr_mutex);	
                PsdkLogger_UserLogInfo("%s", MsgToMSDK_message);
                PsdkDataTransmission_SendDataToMobile((uint8_t *)MsgToMSDK_message, len);
                // xSemaphoreGive(RfidDev_mutex);


            }
            else{   // no labels found
                // xSemaphoreTake(logStr_mutex, portMAX_DELAY);
                snprintf(MsgToMSDK_log, 250, "[%.2f] | no labels!", timestampForScanning1sCnt);
                len = snprintf(MsgToMSDK_message, 255, "%s,,%s", (rfid_dev.status == UHF_STATUS_SCANNING)? "true":"false", MsgToMSDK_log);
                sprintf(MsgToMSDK_log, "");
                // xSemaphoreGive(logStr_mutex);
                PsdkLogger_UserLogInfo("%s", MsgToMSDK_message);
                PsdkDataTransmission_SendDataToMobile((uint8_t *)MsgToMSDK_message, len);
            }
        } else { // switch off

            uhf_switch_trigged = false;
            rfid_dev.status = UHF_STATUS_IDLE;

            sprintf(MsgToMSDK_log, "");
            len = snprintf(MsgToMSDK_message, 255, "%s,,%s", (rfid_dev.status == UHF_STATUS_SCANNING)? "true":"false", MsgToMSDK_log);
            sprintf(MsgToMSDK_log, "");
            // xSemaphoreGive(logStr_mutex);
            PsdkLogger_UserLogInfo("%s", MsgToMSDK_message);
            PsdkDataTransmission_SendDataToMobile((uint8_t *)MsgToMSDK_message, len);
        }

		Osal_TaskSleepMs(200);
        timestampForScanning1sCnt += 0.2;
    }
}



/**
 * @brief uhf_timer_task  main body.
 */
void uhf_timer_task_demo(void* parameter)
{	

	while(1){
        Osal_TaskSleepMs(1);
        uhf_time_process();
	}
}



void uhf_turn_on() {
    uhfSwitch = UHF_SWITCH_ON;
    uhf_switch_trigged = true;
}
void uhf_turn_off() {
    uhfSwitch = UHF_SWITCH_OFF;
    uhf_switch_trigged = true;
}


