#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define BAD_ARGUMENT_COUNT              1
#define ERROR_OPENING_FILE              2
#define ERROR_SEEKING_IN_FILE           3
#define CAPTURE_TRUNCATED               4
#define CAPTURE_INVALID                 5
#define PCAP_LEN_TIMESTAMPS             8
#define PCAP_LEN_DATA_FIELDS            8
#define PCAP_LEN_GLOBAL_HEADER         24
#define PCAP_LEN_DATA_CAPTURED_FIELD    4
#define ETHER_MAC_ADDRESS_LEN           6
#define ETHER_TYPE_LEN                  2
#define IP_HEADER_REQUIRED_FIELDS_LEN  20
#define IP_HEADER_OFFSET_TOTAL_LENGTH   2
#define IP_HEADER_OFFSET_PROTOCOL       9
#define IP_HEADER_OFFSET_SRC_IP_ADDR   12
#define IP_HEADER_OFFSET_DEST_IP_ADDR  16
#define TCP_HEADER_REQUIRED_FIELDS_LEN 20
#define TCP_HEADER_OFFSET_DEST_PORT     2
#define TCP_HEADER_OFFSET_SEQUENCE_NUM  4
#define TCP_HEADER_OFFSET_HEADER_LEN   12
#define TCP_HEADER_OFFSET_FLAGS        13
#define TCP_HEADER_FLAGS_MASK_SYN     0x2
#define TCP_HEADER_FLAGS_MASK_ACK    0x10

typedef struct TCPPayload {
    char *data;
    unsigned short payloadLen;
    unsigned int sequenceNumber;
} TCPPayload_t;

typedef struct TCPPayloadNode {
    TCPPayload_t          *payload;
    struct TCPPayloadNode *next;
} TCPPayloadNode_t;

int compareSequenceNumbers(const void *lhs, const void *rhs) {
    unsigned int sequenceNumberLhs = (*(const TCPPayload_t**)lhs)->sequenceNumber;
    unsigned int sequenceNumberRhs = (*(const TCPPayload_t**)rhs)->sequenceNumber;
    if(sequenceNumberLhs < sequenceNumberRhs) {
        return -1;
    }
    else if(sequenceNumberLhs > sequenceNumberRhs) {
        return 1;
    }
    else
        return 0;
}

void Seek(int fd, off_t offset, int whence) {
    off_t location = lseek(fd, offset, whence);
    if(location < 0) {
        fprintf(stderr, "Error seeking in file.\n");
        exit(ERROR_SEEKING_IN_FILE);
    }
}

