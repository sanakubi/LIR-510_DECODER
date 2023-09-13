// COMtest.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <sstream>

#pragma pack(push, 1)
union prot
{
    uint8_t data[4];
    struct
    {
        uint32_t A;
    };
};
#pragma pack(pop)

prot gotnum;
std::string strnum;
int count = 0;

std::string convert_str(int num){


    std::ostringstream strnum;
    strnum << std::hex << num;
    std::string result = strnum.str();
    
    //check negative
    int temp = std::stoi(result);
    if (temp > 90000000) temp = -100000000 + temp;
    
    if (temp < 0 && temp > -9999) {
        temp *= -1;
        result = std::to_string(temp);

        while (result.length() < 5) result.insert(0, "0");
        result.insert(result.length() - 4, ".");
        result.insert(result.length() - 6, "-");

    } else {
        result = std::to_string(temp);
        while(result.length() < 5) result.insert(0, "0");
        result.insert(result.length()-4, ".");
    }
    
    return result;
}

void ReadCOM(HANDLE* hSerial)
{
    DWORD iSize;
    char sReceivedChar;

    while (true)
    {
        ReadFile(*hSerial, &sReceivedChar, 1, &iSize, 0);
        if (iSize > 0) {
            if (sReceivedChar == 10) { 
                count = 0;
            }
            else if (sReceivedChar == 11) { 
                std::string result = convert_str(gotnum.A);

                std::cout << result << std::endl;
            } else {
                gotnum.data[count] = sReceivedChar;
                count++;
            }
        }
    }
}

void ShowCOMPorts()
{

    
    int r = 0;
    HKEY hkey = NULL;
    //Открываем раздел реестра, в котором хранится иинформация о COM портах
    r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM\\"), 0, KEY_READ, &hkey);
    if (r != ERROR_SUCCESS)
        return;

    unsigned long CountValues = 0, MaxValueNameLen = 0, MaxValueLen = 0;
    //Получаем информацию об открытом разделе реестра
    RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &CountValues, &MaxValueNameLen, &MaxValueLen, NULL, NULL);
    ++MaxValueNameLen;
    //Выделяем память
    TCHAR* bufferName = NULL, * bufferData = NULL;
    bufferName = (TCHAR*)malloc(MaxValueNameLen * sizeof(TCHAR));
    if (!bufferName)
    {
        RegCloseKey(hkey);
        return;
    }
    bufferData = (TCHAR*)malloc((MaxValueLen + 1) * sizeof(TCHAR));
    if (!bufferData)
    {
        free(bufferName);
        RegCloseKey(hkey);
        return;
    }

    unsigned long NameLen, type, DataLen;
    //Цикл перебора параметров раздела реестра
    for (unsigned int i = 0; i < CountValues; i++)
    {
        NameLen = MaxValueNameLen;
        DataLen = MaxValueLen;
        r = RegEnumValue(hkey, i, bufferName, &NameLen, NULL, &type, (LPBYTE)bufferData, &DataLen);
        if ((r != ERROR_SUCCESS) || (type != REG_SZ))
            continue;

        _tprintf(TEXT("%s\n"), bufferData);
    }
    //Освобождаем память
    free(bufferName);
    free(bufferData);
    //Закрываем раздел реестра
    RegCloseKey(hkey);
}


int main()
{
    bool check = 1;

    ShowCOMPorts();

    std::cout << " !\n";

    std::string comp;
    HANDLE hSerial;
    LPCTSTR sPortName;

    while (check) {
        std::string comp = "\\\\.\\COM";
        std::string ctemp;
        std::cin >> ctemp;
        comp += ctemp;

        std::cout << comp << std::endl;


        std::wstring temp = std::wstring(comp.begin(), comp.end());

        HANDLE hSerial;
        LPCTSTR sPortName = temp.c_str();

        hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hSerial == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                std::cout << "serial port does not exist.\n";
            }
            std::cout << "some other error occurred.\n";
            continue;

        }else {
            std::cout << "OPEN.\n";
        }

        DCB dcbSerialParams = { 0 };
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams))
        {
            std::cout << "getting state error\n";
            continue;
        }

        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        
        if (!SetCommState(hSerial, &dcbSerialParams))
        {
            std::cout << "error setting serial port state\n";
            continue;
        }

        ReadCOM(&hSerial);

        CloseHandle(hSerial);

        check = 0;
    }

    return 0;

}

