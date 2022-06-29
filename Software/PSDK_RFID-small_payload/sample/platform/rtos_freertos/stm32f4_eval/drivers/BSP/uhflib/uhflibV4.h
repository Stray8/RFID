/**
 * @file uhflibV4.h
 * @date 2021.8.23
 * @brief a lib for RFID module.
 * @author firestaradmin
 * @ps fuxxxxk RFID, this is forth time I write the fuxked program. by the way, this version of dataSheet of 
 * the new module is like a shit.
*/


#ifndef __UHFLIBV4_H_
#define __UHFLIBV4_H_

#include "stm32f4xx.h"

#ifndef  bool
	#define bool _Bool
    #define true 1
    #define false 0
#endif 

/* frame max size in bytes */
#define FRAME_MAX_SIZE 255

#define RECEIVE_FINISH_TRIGGER_TIME				(uint8_t)3		/* unit ms */




/* Frame format const */
#define FRAME_HEAD  0xFF



/* bootload layer cmd */
#define BL_CMD_WRITE_FALSH                  0x01   //:写FLASH命令；
#define BL_CMD_READ_FLASH                   0x02   //:读FLASH命令；
#define BL_CMD_GET_FIRMWARE_VERSION         0x03   //:获取BOOTLOADER/FIRMWARE版本信息；（APP应用层也可用）
#define BL_CMD_RUN_APP                      0x04   //:BOOT FIRMWARE命令，运行到APP层；（APP应用层也可用）
#define BL_CMD_SET_BAUDRATE                 0x06   //:设置波特率命令；（APP应用层也可用）
#define BL_CMD_VERIFY_FIRMWARE              0x08   //:校验烧录固件；
#define BL_CMD_RETURN_BL                    0x09   //:运行到BOOTLOADER层；（APP应用层也可用）
#define BL_CMD_GET_RUNNING_LAYER            0x0C   //:获取目前是运行在BOOTLOADER层还是APP应用层。（APP应用层也可用）
#define BL_CMD_GET_MODULE_SERIALNUMBER      0x10   //:获取模块序列号；（APP应用层也可用）

/* APP层标签操作命令 */
#define AL_LABEL_CMD_SINGLE_SCAN            0X21  //:单标签盘存命令；
#define AL_LABEL_CMD_MULTI_SCAN             0X22  //:多标签盘存命令；
#define AL_LABEL_CMD_WRITE_EPC              0X23  //:写标签EPC命令；
#define AL_LABEL_CMD_WRITE_FLASH            0X24  //:写标签存储区命令；
#define AL_LABEL_CMD_LOCK_LABEL             0X25  //:LOCK标签命令;
#define AL_LABEL_CMD_KILL_LABEL             0X26  //:KILL标签命令；
#define AL_LABEL_CMD_READ_FLASH             0X28  //：读标签存储区命令；
#define AL_LABEL_CMD_GET_STORED_EPC         0X29  //: 获取盘存到标签信息命令；
#define AL_LABEL_CMD_CLEAR_EPC_BUF          0X2A  //:清除标签缓存区命令；
#define AL_LABEL_CMD_SPECIAL_LABEL_CMD      0X2D  //:特定标签命令，目前只有QT相关命令。
#define AL_LABEL_CMD_WRITE_BLOCK            0X2D  //:块写命令；
#define AL_LABEL_CMD_LOCK_BLOCK             0X2E  //:块LOCK命令，目前未使用；
#define AL_LABEL_CMD_ERASE_BLOCK            0X2E  //：块擦除命令；

/* APP层设置命令： */
#define AL_SET_CMD_SET_ANT_INDEX            0X91    //:天线口设置命令；
#define AL_SET_CMD_SET_READ_WATT            0X92    //:设置读发射功率；目前该命令设置的值模块会忽略；
#define AL_SET_CMD_SET_PROTOCOL             0X93    //:设置当前工作标签协议；
#define AL_SET_CMD_SET_WRITE_WATT           0X94    //:设置写发射功率；目前该命令设置的值模块会忽略；
#define AL_SET_CMD_SET_JUMP_FREQUENCE       0X95    //:跳频设置；
#define AL_SET_CMD_SET_GPIO_OUTPUT          0X96    //:GPIO输出设置；
#define AL_SET_CMD_SET_FREQUENCE_RANGE      0X97    //:设置当前工作频率区域；
#define AL_SET_CMD_SET_WATT_MODE            0X98    //:设置功率模式；目前该命令设置的值模块会忽略；
#define AL_SET_CMD_SET_USER_MODE            0X99    //:设置用户模式；目前该命令设置的值模块会忽略；
#define AL_SET_CMD_SET_READER_CONFIG        0X9A    //:设置阅读器配置；
#define AL_SET_CMD_SET_PROTOCOL_CONFIG      0X9B    //:设置协议配置；

