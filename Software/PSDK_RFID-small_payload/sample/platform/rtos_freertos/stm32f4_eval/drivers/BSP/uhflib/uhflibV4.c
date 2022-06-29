/**
 * @file uhflibV4.c
 * @date 2021.8.23
 * @brief a lib for RFID module.
 * @author firestaradmin
 * @ps fuxxxxk RFID, this is forth time I write the fuxked program. by the way, this version of dataSheet of 
 * the new module is like a shit.
*/

#include <stdio.h>
#include "uhflibV4.h"

/* my libs */
// #include "ug_log.h"

uhf_dev_t *p_dev;
// static uint8_t sendBuf[FRAME_MAX_SIZE];
// static uint32_t sendBuf_tail = 0;

#define MSG_CRC_INIT		    0xFFFF
#define MSG_CCITT_CRC_POLY		0x1021



static void _CRC_calcCrc8(uint16_t *crcReg, uint16_t poly, uint16_t u8Data)
{
	uint16_t i;
	uint16_t xorFlag;
	uint16_t bit;
	uint16_t dcdBitMask = 0x80;
	for(i=0; i<8; i++)
	{
		xorFlag = *crcReg & 0x8000;
		*crcReg <<= 1;
		bit = ((u8Data & dcdBitMask) == dcdBitMask);
		*crcReg |= bit;
		if(xorFlag)
		{
			*crcReg = *crcReg ^ poly;
		}
		dcdBitMask >>= 1;	
	}
}


static uint16_t _CalcCRC(uint8_t *msgbuf,uint8_t msglen)
{
	uint16_t calcCrc = MSG_CRC_INIT;
	uint8_t  i;
	for (i = 1; i < msglen; ++i)
		_CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, msgbuf[i]);
	return calcCrc;
}

void uhf_clear_recvBuf(void) {

    p_dev->recvBuf_tail = 0; // clear recv buf
    p_dev->recvStatus = RECV_NO_DATA;
    p_dev->parsedResult = PARSE_UNPARSED;
}


/* 
主机到模块的通信格式：
Header   +   Data Length  +  Command  +  Data  +  CRC-16
    Header: 一字节 固定0XFF
    DataLength: 一字节，Data数据块的字节数
    Command：一字节，命令码
    Data：数据块，高字节在前面。
    CRC-16: 二字节循环冗余码，高字节在前，从DataLength到Data结束的所有数据参与计算所得。
备注：整个通信数据串的字节数不得大于255个字节。 
*/
/**  
 * @brief make a complete frame to send to uhf module 
 * @param cmdCode command code
 * @param data dataBuf point 
 * @param dataLend dataBuf length
*/
static uhf_ret_t _create_frame(uint8_t cmdCode, uint8_t *data, uint8_t dataLen) {
    p_dev->sendBuf_tail = 0;    // clear send buf

    /* Frame HEAD */
    p_dev->sendBuf[p_dev->sendBuf_tail++] = FRAME_HEAD;

    /* Frame length */
    p_dev->sendBuf[p_dev->sendBuf_tail++] = dataLen;

    /* Frame cmdCode */
    p_dev->sendBuf[p_dev->sendBuf_tail++] = cmdCode;

    /* Frame dataBlock */
    if(data == NULL)
        dataLen = 0;
    for (uint8_t i = 0; i < dataLen; i++)
        p_dev->sendBuf[p_dev->sendBuf_tail++] = data[i];

    /* Frame CRC-16 bigEndian */
    uint16_t crcCode_16 = _CalcCRC(p_dev->sendBuf, p_dev->sendBuf_tail);
    p_dev->sendBuf[p_dev->sendBuf_tail++] = (uint8_t)(crcCode_16 >> 8);
    p_dev->sendBuf[p_dev->sendBuf_tail++] = (uint8_t)(crcCode_16 & 0x00FF);

    return RET_OK;
}

/*
模块到主机的通信格式：
   Header   +   DataLength  +  Command +  Status   +  Data  +  CRC-16 
   Header: 一字节 固定0XFF
DataLength: 一字节，Data数据块的字节数
Status: 二字节，状态位，为0时表示操作成功，非0值为操作失败具体请看后面返回状态码解释，如非0且非后面解释的错误状态码则仅表示操作失败。
Command：一字节，命令码，同上一条 主机发来的命令码
Data：数据块，高字节在前面。
CRC-16: 二字节循环冗余码，高字节在前，从DataLength到Data结束的所有数据参与计算所得。
备注：整个通信数据串的字节数不得大于255个字节。
*/
/**
 * @param cmdCode the command in last sended frame.
 * @param timeOut the timeOut for wait received finished, unit: Ms.
 * timeOut = 0: always wait until recvfinished.
 */
