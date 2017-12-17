#ifndef TERMCHAT_H
#define TERMCHAT_H

/*
 *  @filename: termchat.h
 *  @function: simple termchat protocal
 *
 *      message: <single byte>  + <real data>
 *
 *      8 bits:
 *          [0]     [1]     [2]     [3]     [4]     [5]     [6]     [7]
 *
 *              MSB  ========>   network bit order   ===========> LSB
 *
 *          bit0: 置1，表示为流消息
 *          bit1: 置1，表示为
 */
#define TC_OPER_OK      0b00000001
#define TC_REG_MSG      0b00001000  /* 0x08 */
#define TC_MSG_SIN      0b00001100  /* 0x0c */
#define TC_MSG_SUP      0b00001010  /* 0x0a */
#define TC_MSG_ERR      0b00001001  /* 0x09 */

#define TC_FILE_SYN     0b00010000  /* 0x10 */
#define TC_FILE_ERR     0b00010001  /* 0x11 */


#endif
