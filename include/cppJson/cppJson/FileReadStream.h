#pragma once

#include <stdio.h>
#include <assert.h>

#include <vector>

#include <cppJson/noncopyable.h>

namespace json
{

class FileReadStream: noncopyable
{
public:
    using Iterator = std::vector<char>::iterator;

public:
    explicit FileReadStream(FILE* input)
    {
        // 在初始化就将数据完全读取到 buffer_中
        // 并且设置 iter_ buffer_ 首迭代器
        readStream(input);
        iter_ = buffer_.begin();
    }

    bool hasNext() const
    { return iter_ != buffer_.end(); }

    char next()
    {
        if (hasNext()) {
            char ch = *iter_;
            iter_++;
            return ch;
        }
        return '\0';
    }

    char peek()
    {
        return hasNext() ? *iter_ : '\0';
    }

    const Iterator getIter() const
    { return iter_; }

    void assertNext(char ch)
    {
        assert(peek() == ch);
        next();
    }

private:
    // 在读取的时候，尽可能一次性的将
    void readStream(FILE *input)
    {
        char buf[65536];
        while(true) {
            size_t n = ::fread(buf, 1, sizeof(buf), input);
            if (n == 0)
                break;
            buffer_.insert(buffer_.end(), buf, buf + n);
        }
    }

private:
    std::vector<char> buffer_;
    Iterator iter_;
};

}