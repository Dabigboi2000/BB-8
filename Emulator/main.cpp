#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <numeric>
#include <cstdint>
#include <conio.h>
#include <chrono>
#include <thread>
#include <csignal>
#include <windows.h>
#include <queue>

#include "mcu_rom.h"

using namespace std;

template <typename StorageType, size_t Bits>
class CustomInt {
private:
    StorageType value;
    static constexpr StorageType MASK = (StorageType(1) << Bits) - 1;

public:
    CustomInt() : value(0) {}
    CustomInt(StorageType val) : value(val & MASK) {}

    operator StorageType() const { return value; }

    CustomInt& operator=(StorageType val) {
        value = val & MASK;
        return *this;
    }

    CustomInt& operator++() { value = (value + 1) & MASK; return *this; }
    CustomInt operator++(int) { CustomInt temp = *this; ++(*this); return temp; }
        
};

using uint10_t = CustomInt<uint16_t, 10>;
using uint4_t  = CustomInt<uint8_t, 4>;
using uint2_t  = CustomInt<uint8_t, 2>;

uint8_t REG[7] = {0};
uint8_t ACC = 0;

uint8_t ALU_RES = 0;
uint2_t ALU_OP = 0;
uint8_t ALU_REG[2] = {0};
uint8_t ALU_SHI = 0;

bool FLG[16] = {0};

uint8_t PC_HI = 0;
uint8_t PC_LO = 0;

uint8_t PRT_C = 0;
uint8_t PRT_D = 0;

uint8_t MEMORY[0x10000] = {0};

uint10_t IR = 0x3ff;

bool PRT_EN[4] = {false};

bool HLT = false;

bool debug = false;
int sleep = 1000;
bool keys[0x100] = {0};
queue<uint8_t> buffer;

void init();
void dataBus();
void loadInstruction();
void regMem(uint4_t sel);
void PCHiAcc(uint4_t sel);
void PCLoAcc(uint4_t sel);
void incPC();
void regPCHi();
void regPCLo();
void regALU();
void ALUAcc(uint4_t sel);
void reqIf(uint4_t sel);
void regShi(uint4_t sel);
void prtA();
void prtB();
void prtC();
void prtD();
void sys(uint4_t sel);
void handlePorts();
void handleExternalFlags();

volatile sig_atomic_t running = 1;

streambuf* mainWindowBuffer = nullptr;
streambuf* debugWindowBuffer = nullptr;
ofstream debugFileStream;

void initDebugTerminal() {
    const string logFile = "debug_pipe.txt";

    { ofstream clear(logFile, ios::trunc); }

    debugFileStream.open(logFile, ios::app);
    debugWindowBuffer = debugFileStream.rdbuf();
    mainWindowBuffer = cout.rdbuf();

#if defined(_WIN32)
   
    string cmd = "start powershell.exe -NoExit -Command \"Clear-Host; Write-Host '--- DEBUG STREAM ACTIVE ---' -ForegroundColor Cyan; Get-Content " + logFile + " -Wait -Tail 0\"";
    system(cmd.c_str());
#else
   
    string cmd = "gnome-terminal -- sh -c \"clear; echo '--- DEBUG STREAM ACTIVE ---'; tail -f " + logFile + "\" &";
    system(cmd.c_str());
#endif

    this_thread::sleep_for(chrono::milliseconds(500));
}

int main(int argc, char* argv[])
{    
    if (argc < 2)
    {
        cout << "usage: emulator.exe <source file>\n" << flush;
        return 1;
    }

    debug = false;

    if (argc >= 3)
    {
        if (string(argv[2]) == "-d" || string(argv[2]) == "-D")
        {
            debug = true;
            cout << "opening debug monitor..\n" << flush;
        }

        if (string(argv[2]) == "-s" || string(argv[2]) == "-S")
        {
            debug = false;
            cout << "debug: false\n" << flush;
        }
    }

    if (argc >= 4)
    {
        sleep = stoi(argv[3]);
    }

    ifstream file(argv[1], ios::binary | ios::ate);

    if (!file.is_open())
    {
        cout << "error: could not open file" << argv[1] << '\n' << flush;
        return 1;
    }

    streamsize size = file.tellg();

    if (size > 0x10000)
    {
        cout << "error: file too large\nmax filesize is 0xffff bytes\n" << flush;
        return 1;
    }

    file.seekg(0, ios::beg);

    if (file.read(reinterpret_cast<char*>(MEMORY), size))
    {
        cout << "sucsessfully loaded " << size << "bytes\n" << flush;
    }

    else
    {
        cout << "error: could not load file" << argv[1] << '\n' << flush;
        return 1;
    }

    init();
    cout << "init complete" << flush;
    cout << "\033[2J\033[H" << flush;

    HLT = false;
    while (!HLT && running)
    {
        for (int i = 0; i < 4; i++) PRT_EN[i] = false;
        dataBus();
        handlePorts();
        handleExternalFlags();

        if (debug) cout.rdbuf(mainWindowBuffer);


        if (debug)
        {
            cout.rdbuf(debugWindowBuffer);

            cout << "\033[2J\033[H";

            cout << "REGISTERS:\n";

            for (int i = 0; i < 7; i++)
                cout << i << " | " << (int)REG[i] << '\n';

            cout << "\nACC | " << (int)ACC;
            cout << "\nADDR | " << hex << ((PC_HI * 0x100) + PC_LO);
            cout << "\n" << flush;

            cout.rdbuf(mainWindowBuffer);
        }

        this_thread::sleep_for(chrono::microseconds(sleep));

    }
    if (!running) cout << "interuppted by user\n";
    cout << "ended on address " << hex << ((PC_HI * 0x100) + PC_LO) << flush;
    cout << '\n';

    return 0;
}

