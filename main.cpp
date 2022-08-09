#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <unordered_map>

/** supported instructions **/
enum {
    // 0     1        2       3
    PUSH,   POP,    STORE,  LOAD,           // stack oteration
    // 4     5        6       7
    ADD,    SUB,    MUL,    DIV,            // arithmetic
    // 8     9       10      11     12
    JMP,    JL,     JG,     JE,     JNE,    // code-flow manipulation
    // 13   14       15
    CALL,   RET,    LABEL,                  // procedure programming
    // 16   17       18
    COMMENT,EXIT,   UNKNOWN                 // special
};
static const std::array<std::string, 18> opcodeList = {
    "push", "pop",  "store", "load",
    "add",  "sub",  "mul",   "div",
    "jmp",  "jl",   "jg",    "je",   "jne",
    "call", "ret",  "label",
    ";",    "exit"
};

/** parse information about operation and arguments **/
static uint8_t readOpcode(const std::string & line, std::string & token1, std::string & token2) {
    uint8_t opcode = (uint8_t)UNKNOWN;
    uint64_t top = 0;
    std::string oper;
    std::string t1;
    std::string t2;

    /** read operation token **/
    while(line[top] == ' ' && top < line.size()) top++;
    while(line[top] != ' ' && top < line.size()) oper+=line[top++];

    /** read first argument token **/
    while((line[top] == ' ' || line[top] == '$') && top < line.size()) top++;
    while((line[top] != ' ' || line[top] == '$') && top < line.size()) t1+=line[top++];

    /** read second argument token **/
    while((line[top] == ' ' || line[top] == '$') && top < line.size()) top++;
    while((line[top] != ' ' || line[top] == '$') && top < line.size()) t2+=line[top++];

    for(uint64_t idx = 0; idx < opcodeList.size(); idx++)
        if(oper == opcodeList[idx]) opcode = (uint8_t)idx;

    if(t1.size()) token1 = t1;
    if(t2.size()) token2 = t2;

    return opcode;
}

/** check line on void **/
static bool isEmptyLine(const std::string & line) {
    bool isEmpty = true;
    for(uint64_t idx = 0; idx < line.size(); idx++)
        if(line[idx] != ' ') isEmpty = false;
    return isEmpty;
}

/** write needed count of bytes in given file **/
static void writeNumberBytes(std::ofstream & out_file, uint64_t bytes, void * value) {
    uint8_t * base = (uint8_t *)value;
    for(uint64_t offset = bytes; offset > 0; offset--)
        out_file << (*(base + offset - 1));
}

/** read needed count of bytes in 'value' variable **/
static void readNumberBytes(const std::string & program, uint64_t bytes, void * value, uint32_t & top) {
    uint8_t * base = (uint8_t *)value;
    for(uint32_t idx = bytes + top; idx > top; idx--)
        *(base + (bytes + top) - idx) = program[idx - 1];
    top += bytes;
}
int8_t compileCode(const std::string & input, const std::string & output) {
    std::ifstream in_file   (input,     std::ios::binary | std::ios::in );
    std::ofstream out_file  (output,    std::ios::binary | std::ios::out);
    std::unordered_map<std::string, uint64_t> hashtable;

    uint8_t     opcode;
    bool        isEmpty;
    uint64_t    line_pos = 0;
    uint64_t    byte_pos = 0;
    bool        exist_err = false;

    std::string line;
    std::string token1;
    std::string token2;
    std::string program;

    /** 1) check syntax error and find all labels **/
    while(std::getline(in_file, line)) {
        ++line_pos;

        isEmpty = isEmptyLine(line);
        opcode  = readOpcode(line, token1, token2);

        if(opcode == UNKNOWN && isEmpty)    continue;
        if(opcode == COMMENT)               continue;
        if(opcode == UNKNOWN && !isEmpty)
        { std::cerr << "Syntax error: line " << line_pos << std::endl; continue; }

        program += line;
        switch(opcode) {
            case PUSH: case JMP: case JL: case JG: case JE: case JNE: case LOAD: case CALL:
                byte_pos += 5;
                break;
            case POP: case ADD: case SUB: case MUL: case DIV: case RET: case EXIT:
                byte_pos += 1;
                break;
            case STORE:
                byte_pos += 9;
                break;
            case LABEL:
                hashtable[token1] = byte_pos;
                break;
        }
    }
    if(exist_err) return exist_err;

    /** 2) compilation stage **/
    in_file.clear();
    in_file.seekg(0, std::ios::beg);
    while(std::getline(in_file, line)) {
        opcode = readOpcode(line, token1, token2);
        if(opcode == UNKNOWN || opcode == COMMENT || opcode == LABEL)
            continue;

        writeNumberBytes(out_file, sizeof(opcode), (void *)(&opcode));
        switch(opcode) {
            case PUSH: case LOAD:
            {
                int32_t value = std::stoi(token1);
                writeNumberBytes(out_file, sizeof(value) ,  (void *)(&value ));
                break;
            }
            case POP: case ADD: case SUB: case MUL: case DIV: case RET: case EXIT:
                break;
            case JMP: case JL: case JG: case JE: case JNE: case CALL:
            {
                int32_t value = hashtable[token1];
                writeNumberBytes(out_file, sizeof(value) ,  (void *)(&value ));
                break;
            }
            case STORE:
            {
                int32_t value1 = std::stoi(token1);
                int32_t value2 = std::stoi(token2);
                writeNumberBytes(out_file, sizeof(value1),  (void *)(&value1));
                writeNumberBytes(out_file, sizeof(value1),  (void *)(&value2));
                break;
            }
        }
    }

    return 0;
}

