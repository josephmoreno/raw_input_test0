#include <windows.h>
#include <iostream>

#define VID_PID_ROWS 3
#define BARCODE_LENGTH 256

RAWINPUTDEVICE rid;
char** vid_pid;
char barcode[BARCODE_LENGTH];

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
static int scan2ascii(DWORD scancode, LPWORD result);
void barcode_append(char* bc, char c);
BOOL GetMessageWithTimeout(MSG* msg);

int main()
{
    // Initialize barcode string.
    memset(barcode, '\0', BARCODE_LENGTH);

    // Initialize vid_pid with the vendor IDs / product IDs to listen to.
    vid_pid = (char **)malloc(VID_PID_ROWS * sizeof(char*));

    vid_pid[0] = (char*)malloc(sizeof(char) * (strlen("HID#VID_0C2E&PID_0901") + 1));
    strcpy_s(vid_pid[0], (strlen("HID#VID_0C2E&PID_0901") + 1), "HID#VID_0C2E&PID_0901");

    vid_pid[1] = (char*)malloc(sizeof(char) * (strlen("HID#VID_1D57&PID_001C") + 1));
    strcpy_s(vid_pid[1], (strlen("HID#VID_1D57&PID_001C") + 1), "HID#VID_1D57&PID_001C");

    vid_pid[2] = (char*)malloc(sizeof(char) * (strlen("HID#VID_0461&PID_4D81") + 1));
    strcpy_s(vid_pid[2], (strlen("HID#VID_0461&PID_4D81") + 1), "HID#VID_0461&PID_4D81");

    // Initialize data for creating a window.
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;               // Assign this procedure to this window class.
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("RawInputWnd");
    
    // Register the window class.
    if (!RegisterClass(&wc))
        return(-1);

    // Create the window with the window class that was just registered.
    HWND hwnd0 = CreateWindowEx(0, wc.lpszClassName, NULL, NULL, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hwnd0)
        return(-2);

    // Hide the window.
    ShowWindow(hwnd0, SW_HIDE);

    // Initialize data for raw input devices to listen to (reference: https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/hid-usages#usage-page).
    rid.usUsage = 0x06;             // Listen to keyboard devices.
    rid.usUsagePage = 0x01;
    rid.dwFlags = RIDEV_INPUTSINK;  // Listen even when the window is in the background.
    rid.hwndTarget = hwnd0;         // Target the window that was just created.

    // Register the raw input devices to listen to.
    if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == false)
        return(-3);

 // BEGIN CODE FOR LISTING ALL RAW INPUT DEVICES
    /*UINT numOfDev;
    PRAWINPUTDEVICELIST pridl = NULL;
    GetRawInputDeviceList(NULL, &numOfDev, sizeof(RAWINPUTDEVICELIST));

    printf("# of Devices: %d\n\n", numOfDev);

    pridl = (PRAWINPUTDEVICELIST) malloc(sizeof(RAWINPUTDEVICELIST) * numOfDev);
    if (GetRawInputDeviceList(pridl, &numOfDev, sizeof(RAWINPUTDEVICELIST)) == -1)
        return(-4);

    UINT i;
    LPVOID data_ptr = NULL;
    unsigned char data[32];
    unsigned int data_length;

    for (i = 0; i < numOfDev; ++i) {
        // This 1st GetRawInputDeviceInfoA() will write the length of the device name data to the address pointed to
        // by &data_length. LPVOID parameter must be NULL.
        if (GetRawInputDeviceInfoA(pridl[i].hDevice, RIDI_DEVICENAME, NULL, (PUINT)&data_length) == -1)
            return(-5);

        printf("%d\n", data_length);

        // This 2nd GetRawInputDeviceInfoA() will write the device name to the address pointed to by the LPVOID
        // parameter (data_ptr). The length is being read from the address &data_length.
        data_ptr = (LPVOID)malloc(data_length);
        if (GetRawInputDeviceInfoA(pridl[i].hDevice, RIDI_DEVICENAME, data_ptr, (PUINT)&data_length) == -1)
            return(-6);

        printf("%s\n", (const char*)data_ptr);

        free(data_ptr);
        data_ptr = NULL;
    }*/