uhf_ret_t parse_frame(uint8_t cmdCode, uint32_t timeOut) {
    uint32_t timestamp_old = p_dev->timestamp;
    

    while(p_dev->recvStatus != RECV_FINISHED) {
        if(timeOut != 0 && p_dev->timestamp - timestamp_old > timeOut) {
            p_dev->recvBuf_tail = 0; // clear recv buf
            return RET_PARSE_TIMEOUT;
        }
        p_dev->systemDelay(10); // delay 10 Ms
    }

    p_dev->parsedcmdCode = cmdCode;

    /* verify FRAME HEAD */
    if(p_dev->recvBuf[0] != FRAME_HEAD) {
        p_dev->parsedResult = PARSE_HEAD_ERROR;
        return RET_PARSE_HEAD_ERROR;
    }
    /* verify cmdCode */
    if(p_dev->recvBuf[2] != cmdCode) {
        p_dev->parsedResult = PARSE_CMD_ERROR;
        return RET_PARSE_CMD_ERROR;
    }
    /* verify status code */
    uint16_t statusCode = 0x0000;
    statusCode = (uint16_t)p_dev->recvBuf[3] << 8;
    statusCode |= (uint16_t)p_dev->recvBuf[4];
    if(statusCode != STATUS_CODE_SUCCESS) {
        p_dev->parsedResult = PARSE_MODULE_STATUS_ERROR;
        return RET_PARSE_MODULE_STATUS_ERROR;
    }
    /* verify CRC16 */
    uint8_t length = p_dev->recvBuf[1];
    uint16_t crc16 = 0x0000;
    crc16 = (uint16_t)p_dev->recvBuf[5 + length] << 8;
    crc16 |= (uint16_t)p_dev->recvBuf[6 + length];
    if(crc16 != _CalcCRC(p_dev->recvBuf, p_dev->recvBuf_tail - 2)) {
        p_dev->parsedResult = PARSE_CRC16_ERROR;
        return RET_PARSE_CRC16_ERROR;
    }

    p_dev->parsedResult = PARSE_OK;
    
    /* update data var */
    p_dev->dataLength = length;
    p_dev->p_recvData = p_dev->recvBuf + 5;

    return RET_OK;
};









uhf_ret_t send_command(uint8_t cmdCode, uint8_t *data, uint8_t dataLen) {

    _create_frame(cmdCode, data, dataLen);

    uint8_t cnt = p_dev->writeDataCB(p_dev->sendBuf, p_dev->sendBuf_tail);
    if(cnt != p_dev->sendBuf_tail)
        return RET_TRANSFER_ERROR;

    return RET_OK;
}


uhf_ret_t uhf_init(uhf_dev_t *uhf_dev, write_data_cb_t write_data_cb, system_delay_cb_t delay_cb) {
    if(uhf_dev == NULL)
        return RET_ERROR;

    /* init global var point */
    p_dev = uhf_dev;

    /* init dev struct */
    p_dev->status = UHF_STATUS_UNINIT;
    p_dev->writeDataCB = write_data_cb;
    p_dev->systemDelay = delay_cb;
    p_dev->timestamp = 0;
    p_dev->parsedResult = PARSE_UNPARSED;
    p_dev->recvBuf_tail = 0;
    p_dev->sendBuf_tail = 0;
    p_dev->rece_trigger_timeCnt = 0;


}




/**
 * @brief the time heart of uhf lib, need be called every 1 Ms.
 * Maybe you can call it in timer interrupt.
*/
void uhf_time_process(void) {
    p_dev->timestamp++; // uhf lib timestamp

    /* data receiving finished judge */
	if(p_dev->recvStatus == RECV_ING) // if recv have started, judges if recv complete. 
	{
		p_dev->rece_trigger_timeCnt ++;		// trigger timeCnt add 1
		if(p_dev->rece_trigger_timeCnt == RECEIVE_FINISH_TRIGGER_TIME)     /* recv complete. */
		{
			p_dev->recvStatus = RECV_FINISHED;

            //TODO: if in finding label mode, maybe need to parse finded label here
		}
	}



}

/**
 * @brief the recv heart of uhf lib, transfer data one by one byte.
 * Maybe you can call it in uart interrupt, and transfer the data from module to it.
*/
void uhf_recv_process(uint8_t byte) {
    p_dev->recvBuf[p_dev->recvBuf_tail++] = byte;
    p_dev->recvStatus = RECV_ING;   /* data begin receiving */
	p_dev->rece_trigger_timeCnt = 0;    /* reset uart receiving judgement */
    p_dev->parsedResult = PARSE_UNPARSED;
}






uhf_ret_t uhf_get_bootloader_ver(uint8_t *const bootloadVerBuf) {
    uint8_t ret;
    ret = send_command(BL_CMD_GET_FIRMWARE_VERSION, NULL, 0);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return ret;
    }

    ret = parse_frame(BL_CMD_GET_FIRMWARE_VERSION, 1000);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return ret;
    }

    
    // bootloadVerBuf[0] = p_dev->p_recvData[0];
    // bootloadVerBuf[1] = p_dev->p_recvData[1];
    // bootloadVerBuf[2] = p_dev->p_recvData[2];
    // bootloadVerBuf[3] = p_dev->p_recvData[3];
    *(uint32_t *)bootloadVerBuf = *(uint32_t *)(p_dev->p_recvData);

    uhf_clear_recvBuf();
    return RET_OK;
}