/** execute compiled code **/
int32_t executeCode(const std::string & filename, bool debug_mode = false) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    file.seekg(0, std::ios::end);
    std::vector<int32_t> container; // <-- Stack of programm

    uint32_t    size = (uint32_t)file.tellg();
    int32_t     result = 0;         // <-- Output of programm
    uint8_t     opcode;
    uint32_t    top = 0;            // <-- Instruction Pointer

    /** read program into string **/
    char byte;
    std::string program;
    file.seekg(0, std::ios::beg);
    while(file.get(byte)) program += byte;

    /** executable program **/
    while(top < size)
    {
        /** read operation **/
        readNumberBytes(program, sizeof(opcode), (void *)(&opcode), top);
        if(debug_mode) std::cout << opcodeList[opcode] << "\t";

        switch(opcode) {
            case PUSH: case LOAD: case CALL:
            {
                int32_t value; readNumberBytes(program, sizeof(value) ,  (void *)(&value ), top);
                if(opcode == PUSH) container.push_back(value);
                if(opcode == LOAD) container.push_back(value >= 0 ?
                                   container[value] : container[container.size()+value]);
                if(opcode == CALL) { container.push_back(top); top = value; }
                if(debug_mode) std::cout << value << std::endl;
                break;
            }
            case POP: case RET:
            {
                if(opcode == POP) { result = container.back(); container.pop_back(); }
                if(opcode == RET) { top = container.back(); container.pop_back(); }
                break;
            }
            case ADD: case SUB: case MUL: case DIV:
            {
                int32_t x = container.back(); container.pop_back();
                int32_t y = container.back(); container.pop_back();
                switch(opcode) {
                    case ADD: x += y; break;
                    case SUB: x -= y; break;
                    case MUL: x *= y; break;
                    case DIV: x /= y; break;
                }
                container.push_back(x); break;
            }
            case JMP:
            {
                int32_t value; readNumberBytes(program, sizeof(value) ,  (void *)(&value ), top);
                top = value;
                if(debug_mode) std::cout << value << std::endl;
                break;
            }
            case JL: case JG: case JE: case JNE:
            {
                int32_t value; readNumberBytes(program, sizeof(value) ,  (void *)(&value ), top);
                int32_t x = container.back(); container.pop_back();
                int32_t y = container.back(); container.pop_back();
                switch(opcode) {
                    case JL:    if(x < y)   { top = value; } break;
                    case JG:    if(x > y)   { top = value; } break;
                    case JE:    if(x == y)  { top = value; } break;
                    case JNE:   if(x != y)  { top = value; } break;
                }
                if(debug_mode) std::cout << value << std::endl;
                break;
            }
            case STORE:
            {
                int32_t value1; readNumberBytes(program, sizeof(value1),  (void *)(&value1), top);
                int32_t value2; readNumberBytes(program, sizeof(value1),  (void *)(&value2), top);
                int32_t idx1 = value1 >= 0 ? value1 : container.size() + value1;
                int32_t idx2 = value2 >= 0 ? value2 : container.size() + value2;
                container[idx1] = container[idx2];

                if(debug_mode) std::cout << value1 << value2 << std::endl;
                break;
            }
            case EXIT:
            {
                goto close;
                break;
            }
        }

        if(debug_mode) {
            std::cout << "Stack state: " << std::endl;
            for(uint64_t i = 0; i < container.size(); i++)
                std::cout << "[" << i << "]" << container[i] << std::endl;
            std::cout << "NEXT STEP: (any symbol): ";
            int x; std::cin >> x;
            std::cout << std::endl << std::endl;
        }
    }
    close:
    return result;
}
int main(int n, char ** v)
{
    /** check correct of arguments **/
    assert(n == 3);
    char const * input  = v[1];
    char const * output = v[2];

    /** compile program in byte-code **/
    int8_t compile = compileCode(input, output);
    std::cout << "Compile status: " << +compile << std::endl;

    /** run byte-code in virtual machine **/
    if(compile == 0)
    {
        int32_t execute = executeCode(output);
        std::cout << "Run status: " << execute << std::endl;
    }
    return 0;
}
