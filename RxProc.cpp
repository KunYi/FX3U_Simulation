

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <iomanip>
#include "RxProc.h"

struct RX_PROC rxProc;

BYTE asc2hex(BYTE v)
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

uint16_t ReadCmdProc(uint16_t address, uint8_t len)
{
    if (address >= 0x8000) {
        return 0x0800;
    }
    else if (address >= 0x0E00) {
        switch (address) {
        case 0x0E02: // type, 0x5EA8
            return 0xA85E; // 0xA85E for FX3U, 0x0800 for FX1, 0x5EF6 fo FX1N
        case 0x0E00:
            break;
        case 0x0ECA:
            return 0x683F;
        default:
            return 0xA85E;
        }
    }

    return 0;
}

void reponseAck(HANDLE hComm, struct RX_PROC& rp)
{
    uint8_t buff[1];
    DWORD dwWrite;
    buff[0] = 06;
    WriteFile(hComm, buff, 1, &dwWrite, &rp.osWrite);
    std::cout << std::endl << "<<< " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buff[0]);
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
    buff[len * 2 + 2] = (u16Data >> 8);
    buff[len * 2 + 3] = u16Data & 0xFF;
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

    buff[0] = STX;
    u16Data = hex2asc(val >> 8);
    buff[1] = (u16Data >> 8);
    buff[2] = (u16Data & 0xFF);
    u16Data = hex2asc(val & 0xFF);
    buff[3] = (u16Data >> 8);
    buff[4] = (u16Data & 0xFF);
    buff[5] = ETX;
    chksum = calcChecksum(&buff[1]);
    u16Data = hex2asc(chksum);
    buff[6] = (u16Data >> 8);
    buff[7] = (u16Data & 0xFF);

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
        uint16_t value = ReadCmdProc(address, len);
        if (len == 2) {
            reponseUINT16(hComm, rp, value);
        }
    }
        break;
    case '1':
        break;

    case 'E': {
        uint16_t cmd = (rp.Data[1] << 8) + rp.Data[2];
        if (cmd == 0x3031) {
            uint16_t address = Array2UINT16(&rp.Data[3]);
            uint8_t len = Array2UINT8(&rp.Data[7]);
            if (len == 2) {
                uint16_t value = ReadCmdProc(address, len);
                reponseUINT16(hComm, rp, value);
            }
            else {
                reponseArray(hComm, rp, address, len);
            }
        }
        else if (cmd == 0x3030) {
            uint16_t address = Array2UINT16(&rp.Data[3]);
            uint8_t len = Array2UINT8(&rp.Data[7]);
            reponseArray(hComm, rp, address, len); // len max 254(0xFE) + 4 (STX, EXT, CHKSUM), buffer 512
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
                std::cout << std::endl;
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