/**
 ********************************************************************
 * @file    test_data_subscription.c
 * @version V2.0.0
 * @date    2019/8/15
 * @brief
 *
 * @copyright (c) 2017-2018 DJI. All rights reserved.
 *
 * All information contained herein is, and remains, the property of DJI.
 * The intellectual and technical concepts contained herein are proprietary
 * to DJI and may be covered by U.S. and foreign patents, patents in process,
 * and protected by trade secret or copyright law.  Dissemination of this
 * information, including but not limited to data and other proprietary
 * material(s) incorporated within the information, in any form, is strictly
 * prohibited without the express written consent of DJI.
 *
 * If you receive this source code without DJI’s authorization, you may not
 * further disseminate the information, and you must immediately remove the
 * source code and notify DJI of its removal. DJI reserves the right to pursue
 * legal actions against you for any loss(es) or damage(s) caused by your
 * failure to do so.
 *
 *********************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include <utils/util_misc.h>
#include "test_data_subscription.h"
#include "psdk_logger.h"
#include "psdk_platform.h"

/* Private constants ---------------------------------------------------------*/
#define DATA_SUBSCRIPTION_TASK_FREQ         (1)
#define DATA_SUBSCRIPTION_TASK_STACK_SIZE   (2048)

/* Private types -------------------------------------------------------------*/


/* Private functions declaration ---------------------------------------------*/
static void *UserDataSubscription_Task(void *arg);
static T_PsdkReturnCode PsdkTest_DataSubscriptionReceiveQuaternionCallback(const uint8_t *data, uint16_t dataSize,
                                                                           const T_PsdkDataSubscriptiontTimestamp *timestamp);

/* Private variables ---------------------------------------------------------*/
static T_PsdkTaskHandle s_userDataSubscriptionThread;

/* Exported functions definition ---------------------------------------------*/
T_PsdkReturnCode PsdkTest_DataSubscriptionInit(void)
{
    T_PsdkReturnCode psdkStat;

    psdkStat = PsdkDataSubscription_Init();
    if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        PsdkLogger_UserLogError("init data subscription module error.");
        return PSDK_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }
	
	/* Subscribe batteryInfo. */
    psdkStat = PsdkDataSubscription_RegTopicSync(PSDK_DATA_SUBSCRIPTION_TOPIC_BATTERY_INFO, 10, NULL);
    if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        PsdkLogger_UserLogError("Subscribe topic batteryInfo error.");
        return PSDK_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

//    psdkStat = PsdkDataSubscription_RegTopicSync(PSDK_DATA_SUBSCRIPTION_TOPIC_QUATERNION, 10,
//                                                 PsdkTest_DataSubscriptionReceiveQuaternionCallback);
//    if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
//        PsdkLogger_UserLogError("Subscribe topic QUATERNION error.");
//        return PSDK_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//    }

