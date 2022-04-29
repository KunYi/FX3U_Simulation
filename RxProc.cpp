

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <iomanip>
#include "RxProc.h"

struct RX_PROC rxProc;


const uint16_t kSpecial[128] = {
    0x00C8, 0x5EA8, 0x0008, 0x0010, 0x1F82, 0x0020, 0x0018, 0x0003, /*   8 */
    0x000A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /*  16 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x000A, 0x0000, 0x0000, 0x0000, /*  24 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x003D, 0x001C, /*  32 */
    0x0000, 0x0000, 0x0014, 0x00FF, 0x03D7, 0x0000, 0x0000, 0x0000, /*  40 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, /*  48 */
    0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /*  56 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x183B, 0x0000, /*  64 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x01F4, 0x0000, /*  72 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /*  80 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /*  88 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /*  96 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0064, 0x3F68, 0x0008, 0x0000, /* 104 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /* 112 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0DDC, 0x3DB6, 0x0000, 0x0000, /* 120 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0003, 0x0000, 0x0000, /* 128 */
};

uint16_t PLCBuffer[(16 * 1024) + 216];

/* PLC code buffer*/
uint8_t CodeBuf[(33 * 1024) + 208] = {
    0x10, 0x00, 0xD8, 0xBA, 0x00, 0x00, 0x00, 0x00, /*   8 */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /*  16 */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /*  24 */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /*  32 */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /*  40 */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /*  48 */
    0xF4, 0x09, 0xFF, 0x0B, 0xF4, 0x01, 0xE7, 0x03, /*  56 */
    0x64, 0x0E, 0xC7, 0x0E, 0xDC, 0x0E, 0xFF, 0x0E, /*  64 */
    0x90, 0x01, 0xFE, 0x03, 0x00, 0x00, 0x00, 0x00, /*  72 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*  80 */
    0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*  88 */
    0x00, 0x00, 0x00, 0x00, 0X0F, 0X00, 0XFF, 0XFF, /*  96 */
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, /* 104 */
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, /* 112 */
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF,                   /* 117 */
};

uint8_t Ex8000h[1024];

void initSystem(void)
{
    for (uint8_t i = 0; i < 126; i++)
    {   // offset 3KW
        PLCBuffer[0x0C00 + i] = kSpecial[i];
    }
    
    PLCBuffer[0x2000] = CodeBuf[52] * 256;
    PLCBuffer[0x2000] += CodeBuf[53];
}

uint8_t asc2hex(BYTE v)
{
    return (v > '9') ? v - 'A' + 10 : v - '0';
}

UINT16 hex2asc(BYTE v)
{
    const char table[] = { '0', '1', '2', '3',
                           '4', '5', '6', '7',
                           '8', '9', 'A', 'B',
                           'C', 'D', 'E', 'F' };
    return (table[(v >> 4) & 0xF] * 0x100) + table[v & 0xF];
}

UINT8  Array2UINT8(BYTE* data)
{
    return (asc2hex(data[0]) << 4) +
        asc2hex(data[1]);
}

UINT16 Array2UINT16(BYTE* data)
{
    return (asc2hex(data[0]) << 12) +
        (asc2hex(data[1]) << 8) +
        (asc2hex(data[2]) << 4) +
        asc2hex(data[3]);
}

UINT32 Array2UINT32(BYTE* data)
{
    return (asc2hex(data[0]) << 28) +
        (asc2hex(data[1]) << 24) +
        (asc2hex(data[2]) << 20) +
        (asc2hex(data[3]) << 16) +
        (asc2hex(data[4]) << 12) +
        (asc2hex(data[5]) << 8) +
        (asc2hex(data[6]) << 4) +
        asc2hex(data[7]);
}

uint8_t calcChecksum(uint8_t* pB)
{
    uint8_t chksum = 0;
    while (*pB != ETX) {
        chksum += *pB;
        pB++;
    }
    return (chksum + ETX);
}

void reponseAck(HANDLE hComm, struct RX_PROC& rp)
{
    uint8_t buff[1];
    DWORD dwWrite;
    buff[0] = ACK;
    WriteFile(hComm, buff, 1, &dwWrite, &rp.osWrite);
    std::cout << std::endl << "<<< " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[0]);
    std::cout << std::endl;
}

