#include "packet.h"
#include <stdlib.h>
#include <string.h>

int check_syn(struct packet *p)
{
    return p->flags & SYN_FLAG;
}

int check_ack(struct packet *p)
{
    return p->flags & ACK_FLAG;
}

int check_fin(struct packet *p)
{
    return p->flags & FIN_FLAG;
}

void set_syn(struct packet *p)
{
    p->flags |= SYN_FLAG;
}

void set_ack(struct packet *p)
{
    p->flags |= ACK_FLAG;
}

void set_fin(struct packet *p)
{
    p->flags |= FIN_FLAG;
}

struct packet* make_packet()
{
    struct packet *p = malloc(sizeof(struct packet));

    p->flags = 0;
    p->d_length = 0;

    memset(p->data, 0, DATA_SIZE);

    return p;
}

void set_data(struct packet* p, char *data)
{
    memcpy(p->data, data, strlen(data));

    p->d_length = strlen(data);
}