//    psdkStat = PsdkDataSubscription_RegTopicSync(PSDK_DATA_SUBSCRIPTION_TOPIC_VELOCITY, 1, NULL);
//    if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
//        PsdkLogger_UserLogError("Subscribe topic VELOCITY error.");
//        return PSDK_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//    }

    if (PsdkOsal_TaskCreate(&s_userDataSubscriptionThread, UserDataSubscription_Task, "user_subscription_task",
                            DATA_SUBSCRIPTION_TASK_STACK_SIZE, NULL) !=
        PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        PsdkLogger_UserLogError("user data subscription task create error.");
        return PSDK_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    return PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Private functions definition-----------------------------------------------*/
#ifndef __CC_ARM
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif


///**
// * @brief Battery information topic data structure.
//*/
//typedef struct BatteryInfo {
//    uint32_t capacity; /*!< Battery capacity, unit: mAh. */
//    int32_t voltage; /*!< Battery voltage, unit: mV. */
//    int32_t current; /*!< Battery current, unit: mA. */
//    uint8_t percentage; /*!< Battery capacity percentage, unit: 1%. */
//} T_PsdkDataSubscriptionBatteryInfo;




static void *UserDataSubscription_Task(void *arg)
{
    T_PsdkReturnCode psdkStat;
    T_PsdkDataSubscriptionQuaternion quaternion = {0};
    T_PsdkDataSubscriptionVelocity velocity = {0};
    T_PsdkDataSubscriptiontTimestamp timestamp = {0};
	T_PsdkDataSubscriptionBatteryInfo battery = {0};
    
    USER_UTIL_UNUSED(arg);

    while (1) {
        PsdkOsal_TaskSleepMs(1000 / DATA_SUBSCRIPTION_TASK_FREQ);
		
		
        psdkStat = PsdkDataSubscription_GetValueOfTopicWithTimestamp(PSDK_DATA_SUBSCRIPTION_TOPIC_BATTERY_INFO,
                                                                     (uint8_t *) &battery,
                                                                     sizeof(T_PsdkDataSubscriptionBatteryInfo),
                                                                     &timestamp);
        if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            PsdkLogger_UserLogError("get value of topic quaternion error.");
        } else {
            PsdkLogger_UserLogDebug("timestamp: millisecond %u microsecond %u.", timestamp.millisecond,
                                    timestamp.microsecond);
            PsdkLogger_UserLogInfo("battery info:\r\n 	\
			capacity: %d(mAh)\r\n	\
			voltage:  %d(mV) \r\n	\
			current:  %d(mA) \r\n	\
			capacity percentage:%%%d", battery.capacity, battery.voltage, battery.current, battery.percentage);
        }
		
//    uint32_t capacity; /*!< Battery capacity, unit: mAh. */
//    int32_t voltage; /*!< Battery voltage, unit: mV. */
//    int32_t current; /*!< Battery current, unit: mA. */
//    uint8_t percentage; /*!< Battery capacity percentage, unit: 1%. */
		
//        psdkStat = PsdkDataSubscription_GetValueOfTopicWithTimestamp(PSDK_DATA_SUBSCRIPTION_TOPIC_QUATERNION,
//                                                                     (uint8_t *) &quaternion,
//                                                                     sizeof(T_PsdkDataSubscriptionQuaternion),
//                                                                     &timestamp);
//        if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
//            PsdkLogger_UserLogError("get value of topic quaternion error.");
//        } else {
//            PsdkLogger_UserLogDebug("timestamp: millisecond %u microsecond %u.", timestamp.millisecond,
//                                    timestamp.microsecond);
//            PsdkLogger_UserLogInfo("quaternion: %f %f %f %f.", quaternion.q0, quaternion.q1, quaternion.q2,
//                                    quaternion.q3);
//        }

//        psdkStat = PsdkDataSubscription_GetValueOfTopicWithTimestamp(PSDK_DATA_SUBSCRIPTION_TOPIC_VELOCITY,
//                                                                     (uint8_t *) &velocity,
//                                                                     sizeof(T_PsdkDataSubscriptionVelocity),
//                                                                     &timestamp);
//        if (psdkStat != PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
//            PsdkLogger_UserLogDebug("get value of topic velocity error.");
//        } else {
//            PsdkLogger_UserLogDebug("velocity: x %f y %f z %f, healthFlag %d.", velocity.data.x, velocity.data.y,
//                                    velocity.data.z, velocity.health);
//        }
    }
}

#ifndef __CC_ARM
#pragma GCC diagnostic pop
#endif

static T_PsdkReturnCode PsdkTest_DataSubscriptionReceiveQuaternionCallback(const uint8_t *data, uint16_t dataSize,
                                                                           const T_PsdkDataSubscriptiontTimestamp *timestamp)
{
    T_PsdkDataSubscriptionQuaternion *quaternion = (T_PsdkDataSubscriptionQuaternion *) data;

    USER_UTIL_UNUSED(dataSize);

    PsdkLogger_UserLogDebug("receive quaternion data.");

    PsdkLogger_UserLogDebug("timestamp: millisecond %u microsecond %u.", timestamp->millisecond,
                            timestamp->microsecond);
    PsdkLogger_UserLogDebug("quaternion: %f %f %f %f.", quaternion->q0, quaternion->q1, quaternion->q2, quaternion->q3);

    return PSDK_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/****************** (C) COPYRIGHT DJI Innovations *****END OF FILE****/