void init()
{
    mainWindowBuffer = cout.rdbuf();
    
    FLG[0] = true;
    FLG[1] = false;
    IR = 0x3ff;

    if (debug) initDebugTerminal();
}

void dataBus()
{
    uint4_t LO_NIB = MCU[IR] & 0x0f;
    uint4_t HI_NIB = (MCU[IR] >> 4) & 0x0f;

    switch(LO_NIB)
    {
        case 0x0:
            regMem(HI_NIB);
            break;

        case 0x1:
            PCHiAcc(HI_NIB);
            break;

        case 0x2:
            PCLoAcc(HI_NIB);
            break;

        case 0x3:
            incPC();
            break;

        case 0x4:
            regPCHi();
            break;

        case 0x5:
            regPCLo();
            break;

        case 0x6:
            // Unused
            break;

        case 0x7:
            regALU();
            break;

        case 0x8:
            ALUAcc(HI_NIB);
            break;

        case 0x9:
            reqIf(HI_NIB);
            break;

        case 0xa:
            prtA();
            break;

        case 0xb:
            regShi(HI_NIB);
            break;

        case 0xc:
            prtB();
            break;

        case 0xd:
            prtC();
            break;

        case 0xe:
            prtD();
            break;

        case 0xf:
            sys(HI_NIB);
            break;
    }
    IR++;
}

void loadInstruction()
{
    IR = (MEMORY[(PC_HI * 0x100) + PC_LO] * 4) - 1;

    // cout << hex << (int)MEMORY[(PC_HI * 0xff) + PC_LO] << '\n' << flush;
}

void regMem(uint4_t sel)
{
    if ((sel & 0x1) == 0)
    {
        sel = sel >> 1;
        if((sel & 0x7) == 0x7) ACC = MEMORY[(PC_HI * 0x100) + PC_LO];
        else ACC = REG[sel & 0x7];
    }

    else
    {
        sel = sel >> 1;
        if((sel & 0x7) == 0x7) MEMORY[(PC_HI * 0x100) + PC_LO] = ACC;
        else REG[sel & 0x7] = ACC;
    }
}

void PCHiAcc(uint4_t sel)
{
    PC_HI = ACC;
}

void PCLoAcc(uint4_t sel)
{
    PC_LO = ACC;
}

void incPC()
{
    PC_LO++;
    if (PC_LO == 0) PC_HI++;
}

void regPCHi()
{
    ACC = PC_HI;
}

void regPCLo()
{
    ACC = PC_LO;
}

void regALU()
{
    ACC = ALU_RES;
}

