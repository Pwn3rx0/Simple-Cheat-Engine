#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tlhelp32.h>

#define MAX_ADDRESSES 10000
HANDLE hProcess = NULL;
DWORD addresses[MAX_ADDRESSES];
int addressCount = 0;

void ListProcesses() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Failed to create process snapshot\n");
        return;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe)) {
        printf("\n=== RUNNING PROCESSES ===\n");
        do {
            char exeName[256];
            WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, exeName, sizeof(exeName), NULL, NULL);

            printf("[%d] %s (PID: %d)\n", pe.th32ProcessID, exeName, pe.th32ProcessID);

        } while (Process32NextW(hSnapshot, &pe));
        printf("==========================\n");
    }
    else {
        printf("Failed to enumerate processes (Error: %d)\n", GetLastError());
    }

    CloseHandle(hSnapshot);
}

int OpenProcessByPID(DWORD pid) {
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        printf("Failed to open process (Error: %d)\n", GetLastError());
        return 0;
    }
    return 1;
}

void Scan(int value) {
    if (hProcess == NULL) {
        printf("No process opened! Open a process first.\n");
        return;
    }

    printf("Scanning for %d...\n", value);
    addressCount = 0;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION memInfo;
    DWORD_PTR addr = 0;

    while (addr < (DWORD_PTR)sysInfo.lpMaximumApplicationAddress && addressCount < MAX_ADDRESSES) {
        if (!VirtualQueryEx(hProcess, (LPCVOID)addr, &memInfo, sizeof(memInfo))) break;

        if (memInfo.State == MEM_COMMIT && memInfo.Protect != PAGE_NOACCESS) {
            BYTE buffer[4096];
            DWORD_PTR regionStart = (DWORD_PTR)memInfo.BaseAddress;
            DWORD_PTR regionSize = memInfo.RegionSize;

            for (DWORD_PTR offset = 0; offset < regionSize && addressCount < MAX_ADDRESSES; offset += sizeof(buffer)) {
                SIZE_T bytesRead;
                SIZE_T toRead = sizeof(buffer);
                if (offset + toRead > regionSize) toRead = regionSize - offset;

                if (ReadProcessMemory(hProcess, (LPCVOID)(regionStart + offset), buffer, toRead, &bytesRead)) {
                    for (SIZE_T i = 0; i <= bytesRead - 4; i += 4) {
                        int foundValue = *(int*)&buffer[i];
                        if (foundValue == value) {
                            addresses[addressCount++] = (DWORD)(regionStart + offset + i);
                        }
                    }
                }
            }
        }
        addr = (DWORD_PTR)memInfo.BaseAddress + memInfo.RegionSize;
    }

    printf("Found %d addresses\n", addressCount);
}

void Filter(int value) {
    if (hProcess == NULL) {
        printf("No process opened!\n");
        return;
    }

    if (addressCount == 0) {
        printf("No addresses to filter!\n");
        return;
    }

    int newCount = 0;
    for (int i = 0; i < addressCount; i++) {
        int current;
        SIZE_T bytesRead;
        if (ReadProcessMemory(hProcess, (LPCVOID)addresses[i], &current, 4, &bytesRead)) {
            if (current == value) {
                addresses[newCount++] = addresses[i];
            }
        }
    }

    addressCount = newCount;
    printf("Now %d addresses match value %d\n", addressCount, value);
}

void Show() {
    if (hProcess == NULL) {
        printf("No process opened!\n");
        return;
    }

    if (addressCount == 0) {
        printf("No addresses found yet\n");
        return;
    }

    printf("\n=== %d ADDRESSES FOUND ===\n", addressCount);
    for (int i = 0; i < addressCount && i < 20; i++) {
        int value;
        SIZE_T bytesRead;
        if (ReadProcessMemory(hProcess, (LPCVOID)addresses[i], &value, 4, &bytesRead)) {
            printf("[%d] 0x%p = %d\n", i, (void*)addresses[i], value);
        }
        else {
            printf("[%d] 0x%p = [Cannot Read]\n", i, (void*)addresses[i]);
        }
    }
}

void Change(int index, int newValue) {
    if (hProcess == NULL) {
        printf("No process opened!\n");
        return;
    }

    if (index < 0 || index >= addressCount) {
        printf("Invalid index!\n");
        return;
    }

    SIZE_T bytesWritten;
    if (WriteProcessMemory(hProcess, (LPVOID)addresses[index], &newValue, 4, &bytesWritten)) {
        printf("Changed 0x%p to %d\n", (void*)addresses[index], newValue);
    }
    else {
        printf("Failed to change value (Error: %d)\n", GetLastError());
    }
}

// Freeze value (keep it constant)
void Freeze(int index, int value) {
    if (hProcess == NULL) {
        printf("No process opened!\n");
        return;
    }

    if (index < 0 || index >= addressCount) {
        printf("Invalid index!\n");
        return;
    }

    printf("\nFreezing address 0x%p at value %d\n", (void*)addresses[index], value);
    printf("Press SPACE to stop freezing\n");

    while (1) {
        if (_kbhit()) {
            if (_getch() == ' ') break;
        }

        WriteProcessMemory(hProcess, (LPVOID)addresses[index], &value, 4, NULL);
        Sleep(10);
    }

    printf("Stopped freezing\n");
}

int main() {
    printf("=== UNIVERSAL GAME CHEAT TOOL ===\n");
    printf("This tool works with ANY game!\n\n");

    while (1) {
        printf("\n=== MAIN MENU ===\n");
        printf("1. List running processes\n");
        printf("2. Open process by PID\n");
        printf("3. Scan for value\n");
        printf("4. Filter addresses\n");
        printf("5. Show addresses\n");
        printf("6. Change value\n");
        printf("7. Freeze value\n");
        printf("8. Exit\n");
        printf("Choice: ");

        int choice;
        scanf("%d", &choice);

        if (choice == 1) {
            ListProcesses();
        }
        else if (choice == 2) {
            DWORD pid;
            printf("Enter Process ID (PID): ");
            scanf("%d", &pid);

            if (OpenProcessByPID(pid)) {
                printf("Successfully opened process!\n");
            }
        }
        else if (choice == 3) {
            int value;
            printf("Enter value to scan: ");
            scanf("%d", &value);
            Scan(value);
        }
        else if (choice == 4) {
            int value;
            printf("Enter value to filter: ");
            scanf("%d", &value);
            Filter(value);
        }
        else if (choice == 5) {
            Show();
        }
        else if (choice == 6) {
            if (addressCount == 0) {
                printf("No addresses! Scan first.\n");
                continue;
            }

            int index, value;
            printf("Enter address index (0-%d): ", addressCount - 1);
            scanf("%d", &index);
            printf("Enter new value: ");
            scanf("%d", &value);
            Change(index, value);
        }
        else if (choice == 7) {
            if (addressCount == 0) {
                printf("No addresses! Scan first.\n");
                continue;
            }

            int index, value;
            printf("Enter address index (0-%d): ", addressCount - 1);
            scanf("%d", &index);
            printf("Enter value to freeze at: ");
            scanf("%d", &value);
            Freeze(index, value);
        }
        else if (choice == 8) {
            break;
        }
        else {
            printf("Invalid choice!\n");
        }
    }

    if (hProcess) CloseHandle(hProcess);
    printf("\nGoodbye!\n");
    getchar(); getchar();
    return 0;
}