/* 获取APP层设置信息命令： */
#define AL_INFO_CMD_GET_ANT_CONFIG              0X61    //:获取天线口配置信息；
#define AL_INFO_CMD_GET_READ_WATT               0X62    //:获取读发射功率信息；
#define AL_INFO_CMD_GET_PROTOCOL                0X63    //:获取当前工作标签协议；
#define AL_INFO_CMD_GET_WRITE_WATT              0X64    //:获取写发射功率信息；
#define AL_INFO_CMD_GET_JUMP_FREQUENCE          0X65    //:获取跳频表；
#define AL_INFO_CMD_GET_GPIO_INPUT              0X66    //:获取输入GPIO的值；
#define AL_INFO_CMD_GET_FREQUENCE_RANGE         0X67    //:获取当前工作频率区域；
#define AL_INFO_CMD_GET_WATT_MODE               0X68    //:获取功率模式；
#define AL_INFO_CMD_GET_USER_MODE               0X69    //:获取用户模式；
#define AL_INFO_CMD_GET_READER_CONFIG           0X6A    //:获取读写器配置；
#define AL_INFO_CMD_GET_PROTOCOL_CONFIG         0X6B    //:获取协议配置；
#define AL_INFO_CMD_GET_USABLE_PROTOCOL         0X70    //:获取可用标签协议；
#define AL_INFO_CMD_GET_USABLE_FRE_RANGE        0X71    //:获取可用的工作频率区域；
#define AL_INFO_CMD_GET_CURRENT_TEMPERATURE     0X72    //：获取当前模块温度；


/* Module excute return const */
enum MODULE_STATUS_CODE
{
    STATUS_CODE_SUCCESS = 0x0000
};


typedef uint8_t uhf_ret_t;

/* uhfLib func return code */
enum UHF_RET
{
    RET_OK = 0,

    RET_PARSE_TIMEOUT = 0xD0,
    RET_PARSE_HEAD_ERROR = 0xD2,
    RET_PARSE_CMD_ERROR = 0xD3,
    RET_PARSE_CRC16_ERROR = 0xD4,
    RET_PARSE_MODULE_STATUS_ERROR = 0xD5,


    RET_TRANSFER_ERROR = 0xE0,


    RET_ERROR = 0xFF
};



/* UHF_Module status */
enum UHF_STATUS
{
    UHF_STATUS_UNINIT = 0,
    UHF_STATUS_INITOK,
    UHF_STATUS_IDLE,
    UHF_STATUS_SCANNING,
    UHF_STATUS_ERROR,

};

/** 
 * @brief the callBack func for writing frame data to module
 * @return !!!!! the Length of data be transfered successfully
 */
typedef uint8_t (*write_data_cb_t)(uint8_t *data, uint8_t dataLen);

/** 
 * @brief the delay func for uhf lib. if used in RTOS, you need call OS_delay func in it.
 * @param timeToDelay delay times in Ms.
 */
typedef void (*system_delay_cb_t)(uint32_t timeToDelay);

enum RECV_STATUS
{
    RECV_NO_DATA = 0,
    RECV_ING,
    RECV_FINISHED
};

enum PARSE_RESULT
{
    PARSE_UNPARSED = 0,
    PARSE_OK,
    PARSE_TIMEOUT = 0xD0,
    PARSE_HEAD_ERROR = 0xD2,
    PARSE_CMD_ERROR = 0xD3,
    PARSE_CRC16_ERROR = 0xD4,
    PARSE_MODULE_STATUS_ERROR = 0xD5,
};

