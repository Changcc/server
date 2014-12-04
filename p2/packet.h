#define SYN_FLAG 0x0001
#define ACK_FLAG 0x0002
#define FIN_FLAG 0x0004
#define DATA_SIZE 982

struct packet
{
    // Source & destination ports
    short source_port;
    short dest_port;

    // Sequence and acknowledgement flags
    int seq_num;
    int ack_num;

    // SYN, ACK, FIN flags
    short flags;

    // Data
    int d_length;
    char data[DATA_SIZE];
};

int check_syn(struct packet *p);
void set_syn(struct packet *p);

int check_ack(struct packet *p);
void set_ack(struct packet *p);

int check_fin(struct packet *p);
void set_fin(struct packet *p);

struct packet* make_packet();

void set_data(struct packet* p, char *data);