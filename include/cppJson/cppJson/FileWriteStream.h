#pragma once 

#include <string>
#include <stdio.h>

#include <cppJson/noncopyable.h>

namespace json
{

class FileWriteStream: noncopyable
{
public:
    explicit FileWriteStream(FILE* output)
    : output_(output) 
    { }

    void put(char c)
    {
        ::putc(c, output_);
    }

    void put(const char* str)
    {
        ::fputs(str, output_);
    }

    void put(std::string_view str)
    {
        ::fprintf(output_, "%.*s", static_cast<int>(str.length()), str.data());
    }

private:
    FILE* output_;
};

}

#endif //TJSON_FILEWRITESTREAM_H
