#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <fstream>
#include <sstream>
#include <map>
#include <vector>

typedef unsigned short uint16_t;

class Assembler
{
    public:
        class Error : public std::exception
        {
            public:
                Error(const std::string& fname, int line, const std::string& message)
                {
                    std::stringstream ss;
                    ss << "Error: " << fname << ':' << line << ": "
                       << message;
                    _what = ss.str();
                }
                virtual ~Error() throw () {}
                virtual const char* what() const throw ()
                {
                    return _what.c_str();
                }
            private:
                std::string _what;
        };
    public:
        Assembler(const std::string& fname); 
        const std::vector<char>& Bin() const { return _bin; }
        const std::map<int, std::map<std::string, int> >& debugInfo() const
        {
            return _debugInfo;
        }

    private:
        void assemble(const std::string& fname);
        void handleOperand(const std::string& operand, bool bits16);
        void resolveLabels(std::map<std::string, uint16_t>& labels,
                           std::vector<std::pair<std::string, int> >& labelRefs);
        static int toInt(const std::string& operand);
        static std::vector<std::string> tokenize(const std::string& line);

    private:
        std::map<std::string, char> _insts;
        std::map<std::string, uint16_t> _labels;
        std::map<std::string, uint16_t> _localLabels;
        std::vector< std::pair<std::string, int> > _labelRefs;
        std::vector< std::pair<std::string, int> > _localLabelRefs;
        std::vector<char> _bin;
        std::string _topLabel;
        // This should really be a map to a pair...
        std::map<int, std::map<std::string, int> > _debugInfo;
};

#endif // ASSEMBLER_HPP

