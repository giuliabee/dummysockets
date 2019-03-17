#ifndef SW_H
#define SW_H

typedef struct sw_ctx sw_ctx_t;

typedef enum
{
    SW_SEND_CB,
    SW_RECV_CB,
    SW_READ_DATA_CB,
    SW_WRITE_DATA_CB,
    SW_TIMEOUT_CB,
} sw_cb_type_t;

typedef int (*send_cb_t)(void *data, int data_len, void *ctx);
typedef int (*recv_cb_t)(void *buffer, int buffer_size, void *ctx);
typedef int (*read_data_cb_t)(void *buffer, int buffer_size, void *ctx);
typedef int (*write_data_cb_t)(void *data, int data_len, void *ctx);
typedef int (*timeout_cb_t)(int timeout, void *ctx);


void sw_set_cb(sw_ctx_t *sw_ctx, sw_cb_type_t cb_type, void *cb, void *cb_ctx);

sw_ctx_t *sw_init(int timeout);
void sw_deinit(sw_ctx_t *sw_ctx);

int sw_send_file(sw_ctx_t *sw_ctx);
int sw_recv_file(sw_ctx_t *sw_ctx);



#endif /* SW_H */
