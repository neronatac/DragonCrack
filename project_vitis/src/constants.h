/*
 * commands.h
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */

#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_


#define RESP_LEN_FIELD_LENGTH 2
#define RESP_DATA_MAX_SIZE 255

/* GENERAL COMMANDS (0x0X) */
#define CMD_GENERAL_BASE    0x00
#define CMD_GET_VERSION     0x01
#define CMD_ECHO            0x02
#define CMD_GET_TEMPERATURE 0x03

/* DES COMMANDS */
#define CMD_DES_GET_VERSION             0x11
#define CMD_DES_GET_PARAMS              0x12
#define CMD_DES_SET_PLAINTEXT           0x13
#define CMD_DES_GET_PLAINTEXT           0x14
#define CMD_DES_SET_MASK                0x15
#define CMD_DES_GET_MASK                0x16
#define CMD_DES_SET_REF                 0x17
#define CMD_DES_GET_REF                 0x18
#define CMD_DES_SET_KEY_RANGE           0x19
#define CMD_DES_GET_KEY_RANGE           0x1A
#define CMD_DES_GET_CURRENT_CHUNK       0x1B
#define CMD_DES_GET_STATUS              0x1C
#define CMD_DES_GET_RESULTS             0x1D
#define CMD_DES_SET_NEW_RESULTS_STATUS  0x1E
#define CMD_DES_GET_NEW_RESULTS_STATUS  0x1F

#endif /* SRC_CONSTANTS_H_ */
