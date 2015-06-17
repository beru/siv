#ifndef FILE_H
#define FILE_H 1

#include <cstddef>

class InputFile
{
public:
    explicit InputFile(const char *filename);
    explicit InputFile(const wchar_t *filename);
    ~InputFile();

    bool isReadable() const;
    const unsigned char *data() const;
    size_t dataSize() const;

private:
    class Private;
    Private * const d;
};

class OutputFile
{
public:
    explicit OutputFile(const char *filename);
    explicit OutputFile(const wchar_t *filename);
    ~OutputFile();

    bool isWritable() const;
    unsigned char *prepareData(size_t maxSize);
    void commitData(unsigned char *data, size_t size);

private:
    class Private;
    Private * const d;
};

#endif