int main(int argc, char *argv[]) {

    if(argc != 2) {
        fprintf(stderr, "usage: readcapture [file]\n");
        return BAD_ARGUMENT_COUNT;
    }
 
    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if(fd <= 0) {
        fprintf(stderr, "Could not open file: %s.\n", filename);
        return ERROR_OPENING_FILE;
    }

    // Move past the pcap file global header.
    Seek(fd, PCAP_LEN_GLOBAL_HEADER, SEEK_SET);

    int numPackets = 0, lenDataCaptured = 0, lenDataUntruncated = 0, offsetToNextPacket = 0;
    unsigned short etherType = 0, transportProtocol = 0;
    TCPPayloadNode_t *head = NULL, *prev = NULL;
    int numPayloadNodes = 0;
    while(1) {

        // Packet Header.

        // Move to the `data captured` field of the next packet.
       Seek(fd, offsetToNextPacket + PCAP_LEN_TIMESTAMPS, SEEK_CUR);

        char lenBuffer[PCAP_LEN_DATA_FIELDS] = {0};
        ssize_t bytesRead = read(fd, lenBuffer, PCAP_LEN_DATA_FIELDS);
        if(bytesRead == 0) {
            break; // EOF.
        }

        lenDataCaptured     = *(int*)(lenBuffer);
        lenDataUntruncated  = *(int*)(lenBuffer + PCAP_LEN_DATA_CAPTURED_FIELD);
        if(lenDataCaptured != lenDataUntruncated) {
            fprintf(stderr, "Capture truncated, unable to reconstruct data.\n");
            return CAPTURE_TRUNCATED;
        }
        offsetToNextPacket = lenDataCaptured;

        // Ethernet Frame.
        if(numPackets > 0) {
            printf("\n");
        }
        printf("---------\nPACKET %d\n---------\n", numPackets+1);

        unsigned char destSrcMacAddressBuffer[ETHER_MAC_ADDRESS_LEN * 2] = {0};
        bytesRead = read(fd, destSrcMacAddressBuffer, ETHER_MAC_ADDRESS_LEN * 2);
        if(bytesRead == 0) {
            break; // EOF.
        }
        offsetToNextPacket -= bytesRead;
        printf("%-25s", "Destination MAC Address:");
        for(size_t i = 0; i < ETHER_MAC_ADDRESS_LEN; ++i) {
            char delim = i < (ETHER_MAC_ADDRESS_LEN-1) ? ':' : '\n';
            printf("%x%c", destSrcMacAddressBuffer[i], delim);
        }
        printf("%-25s", "Source MAC Address:");
        for(size_t i = 0; i < ETHER_MAC_ADDRESS_LEN; ++i) {
            char delim = i < (ETHER_MAC_ADDRESS_LEN-1) ? ':' : '\n';
            printf("%x%c", destSrcMacAddressBuffer[ETHER_MAC_ADDRESS_LEN + i], delim);
        }

        unsigned char etherTypeBuffer[ETHER_TYPE_LEN] = {0};
        bytesRead = read(fd, etherTypeBuffer, ETHER_TYPE_LEN);
        if(bytesRead == 0) {
            break; // EOF
        }
        offsetToNextPacket -= bytesRead;
        
        unsigned short etherTypeNetwork    = *(unsigned short*)(etherTypeBuffer);
        unsigned short etherTypeCurrPacket = ntohs(etherTypeNetwork);
        printf("%-25s0x%x\n", "EtherType:", etherTypeCurrPacket);

        if(numPackets == 0) {
            etherType = etherTypeCurrPacket;
        }
        else {
            if(etherType != etherTypeCurrPacket) {
                fprintf(stderr, "Error: IP datagrams have different formats: "
                               "0x%x and 0x%x, exiting.\n",
                               etherType, etherTypeCurrPacket);
                exit(CAPTURE_INVALID);
            }
        }

        // IP Header.

        unsigned char ipHeaderBuffer[IP_HEADER_REQUIRED_FIELDS_LEN] = {0};
        bytesRead = read(fd, ipHeaderBuffer, IP_HEADER_REQUIRED_FIELDS_LEN);
        if(bytesRead == 0) {
            break; // EOF
        }
        offsetToNextPacket -= bytesRead;

        unsigned short headerLen = *ipHeaderBuffer & 0x0F;
        headerLen *= 4; // Header length is encoded in units of 32-bit words.
        unsigned short totalLen = *(unsigned short*)(ipHeaderBuffer + IP_HEADER_OFFSET_TOTAL_LENGTH);
        totalLen = ntohs(totalLen);
        unsigned short payloadLen = totalLen - headerLen;
        unsigned char transportProtocolCurr = ipHeaderBuffer[IP_HEADER_OFFSET_PROTOCOL];
        if(numPackets == 0) {
            transportProtocol = transportProtocolCurr;
        }
        else if(transportProtocol != transportProtocolCurr) {
            fprintf(stderr, "Error: IP datagrams have different protocols: "
                            "0x%x and 0x%x, exiting.\n",
                            transportProtocol, transportProtocolCurr);
            exit(CAPTURE_INVALID);
        }
        char sourceIPAddr[128] = {0};
        snprintf(sourceIPAddr, sizeof sourceIPAddr, "%u.%u.%u.%u", 
                 ipHeaderBuffer[IP_HEADER_OFFSET_SRC_IP_ADDR],
                 ipHeaderBuffer[IP_HEADER_OFFSET_SRC_IP_ADDR + 1],
                 ipHeaderBuffer[IP_HEADER_OFFSET_SRC_IP_ADDR + 2],
                 ipHeaderBuffer[IP_HEADER_OFFSET_SRC_IP_ADDR + 3]);
        char destIPAddr[128] = {0};
        snprintf(destIPAddr, sizeof destIPAddr, "%u.%u.%u.%u", 
                 ipHeaderBuffer[IP_HEADER_OFFSET_DEST_IP_ADDR],
                 ipHeaderBuffer[IP_HEADER_OFFSET_DEST_IP_ADDR + 1],
                 ipHeaderBuffer[IP_HEADER_OFFSET_DEST_IP_ADDR + 2],
                 ipHeaderBuffer[IP_HEADER_OFFSET_DEST_IP_ADDR + 3]);
   
        printf("%-25s%u\n", "IP Header Length: ",      headerLen);
        printf("%-25s%u\n", "IP Payload Length: ",     payloadLen);
        printf("%-25s%u\n", "Transport Protocol ",     transportProtocolCurr);
        printf("%-25s%s\n", "Destination IP Address:", destIPAddr);
        printf("%-25s%s\n", "Source IP Address:",      sourceIPAddr);

        // TCP Header.

        unsigned char tcpHeader[TCP_HEADER_REQUIRED_FIELDS_LEN] = {0};
        bytesRead = read(fd, tcpHeader, TCP_HEADER_REQUIRED_FIELDS_LEN);
        if(bytesRead == 0) {
            break; // EOF
        }
        offsetToNextPacket -= bytesRead;
        unsigned short sourcePort = *(unsigned short*)tcpHeader;
        sourcePort = ntohs(sourcePort);
        unsigned short destinationPort = *(unsigned short*)(tcpHeader + TCP_HEADER_OFFSET_DEST_PORT);
        destinationPort = ntohs(destinationPort);
        unsigned tcpHeaderLen = tcpHeader[TCP_HEADER_OFFSET_HEADER_LEN] >> 4;
        tcpHeaderLen *= 4; // Stored in units of 32-bit words.
        unsigned int sequenceNumber = *(unsigned int*)(tcpHeader + TCP_HEADER_OFFSET_SEQUENCE_NUM);
        sequenceNumber = ntohl(sequenceNumber);
        unsigned short isSyn = (tcpHeader[TCP_HEADER_OFFSET_FLAGS] & TCP_HEADER_FLAGS_MASK_SYN) != 0;
        unsigned short isAck = (tcpHeader[TCP_HEADER_OFFSET_FLAGS] & TCP_HEADER_FLAGS_MASK_ACK) != 0;
        unsigned short tcpPayloadLen = payloadLen - tcpHeaderLen;
        printf("%-25s%u\n", "TCP Source Port: ",      sourcePort);
        printf("%-25s%u\n", "TCP Destination Port: ", destinationPort);
        printf("%-25s%u\n", "TCP Header Length: ",    tcpHeaderLen);
        printf("%-25s%u\n", "Is SYN Packet: ",        isSyn);
        printf("%-25s%u\n", "Is ACK Packet: ",        isAck);
        printf("%-25s%u\n", "Sequence Number: ",      sequenceNumber);
        printf("%-25s%u\n", "TCP Payload Length: ",   tcpPayloadLen);

        // Accumulate the HTTP response payloads.
        if(tcpPayloadLen && sourcePort == 80) {
            char *payloadData = (char*)calloc(sizeof(char), tcpPayloadLen + 1);
            Seek(fd, tcpHeaderLen - TCP_HEADER_REQUIRED_FIELDS_LEN, SEEK_CUR);
            offsetToNextPacket -= tcpHeaderLen - TCP_HEADER_REQUIRED_FIELDS_LEN;
            bytesRead = read(fd, payloadData, tcpPayloadLen);
            if(bytesRead == 0) {
                break; // EOF
            }
            offsetToNextPacket -= bytesRead;
            TCPPayload_t *tcpPayload = (TCPPayload_t*)calloc(sizeof(TCPPayload_t), 1);
            tcpPayload->data = payloadData;
            tcpPayload->payloadLen = tcpPayloadLen;
            tcpPayload->sequenceNumber = sequenceNumber;
            TCPPayloadNode_t *payloadNode = (TCPPayloadNode_t*)calloc(sizeof(TCPPayloadNode_t), 1);
            payloadNode->payload = tcpPayload;
            if(head == NULL) {
                head = payloadNode;
            }
            if(prev != NULL) {
                prev->next = payloadNode;
            }
            prev = payloadNode;
            numPayloadNodes++;
        }
        numPackets++;
    }

    TCPPayload_t **sortedPayloads = (TCPPayload_t **)calloc(sizeof(TCPPayload_t*), numPayloadNodes);
    prev = NULL;
    int payloadIndex = 0;
    while(head) {
        sortedPayloads[payloadIndex++] = head->payload;
        prev = head;
        head = head->next;
        free(prev);
    }
    qsort(sortedPayloads, numPayloadNodes, sizeof(TCPPayload_t*), compareSequenceNumbers);

    

    int imageFD = creat("image.jpg", S_IRWXU | S_IRGRP | S_IROTH);
    unsigned int prevSequenceNumber = 0;
    for(int i = 0; i < numPayloadNodes; ++i) {
        const char httpHeaderDelimeter[] = "\r\n\r\n";
        const int httpHeaderDelimeterLen = 4;
        char *imagePayload = sortedPayloads[i]->data;
        unsigned int imagePayloadLen = sortedPayloads[i]->payloadLen;
        if(prevSequenceNumber == sortedPayloads[i]->sequenceNumber) {
            continue;
        }
        else {
            prevSequenceNumber = sortedPayloads[i]->sequenceNumber;
        }
        if(i == 0) {
            imagePayload = strstr(sortedPayloads[i]->data, httpHeaderDelimeter);
            imagePayload += httpHeaderDelimeterLen;
            imagePayloadLen -= imagePayload - sortedPayloads[i]->data;
        }
        write(imageFD, imagePayload, imagePayloadLen);
    }

    return 0;

}
