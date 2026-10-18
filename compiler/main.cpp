#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <numeric>
using namespace std;

string raw;
string lis[0x10000];
string half[0x10000];
string full[0x10000];
string routines[100];
string routine_addr[100];

string words[150] = 
{
    "JMR", "JMP", "WRA", "WRB", "WRC", "LDA", "LDB", "LDC", "LDD", "LDE", "LDF", "LDG", "RDA", "RDB", "RDC", "RDD", "RDE", "RDF", "RDG",
    "A>B", "A>C", "A>D", "A>E", "A>F", "A>G", "B>A", "B>C", "B>D", "B>E", "B>F", "B>G", "C>A", "C>B", "C>D", "C>E", "C>F", "C>G", "D>A",
    "D>B", "D>C", "D>E", "D>F", "D>G", "E>A", "E>B", "E>C", "E>D", "E>F", "E>G", "F>A", "F>B", "F>C", "F>D", "F>E", "F>G", "G>A", "G>B",
    "G>C", "G>D", "G>E", "G>F", "AA0", "BA0", "CA0", "DA0", "EA0", "FA0", "GA0", "AA1", "BA1", "CA1", "DA1", "EA1", "FA1", "GA1", "ALA",
    "ALB", "ALC", "ALD", "ALE", "ALF", "ALG", "ADD", "SUB", "MUL", "DIV", "TRU", "FAL", "ZER", "CAR", "NZE", "NCA", "GRE", "EQU", "LES",
    "NEQ", "EX0", "EX1", "EX2", "EX3", "EX4", "EX5", "PAA", "PAB", "PAC", "PAD", "PAE", "PAF", "PAG", "SHA", "SHB", "SHC", "SHD", "SHE",
    "SHF", "SHG", "PBA", "PBB", "PBC", "PBD", "PBE", "PBF", "PBG", "PCA", "PCB", "PCC", "PCD", "PCE", "PCF", "PCG", "PDA", "PDB", "PDC",
    "PDD", "PDE", "PDF", "PDG", "HLT", "NOP"
};

string cores[150] = 
{
    "00", "02", "05", "09", "0d", "11", "13", "15", "17", "19", "1b", "1d",
    "21", "25", "29", "2d", "31", "35", "39", "3e", "3f", "40", "41", "42",
    "43", "44", "45", "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e",
    "4f", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5a",
    "5b", "5c", "5d", "5e", "5f", "60", "61", "62", "63", "64", "65", "66",
    "67", "68", "69", "6a", "6b", "6c", "6d", "6e", "70", "71", "72", "73",
    "74", "75", "76", "78", "79", "7a", "7b", "7c", "7d", "7e", "80", "81",
    "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b", "8c", "8d",
    "8e", "8f", "90", "91", "92", "93", "94", "95", "96", "97", "98", "99",
    "9a", "9c", "9d", "9e", "9f", "a0", "a1", "a2", "d2", "d3", "d4", "d5",
    "d6", "d7", "d8", "da", "db", "dc", "dd", "de", "df", "e0", "e2", "e3",
    "e4", "e5", "e6", "e7", "e8", "ea", "eb"
};

int rout = 0;
int addr = 0;
int length = 0;

int load_lis();
void parse();
int finish();
string toHex(int decimalValue);
string searchSub(string target);
string format(string input, unsigned int start);
string searchCores(string target);
string stringToHex(string input);
string charToHex(char c);

int main(int argc, char* argv[])
{
    
    if (argc < 3)
    {
        cout << "Usage: compiler <source file> <index>\n";
        return 1;
    }

    string filename = argv[1];
    unsigned int start = stoi(argv[2], nullptr, 16);
    ifstream source(filename);

    if (!source)
    {
        cout << "Error: Could not open " << filename << "\n";
        return 1;
    }

    cout << "Opened " << filename << "\n";

    string a;

    while (getline(source, a))
    {
        raw += a;
        raw += '\n';
    }

    source.close();

    length = load_lis();

    parse();

    int inst = finish();

    for (int i = 0; i < inst; i++)
    {
        full[i] += ' ';
    }
    string all = accumulate(full, full + inst, string());


    string outputname = filename.substr(0, filename.find('.'));
    outputname += ".txt";

    ofstream output(outputname);

    if (!output)
    {
        cout << "Error: Could not create output file\n";
        return 1;
    }

    output << format(all, start);

    output.close();
}

