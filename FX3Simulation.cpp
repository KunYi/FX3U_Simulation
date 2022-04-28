// FX3Simulation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <iomanip>
#include "RxProc.h"

// Want to open communcation port
#define  LINK_COMPORT L"COM4"

int main()
{
    std::cout << "FX3 Simulation Application\n";

    HANDLE hComm;
    DCB dcb;
    HANDLE  hArray[2];
    OVERLAPPED osReader = { 0 };
    OVERLAPPED osEvent = { 0 };
    OVERLAPPED osWrite = { 0 };
    const DWORD dwCommEvent = EV_TXEMPTY | EV_RXCHAR | EV_ERR | EV_BREAK;
    const WCHAR COMPORT[] = L"\\\\.\\" LINK_COMPORT;
    COMMTIMEOUTS timeout = { 0 };
    COMSTAT comState = { 0 };
    DWORD   dwError = 0;
    BOOL    bWaitingOnRead = FALSE;
    BOOL    bWaitingOnStat = FALSE;

    do {
        hComm = CreateFileW(COMPORT,          //port name
            GENERIC_READ | GENERIC_WRITE,    //Read/Write
            0,                               // No Sharing
            NULL,                            // No Security
            OPEN_EXISTING,                   // Open existing port only
            FILE_FLAG_OVERLAPPED,            // Non Overlapped I/O
            NULL);                           // Null for Comm Devices

        if (hComm == INVALID_HANDLE_VALUE) {
            std::cout << "Error in opening serial port" << std::endl;
            break;
        }
        else
            std::cout << "opening serial port successful" << std::endl;

        dcb.DCBlength = sizeof dcb;
        if (GetCommState(hComm, &dcb)) {
            // 115200,E,7,1
            //dcb.BaudRate = CBR_19200;
            dcb.BaudRate = CBR_115200;
            dcb.Parity = EVENPARITY;
            dcb.ByteSize = 7;
            dcb.StopBits = ONESTOPBIT;
            dcb.fParity = TRUE;
            dcb.fBinary = TRUE;
            dcb.fInX = FALSE;
            dcb.fOutX = FALSE;
            dcb.fDtrControl = DTR_CONTROL_DISABLE;
            dcb.fRtsControl = RTS_CONTROL_DISABLE;
            if (FALSE == SetCommState(hComm, &dcb)) {
                std::cout << "Error in configuration of serial port" << std::endl;
                break;
            }
            else
                std::cout << "configuration 115200,E,7,1 successful" << std::endl;
        }

        if (FALSE == SetupComm(hComm, 1024, 1024)) {
            std::cout << "Error in setting buffer length of serial port" << std::endl;
            break;
        }

        timeout.ReadIntervalTimeout = 100; // 100 milliseconds
        timeout.ReadTotalTimeoutMultiplier = 50; // 50 milliseconds
        timeout.ReadTotalTimeoutConstant = 200;
        if (FALSE == SetCommTimeouts(hComm, &timeout)) {
            std::cout << "Error in setting timeout of serial port" << std::endl;
            break;
        }

        if (FALSE == PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
            std::cout << "Error in purge buffer of serial port" << std::endl;
            break;
        }

        if (FALSE == SetCommMask(hComm, dwCommEvent)) {
            std::cout << "Error in SetCommMask of serial port" << std::endl;
            break;
        }

        if (FALSE == ClearCommError(hComm, &dwError, &comState)) {

        }

        // Create an event object for use by WaitCommEvent.
        osEvent.hEvent = CreateEvent(
                NULL,   // default security attributes
                TRUE,   // manual-reset event
                FALSE,  // not signaled
                NULL    // no name
        );
        if (osEvent.hEvent == NULL) {
            std::cout << "Error in CreateEvent for osEvent" << std::endl;
            break;
        }

        osReader.hEvent = CreateEvent(
            NULL,   // default security attributes
            TRUE,   // manual-reset event
            FALSE,  // not signaled
            NULL    // no name
        );
        if (osReader.hEvent == NULL) {
            std::cout << "Error in CreateEvent for osReader" << std::endl;
            break;
        }

        rxProc.osWrite.hEvent = CreateEvent(
            NULL,   // default security attributes
            TRUE,   // manual-reset event
            FALSE,  // not signaled
            NULL    // no name
        );
        if (rxProc.osWrite.hEvent == NULL) {
            std::cout << "Error in CreateEvent for osReader" << std::endl;
            break;
        }

        hArray[0] = osReader.hEvent;
        hArray[1] = osEvent.hEvent;

        initSystem();
        rxProc.State = STATE_IDLE;
        rxProc.DataLen = 0;

        do {
            BYTE    Buffer[512];
            BOOL    bExit = FALSE;
            DWORD   dwRead = 0;
            DWORD   dwEvent = 0;

            if (bWaitingOnStat == FALSE)
            {
                if (WaitCommEvent(hComm, &dwEvent, &osEvent))
                {
                    if (dwEvent != 0x01 )
                         std::cout << "got event 0x" << std::setfill('0') << std::setw(4) << dwEvent << std::endl;

                    if (dwEvent & EV_BREAK)
                        ClearCommBreak(hComm);

                    if (dwEvent & EV_ERR)
                        ClearCommError(hComm, &dwError, &comState);

                }
                else
                {
                    DWORD error = GetLastError();
                    if (error != ERROR_IO_PENDING) {

                    }
                    bWaitingOnStat = TRUE;
                }
            }

            if (bWaitingOnRead == FALSE)
            {
                if (FALSE == ReadFile(hComm, Buffer, sizeof(Buffer), &dwRead, &osReader))
                {
                    bWaitingOnRead = TRUE;
                }
                else {

                }
            }

            if (bWaitingOnRead && bWaitingOnStat)
            {
                //DWORD dwReadSize;
                dwEvent = WaitForMultipleObjects(2, hArray, FALSE, 50);
                switch (dwEvent) {
                case WAIT_OBJECT_0:
                    bWaitingOnRead = FALSE;
                    if (GetOverlappedResult(hComm, &osReader, &dwRead, FALSE))
                    {
                    }
                    else
                    {
                        DWORD error = GetLastError();
                        if (error != ERROR_IO_INCOMPLETE) {
                            std::cout << "GetOverlappedResult(), got error code:" << std::hex << error << std::endl;
                        }
                    }
                    break;
                case (WAIT_OBJECT_0 + 1):
                    bWaitingOnStat = FALSE;
                    break;
                case WAIT_TIMEOUT:
                    break;
                default:
                    bExit = TRUE;
                    break;
                }
            }

            if (dwRead) {
                RxProc(hComm, rxProc, Buffer, dwRead);
            }

            if (bExit == TRUE) {
                std::cout << "exit program" << std::endl;
                break;
            }

            if (_kbhit()) {
               int ch;
               switch ((ch = _getch()))
               {
               case 27:
                   std::cout << "got ESC key" << std::endl;
                   bExit = TRUE;
                   break;
               case '\r':
                   // bExit = TRUE;
                   break;
               case '\n':
                   break;
               }
               if (bExit == TRUE)
                   break;
            }

            if (bExit == TRUE) {
                std::cout << "exit program" << std::endl;
                break;
            }
        } while(1);

    } while (0);

    if (rxProc.osWrite.hEvent)
        CloseHandle(rxProc.osWrite.hEvent);

    if (osReader.hEvent)
        CloseHandle(osReader.hEvent);

    if (osEvent.hEvent)
        CloseHandle(osEvent.hEvent);

    //ReadFile(hComm, )
    if (hComm != INVALID_HANDLE_VALUE)
        CloseHandle(hComm);//Closing the Serial Port
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
