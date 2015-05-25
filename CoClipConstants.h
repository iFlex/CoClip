//PACKET SIZES
#define UDP_PACKET_SIZE      375
#define MAX_MTU              2033
#define PACKET_START_SIGNAL  0
#define HDR_SIZE             32
//PROTOCOL DATA
#define UDP_PORT 8888
#define PROTO_ESTABLISH       0
#define PROTO_UDP             1
#define PROTO_LAN_TCP         2
#define PROTO_WWW_TCP         3
#define PROTO_WWW_UDP         4
#define PROTO_NAT_TCP         5

#define UDP_MAX_GAP           500
#define ACK_SEND_INTERV       10
#define MAX_TRANSMISSION_GAP  1 // put back to 100 ms
#define MAX_TRANSMISSION_WAIT 2 // wait-gap gives the waiting time

#define USER_IDLE_TIMEOUT     20000 //5 seconds
#define USER_TIMEOUT          35000 //10 seconds
//ADAPTER LIST
#define MAX_NET_ADAPTERS 32
//THREADING
#define THREAD_IDLE_WAIT 500
//DATA
#define MAX_STREAM_ID 0xffffffff
#define MAX_FILESYS_PATH     32000
#define MAX_FILENAME         256
#define DATA_FILE            1
#define CLIPBOARD_FILE       2
#define RECV_KEY_LENGTH      30
//PACKETS
#define CHANNEL_ESTABLISH_TIME 15000//15 seconds for channel establishing
#define CHANNEL_HAIL_INTERVAL  2000//.ms
#define NETWORK_HAIL_INTERVAL  2000//back to 2000
#define NO_HOLE_FAIL_ACCEPTED  5

#define PACKET_WHO_ONLINE      0x0000 //16 bit packet type
#define PACKET_ME_ONLINE       0x0001 //16 bit packet type
#define PACKET_ME_ONLINE2      0x0002
#define PACKET_BYE             0x0003
#define PACKET_WELLCOME        0x0004
#define PACKET_ACCESS_DENIED   0x0005
#define PACKET_CONTACT_DETAILS 0x000a

#define PACKET_AVATAR_REQUEST  0x0019
#define PACKET_AVATAR_OFFER    0x0020
#define PACKET_STREAM_OFFER    0x0006
#define PACKET_STREAM_ACCEPT   0x0007
#define PACKET_STREAM_DENY     0x0008
#define PACKET_STREAM_DATA     0x0009
#define PACKET_STREAM_CANCEL   0x000b
#define PACKET_STREAM_SCANCEL  0x000c
#define PACKET_STREAM_RCANCEL  0x0018
#define PACKET_RELAY_FAIL      0x000d
#define PACKET_STREAM_ACK      0x000e

#define PACKET_OPENCHANNEL     0x000f
#define PACKET_CHANNEL_OK      0x0010
#define PACKET_CH_KEEP_ALIVE   0x0011
#define PACKET_CH_ALIVE_ACK    0x0012
#define PACKET_REQUEST_HOLE    0x0013
#define PACKET_INIT_HOLE       0x0014

#define PACKET_STILL_ALIVE     0x0015
#define PACKET_ME_ALIVE        0x0016
#define PACKET_TCP_RETRANSMIT  0x0017
//Return code:
#define GENERAL_ERROR   0
#define SUCCESS        -1
#define FINAL_SUCCESS  -2
//IPC constants
#define MAX_FIELD_LEN 100
#define MAX_ARGS      20
