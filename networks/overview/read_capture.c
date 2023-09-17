#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define BAD_ARGUMENT_COUNT             1
#define ERROR_OPENING_FILE             2
#define ERROR_SEEKING_IN_FILE          3
#define CAPTURE_TRUNCATED              4
#define PCAP_LEN_TIMESTAMPS            8
#define PCAP_LEN_DATA_FIELDS           8
#define PCAP_LEN_GLOBAL_HEADER        24
#define PCAP_LEN_DATA_CAPTURED_FIELD   4

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

    int numPackets = 0, lenDataCaptured = 0, lenDataUntruncated = 0;

    while(1) {

        // Move to the `data captured` field of the next packet.
       Seek(fd, lenDataCaptured + PCAP_LEN_TIMESTAMPS, SEEK_CUR);

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
        numPackets++;
    }

    printf("Number of packets: %d.", numPackets);
    return 0;
}
