/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    UDP_Socket.c
 * Purpose: UDP Socket Code Template
 * Rev.:    V7.0.0
 *----------------------------------------------------------------------------*/
//! [code_UDP_Socket]
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int32_t udp_sock;                       // UDP socket handle
extern osMessageQueueId_t socket_Q; 
static data_received_t data_received;          // Sensor data received from UDP socket

extern uint8_t isAlarmEnabled;
extern uint8_t isBrakeEnabled;
 
// Notify the user application about UDP socket events.
uint32_t udp_callback (int32_t socket, const NET_ADDR *addr, const uint8_t *buf, uint32_t len) {
 
  /*
    message format:
    id:battery_perct:battery_in:battery_out:uPconsumption:blade_speed:x/y/z
  */
  static char *accel;
  static char *token_id;
  static uint8_t time_buf[10] = {0};
//  static char *buffer;

  token_id = strtok((char *)buf, ":");
  strcpy(data_received.id, token_id); 
  data_received.addr = addr;
  data_received.battery_perct = atof(strtok(NULL, ":"));
  data_received.battery_in    = atof(strtok(NULL, ":"));
  data_received.battery_out   = atof(strtok(NULL, ":"));
  data_received.uPconsumption = atof(strtok(NULL, ":"));
  data_received.blade_speed   = atof(strtok(NULL, ":"));
    accel = strtok(NULL, ":");
  data_received.accel_data[0] = atof(strtok(accel, "/"));
  data_received.accel_data[1] = atof(strtok(NULL, "/"));
  data_received.accel_data[2] = atof(strtok(NULL, "/"));
  RTC_GetTime(time_buf);
  strcpy(data_received.timestamp, (char *)time_buf);
  
  osMessageQueuePut(socket_Q, &data_received, 0U, 0U);
//  free(buffer);
  return (0); // currently not used
}
 
// Send UDP data to destination client.
void send_udp_data (char *line1, char *line2) {
 
  if (udp_sock > 0) {
    // IPv4 address: 192.168.1.117
//    NET_ADDR addr = { NET_ADDR_IP4, 2000, 192, 168, 1, 117 };
    NET_ADDR addr = { NET_ADDR_IP4, 2000, 10, 48, 11, 118 };
    uint8_t *sendbuf;
    uint32_t message_length;
    netStatus status;
    uint8_t *tmp;
    if (strlen(line1) == 0) {
      *line1 = ' ';
    }
    if (strlen(line2) == 0) {
      *line2 = ' ';
    }
    message_length = strlen(line1) + strlen(line2) + 5; // alarm flag + brake flag + 3 separators
    sendbuf = netUDP_GetBuffer (message_length); 
    tmp = sendbuf;
    *tmp = isAlarmEnabled + 48; // to obtain the ascii character
    tmp++;
    *tmp = (uint8_t) ':';
    tmp++;
    *tmp = isBrakeEnabled + 48; // to obtain the ascii character
    tmp++;
    *tmp = (uint8_t) ':';
    tmp++;
    strcpy((char *)tmp, line1);
    tmp += strlen(line1);
    *tmp = (uint8_t) ':';
    tmp++;
    strcpy((char *)tmp, line2);
 
    status = netUDP_Send (udp_sock, &addr, sendbuf, message_length);
    isAlarmEnabled = 0;
    isBrakeEnabled = 0;
  }
}

__NO_RETURN void Th_UDP_Socket (void *arg) {
  (void)arg;

  // netInitialize(); Already initialized
  // Initialize UDP socket and open port 2000
  udp_sock = netUDP_GetSocket (udp_callback);
  if (udp_sock > 0) {
    netUDP_Open (udp_sock, 2000);
  }
  else
    fprintf(stderr, "Unable to open a UDP Socket");
  osThreadExit();
}
