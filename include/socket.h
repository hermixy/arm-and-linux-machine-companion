#ifndef __SOCKET_H__
#define __SOCKET_H__

void socket_server_init(int port);
int socket_accept();
void send_data(char *buf);
void socket_send_file();
void socket_recv_ack(char *rec_rslt, int len);

#endif
