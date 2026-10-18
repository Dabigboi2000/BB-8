#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <numeric>
#include <vector>
#include <cstdint>
#include <algorithm>
using namespace std;


vector<uint8_t> allDat(1);
vector<uint8_t> newFile(1);
vector<uint16_t> usedBlocks(1);

bool isFreeBlock(uint16_t target);
uint16_t findFreeBlock();
void newSys();
void exportSys();
void importSys();
bool importFile();
void addFile();
void listDirectory();
void printFileName(uint8_t dirLis);
void exportFile();
int findFile(string name, string ext);

string input = "";
bool isLoaded = false;
unsigned int len = 0;
unsigned int blockSize = 0;
unsigned int dirLen = 0;
unsigned int datStart = 0;

int main()
{
    while (true)
    {
        cin >> input;

        for (char &c : input) {
            c = tolower(static_cast<unsigned char>(c)); 
        }

        if (input == "new")
        {
            newSys();
        }

        if (input == "clean")
        {
            allDat.resize(len);
            cout << ("Cleaned!\n");
        }

        if (input == "end")
        {
            return 0;
        }

        if (input == "save")
        {
            exportSys();
        }

        if (input == "open")
        {
            importSys();
        }

        if (input == "import")
        {
            addFile();
            cout << "Imported!\n";
        }

        if (input == "dir" || input == "ls")
        {
            listDirectory();
        }

        if (input == "export")
        {
            exportFile();
        }
        
    }
}

void newSys()
{
    cout << "Size: ";
    cin >> hex >> len;

    if (len > 0xffffff)
    {
        cout << "File size may not exceed 0xffffff\n";
        isLoaded = false;
        return;
    }

    isLoaded = true;
    allDat.resize(len);

    allDat[0] = (len >> 16) & 0xFF;
    allDat[1] = (len >> 8) & 0xFF;
    allDat[2] = (len >> 0) & 0xFF;

    cout << "Block size: ";
    cin >> hex >> blockSize;

    if (blockSize > 0xffff)
    {
        cout << "Block size may not exceed 0xffff\n";
        isLoaded = false;
        return;
    }

    allDat[3] = (blockSize >> 8) & 0xFF;
    allDat[4] = (blockSize >> 0) & 0xFF;

    allDat[5] = 0x40;

    cout << "Directory length: ";
    cin >> hex >> dirLen;

    if (dirLen > 0xff)
    {
        cout << "Directory length may not exceed 0xff\n";
        isLoaded = false;
        return;
    }

    allDat[6] = dirLen;

    cout << "Start of file data: ";
    cin >> hex >> datStart;

    if (datStart > len)
    {
        cout << "The beginning of file data may not exceed ";
        cout << hex << len;
        cout << "\n";
        isLoaded = false;
        return;
    }

    allDat[7] = (datStart >> 16) & 0xFF;
    allDat[8] = (datStart >> 8) & 0xFF;
    allDat[9] = (datStart >> 0) & 0xFF;

    cout << "New filesystem created successfully\n";

}

void exportSys()
{
    ofstream file("filesystem.bin", ios::binary);

    if (!file) {
        cout << "Failed to export\n";
        return;
    }

    file.write(
    reinterpret_cast<const char*>(allDat.data()),
    allDat.size()
    );

    file.close();

    cout << "Exported!\n";
}

void importSys()
{
    string path = "";
    cout << "Path: ";
    cin >> path;
    ifstream file(path, ios::binary);

    if (!file)
    {
        cout << "Failed to open file\n";
        return;
    }

    allDat.assign(
        istreambuf_iterator<char>(file),
        istreambuf_iterator<char>()
    );

    cout << "Loaded ";
    cout << hex << allDat.size();
    cout << " bytes\n";
    
    len = ((uint32_t)allDat[0x00] << 16)
        | ((uint32_t)allDat[0x01] << 8)
        | allDat[0x02];

    blockSize = ((uint16_t)allDat[0x03] << 8)
            | allDat[0x04];

    dirLen = allDat[0x06];

    datStart = ((uint32_t)allDat[0x07] << 16)
            | ((uint32_t)allDat[0x08] << 8)
            | allDat[0x09];

    cout << "Filesystem length: " << hex << len << "\n";
    cout << "Block size: " << hex << blockSize << "\n";
    cout << "Directory length: " << hex << dirLen << "\n";
    cout << "File data starts at: " << hex << datStart << "\n";

    return;
}