typedef struct uhf_dev{
    /* uhf timestamp in Ms, max 49 days */
    uint32_t timestamp;
    /* the uhf module's status */
    enum UHF_STATUS status;


    /* the callback func for write data to uhf module */
    write_data_cb_t writeDataCB;
    /* the callback func for delay */
    system_delay_cb_t systemDelay;



    /* data buf to be sended */
    uint8_t sendBuf[FRAME_MAX_SIZE];
    /* point to the last data of sendBuf */
    uint8_t sendBuf_tail;




    /* receive status */
    uint8_t recvStatus;
    /* receive finished trigger, if timeCnt equals to macro RECEIVE_FINISH_TRIGGER_TIME, it indicates received OK */
    uint8_t rece_trigger_timeCnt;


    /* receive data buf */
    uint8_t recvBuf[FRAME_MAX_SIZE];
    /* point to the last data of recvBuf */
	uint8_t recvBuf_tail;
    /* when parsed OK, it will point to the data section */
    uint8_t *p_recvData;
    /* data section length */
    uint8_t dataLength;


    /* the parsed ret of frame in recvBuf */
    uint8_t parsedResult;
    /* the parsed cmdCode of frame in recvBuf */
    uint8_t parsedcmdCode;

} uhf_dev_t;


typedef struct uhf_labelInfo {
    uint8_t readCount;
    int8_t rssi;
    uint8_t antID;

    uint8_t protocolID;

    uint16_t tag_data_len;
    uint8_t *p_tag_data;

    uint16_t epc_and_pc_and_crc_len;
    uint16_t epc_len;
    uint8_t pc_word[2];
    uint8_t *p_epc_ID;
    uint16_t epc_crc;

} uhf_labelInfo_t;


/*********************************************
 * func definition
 * *********************************************/




/**
 * @brief the time heart of uhf lib, need be called every 1 Ms.
 * Maybe you can call it in timer interrupt.
*/
void uhf_time_process(void);

/**
 * @brief the recv heart of uhf lib, transfer data one by one byte.
 * Maybe you can call it in uart interrupt, and transfer the data from module to it.
*/
void uhf_recv_process(uint8_t byte);


/* init uhf lib */
uhf_ret_t uhf_init(uhf_dev_t *uhf_dev, write_data_cb_t write_data_cb, system_delay_cb_t delay_cb);

/**
 * @brief get bootloader version, for testing the comminucation.
 * @param bootloadVerBuf where to stored the version. the verison data is 4 bytes.
*/
uhf_ret_t uhf_get_bootloader_ver(uint8_t *bootloadVerBuf);


/**
 * @brief Enter the APP environment
*/
uhf_ret_t uhf_run_in_app_env(void);

/**
 * @brief set module scanning watt
 * @param dbm the power of module scanning, unit: 0.01 db, range(500 ~ 3300) = range(5 ~ 33 dbm)
*/
uhf_ret_t uhf_set_scanning_watt(uint16_t dbm);


/**
 * @brief start scanning labels
 * @param durationMs the time module scanning, unit: Ms
 * @return return labels found count.
*/
uint32_t uhf_start_scanning(uint16_t durationMs);


/**
 * @brief when scanning labels success, use the func to acquire label found info
 * @param label_info where to store label info
 * @param max_label_count decide max label count will be put in label_info.
 * @return return labels found count.
 * @ps the func only acquire EPC and label found times, because it be seted in func by variable metadataFlag.
*/
uint32_t uhf_get_found_labels_info(uhf_labelInfo_t *label_info, uint8_t max_label_count);















void uhf_clear_recvBuf(void);



uhf_ret_t send_command(uint8_t cmdCode, uint8_t *data, uint8_t dataLen);

/**
 * @param cmdCode the command in last sended frame.
 * @param timeOut the timeOut for wait received finished, unit: Ms
 * timeOut = 0: always wait until recvfinished.
*/
uhf_ret_t parse_frame(uint8_t cmdCode, uint32_t timeOut);



/* private func */
static uhf_ret_t _create_frame(uint8_t cmdCode, uint8_t *data, uint8_t dataLen);





static uint16_t _CalcCRC(uint8_t *msgbuf, uint8_t msglen);

static void _CRC_calcCrc8(uint16_t *crcReg, uint16_t poly, uint16_t u8Data);

#endif // !__UHFLIBV4_H_