#include "sw.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFF_SIZE 60000
#define ACK_SIZE 4
//#define DEBUG
typedef enum
{
    /* Send */
    SW_READ_DATA,
    SW_SEND_PACKET,
    SW_RECV_ACK,

    /* Receive */
    SW_RECV_PACKET,
    SW_SEND_ACK,
    SW_WRITE_DATA,

    SW_COMPLETE,
    SW_ERROR,
} sw_state_t;

typedef struct
{
    int seq;
    int buff_size;
    char buffer[BUFF_SIZE];
} sw_packet_t;

typedef struct
{
    send_cb_t send_cb;
    void *send_cb_ctx;

    recv_cb_t recv_cb;
    void *recv_cb_ctx;

    read_data_cb_t read_data_cb;
    void *read_data_cb_ctx;

    write_data_cb_t write_data_cb;
    void *write_data_cb_ctx;

    timeout_cb_t timeout_cb;
    void *timeout_cb_ctx;
} sw_cb_data_t;

struct sw_ctx
{
    sw_state_t state;
    int seq;
    sw_cb_data_t cb_data;
    sw_packet_t packet;
    int timeout;
};

typedef struct {
	 char code[ACK_SIZE];
	 int  seq;
} sw_ack_packet_t;

void sw_set_cb(sw_ctx_t *sw_ctx, sw_cb_type_t cb_type, void *cb, void *cb_ctx)
{
    sw_cb_data_t *sw_cb = &(sw_ctx->cb_data);

    switch(cb_type)
    {
    case SW_SEND_CB:
	sw_cb->send_cb = (send_cb_t)cb;
	sw_cb->send_cb_ctx = cb_ctx;
	break;
    case SW_RECV_CB:
	sw_cb->recv_cb = (recv_cb_t)cb;
	sw_cb->recv_cb_ctx = cb_ctx;
	break;
    case SW_READ_DATA_CB:
	sw_cb->read_data_cb = (read_data_cb_t)cb;
	sw_cb->read_data_cb_ctx = cb_ctx;
	break;
    case SW_WRITE_DATA_CB:
	sw_cb->write_data_cb = (write_data_cb_t)cb;
	sw_cb->write_data_cb_ctx = cb_ctx;
	break;
    case SW_TIMEOUT_CB:
	sw_cb->timeout_cb = (timeout_cb_t)cb;
	sw_cb->timeout_cb_ctx = cb_ctx;
	break;
    }
}

sw_ctx_t *sw_init(int timeout)
{
    sw_ctx_t *sw_ctx = calloc(1, sizeof(sw_ctx_t));
    if (!sw_ctx)
	return NULL;

    sw_ctx->timeout = timeout;
    sw_ctx->seq = 0;

    return sw_ctx;
}

void sw_deinit(sw_ctx_t *sw_ctx)
{
    free(sw_ctx);
}

/* Read data form file */
static void sw_read_data(sw_ctx_t *sw_ctx)
{
    int ret;
    sw_cb_data_t cb_data = sw_ctx->cb_data;
    sw_packet_t *packet = &sw_ctx->packet;

    ret = cb_data.read_data_cb(packet->buffer, BUFF_SIZE, cb_data.read_data_cb_ctx);
    if (ret >= 0)
	sw_ctx->state = SW_SEND_PACKET;
    else
	sw_ctx->state = SW_ERROR;

    packet->buff_size = ret;
    packet->seq = sw_ctx->seq;
#ifdef DEBUG
    fprintf(stderr, "Read %d bytes of data\n", packet->buff_size);
#endif
}

/* Send chunk of data */
static void sw_send_packet(sw_ctx_t *sw_ctx)
{
    int ret;
    sw_cb_data_t cb_data = sw_ctx->cb_data;
    sw_packet_t *packet = &sw_ctx->packet;

    sw_ctx->state = SW_RECV_ACK;
#ifdef DEBUG
    if (!packet->buff_size)
	fprintf(stderr, "All data is already sent. Sending empty packet.\n");
#endif

    ret = cb_data.send_cb(packet, sizeof(sw_packet_t), cb_data.send_cb_ctx);
    if (ret < 0)
	sw_ctx->state = SW_ERROR;
#ifdef DEBUG
    fprintf(stderr, "Sent packet #%d, length: %d\n", packet->seq, packet->buff_size);
#endif
}