bool importFile()
{
    string path = "";
    cout << "Path: ";
    cin >> path;
    ifstream file(path, ios::binary);

    if (!file)
    {
        cout << "Failed to open file\n";
        return false;
    }

    newFile.assign(
        istreambuf_iterator<char>(file),
        istreambuf_iterator<char>()
    );

    cout << "Loaded ";
    cout << hex << newFile.size();
    cout << " bytes\n";

    return true;
}

void addFile()
{
    bool fileAvailable = false;
    int newDir = 0;
    for (int i = 0; i < dirLen; i++)
    {
        if (allDat[0x10 + (i * 0x40)] == 0x00)
        {
            fileAvailable = true;
            newDir = i;
            break;
        }
    }
    if (fileAvailable)
    {
        if (importFile())
        {
            if (blockSize == 0)
            {
                cout << "Block size cannot be zero\n";
                return;
            }

            if (newFile.size() > 24 * blockSize)
            {
                cout << "File too large\n";
                return;
            }

            else
            {
                allDat[0x10 + (newDir * 0x40)] = 0xff;

                string ans = "";
                cout << "Is read only? [y]yes [n]no ";
                cin >> ans;

                cout << "Name: ";
                string name = "";
                cin >> name;

                size_t targetLen = 8;
                size_t offset = 0x15 + (newDir * 0x40);

                name.resize(targetLen, ' ');
                
                cout << "File extention: ";
                string ext = "";
                cin >> ext;

                targetLen = 3;

                ext.resize(targetLen, ' ');
                name += ext;

                copy(
                    name.begin(),
                    name.end(),
                    allDat.begin() + offset
                );

                if (ans == "y" || ans == "Y") allDat[0x11 + (newDir * 0x40)] = 0x01;
                else allDat[0x11 + (newDir * 0x40)] = 0x00;

                unsigned int type = 0;
                cout << "Filetype (refer to list): ";
                cin >> hex >> type;
                if (type > 0xff)
                {
                    cout << "may not exceed 0xff";
                    return;
                }
                allDat[0x14 + (newDir * 0x40)] = static_cast<uint8_t>(type);


                usedBlocks.resize(24 * dirLen);

                for (int offset = 0; offset < dirLen; offset ++)
                {
                    for (int i = 0; i < 24; i++)
                    {
                        uint8_t high = allDat[(2 * i) + 0x20 + (offset * 0x40)];
                        uint8_t low = allDat[(2 * i) + 0x21 + (offset * 0x40)];
                        usedBlocks[(offset * 24) + i] = ((uint16_t)high << 8) | low;
                    }
                }
                
                if (blockSize == 0)
                {
                    cout << "Block size cannot be zero\n";
                    return;
                }

                if (newFile.size() > 24ULL * blockSize)
                {
                    cout << "File too large\n";
                    return;
                }

                size_t blocksNeeded =
                    (newFile.size() + blockSize - 1) / blockSize;

                for (size_t i = 0; i < blocksNeeded; i++)
                {
                    uint16_t freeBlock = findFreeBlock();

                    if (freeBlock == 0xffff)
                    {
                        cout << "There is no more available space\n";
                        return;
                    }

                    // Record the block number in the directory entry.
                    size_t entry = 0x10 + newDir * 0x40;

                    allDat[entry + 0x10 + i * 2] =
                        (freeBlock >> 8) & 0xff;

                    allDat[entry + 0x11 + i * 2] =
                        freeBlock & 0xff;

                    // Mark this block as occupied immediately.
                    usedBlocks[newDir * 24 + i] = freeBlock;

                    // Calculate the source and destination positions.
                    size_t srcOffset = i * blockSize;
                    size_t dstOffset = datStart + static_cast<size_t>(freeBlock) * blockSize;

                    size_t bytesToCopy = min(
                        static_cast<size_t>(blockSize),
                        newFile.size() - srcOffset
                    );

                    if (dstOffset + bytesToCopy > allDat.size())
                    {
                        cout << "Block exceeds filesystem bounds\n";
                        return;
                    }

                    copy_n(
                        newFile.begin() + srcOffset,
                        bytesToCopy,
                        allDat.begin() + dstOffset
                    );
                }
            }
        }
    }
    else
    {
        cout << "All directories are populated\n";
    }
}

