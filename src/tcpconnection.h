#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

struct ELoop;
struct TcpConnection;

typedef void (*TcpConnectionReadCallback)(struct TcpConnection *connection, const void *data);
typedef void (*TcpConnectionWriteCallback)(struct TcpConnection *connection, const void *data);

struct TcpConnection *tcp_connection_with_fd(int fd, struct ELoop *eloop);
void tcp_connection_destroy(struct TcpConnection *connection);

void tcp_connection_close(struct TcpConnection *connection);

int tcp_connection_set_callbacks(struct TcpConnection *connection, TcpConnectionReadCallback read_callback, TcpConnectionWriteCallback write_callback, const void *data);

ssize_t tcp_connection_read_data(struct TcpConnection *connection, void *data, size_t length);
ssize_t tcp_connection_write_data(struct TcpConnection *connection, const void *data, size_t length);

#endif