/* Wait for acknowledgement */
static void sw_recv_ack(sw_ctx_t *sw_ctx)
{
    int ret;
    sw_cb_data_t cb_data = sw_ctx->cb_data;
    sw_packet_t *packet = &sw_ctx->packet;
    sw_ack_packet_t ack;

    sw_ctx->state = SW_READ_DATA;
    ret = cb_data.timeout_cb(sw_ctx->timeout, cb_data.timeout_cb_ctx);
    if (ret == 0)
    {
#ifdef DEBUG
	fprintf(stderr, "Timeout, going to resend packet #%d", packet->seq);
	sw_ctx->state = SW_SEND_PACKET;
#endif
	return;
    }
    ret = cb_data.recv_cb(&ack, sizeof(sw_ack_packet_t), cb_data.recv_cb_ctx);
    if (ret < 0)
	sw_ctx->state = SW_ERROR;
#ifdef DEBUG
    else
	fprintf(stderr, "Received ACK for packet #%d\n", ack.seq);
#endif
    /* Checking the sequence after ACK received. */
    if (sw_ctx->seq != ack.seq)
    {
#ifdef DEBUG
	fprintf(stderr, "Received ACK for packet #%d. But waiting for #%d\n",
	    ack.seq, sw_ctx->seq);
#endif
	sw_ctx->state = SW_SEND_PACKET;
	return;
    }
    /* Goint to send the next packet if everything is fine*/

    if (!packet->buff_size)/* ACK received for last empty packet */
	sw_ctx->state = SW_COMPLETE; /* File sent */
    memset(packet->buffer, 0, BUFF_SIZE);
    packet->buff_size = 0;

    sw_ctx->seq++;
}

/* Receive data */
static void sw_recv_packet(sw_ctx_t *sw_ctx)
{
    int ret;
    sw_cb_data_t cb_data = sw_ctx->cb_data;
    sw_packet_t *packet = &sw_ctx->packet;

    sw_ctx->state = SW_WRITE_DATA;

    ret = cb_data.recv_cb(packet, sizeof(sw_packet_t), cb_data.recv_cb_ctx);

    if (ret < 0)
	sw_ctx->state = SW_ERROR;
    else
    {
	/* Check if the arrived packet is the one we want */
	if (packet->seq != sw_ctx->seq)
	{
	    fprintf(stderr, "Received packet #%d. But waiting for #%d\n",
		packet->seq, sw_ctx->seq);
	    sw_ctx->state = SW_SEND_ACK;
	    sw_ctx->seq = packet->seq;
	}
    }

#ifdef DEBUG
    fprintf(stderr, "Received packet #%d, length: %d\n",packet->seq, packet->buff_size);
#endif
}

/* Write data to file */
static void sw_write_data(sw_ctx_t *sw_ctx)
{
    int ret;
    sw_cb_data_t cb_data = sw_ctx->cb_data;
    sw_packet_t *packet = &sw_ctx->packet;

    sw_ctx->state = SW_SEND_ACK;
    if (!packet->buff_size)
	return;

    ret = cb_data.write_data_cb(packet->buffer, packet->buff_size, cb_data.write_data_cb_ctx);
    if (ret < 0)
	sw_ctx->state = SW_ERROR;
#ifdef DEBUG
    fprintf(stderr, "Wrote %d bytes of data\n", packet->buff_size);
#endif
}

/* Send acknowledgement */
static void sw_send_ack(sw_ctx_t *sw_ctx)
{
    int ret;
    sw_cb_data_t cb_data = sw_ctx->cb_data;
    sw_packet_t *packet = &sw_ctx->packet;

    sw_ack_packet_t ack = {"ACK", packet->seq};
    if (!packet->buff_size)
	sw_ctx->state = SW_COMPLETE;
    else
	sw_ctx->state = SW_RECV_PACKET;
    ret = cb_data.send_cb(&ack, sizeof(sw_ack_packet_t), cb_data.send_cb_ctx);
    if (ret < 0)
	sw_ctx->state = SW_ERROR;

    memset(packet, 0, BUFF_SIZE);
    packet->buff_size = 0;
    sw_ctx->seq++;

#ifdef DEBUG
    fprintf(stderr, "Sendig ACK fro packet #%d\n", packet->seq);
#endif
}

/* Main function for sending file */
int sw_send_file(sw_ctx_t *sw_ctx)
{
    int ret = 0;
    sw_ctx->state = SW_READ_DATA;

    while (sw_ctx->state != SW_COMPLETE && sw_ctx->state != SW_ERROR)
    {
	switch (sw_ctx->state)
	{
	case SW_READ_DATA:
	    sw_read_data(sw_ctx);
	    break;
	case SW_SEND_PACKET:
	    sw_send_packet(sw_ctx);
	    break;
	case SW_RECV_ACK:
	    sw_recv_ack(sw_ctx);
	    break;
	case SW_COMPLETE:
	    break;
	case SW_ERROR:
	    ret = -1;
	    break;
	default: /* We will never get here */
	    break;
	}
    }

    return ret;
}

/* Main function for receiving file */
int sw_recv_file(sw_ctx_t *sw_ctx)
{
    int ret = 0;
    sw_ctx->state = SW_RECV_PACKET;

    while (sw_ctx->state != SW_COMPLETE && sw_ctx->state != SW_ERROR)
    {
	switch (sw_ctx->state)
	{
	case SW_RECV_PACKET:
	    sw_recv_packet(sw_ctx);
	    break;
	case SW_SEND_ACK:
	    sw_send_ack(sw_ctx);
	    break;
	case SW_WRITE_DATA:
	    sw_write_data(sw_ctx);
	    break;
	case SW_COMPLETE:
	    break;
	case SW_ERROR:
	    ret = -1;
	    break;
	default: /* We will never get here */
	    break;
	}
    }

    return ret;
}
