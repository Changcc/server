#include "packet.h"

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