#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

int importMalt();
void tokeniser();
int nextToken(int start);

string filename = "";
string raw = "";

struct normal_token {
    string type;
    string name;
    vector<string> arguments;
};

struct function_token {
    string type;
    string name;
    vector<normal_token> arguments;
};

struct parsed_function_token {
    string name;
    vector<string> arguments;
};

vector<function_token> functions;
vector<parsed_function_token> parsed_functions;

namespace
{
    size_t pos = 0;

    void skipWhitespace()
    {
        while (pos < raw.size() &&
               isspace(static_cast<unsigned char>(raw[pos])))
        {
            pos++;
        }
    }

    string readWord()
    {
        skipWhitespace();

        string result;

        while (pos < raw.size() &&
               !isspace(static_cast<unsigned char>(raw[pos])) &&
               raw[pos] != '{' &&
               raw[pos] != '}' &&
               raw[pos] != '=' &&
               raw[pos] != ';' &&
               raw[pos] != ',')
        {
            result += raw[pos];
            pos++;
        }

        return result;
    }

    bool skipToOpeningBrace()
    {
        while (pos < raw.size() && raw[pos] != '{')
        {
            pos++;
        }

        if (pos >= raw.size())
        {
            cerr << "Error: Expected '{'.\n";
            return false;
        }

        pos++;
        return true;
    }

    vector<string> parseValues(char closing)
    {
        vector<string> values;
        string value;

        while (pos < raw.size())
        {
            char c = raw[pos];

            if (c == closing)
            {
                if (!value.empty())
                {
                    values.push_back(value);
                }

                pos++;
                return values;
            }

            if (c == ',' || c == ';' ||
                isspace(static_cast<unsigned char>(c)))
            {
                if (!value.empty())
                {
                    values.push_back(value);
                    value.clear();
                }

                pos++;
                continue;
            }

            // Preserve nested braces, such as {$5}.
            if (c == '{')
            {
                int depth = 0;

                do
                {
                    c = raw[pos++];
                    value += c;

                    if (c == '{')
                        depth++;
                    else if (c == '}')
                        depth--;

                } while (pos < raw.size() && depth > 0);

                if (depth != 0)
                {
                    cerr << "Error: Unmatched '{'.\n";
                    return values;
                }

                continue;
            }

            value += c;
            pos++;
        }

        cerr << "Error: Missing closing '" << closing << "'.\n";

        return values;
    }

    bool parseStatement(function_token& fn)
    {
        skipWhitespace();

        // Skip statement separators.
        while (pos < raw.size() &&
               (raw[pos] == ';' || raw[pos] == ','))
        {
            pos++;
            skipWhitespace();
        }

        if (pos >= raw.size() || raw[pos] == '}')
        {
            return false;
        }

        if (raw[pos] != '_')
        {
            cerr << "Error at character " << pos
                 << ": Expected a keyword beginning with '_'.\n";

            pos++;
            return true;
        }

        normal_token statement;

        statement.type = readWord();
        statement.name = readWord();

        skipWhitespace();

        // Skip '=' if present.
        if (pos < raw.size() && raw[pos] == '=')
        {
            pos++;
        }

        if (!skipToOpeningBrace())
        {
            return false;
        }

        statement.arguments = parseValues('}');

        fn.arguments.push_back(statement);

        return true;
    }

    bool parseFunction()
    {
        skipWhitespace();

        if (pos >= raw.size())
        {
            return false;
        }

        string type = readWord();

        if (type != "_func")
        {
            cerr << "Error: Expected '_func', got '"
                 << type << "'.\n";

            return false;
        }

        function_token fn;

        fn.type = type;
        fn.name = readWord();

        if (fn.name.empty())
        {
            cerr << "Error: Function is missing a name.\n";
            return false;
        }

        if (!skipToOpeningBrace())
        {
            return false;
        }

        while (pos < raw.size())
        {
            skipWhitespace();

            if (pos >= raw.size())
            {
                break;
            }

            // End of this function.
            if (raw[pos] == '}')
            {
                pos++;
                functions.push_back(fn);
                return true;
            }

            if (!parseStatement(fn))
            {
                skipWhitespace();

                if (pos < raw.size() && raw[pos] == '}')
                {
                    pos++;
                    functions.push_back(fn);
                    return true;
                }

                break;
            }
        }

        cerr << "Error: Function '" << fn.name
             << "' is missing its closing '}'.\n";

        return false;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: malt.exe <source file>\n";
        return 1;
    }

    filename = argv[1];

    if (importMalt() != 0)
    {
        return 1;
    }

    tokeniser();

    // Print parsed tokens for testing.
    for (const auto& fn : functions)
    {
        cout << "Function: "
             << fn.type << " " << fn.name << "\n";

        for (const auto& statement : fn.arguments)
        {
            cout << "  '"
                 << statement.type << "' '"
                 << statement.name << "' = {'";

            for (size_t i = 0;
                 i < statement.arguments.size();
                 i++)
            {
                if (i > 0)
                {
                    cout << "', '";
                }

                cout << statement.arguments[i];
            }

            cout << "'}\n";
        }
    }

    return 0;
}

void tokeniser()
{
    pos = 0;
    functions.clear();

    while (pos < raw.size())
    {
        skipWhitespace();

        if (pos >= raw.size())
        {
            break;
        }

        if (!parseFunction())
        {
            break;
        }
    }
}

int importMalt()
{
    ifstream source(filename);

    if (!source)
    {
        cerr << "Error: Could not open "
             << filename << "\n";

        return 1;
    }

    raw.clear();

    string line;

    while (getline(source, line))
    {
        raw += line;
        raw += '\n';
    }

    cout << "Opened " << filename << "\n";

    return 0;
}

int nextToken(int start)
{
    if (start < 0)
    {
        start = 0;
    }

    for (size_t i = static_cast<size_t>(start);
         i < raw.size();
         i++)
    {
        if (raw[i] == '_')
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void parser()
{
    int function count = 0;

    for (int a = 0; a < functions[function_count].arguments.size(); a++)
    {
        for (int b = 0; b < functions[function_count].arguments[a].arguments.size(); a++)
        {
            typeConvert(functions[function_count].arguments[a].type)


        }
    }
}

void typeConvert(string type)
{
    if (type == "_dat") return 0;
    else if (type == "_alu") return 1;
    else if (type == "_ret") return 2;
    else if (type == "_sub") return 3;

    else return -1;
}