int load_lis()
{
    int is_comment = 0;
    int b = 0;
    int len = raw.length();
    for (int i = 0; i < len; i++)
    {
        if (raw[i] == '\n' || raw[i] == ' ')
        {
            b++;

            if (raw[i] == '\n' && is_comment == 1)
            {
                is_comment = 0;
            }
        }
        else if (raw[i] == ';')
        {
            if (raw[i + 1] == '*')
            {
                is_comment = 2;
            }
            else if (is_comment != 2)
            {
                is_comment = 1;
            }
        }

        else if (raw[i] == '*' && raw[i + 1] == ';')
        {
            is_comment = 0;
            b++;
        }
        else if (is_comment == 0)
        {
            lis[b] += raw[i];
        }

    }
    b++;
    return b;
}

void parse()
{
    int b = 0;
    int offset = 0;
    string cur = "";
    for (int a = 0; a < length; a++)
    {
        cur = lis[a];
        if (cur[0] == '@')
        {
            string sub = cur.substr(1);
            int dist = stoi(sub, nullptr, 16) - addr;
            addr = stoi(sub, nullptr, 16);

            half[b] = "eb";
            b++;

            for (int i = 0; i < 2; i++)
            {
                half[b] = "\n";
                b++;
            }
            offset = a;
        }
        else if (cur[0] == '#')
        {
            string sub = cur.substr(1);
            routines[rout] = sub;
            routine_addr[rout] = toHex(a + addr - offset);
            rout++;
            half[b] = "eb";
            b++;
            cout << a << " " << addr << " " << offset << "\n";
        }
        else if (cur[0] == '$')
        {
            half[b] = cur.substr(1);
            b++;
        }
        else if (cur[0] == '*')
        {
            half[b] = charToHex(cur[1]);
            b++;
        }
        else if (cur[0] == '%')
        {
            half[b] = stringToHex(cur.substr(1));
            b++;
        }
        else if (cur[0] == '&')
        {
            half[b] = cur;
            b++;
        }
        else if (cur == "")
        {
            offset ++;
        }
        else
        {
            half[b] = searchCores(cur);
            b++;
        }
    }
}

string toHex(int decimalValue)
{
    stringstream ss;
    ss << hex         
       << setw(4)     
       << setfill('0')
       << decimalValue;
    
    return ss.str();
}

int finish()
{
    int b = 0;
    for (int a = 0; a < length; a++)
    {
        string cur = half[a];
        if (cur[0] == '&')
        {
            string sub = cur.substr(1, 4);
            string ret = searchSub(sub);
            if (cur[5] == 'H' || cur[5] == 'h')
            {
                string fin = ret.substr(0,2);
                full[a] = fin;
            }
            else if (cur[5] == 'L' || cur[5] == 'l')
            {
                string fin = ret.substr(2,2);
                full[a] = fin;
            }
        }
        else
        {
            full[a] = half[a];
        }
        b = a;
    }
    return b;
}

string searchSub(string target)
{
    for (int a = 0; a < rout; a++)
    {
        if (target == routines[a])
        {
            return routine_addr[a];
        }
    }
    return "0000";
}

string format(string input, unsigned int start)
{
    start = start & 0xFFFF;
    stringstream ss(input);
    string value;
    string output;

    int address = start;
    int count = 0;

    while (ss >> value)
    {
        if (count % 16 == 0)
        {
            if (count != 0)
                output += "\n";

            stringstream addressStream;
            addressStream << uppercase
                          << hex
                          << setw(4)
                          << setfill('0')
                          << address;

            output += addressStream.str() + ": ";

            address += 16;
        }
        else
        {
            output += " ";
        }

        output += value;
        count++;
    }

    return output;
}

string searchCores(string target)
{
    for (int a = 0; a < 140; a++)
    {
        if (target == words[a])
        {
            return cores[a];
        }
    }
    cout << "'" + target + "' not found";
    return "00";
}

string stringToHex(string input)
{
    int num = stoi(input);

    stringstream ss;
    ss << hex << num;

    return ss.str();
}

string charToHex(char c)
{
    stringstream ss;
    ss << uppercase << hex << setw(2) << setfill('0') << (int)(unsigned char)c;
    return ss.str();
}