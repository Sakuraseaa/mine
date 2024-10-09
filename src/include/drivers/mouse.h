#ifndef __MOUSE_H__
#define __MOUSE_H__

#define KBCMD_SENDTO_MOUSE 0xd4     // 向鼠标设备发送数据
#define MOUSE_ENABLE 0xf4           // 允许鼠标发送数据包
#define KBCMD_EN_MOUSE_INTFACE 0xa8 // 开启鼠标端口

struct mouse_packet
{
    u8_t Byte0;
    // 7:Y overflow, 6:X overflow 5:Y sign bit, 4:X sign bit
    // 3:Always, 2:Middle Btn, 1:Right Btn, 0:Left Btn
    char Byte1; // X movement
    char Byte2; // Y movement
};

struct mouse_packet mouse;



#endif