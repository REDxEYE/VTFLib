#include "VTFLib.h"
#include "FileReader.h"

using namespace VTFLib;
using namespace VTFLib::IO::Readers;

CFileReader::CFileReader(const char *filePath)
{
    this->mHandle = nullptr;

    this->mFilePath = new char[strlen(filePath) + 1];
    strcpy(this->mFilePath, filePath);
}

CFileReader::~CFileReader()
{
    this->CFileReader::Close();
    delete[] this->mFilePath;
}

bool CFileReader::IsOpen() const
{
    return this->mHandle != nullptr;
}

bool CFileReader::Open(Diagnostics::CError &error)
{
    this->Close();

    this->mHandle = fopen(this->mFilePath, "rb");

    if(this->mHandle == nullptr)
    {
        VTFError_Set_SE(error, "Error opening file.");
        return false;
    }

    return true;
}

void CFileReader::Close()
{
    if(this->mHandle != nullptr)
    {
        fclose(this->mHandle);
        this->mHandle = nullptr;
    }
}

ssize_t CFileReader::GetStreamSize(Diagnostics::CError &error) const
{
    if(this->mHandle == nullptr)
    {
        return 0;
    }

    const ssize_t currentPos = ftell(this->mHandle);

    if(currentPos < 0)
    {
        return 0;
    }

    if(fseek(this->mHandle, 0, SEEK_END) != 0)
    {
        return 0;
    }

    const ssize_t size = ftell(this->mHandle);

    fseek(this->mHandle, currentPos, SEEK_SET);

    if(size < 0)
    {
        return 0;
    }

    return size;
}

ssize_t CFileReader::GetStreamPointer(Diagnostics::CError &error) const
{
    if(this->mHandle == nullptr)
    {
        return 0;
    }

    const long lPosition = ftell(this->mHandle);

    if(lPosition < 0)
    {
        return 0;
    }

    return static_cast<uint32_t>(lPosition);
}

ssize_t CFileReader::Seek(const ssize_t offset, const uint32_t seekMode, Diagnostics::CError &error)
{
    if(this->mHandle == nullptr)
    {
        return 0;
    }

    int origin;

    switch(seekMode)
    {
        case FILE_BEGIN:
            origin = SEEK_SET;
            break;

        case FILE_CURRENT:
            origin = SEEK_CUR;
            break;

        case FILE_END:
            origin = SEEK_END;
            break;

        default:
            VTFError_Set(error, "Invalid seek mode.");
            return 0;
    }

    if(fseek(this->mHandle, offset, origin) != 0)
    {
        VTFError_Set_SE(error, "fseek() failed.");
        return 0;
    }

    return this->GetStreamPointer(error);
}

bool CFileReader::Read(char &dstChr, Diagnostics::CError &error)
{
    if(this->mHandle == nullptr)
    {
        VTFError_Set(error,"Error file handle is null.");
        return false;
    }

    const size_t uiBytesRead = fread(&dstChr, 1, 1, this->mHandle);

    if(uiBytesRead != 1 && ferror(this->mHandle))
    {
        VTFError_Set_SE(error, "fread() failed.");
    }

    return uiBytesRead == 1;
}

ssize_t CFileReader::Read(void *dst, const uint32_t size, Diagnostics::CError &error)
{
    if(this->mHandle == nullptr)
    {
        return 0;
    }

    const size_t uiBytesRead = fread(dst, 1, size, this->mHandle);

    if(uiBytesRead < size && ferror(this->mHandle))
    {
        VTFError_Set_SE(error, "fread() failed.");
    }

    return static_cast<uint32_t>(uiBytesRead);
}