void ALUAcc(uint4_t sel)
{
    uint2_t CRUMB_HI = (sel >> 2) & 0x03;
    uint2_t CRUMB_LO = sel & 0x03;

    ALU_OP = CRUMB_LO;

    if (CRUMB_HI == 1)
    {
        ALU_REG[0] = ACC;
    }

    if (CRUMB_HI == 2)
    {
        ALU_REG[1] = ACC;
    }

    ALU_SHI = (ALU_REG[0] << ALU_REG[1]) >> 4 & 0x0F;

    if (ALU_REG[0] > ALU_REG[1])
    {
        FLG[6] = true;
        FLG[7] = false;
        FLG[8] = false;

        FLG[9] = !FLG[7];
    }

    else if (ALU_REG[0] == ALU_REG[1])
    {
        FLG[6] = false;
        FLG[7] = true;
        FLG[8] = false;

        FLG[9] = !FLG[7];
    }

    
    else if (ALU_REG[0] < ALU_REG[1])
    {
        FLG[6] = false;
        FLG[7] = false;
        FLG[8] = true;

        FLG[9] = !FLG[7];
    }
        if (ALU_OP == 0x3)
        {

            uint16_t result = (uint16_t)ALU_REG[0] + (uint16_t)ALU_REG[1];

            ALU_RES = result & 0xff;
            FLG[3] = result > 0xff;  // Carry
            FLG[2] = ALU_RES == 0;   // Zero

            FLG[4] = !FLG[2];
            FLG[5] = !FLG[3];

        }
        
        if (ALU_OP == 0x2)
        {
            ALU_RES = ALU_REG[0] - ALU_REG[1];

            if (ALU_REG[0] - ALU_REG[1] < 0) FLG[3] = true;
            else FLG[3] = false;

            if (ALU_RES == 0) FLG[2] = true;
            else FLG[2] = false;

            FLG[4] = !FLG[2];
            FLG[5] = !FLG[3];

        }
        
        if (ALU_OP == 0x1)
        {
            ALU_RES = ALU_REG[0] * ALU_REG[1];

            if (ALU_REG[0] + ALU_REG[1] > 0xff) FLG[3] = true;
            else FLG[3] = false;

            if (ALU_RES == 0) FLG[2] = true;
            else FLG[2] = false;

            FLG[4] = !FLG[2];
            FLG[5] = !FLG[3];

        }
        
        if (ALU_OP == 0x0)
        {
            if (ALU_REG[1] == 0)
            {
                ALU_RES = 0;
                FLG[3] = false;
            }
            else
            {
                ALU_RES = ALU_REG[0] / ALU_REG[1];

                FLG[3] = (ALU_REG[0] % ALU_REG[1]) > 0;
            }

            FLG[2] = (ALU_RES == 0);
            FLG[4] = !FLG[2];
            FLG[5] = !FLG[3];

        }   

}

void reqIf(uint4_t sel)
{
    if (!FLG[sel]) loadInstruction();
}

void prtA()
{
    PRT_EN[0] = true;
}

void regShi(uint4_t sel)
{
    ACC = ALU_SHI;
}

void prtB()
{
    PRT_EN[1] = true;
}

void prtC()
{
    PRT_EN[2] = true;
    ACC = PRT_C;
}

void prtD()
{
    PRT_EN[3] = true;
}

void sys(uint4_t sel)
{
    if (sel == 0x1)
    {
        HLT = true;
    }

    else if (sel == 0xf)
    {
        loadInstruction();
    }
}

void handlePorts()
{
    if (PRT_EN[0])
        if (ACC == 0x0c) cout << "\033[2J\033[H";
        else if (ACC == 0x0a) cout << '\n' << '\r';
        else cout << static_cast<char>(ACC) << flush;

    if (PRT_EN[1])
    {
        // not implemented
    }

    if (PRT_EN[2])
    {
        // not implemented
    }

    if (PRT_EN[3])
    {
        if (!buffer.empty())
        {
            buffer.pop();
        }
        ACC = PRT_D;
    }
}

void handleExternalFlags()
{
    BYTE keyboardState[256] = {};

    // Get the state of every key
    for (int i = 0; i < 256; i++)
    {
        if (GetAsyncKeyState(i) & 0x8000)
            keyboardState[i] = 0x80;
    }

    for (unsigned int c = 0; c < 256; c++)
    {
        bool pressed = (GetAsyncKeyState(c) & 0x8000) != 0;

        if (pressed && !keys[c])
        {
            keys[c] = true;

            if (c == VK_RETURN)
            {
                buffer.push(0x0a);
            }
            else
            {
                WCHAR character[4] = {};

                int result = ToUnicodeEx(
                    c,
                    MapVirtualKey(c, MAPVK_VK_TO_VSC),
                    keyboardState,
                    character,
                    4,
                    0,
                    GetKeyboardLayout(0)
                );

                if (result > 0)
                {
                    for (int i = 0; i < result; i++)
                    {
                        if (character[i] <= 0x7F)
                            buffer.push((uint8_t)character[i]);
                    }
                }
            }
        }
        else if (!pressed)
        {
            keys[c] = false;
        }
    }

    if (!buffer.empty())
        PRT_D = buffer.front();
    else
        PRT_D = 0;

    FLG[0xb] = !buffer.empty();
    FLG[0xc] = buffer.empty();
}