uhf_ret_t uhf_run_in_app_env(void) {
    uint8_t ret;
    ret = send_command(BL_CMD_RUN_APP, NULL, 0);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return ret;
    }

    ret = parse_frame(BL_CMD_RUN_APP, 1000);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return ret;
    }


    uhf_clear_recvBuf();
    return RET_OK;
}

uhf_ret_t uhf_set_scanning_watt(uint16_t dbm) {
    uint8_t power_h = dbm >> 8;
    uint8_t power_l = dbm & 0xFF;
    const uint8_t POWER_OPTION = 0x03;  // just set ant power
    const uint8_t POWER_ANT_ID = 0x01;
    uint8_t power_data[6] = {POWER_OPTION, POWER_ANT_ID, power_h, power_l, power_h, power_l};
    
    uint8_t ret;
    ret = send_command(AL_SET_CMD_SET_ANT_INDEX, power_data, sizeof(power_data));
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return ret;
    }

    ret = parse_frame(AL_SET_CMD_SET_ANT_INDEX, 1000);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return ret;
    }

    // UGINFO("set module power = %.2f dbm", (float)dbm / 100);

    uhf_clear_recvBuf();
    return RET_OK;
}




uint32_t uhf_start_scanning(uint16_t durationMs) {
    if(durationMs == 0)
        return 0;
    uint8_t durationMs_h = durationMs >> 8;
    uint8_t durationMs_l = durationMs & 0xFF;

    uint8_t ret;

    uint8_t databuf[] = {0x00, 0x00, 0x00, durationMs_h, durationMs_l};

    ret = send_command(AL_LABEL_CMD_MULTI_SCAN, databuf, sizeof(databuf));
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return 0;
    }

    ret = parse_frame(AL_LABEL_CMD_MULTI_SCAN, durationMs + 1000);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return 0;
    }

    uint32_t labelsFoundCnt = 0;
    if(p_dev->p_recvData[2] == 0) { // Tag section is 1 byte
        labelsFoundCnt = p_dev->p_recvData[3];
    }
    else { // Tag section is 4 bytes
        labelsFoundCnt = (uint32_t)(p_dev->p_recvData[3]) << 24;
        labelsFoundCnt |= (uint32_t)(p_dev->p_recvData[4]) << 16;
        labelsFoundCnt |= (uint32_t)(p_dev->p_recvData[5]) << 8;
        labelsFoundCnt |= (uint32_t)(p_dev->p_recvData[6]);
    }
    //UGINFO("uhf has scaned %d labels", labelsFoundCnt);

    uhf_clear_recvBuf();
    return labelsFoundCnt;
}






/**
 * @brief when scanning labels success, use the func to acquire label found info
 * @param label_info where to store label info
 * @param max_label_count decide max label count will be put in label_info.
 * @return return the count of labels successfully acquired.
 * @ps the func only acquire EPC and label found times, because it be seted in func by variable metadataFlag.
*/
uint32_t uhf_get_found_labels_info(uhf_labelInfo_t *label_info, uint8_t max_label_count) {
    if(label_info == NULL)
        return 0;

    uint8_t ret;
    uint8_t metadataFlag[2] = {0x00, 0x03}; // get read Count and rssi
    uint8_t databuf[] = {metadataFlag[0], metadataFlag[1], 0x00};

    ret = send_command(AL_LABEL_CMD_GET_STORED_EPC, databuf, sizeof(databuf));
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return 0;
    }

    ret = parse_frame(AL_LABEL_CMD_GET_STORED_EPC, 1000);
    if(ret != RET_OK){
        uhf_clear_recvBuf();
        return 0;
    }

    uint8_t *data = p_dev->p_recvData;
    uint8_t dataTail = 3;   // point to labelsCnt
    uint8_t labelsCnt = data[dataTail++];   // now dataTail point to readCount of the first label's info 


    if(labelsCnt == 0)
        return 0;
    if (labelsCnt < max_label_count) 
        max_label_count = labelsCnt;

    uint16_t epc_and_pc_and_crc_len_in_bit = 0;
    // only acquire EPC and label found times
    for (uint32_t i = 0; i < max_label_count; i++) {
        label_info[i].readCount = data[dataTail++];
        label_info[i].rssi = (int8_t)data[dataTail++];
        
        epc_and_pc_and_crc_len_in_bit = data[dataTail++] << 8;
        epc_and_pc_and_crc_len_in_bit |= data[dataTail++];
        label_info[i].epc_and_pc_and_crc_len = (epc_and_pc_and_crc_len_in_bit + 7) / 8; // 字节对齐
        label_info[i].epc_len = label_info[i].epc_and_pc_and_crc_len - 4;

        label_info[i].pc_word[0] = data[dataTail++];
        label_info[i].pc_word[1] = data[dataTail++];

        label_info[i].p_epc_ID = &data[dataTail];

        dataTail += label_info[i].epc_len;
        label_info[i].epc_crc = data[dataTail++] << 8;
        label_info[i].epc_crc |= data[dataTail++];
    }

    // UGINFO("uhf has %d labels, %d labels were successfully acquired.", labelsCnt, max_label_count);

    uhf_clear_recvBuf();
    return max_label_count;

}

