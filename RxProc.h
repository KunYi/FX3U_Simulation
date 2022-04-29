#pragma once

#define STX     (0x02)
#define ETX     (0x03)
#define ENQ     (0x05)
#define ACK     (0x06)
#define NACK    (0x15)

enum RX_STATE {
    STATE_IDLE,
    STATE_RXPKT,
    STATE_CHK1,
    STATE_CHK2,
};

struct RX_PROC {
    RX_STATE State;
    uint32_t DataLen;
    BYTE     Data[256];
    OVERLAPPED osWrite;
    DWORD    LastTick;
};

extern struct RX_PROC rxProc;

void initSystem(void);
BOOL RxProc(HANDLE hComm, struct RX_PROC& rp, BYTE* buff, DWORD len);
