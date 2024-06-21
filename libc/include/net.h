#pragma once

// TCP header struct
struct TCPHeader {
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t sequenceNumber;
    uint32_t acknowledgmentNumber;
    uint16_t dataOffset : 4;
    uint16_t reserved : 3;
    uint16_t flags : 9;
    uint16_t windowSize;
    uint16_t checksum;
    uint16_t urgentPointer;
};

// UDP header struct
struct UDPHeader {
    uint16_t sourcePort;
    uint16_t destPort;
    uint16_t length;
    uint16_t checksum;
};


// ARP header struct
struct ARPHeader {
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t hardwareAddrLen;
    uint8_t protocolAddrLen;
    uint16_t opcode;
    uint8_t senderMAC[6];
    uint32_t senderIP;
    uint8_t targetMAC[6];
    uint32_t targetIP;
};

struct ICMPHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t restOfHeader;
};

// IPv4 header struct
struct IPv4Header {
    uint8_t version : 4;
    uint8_t headerLength : 4;
    uint8_t typeOfService;
    uint16_t totalLength;
    uint16_t identification;
    uint16_t flagsFragmentOffset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t headerChecksum;
    uint32_t sourceIP;
    uint32_t destIP;
    union
    {
        struct TCPHeader tcpHeader;
        struct UDPHeader udpHeader;
        struct ICMPHeader icmpHeader;
    } transpProtocol;
    
};


struct IPV6HopByHopHeader {
    uint8_t nextHeader;
    uint8_t headerExtLength;
    uint8_t options[6];
    uint8_t padding[2];
    union
    {
        struct TCPHeader tcpHeader;
        struct UDPHeader udpHeader;
        struct ICMPHeader icmpHeader;
    } transpProtocol;
};


// IPv6 header struct
struct IPv6Header {
    uint32_t versionClassFlow;
    uint16_t payloadLength;
    uint8_t nextHeader;
    uint8_t hopLimit;
    uint8_t sourceIP[16];
    uint8_t destIP[16];
    union
    {
        struct TCPHeader tcpHeader;
        struct UDPHeader udpHeader;
        struct IPV6HopByHopHeader hopByHopHeader;
        struct ICMPHeader icmpHeader;
    } transpProtocol;
};


// Ethernet header struct
struct EthernetHeader {
    uint8_t destMAC[6];
    uint8_t sourceMAC[6];
    uint16_t etherType;
    union 
    {
        struct ARPHeader arpHeader;
        struct IPv4Header ipv4Header;
        struct IPv6Header ipv6Header;
    } protocol;
};
