#include "VTFLib.h"
#include "FileReader.h"

using namespace VTFLib;
using namespace VTFLib::IO::Readers;

CFileReader::CFileReader(const vlChar *cFileName)
{
    this->hFile = NULL;

    this->cFileName = new vlChar[strlen(cFileName) + 1];
    strcpy(this->cFileName, cFileName);
}

CFileReader::~CFileReader()
{
    this->Close();
    delete []this->cFileName;
}

vlBool CFileReader::Opened() const
{
    return this->hFile != NULL;
}

vlBool CFileReader::Open()
{
    this->Close();

    this->hFile = fopen(this->cFileName, "rb");

    if(this->hFile == NULL)
    {
        LastError.Set("Error opening file.", vlTrue);
        return vlFalse;
    }

    return vlTrue;
}

vlVoid CFileReader::Close()
{
    if(this->hFile != NULL)
    {
        fclose(this->hFile);
        this->hFile = NULL;
    }
}

vlUInt CFileReader::GetStreamSize() const
{
    if(this->hFile == NULL)
    {
        return 0;
    }

    const long lPosition = ftell(this->hFile);

    if(lPosition < 0)
    {
        return 0;
    }

    if(fseek(this->hFile, 0, SEEK_END) != 0)
    {
        return 0;
    }

    const long lSize = ftell(this->hFile);

    fseek(this->hFile, lPosition, SEEK_SET);

    if(lSize < 0)
    {
        return 0;
    }

    return static_cast<vlUInt>(lSize);
}

vlUInt CFileReader::GetStreamPointer() const
{
    if(this->hFile == NULL)
    {
        return 0;
    }

    const long lPosition = ftell(this->hFile);

    if(lPosition < 0)
    {
        return 0;
    }

    return static_cast<vlUInt>(lPosition);
}

vlUInt CFileReader::Seek(vlLong lOffset, vlUInt uiMode)
{
    if(this->hFile == NULL)
    {
        return 0;
    }

    int iOrigin;

    switch(uiMode)
    {
        case FILE_BEGIN:
            iOrigin = SEEK_SET;
            break;

        case FILE_CURRENT:
            iOrigin = SEEK_CUR;
            break;

        case FILE_END:
            iOrigin = SEEK_END;
            break;

        default:
            return 0;
    }

    if(fseek(this->hFile, lOffset, iOrigin) != 0)
    {
        LastError.Set("fseek() failed.", vlTrue);
        return 0;
    }

    return this->GetStreamPointer();
}

vlBool CFileReader::Read(vlChar &cChar)
{
    if(this->hFile == NULL)
    {
        return vlFalse;
    }

    const size_t uiBytesRead = fread(&cChar, 1, 1, this->hFile);

    if(uiBytesRead != 1 && ferror(this->hFile))
    {
        LastError.Set("fread() failed.", vlTrue);
    }

    return uiBytesRead == 1;
}

vlUInt CFileReader::Read(vlVoid *vData, vlUInt uiBytes)
{
    if(this->hFile == NULL)
    {
        return 0;
    }

    const size_t uiBytesRead = fread(vData, 1, uiBytes, this->hFile);

    if(uiBytesRead < uiBytes && ferror(this->hFile))
    {
        LastError.Set("fread() failed.", vlTrue);
    }

    return static_cast<vlUInt>(uiBytesRead);
}