void reponseNAck(HANDLE hComm, struct RX_PROC& rp)
{
    uint8_t buff[1];
    DWORD dwWrite;
    buff[0] = NACK;
    WriteFile(hComm, buff, 1, &dwWrite, &rp.osWrite);
    std::cout << std::endl << "<<< " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[0]);
    std::cout << std::endl;
}

void reponseNibbleData(HANDLE hComm, struct RX_PROC& rp, uint8_t value)
{
    uint8_t buff[6];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;

    buff[0] = STX;
    buff[1] = '0' + value;
    buff[2] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[4] = (u16Data >> 8);
    buff[5] = u16Data & 0xFF;
    WriteFile(hComm, buff, 6, &dwWrite, &rp.osWrite);
 
    std::cout << std::endl << "<<<";
    for (uint8_t i = 0; i < 6; i++)
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void reponseF50110(HANDLE hComm, struct RX_PROC& rp)
{
    uint8_t buff[8];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;

    buff[0] = STX;
    buff[1] = '1';
    buff[2] = '0';
    buff[3] = '0';
    buff[4] = '0';
    buff[5] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[6] = (u16Data >> 8);
    buff[7] = u16Data & 0xFF;
    WriteFile(hComm, buff, 8, &dwWrite, &rp.osWrite);

    std::cout << std::endl << "<<<";
    for (uint8_t i = 0; i < 8; i++)
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void reponseF50104(HANDLE hComm, struct RX_PROC& rp)
{
    uint8_t buff[8];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;

    buff[0] = STX;
    buff[1] = '0';
    buff[2] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[3] = (u16Data >> 8);
    buff[4] = u16Data & 0xFF;
    WriteFile(hComm, buff, 5, &dwWrite, &rp.osWrite);

    std::cout << std::endl << "<<<";
    for (uint8_t i = 0; i < 5; i++)
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void reponseCodeBuff(HANDLE hComm, struct RX_PROC& rp, uint16_t address, uint8_t len)
{
    uint8_t buff[512];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;
    uint16_t i;

    buff[0] = STX;
    for (i = 0; i < len; i++) {
        u16Data = hex2asc(CodeBuf[i]);
        buff[(i * 2) + 1] = (u16Data / 0x100);
        buff[(i * 2) + 2] = (u16Data % 0x100);
    }

    buff[i * 2 + 1] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[i * 2 + 2] = (u16Data / 0x100);
    buff[i * 2 + 3] = (u16Data % 0x100);
    WriteFile(hComm, buff, (i * 2) + 4, &dwWrite, &rp.osWrite);

    std::cout << std::endl << "<<<";
    for (uint16_t i = 0; i < ((len * 2) + 4); i++)
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void reponseSpecialArray(HANDLE hComm, struct RX_PROC& rp, uint16_t address, uint8_t len)
{
    uint8_t buff[512];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;
    uint16_t i;
    const uint8_t* pByte = (uint8_t*)kSpecial;

    buff[0] = STX;
    for (i = 0; i < len; i++) {
        u16Data = hex2asc(*(pByte + address + i));
        buff[i * 2 + 1] = u16Data / 0x100;
        buff[i * 2 + 2] = u16Data % 0x100;
    }
    buff[i * 2 + 1] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[i * 2 + 2] = (u16Data / 0x100);
    buff[i * 2 + 3] = (u16Data % 0x100);
    WriteFile(hComm, buff, (i * 2) + 4, &dwWrite, &rp.osWrite);

    std::cout << std::endl << "<<<";
    for (uint16_t i = 0; i < ((len * 2) + 4); i++)
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void reponseArray(HANDLE hComm, struct RX_PROC& rp, uint16_t address, uint8_t len)
{
    uint8_t buff[512];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;

    buff[0] = STX;
    for (uint16_t i = 1; i < (len * 2) + 1; i += 2) {
        buff[i] = '0';
        buff[i + 1] = '0';
    }
    buff[len * 2 + 1] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[len * 2 + 2] = (u16Data / 0x100);
    buff[len * 2 + 3] = (u16Data % 0x100);
    WriteFile(hComm, buff, (len * 2) + 4, &dwWrite, &rp.osWrite);
    
    std::cout << std::endl << "<<<";
    for (uint16_t i = 0; i < ((len * 2) + 4); i++)
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void reponseUINT16(HANDLE hComm, struct RX_PROC& rp, uint16_t val)
{
    uint8_t buff[8];
    uint8_t chksum;
    uint16_t u16Data;
    DWORD dwWrite;
    uint8_t* pData = reinterpret_cast<uint8_t*>(&val);

    buff[0] = STX;
    u16Data = hex2asc(*pData);
    buff[1] = (u16Data / 0x100);
    buff[2] = (u16Data % 0x100);
    u16Data = hex2asc(*(pData+1));
    buff[3] = (u16Data / 0x100);
    buff[4] = (u16Data % 0x100);
    buff[5] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[6] = (u16Data / 0x100);
    buff[7] = (u16Data % 0x100);

    WriteFile(hComm, buff, 8, &dwWrite, &rp.osWrite);

    std::cout << std::endl << "<<<";
    for (uint8_t i = 0; i < 8; i++) 
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
    std::cout << std::endl;
}

void DispatchCmd(HANDLE hComm, struct RX_PROC& rp)
{
    switch (rp.Data[0]) {
    case '0': {
        uint16_t address = Array2UINT16(&rp.Data[1]);
        uint8_t len = Array2UINT8(&rp.Data[5]);
        if (address >= 0x0E00) {
            reponseSpecialArray(hComm, rp, (address - 0x0E00), len);
        }
    }
        break;
    case '1':
        break;

    case 'E': {
        const uint16_t cmd = (rp.Data[1] * 0x100) + rp.Data[2];
        // E01, read
        if (cmd == 0x3031) {
            uint16_t address = Array2UINT16(&rp.Data[3]);
            uint8_t len = Array2UINT8(&rp.Data[7]);
            if (address >= 0x8000)
                reponseCodeBuff(hComm, rp, address - 0x8000, len);
            else
                std::cout << "not implement" << std::endl;
        }
        // E00, write
        else if (cmd == 0x3030) {
            uint16_t address = Array2UINT16(&rp.Data[3]);
            uint8_t len = Array2UINT8(&rp.Data[7]);
            if (address >= 0x8000)
                reponseCodeBuff(hComm, rp, address - 0x8000, len);
            else
                std::cout << "not implement" << std::endl;
        }
        else if ((cmd & 0xFF00) == 0x4300) {
            reponseNAck(hComm, rp);
        }
        break;
    }
    case 'F': {
        uint32_t cmd = (rp.Data[1] << 24) + (rp.Data[2] << 16) + (rp.Data[3] << 8) + rp.Data[4];
        if (cmd == 0x35303131)
            reponseF50110(hComm, rp);
        else if (cmd == 0x35303130)
            reponseF50104(hComm, rp);
        else
            reponseNibbleData(hComm, rp, 1);
    }
            break;
    }
}

BOOL RxProc(HANDLE hComm, struct RX_PROC& rp, BYTE* buff, DWORD len)
{
    BOOL ret = FALSE;
    BOOL bNewLine = TRUE;

    // for timeout over 200ms
    if ((rp.State != STATE_IDLE) &&
        (GetTickCount() - rp.LastTick) > 200)
        rp.State = STATE_IDLE;

    rp.LastTick = GetTickCount();

    for (DWORD i = 0; i < len; i++) {
        if (bNewLine) {
            std::cout << ">>>";
            bNewLine = FALSE;
        }
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[i]);
        switch (rp.State) {
        case STATE_IDLE:
            if (buff[i] == ENQ) {
                reponseAck(hComm, rp);
                bNewLine = TRUE;
            }
            if (buff[i] == STX) {
                rp.State = STATE_RXPKT;
                rp.DataLen = 0;
            }
            break;
        case STATE_RXPKT:
            rp.Data[rp.DataLen++] = buff[i];
            if (buff[i] == ETX) {
                rp.State = STATE_CHK1;
            }
            break;
        case STATE_CHK1:
            rp.Data[rp.DataLen++] = buff[i];
            rp.State = STATE_CHK2;
            break;
        case STATE_CHK2:
        {
            rp.Data[rp.DataLen++] = buff[i];
            if (calcChecksum(rp.Data) == Array2UINT8(&rp.Data[rp.DataLen - 2]))
            {
                ret = TRUE;
            }
            rp.State = STATE_IDLE;
        }
        break;
        } /* end of swith(rp.State) */
    }
    if (ret)
        DispatchCmd(hComm, rxProc);
    return ret;
}