bool isFreeBlock(uint16_t target)
{
    for (int i = 0; i < usedBlocks.size(); i++)
    {
        if (target == usedBlocks[i])
        {
            return false;
        }
    }
    return true;
}

uint16_t findFreeBlock()
{
    if (blockSize == 0 || datStart >= len)
        return 0xffff;

    uint32_t totalBlocks = (len - datStart) / blockSize;

    for (uint32_t i = 0; i < totalBlocks && i < 0xffff; i++) {
        if (isFreeBlock(static_cast<uint16_t>(i))) {
            return static_cast<uint16_t>(i);
        }
    }

    return 0xffff;
}

void listDirectory()
{
    int b = 0;

    for (int i = 0; i < dirLen; i++)
    {
        if (allDat[0x10 + (i * 0x40)] == 0xff)
        {
            printFileName(i);
            cout << "   ";

            b++;

            if (b % 3 == 0)
            {
                cout << "\n";
            }
        }
    }

    cout << "\n";
}

void printFileName(uint8_t dirLis)
{
    size_t base = 0x10 + (dirLis * 0x40);

    // Print 8-character filename
    for (size_t i = base + 0x05; i < base + 0x0D; i++)
    {
        cout << static_cast<char>(allDat[i]);
    }

    cout << ".";

    // Print 3-character extension
    for (size_t i = base + 0x0D; i < base + 0x10; i++)
    {
        cout << static_cast<char>(allDat[i]);
    }
}


void exportFile()
{
    cout << "Filename: ";
    string target;
    cin >> target;

    size_t dot = target.find_last_of('.');

    if (dot == string::npos || dot == 0 || dot == target.size() - 1)
    {
        cout << "Invalid filename\n";
        return;
    }

    string name = target.substr(0, dot);
    string ext = target.substr(dot + 1);

    if (name.size() > 8 || ext.size() > 3)
    {
        cout << "Filename must be at most 8 characters "
                "and extension at most 3 characters\n";
        return;
    }

    name.resize(8, ' ');
    ext.resize(3, ' ');

    int result = findFile(name, ext);

    if (result == -1)
    {
        cout << "File not found\n";
        return;
    }

    if (blockSize == 0)
    {
        cout << "Invalid block size\n";
        return;
    }

    size_t entry = 0x10 + static_cast<size_t>(result) * 0x40;

    ofstream file(target, ios::binary);

    if (!file)
    {
        cout << "Failed to create output file\n";
        return;
    }

    size_t totalWritten = 0;

    for (int i = 0; i < 24; i++)
    {
        size_t blockEntry = entry + 0x10 + i * 2;

        uint16_t block =
            (static_cast<uint16_t>(allDat[blockEntry]) << 8)
            | allDat[blockEntry + 1];

        if (block == 0xffff)
            break;

        size_t start = static_cast<size_t>(datStart)
                     + static_cast<size_t>(block) * blockSize;

        if (start > allDat.size() ||
            blockSize > allDat.size() - start)
        {
            cout << "Block is outside filesystem bounds\n";
            file.close();
            return;
        }

        file.write(
            reinterpret_cast<const char*>(allDat.data() + start),
            static_cast<streamsize>(blockSize)
        );

        if (!file)
        {
            cout << "Error writing file\n";
            file.close();
            return;
        }

        totalWritten += blockSize;
    }

    file.close();

    cout << "Exported " << dec << totalWritten << " bytes\n";
}

int findFile(string name, string ext)
{
    // Pad filename and extension to their stored lengths
    name.resize(8, ' ');
    ext.resize(3, ' ');

    for (int i = 0; i < dirLen; i++)
    {
        size_t base = 0x10 + (i * 0x40);

        // Skip empty directory entries
        if (allDat[base] != 0xff)
            continue;

        bool match = true;

        // Compare filename (8 characters)
        for (int j = 0; j < 8; j++)
        {
            if (allDat[base + 0x05 + j] != name[j])
                match = false;
        }

        // Compare extension (3 characters)
        for (int j = 0; j < 3; j++)
        {
            if (allDat[base + 0x0D + j] != ext[j])
                match = false;
        }

        if (match)
            return i;
    }

    return -1;
}