// END CODE FOR LISTING ALL RAW INPUT DEVICES

    // Use this for DLL
    MSG msg = {};
    while (GetMessageWithTimeout(&msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /*MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }*/

    //free(pridl);

    unsigned int j;
    for (j = 0; j < VID_PID_ROWS; ++j) {
        free(vid_pid[j]);
    }
    free(vid_pid);
    DestroyWindow(hwnd0);

    return(0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    switch (uMsg) {
        case WM_INPUT: {
            UINT dwSize = NULL;
            LPVOID data_ptr = NULL;
            WORD make_code_char;
            unsigned int data_length;

            GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == NULL)
            {
                return 0;
            }

            if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if (raw->header.dwType == RIM_TYPEKEYBOARD)
            {
                /*printf("Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n",
                    raw->data.keyboard.MakeCode,
                    raw->data.keyboard.Flags,
                    raw->data.keyboard.Reserved,
                    raw->data.keyboard.ExtraInformation,
                    raw->data.keyboard.Message,
                    raw->data.keyboard.VKey);*/

                // This 1st GetRawInputDeviceInfoA() will write the length of the device name data to the address pointed to
                // by &data_length. LPVOID parameter must be NULL.
                if (GetRawInputDeviceInfoA(raw->header.hDevice, RIDI_DEVICENAME, NULL, (PUINT)&data_length) == -1)
                    break;

                // This 2nd GetRawInputDeviceInfoA() will write the device name to the address pointed to by the LPVOID
                // parameter (data_ptr). The length is being read from the address &data_length.
                data_ptr = (LPVOID)malloc(data_length);
                if (GetRawInputDeviceInfoA(raw->header.hDevice, RIDI_DEVICENAME, data_ptr, (PUINT)&data_length) == -1)
                    break;

                // Use this to print out device name.
                printf("Device Name: %s\n", (const char*)data_ptr);

                unsigned int i;
                unsigned char found = 0x00;
                for (i = 0; i < VID_PID_ROWS; ++i) {
                    if (strstr((const char*)data_ptr, vid_pid[i]) != NULL) {    // Look for the VID/PID of the barcode scanner.
                        found = 0x01;                                           // Set found to 0x01;
                        break;                                                  // Break the loop if found.
                    }
                }

                if ((found == 0x01) && (raw->data.keyboard.Flags == 0x01)) {    // When Flags == 0x00, key is pressed down. When Flags == 0x01, key is up.
                    scan2ascii(raw->data.keyboard.MakeCode, &make_code_char);

                    // Append the character to the barcode string if it's not a new line and not a carriage return.
                    if (((char)make_code_char != '\n') && ((char)make_code_char != '\r')
                        && ((((char)make_code_char >= 'a') && ((char)make_code_char <= 'z')) || (((char)make_code_char >= 'A') && ((char)make_code_char <= 'Z'))
                        || (((char)make_code_char >= '0') && ((char)make_code_char <= '9'))))
                        barcode_append(barcode, (char)make_code_char);
                    else if (((char)make_code_char == '\n') || ((char)make_code_char == '\r')) {
                        printf("%s -- Length = %d\n", barcode, strlen(barcode));
                        memset(barcode, '\0', BARCODE_LENGTH);
                    }
                } else {
                    if (raw->data.keyboard.VKey == VK_ESCAPE) {
                        printf("Exiting...");
                        PostQuitMessage(0);
                    }
                }
            }

            free(data_ptr);
            data_ptr = NULL;

            delete[] lpb;

            break;
        }

        case WM_CLOSE:
            printf("Exiting...");
            PostQuitMessage(0);
            
            break;

        default:
            break;
    }

    return DefWindowProc(hwnd, uMsg, wparam, lparam);
}

static int scan2ascii(DWORD scancode, LPWORD result) {
    static HKL layout = GetKeyboardLayout(0);
    static BYTE State[256];

    if (GetKeyboardState(State) == FALSE)
        return 0;
    
    UINT vk = MapVirtualKeyEx(scancode, 1, layout);
    
    return ToAsciiEx(vk, scancode, State, result, 0, layout);
}

void barcode_append(char* bc, char c) {
    if (strlen(bc) < (BARCODE_LENGTH - 1)) {
        bc[strlen(bc)] = c;
        bc[strlen(bc) + 1] = '\0';
    }

    return;
}

// Code source: https://stackoverflow.com/questions/10866311/getmessage-with-a-timeout
BOOL GetMessageWithTimeout(MSG* msg)
{
    BOOL received;
    UINT_PTR timerId = SetTimer(NULL, NULL, 1000, NULL);
    
    received = GetMessage(msg, NULL, 0, 0);
    KillTimer(NULL, timerId);
    
    if (!received)
        return FALSE;
    
    if (msg->message == WM_TIMER && msg->hwnd == NULL && msg->wParam == timerId)
        return FALSE; // Timeout
    
    return TRUE;
}