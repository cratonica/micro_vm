#include "Assembler.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include "Instructions.hpp"

using namespace std;

Assembler::Assembler(const string& fname)
{
    for (int i = 0; i < sizeof(InstStrings) / sizeof(*InstStrings); i++)
    {
        _insts[InstStrings[i]] = char(i);
    }
    assemble(fname);
    resolveLabels(_localLabels, _localLabelRefs);
    resolveLabels(_labels, _labelRefs);
}

void Assembler::assemble(const string& fname)
{
    std::ifstream file(fname.c_str());
    std::stringstream ss;
    string in;
    int line = 0;
    while (getline(file, in))
    {
        ++line;

        vector<string> tokens = tokenize(in);
        if (tokens.empty())
        {
            continue;
        }

        ss.clear();

        _debugInfo[_bin.size()].clear();
        _debugInfo[_bin.size()][fname] = line;

        if (tokens.front() == "%file")
        {
            if (tokens.size() != 2)
            {
                throw Error(fname, line, "Need 1 argument for %file directive");
            } 
            assemble(tokens.at(1));
        }
        else if (find(tokens.front().begin(), tokens.front().end(), ':') != tokens.front().end())
        {
            // Label
            if (tokens.size() != 1)
            {
                ss << in << " is not a valid identifier";
                throw Error(fname, line, ss.str());
            }
            if (find(tokens.front().begin(), tokens.front().end(), ':') == tokens.front().end() - 1)
            {
                string label(tokens.front().begin(), tokens.front().end() - 1);
                map<string, uint16_t>* labelMap;
                if (label[0] == '.')
                {
                    if (_topLabel.empty())
                    {
                        throw Error(fname, line, "You cannot declare a local label"
                                          " without a top level label");
                    }
                    labelMap = &_localLabels;
                }
                else
                {
                    resolveLabels(_localLabels, _localLabelRefs);
                    _topLabel = label;
                    _localLabels.clear();
                    _localLabelRefs.clear();
                    labelMap = &_labels;
                }
                map<string, uint16_t>::const_iterator it = labelMap->find(label);
                if (it == labelMap->end())
                {
                    (*labelMap)[label] = _bin.size();
                }
                else
                {
                    ss << label << " is defined multiple times";
                    throw Error(fname, line, ss.str());
                }
            }
            else
            {
                ss << in << " is not a valid identifier";
                throw Error(fname, line, ss.str());
            }
        }
        else
        {
            const std::map<string, char>::const_iterator it =
                _insts.find(tokens.front());
            if (it != _insts.end())
            {
                _bin.push_back(it->second); 
                char suffix = tokens.front().at(tokens.front().size()-1);
                if (isdigit(suffix))
                {
                    if (tokens.size() != 2)
                    {
                        throw Error(fname, line, "Argument required");
                    }
                    if (suffix == '1')
                    {
                        handleOperand(tokens.at(1), false);
                    }
                    else if (suffix == '2')
                    {
                        handleOperand(tokens.at(1), true);
                    }
                    else
                    {
                        throw Error(fname, line, "Invalid numeric suffix on instruction");
                    }
                }
                else
                {
                    if (tokens.size() > 1)
                    {
                        throw Error(fname, line, "Too many operands");
                    }
                }
            }
            else if (tokens.front() == "DS")
            {
                // Declare string
                string::iterator it = std::find(in.begin(), in.end(), '"');
                if (it == in.end())
                {
                    ss << "Bad syntax for string declaration";
                    throw Error(fname, line, ss.str());
                }
                ++it; // Step past quotation mark
                const string::const_iterator end = std::find(it, in.end(), '"');
                if (end == in.end())
                {
                    ss << "Bad syntax for string declaration";
                    throw Error(fname, line, ss.str());
                }

                for (; it != end; ++it)
                {
                    _bin.push_back(*it);
                }
                //_bin.push_back(0); // Terminator (should this be explicit?)

            }
            else if (tokens.front() == "D1" || tokens.front() == "D2")
            {
                std::vector<string>::const_iterator it = tokens.begin();
                const std::vector<string>::const_iterator end = tokens.end();
                for (++it; it != end; ++it)
                {
                    int num = toInt(*it);
                    if (tokens.front() == "D2")
                    {
                        _bin.push_back(char(num >> 8));
                    }
                    _bin.push_back(char(num));
                }

            }
            else if (tokens.front() == "R")
            {
                // Reserve bytes
                if (tokens.size() != 2)
                {
                    throw Error(fname, line, "(R)eserve needs one parameter");
                }
                int bytes = toInt(tokens.back());
                if (bytes <= 0)
                {
                    throw Error(fname, line, "(R)eserve needs a positive integer");
                }
                while (bytes-- > 0)
                {
                    _bin.push_back(0);
                }
            }
            else
            {
                throw Error(fname, line, "Bad instruction or directive");
            }
        }
    }
    file.close();
}

void Assembler::handleOperand(const string& operand, bool bits16)
{
    const map<string, uint16_t>::const_iterator lblIt = _labels.find(operand);
    if (operand[0] == '-' || isdigit(operand[0])) 
    {
        int num = toInt(operand);
        if (bits16)
        {
            _bin.push_back(char(num >> 8));
        }
        _bin.push_back(char(num));
    }
    else if (operand[0] == '\'' && operand.size() == 3)
    {
        _bin.push_back(operand[1]);
    }
    else
    {
        // This is an identifier
        if (operand[0] == '.')
        {
            _localLabelRefs.push_back(pair<string, int>(operand, _bin.size()));
        }
        else
        {
            _labelRefs.push_back(pair<string, int>(operand, _bin.size()));
        }
        // Make room. We'll populate this later.
        _bin.push_back(0);
        _bin.push_back(0);
    }
}

vector<string> Assembler::tokenize(const string& line)
{
    vector<string> result;

    // Strip off comments
    string cleaned(line.begin(), find(line.begin(), line.end(), '#'));
    char* buf = new char[cleaned.size()];
    strcpy(buf, cleaned.c_str());

    const char* delim = " \t";

    const char* token = strtok(buf, delim);
    while (token)
    {
        result.push_back(token);
        token = strtok(NULL, delim);
    }
    delete[] buf;

    return result;
}

void Assembler::resolveLabels(map<string, uint16_t>& labels,
                              vector<pair<string, int> >& labelRefs)
{
    const std::vector< std::pair<std::string, int> >::const_iterator end =
        labelRefs.end();
    std::vector< std::pair<std::string, int> >::const_iterator it =
        labelRefs.begin();

    for (; it != end; ++it)
    {
        const map<string, uint16_t>::const_iterator lblIt = labels.find(it->first);
        if (lblIt == labels.end())
        {
            stringstream ss;
            ss << "Unable to resolve identifier " << it->first;
            throw Error("", 0, ss.str()); 
        }
        _bin[it->second] = char(lblIt->second >> 8);
        _bin[it->second + 1] = char(lblIt->second);
    }
}

int Assembler::toInt(const string& num)
{
    int result = 0;
    stringstream ss;
    if (!num.empty())
    {
        if (num.size() > 2 && num[0] == '0' && tolower(num[1]) == 'x')
        {
            ss << string(num.c_str() + 2);
            ss >> hex >> result;
        }
        else
        {
            ss << num;
            ss >> result;
        }
    }
    return result;
}


