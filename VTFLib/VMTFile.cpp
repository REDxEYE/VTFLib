/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VTFLib.h"
#include "VMTFile.h"

using namespace VTFLib;
using namespace VTFLib::Nodes;

CVMTFile::CVMTFile() {
    this->mRoot = nullptr;
    this->mParseErrorLine = 0;
}

CVMTFile::CVMTFile(const CVMTFile &other) {
    this->mParseErrorLine = other.mParseErrorLine;

    if (other.mRoot == nullptr) {
        this->mRoot = nullptr;
    } else {
        this->mRoot = new CVMTGroupNode(*other.mRoot);
    }
}

CVMTFile::~CVMTFile() {
    delete this->mRoot;
}

bool CVMTFile::Create(const char *cRoot) {
    delete this->mRoot;
    this->mRoot = new CVMTGroupNode(cRoot);

    return true;
}

void CVMTFile::Destroy() {
    delete this->mRoot;
    this->mRoot = nullptr;
}

bool CVMTFile::IsLoaded() const {
    return this->mRoot != nullptr;
}

bool CVMTFile::Load(const char *filePath, Diagnostics::CError &error) {
    IO::Readers::CFileReader reader(filePath);
    return this->Load(&reader, error);
}

bool CVMTFile::Load(const void *buffer, const ssize_t bufferSize, Diagnostics::CError &error) {
    IO::Readers::CMemoryReader reader(buffer, bufferSize);
    return this->Load(&reader, error);
}

bool CVMTFile::Load(void *userData, Diagnostics::CError &error) {
    IO::Readers::CProcReader reader(userData);
    return this->Load(&reader, error);
}

bool CVMTFile::Save(const char *filePath, Diagnostics::CError &error) const {
    IO::Writers::CFileWriter writer(filePath);
    return this->Save(&writer, error);
}

bool CVMTFile::Save(void *buffer, const ssize_t bufferSize, ssize_t &realSize, Diagnostics::CError &error) const {
    realSize = 0;

    auto writer = IO::Writers::CMemoryWriter(buffer, bufferSize);

    const bool res = this->Save(&writer, error);

    realSize = writer.GetStreamSize(error);

    return res;
}

bool CVMTFile::Save(void *userData, Diagnostics::CError &error) const {
    IO::Writers::CProcWriter writer(userData);
    return this->Save(&writer, error);
}

enum EToken {
    TOKEN_EOF = 0, // No more tokens to read.
    TOKEN_NEWLINE, // Token is a newline (\n).
    TOKEN_WHITESPACE, // Token is any whitespace other than a newline.
    TOKEN_FORWARD_SLASH, // Token is a forward slash (/).
    TOKEN_QUOTE, // Token is a quote (").
    TOKEN_OPEN_BRACE, // Token is an open brace ({).
    TOKEN_CLOSE_BRACE, // Token is a close brace (}).
    TOKEN_CHAR, // Token is a char (any char).  Use GetChar().
    TOKEN_STRING, // Token is a string.  Use GetString().
    TOKEN_QUOTED_STRING,
    TOKEN_SPECIAL // Token is a specified special char.
};

// Stores token information.
class CToken {
public:
    // Create a normal token.  cChar was the tokenized char.
    explicit CToken(const EToken token, const char chr = '\0') : mToken(token), mChar(chr), mString(nullptr) {
        assert(eToken != TOKEN_CHAR && eToken != TOKEN_STRING && eToken != TOKEN_QUOTED_STRING);
    }

    // Create a char token.
    explicit CToken(const char chr) : mToken(TOKEN_CHAR), mChar(chr), mString(nullptr) {
    }

    // Create a string token.
    CToken(const char *strTok, const bool quoted) : mToken(quoted ? TOKEN_QUOTED_STRING : TOKEN_STRING), mChar('\0'),
                                                    mString(nullptr) {
        this->mString = new char[strlen(strTok) + 1];
        strcpy(this->mString, strTok);
    }

    // Copy a token.
    CToken(const CToken &other) {
        this->mToken = other.mToken;
        this->mChar = other.mChar;
        this->mString = nullptr;

        if (other.mString != nullptr) {
            this->mString = new char[strlen(other.mString) + 1];
            strcpy(this->mString, other.mString);
        }
    }

    ~CToken() {
        delete[] this->mString;
    }

    // Convert the current token to a special token.
    // We need to do this because the tokenizer reads ahead and doen't
    // know if the requested token will be special until after the fact.
    void ToSpecial(const char *lpSpecial) {
        if (this->mToken == TOKEN_EOF) {
            return;
        }

        for (const auto *pSpecial = const_cast<char *>(lpSpecial); *pSpecial != '\0'; pSpecial++) {
            if (this->mChar == *pSpecial) {
                this->mToken = TOKEN_SPECIAL;
                return;
            }
        }

        this->mToken = TOKEN_CHAR;
    }

    // Get the token that was read.
    [[nodiscard]] EToken GetToken() const {
        return this->mToken;
    }

    // Get the char that was tokenized.  Only works if
    // token is a TOKEN_CHAR or was tokenized by the byte
    // tokenizer.
    [[nodiscard]] char GetChar() const {
        return this->mChar;
    }

    // Get the string that was tokenized.  Only works if
    // token is a TOKEN_STRING.
    [[nodiscard]] const char *GetString() const {
        return this->mString;
    }

private:
    EToken mToken;
    char mChar;
    char *mString;
};

// Tokenizes single byte tokens.
class CByteTokenizer {
public:
    CByteTokenizer(IO::Readers::IReader *Reader, Diagnostics::CError &error) : mLine(1), mCurrentTokenLine(1),
                                                                               mNextTokenLine(1), mReader(Reader),
                                                                               mCurrentToken(nullptr),
                                                                               mNextToken(nullptr) {
        this->GetNextToken(error);
    }

    ~CByteTokenizer() {
        delete this->mCurrentToken;
        delete this->mNextToken;
    }

    // Get the current token and return the next one.
    CToken *Next(Diagnostics::CError &error, const char *lpSpecial = nullptr) {
        delete this->mCurrentToken;
        this->mCurrentToken = this->mNextToken;
        this->mNextToken = nullptr;
        this->mCurrentTokenLine = this->mNextTokenLine;

        if (lpSpecial && this->mCurrentToken) {
            this->mCurrentToken->ToSpecial(lpSpecial);
        }

        this->GetNextToken(error);

        return this->mCurrentToken;
    }

    // Get the curret token.
    CToken *Peek() const {
        return this->mNextToken;
    }

    // Get the line the current token starts on.
    uint32_t GetLine() const {
        return this->mCurrentTokenLine;
    }

private:
    void GetNextToken(Diagnostics::CError &error) {
        char curChr;

        this->mNextTokenLine = this->mLine;

        if (!mReader->Read(curChr, error)) {
            this->mNextToken = new CToken(TOKEN_EOF);
            return;
        }

        // Keep track of the line number.
        if (curChr == '\n') {
            this->mLine++;
        }

        if (curChr == '\r' || curChr == '\n') {
            this->mNextToken = new CToken(TOKEN_NEWLINE, curChr);
        } else if (isspace(curChr)) {
            this->mNextToken = new CToken(TOKEN_WHITESPACE, curChr);
        } else if (curChr == '/') {
            this->mNextToken = new CToken(TOKEN_FORWARD_SLASH, curChr);
        } else if (curChr == '\"') {
            this->mNextToken = new CToken(TOKEN_QUOTE, curChr);
        } else if (curChr == '{') {
            this->mNextToken = new CToken(TOKEN_OPEN_BRACE, curChr);
        } else if (curChr == '}') {
            this->mNextToken = new CToken(TOKEN_CLOSE_BRACE, curChr);
        } else {
            this->mNextToken = new CToken(curChr);
        }
    }

    uint32_t mLine;
    uint32_t mCurrentTokenLine;
    uint32_t mNextTokenLine;
    IO::Readers::IReader *mReader;

    CToken *mCurrentToken;
    CToken *mNextToken;
};

// Tokenizes multi byte tokens.
class CTokenizer {
public:
    CTokenizer(CByteTokenizer *ByteTokenizer) : mByteTokenizer(ByteTokenizer), mCurrentToken(nullptr),
                                                mNextToken(nullptr),
                                                mPendingToken(nullptr), mCurrentTokenLine(1), mNextTokenLine(1) {
    }

    // This can throw, so it is kept out of the constructor
    void Prime(Diagnostics::CError &error) {
        this->GetNextToken(error);
    }

    ~CTokenizer() {
        delete this->mCurrentToken;
        delete this->mNextToken;
        delete this->mPendingToken;
    }

    void GetNextToken(Diagnostics::CError &error) {
        try {
            this->GetNextTokenInternal(error);
        } catch (char *cErrorMessage) {
            // The bad token is the one we were reading ahead for
            // so blame it line rather than the line of the last token handed to the parser
            this->mCurrentTokenLine = this->mNextTokenLine;
            throw;
        }
    }

    CToken *Next(Diagnostics::CError &error) {
        delete this->mCurrentToken;
        this->mCurrentToken = this->mNextToken;
        this->mNextToken = nullptr;
        this->mCurrentTokenLine = this->mNextTokenLine;

        this->GetNextToken(error);

        return this->mCurrentToken;
    }

    [[nodiscard]] CToken *Peek() const {
        return this->mNextToken;
    }

    [[nodiscard]] uint32_t GetLine() const {
        return this->mCurrentTokenLine;
    }

private:
    // Consume the rest of a comment
    CToken *ConsumeComment(Diagnostics::CError &error) const {
        CToken *Token;

        do {
            Token = this->mByteTokenizer->Next(error, "\n");
        } while (Token->GetToken() == TOKEN_CHAR);

        return new CToken(Token->GetToken() == TOKEN_EOF ? TOKEN_EOF : TOKEN_NEWLINE);
    }

    // Read the rest of an unquoted string
    CToken *ReadUnquotedString(char *cBuffer, uint32_t uiIndex, Diagnostics::CError &error) {
        while (true) {
            const CToken *peek = this->mByteTokenizer->Peek();

            if (peek->GetToken() == TOKEN_CHAR) {
                cBuffer[uiIndex++] = this->mByteTokenizer->Next(error)->GetChar();
            } else if (peek->GetToken() == TOKEN_FORWARD_SLASH) {
                this->mByteTokenizer->Next(error);

                if (this->mByteTokenizer->Peek()->GetToken() == TOKEN_FORWARD_SLASH) {
                    // followed by a comment
                    this->mByteTokenizer->Next(error);
                    this->mPendingToken = this->ConsumeComment(error);
                    break;
                }

                cBuffer[uiIndex++] = '/';
            } else {
                break;
            }
        }

        cBuffer[uiIndex++] = '\0';

        assert(uiIndex <= 4096);

        return new CToken(cBuffer, false);
    }

    void GetNextTokenInternal(Diagnostics::CError &error) {
        if (this->mPendingToken) {
            this->mNextTokenLine = this->mByteTokenizer->GetLine();
            this->mNextToken = this->mPendingToken;
            this->mPendingToken = nullptr;
            return;
        }

        const CToken *Token = this->mByteTokenizer->Next(error);

        // Consume all whitespace.
        while (Token->GetToken() == TOKEN_WHITESPACE) {
            Token = this->mByteTokenizer->Next(error);
        }

        this->mNextTokenLine = this->mByteTokenizer->GetLine();

        uint32_t uiIndex = 0;
        char cBuffer[4096];

        switch (Token->GetToken()) {
            // Comment (these are removed for the parser)
            // or an unquoted string starting with a slash.
            case TOKEN_FORWARD_SLASH:
                if (this->mByteTokenizer->Peek()->GetToken() != TOKEN_FORWARD_SLASH) {
                    cBuffer[uiIndex++] = Token->GetChar();

                    this->mNextToken = this->ReadUnquotedString(cBuffer, uiIndex, error);
                    break;
                }

                this->mByteTokenizer->Next(error);

                this->mNextToken = this->ConsumeComment(error);
                break;
            // Quoted string.
            case TOKEN_QUOTE:
                while (true) {
                    Token = this->mByteTokenizer->Next(error, "\"");

                    if (Token->GetToken() != TOKEN_CHAR) {
                        break;
                    }

                    if (Token->GetChar() == '\r' || Token->GetChar() == '\n') {
                        throw "newline in string";
                    }

                    cBuffer[uiIndex++] = Token->GetChar();
                }
                cBuffer[uiIndex++] = '\0';

                if (Token->GetToken() != TOKEN_SPECIAL) {
                    throw "expected closing quote";
                } else {
                    this->mNextToken = new CToken(cBuffer, true);
                }
                break;
            // Unquoted string.
            case TOKEN_CHAR:
                cBuffer[uiIndex++] = Token->GetChar();

                this->mNextToken = this->ReadUnquotedString(cBuffer, uiIndex, error);
                break;
            // Let these byte tokens "pass through".
            case TOKEN_EOF:
            case TOKEN_NEWLINE:
            case TOKEN_OPEN_BRACE:
            case TOKEN_CLOSE_BRACE:
                this->mNextToken = new CToken(*Token);
                break;
            // The parser doesn't care about anything else.
            default:
                throw "unexpected token";
                break;
        }
    }

    CByteTokenizer *mByteTokenizer;
    CToken *mCurrentToken;
    CToken *mNextToken;
    CToken *mPendingToken;

    uint32_t mCurrentTokenLine;
    uint32_t mNextTokenLine;
};

// Uses multi byte tokenizer to process the file.
class CParser {
public:
    CParser(CTokenizer *Tokenizer) : mTokenizer(Tokenizer), mErrorLine(0) {
    }

    [[nodiscard]] uint32_t GetErrorLine() const {
        return this->mErrorLine;
    }

    CVMTGroupNode *Parse(Diagnostics::CError &error) {
        CVMTGroupNode *Group = nullptr;

        // Consume all newlines.
        const CToken *token = this->mTokenizer->Next(error);

        if (token==nullptr) {
            VTFError_Set(error, "Unexpected end of file");
            return nullptr;
        }

        while (token->GetToken() == TOKEN_NEWLINE) {
            token = this->mTokenizer->Next(error);
        }

        if (token->GetToken() == TOKEN_STRING || token->GetToken() == TOKEN_QUOTED_STRING) {
            Group = new CVMTGroupNode(token->GetString());
        } else {
            throw "expected shader name";
        }

        // We *may* have a group, parse it.
        this->Parse(Group, error);

        if (uiVMTParseMode == PARSE_MODE_LOOSE) {
            while (true) {
                // Consume all newlines.
                while (this->mTokenizer->Peek()->GetToken() == TOKEN_NEWLINE) {
                    token = this->mTokenizer->Next(error);
                }

                CToken *Peek = this->mTokenizer->Peek();

                if (Peek->GetToken() == TOKEN_EOF) {
                    break;
                } else if (Peek->GetToken() == TOKEN_OPEN_BRACE) {
                    CVMTGroupNode *NextGroup = nullptr;
                    try {
                        NextGroup = new CVMTGroupNode("");
                        this->Parse(NextGroup, error);
                    } catch (char *cErrorMessage) {
                        delete NextGroup;
                        throw;
                    }
                    for (uint32_t i = 0; i < NextGroup->GetNodeCount(); i++) {
                        Group->AddNode(NextGroup->GetNode(i)->Clone());
                    }
                    delete NextGroup;
                } else {
                    throw "expected end of file";
                }
            }
        } else {
            // Consume all newlines.
            token = this->mTokenizer->Next(error);
            while (token->GetToken() == TOKEN_NEWLINE) {
                token = this->mTokenizer->Next(error);
            }

            if (token->GetToken() != TOKEN_EOF) {
                throw "expected end of file";
            }
        }

        return Group;
    }

private:
    // Prase a group starting at the first brace and ending at the last.
    void Parse(CVMTGroupNode *group, Diagnostics::CError &error) {
        // Consume all newlines.
        const CToken *token = this->mTokenizer->Next(error);

        if (token == nullptr) {
            VTFError_Set(error, "Unexpected end of file");
            return;
        }

        while (token->GetToken() == TOKEN_NEWLINE) {
            token = this->mTokenizer->Next(error);
        }

        // The first token better be an open brace.
        if (token->GetToken() != TOKEN_OPEN_BRACE) {
            throw "expected open brace";
        }

        const uint32_t openBraceLine = this->mTokenizer->GetLine();

        // Parse remaining tokens.
        while (true) {
            // Consume all newlines.
            token = this->mTokenizer->Next(error);
            while (token->GetToken() == TOKEN_NEWLINE) {
                token = this->mTokenizer->Next(error);
            }

            // If we have an end brace, we found the end of the group.
            if (token->GetToken() == TOKEN_CLOSE_BRACE) {
                return;
            }

            // Running out of tokens means this group was never closed.
            // Blame the open brace, not the end of the file.
            if (token->GetToken() == TOKEN_EOF) {
                this->mErrorLine = openBraceLine;
                throw "group is missing a close brace";
            }

            // If we have a string we could have a pair or nested group.
            if (token->GetToken() == TOKEN_STRING || token->GetToken() == TOKEN_QUOTED_STRING) {
                const CToken *peek = this->mTokenizer->Peek();
                if (peek->GetToken() == TOKEN_STRING || peek->GetToken() == TOKEN_QUOTED_STRING) {
                    // We have a pair.

                    if (peek->GetToken() == TOKEN_QUOTED_STRING) {
                        group->AddStringNode(token->GetString(), peek->GetString());

                        token = this->mTokenizer->Next(error);
                    } else {
                        auto name = new char[strlen(token->GetString()) + 1];
                        strcpy(name, token->GetString());

                        // Some materials contain properties such as '"$envmaptint" .1 .1 .1', we need to read
                        // the .1's as strings and concat them (way to be consistent Valve).
                        char cBuffer[4096] = "";
                        while (this->mTokenizer->Peek()->GetToken() == TOKEN_STRING) {
                            token = this->mTokenizer->Next(error);

                            if (*cBuffer) {
                                strcat(cBuffer, " ");
                            }
                            strcat(cBuffer, token->GetString());
                        }

                        // match engine behavior
                        if (strpbrk(cBuffer, "[]") != nullptr) {
                            delete[] name;
                            throw "vector and matrix values must be quoted";
                        }

                        int32_t intTest;
                        float floatTest;
                        char stringTest[4096];

                        if (sscanf(cBuffer, "%d%s", &intTest, stringTest) == 1) {
                            // We can interpet the string as an integer, assume it is one.
                            group->AddIntegerNode(name, intTest);
                        } else if (sscanf(cBuffer, "%f%s", &floatTest, stringTest) == 1) {
                            // We can interpet the string as an single, assume it is one.
                            group->AddSingleNode(name, floatTest);
                        } else {
                            // The string must be a string...
                            group->AddStringNode(name, cBuffer);
                        }

                        delete[] name;
                    }

                    bool bNeedNewline = token->GetToken() != TOKEN_QUOTED_STRING;
                    if (bNeedNewline) {
                        EToken Next = this->mTokenizer->Peek()->GetToken();
                        if (Next == TOKEN_NEWLINE) {
                            this->mTokenizer->Next(error);
                        } else if (Next != TOKEN_CLOSE_BRACE && Next != TOKEN_EOF) {
                            throw "expected newline";
                        }
                    }
                } else if (peek->GetToken() == TOKEN_NEWLINE || peek->GetToken() == TOKEN_OPEN_BRACE) {
                    // match engine behavior
                    const char cFirst = *token->GetString();
                    if (peek->GetToken() == TOKEN_OPEN_BRACE && (cFirst == '$' || cFirst == '%')) {
                        throw "vector and matrix values must be quoted";
                    }

                    // We have a nested group, parse it.
                    this->Parse(group->AddGroupNode(token->GetString()), error);
                } else {
                    throw "expected open brace or attribute value";
                }
            } else {
                throw "expected close brace or group name or attribute name";
            }
        }
    }

    CTokenizer *mTokenizer;
    // Line to blame for the last error (0 blames the token instead)
    uint32_t mErrorLine;
};

//
// Load()
// Parses a .vmt file.  Note, the parser is very loose.  .vmt files vary
// so much in the official resources that it is hard to know what is legal.
//
bool CVMTFile::Load(IO::Readers::IReader *reader, Diagnostics::CError &error) {
    delete this->mRoot;
    this->mRoot = nullptr;

    if (!reader->Open(error))
        return false;

    auto byteTokenizer = CByteTokenizer(reader, error);
    auto tokenizer = CTokenizer(&byteTokenizer);
    auto parser = CParser(&tokenizer);

    this->mParseErrorLine = 0;

    try {
        tokenizer.Prime(error);
        this->mRoot = parser.Parse(error);
    } catch (char *errorMessage) {
        uint32_t line = parser.GetErrorLine();
        if (line == 0) {
            line = tokenizer.GetLine();
        }

        this->mParseErrorLine = line;
        VTFError_Set_Formatted(error, "Error parsing material on line %u (%s).", line, errorMessage);
    }

    reader->Close();

    return this->mRoot != nullptr;
}

/*bool CVMTFile::Load(IO::Readers::IReader *Reader)
{
	delete this->Root;
	this->Root = 0;

	if(!Reader->Open())
		return false;

	try
	{
		CVMTNode *Node = this->Load(Reader, false);

		// Make sure we loaded a group.
		if(Node->GetType() == NODE_TYPE_GROUP)
		{
			this->Root = static_cast<CVMTGroupNode *>(Node);
		}
		else
		{
			delete Node;
		}
	}
	catch(...)
	{
		LastError.Set("Error parsing material.");
	}

	Reader->Close();

	return this->Root != 0;
}*/

// This old load code wasn't "loose" enough and had problems
// with *malformed* .vmt files.  Too bad, it was compact.  This
// could be modified to read the stricter configuration files with
// ease.

//
// Load()
// Reads the next node.  Returns a group node, value node or
// null (if there is nothing to parse).  Throws an exception if
// there is an error parsing the file.
//
/*CVMTNode *CVMTFile::Load(IO::Readers::IReader *Reader, bool bInGroup)
{
	char cChar;

	uint32_t uiNameLength, uiValueLength;
	char cNameBuffer[1024], cValueBuffer[1024];

	while(true)
	{
		if(!Reader->Read(cChar))
		{
			if(bInGroup)	// We are expecting a '}'.
				throw 0;
			else
				return 0;	// We are expecting nothing, we are done.
		}

		if(isspace(cChar))
			continue;

		if(cChar == '\"')	// We have the start of something...
		{
			// Read the string.
			uiNameLength = 0;
			while(true)
			{
				if(!Reader->Read(cChar))
					throw 0;

				if(cChar == '\"')	// We found the end of the string.
					break;

				cNameBuffer[uiNameLength++] = cChar;
			}
			cNameBuffer[uiNameLength++] = '\0';

			// Find out if we have a group or value.
			while(true)
			{
				if(!Reader->Read(cChar))
					throw 0;

				if(isspace(cChar))
					continue;

				if(cChar == '{')	// We have a group.
				{
					CVMTGroupNode *Group = new CVMTGroupNode(cNameBuffer);

					try	// Cleanup resources reverse-recursively on error.
					{
						while(true)
						{
							CVMTNode *Node = this->Load(Reader, true);

							if(Node == 0)
								break;

							// We can do this because we are friends.
							Group->AddNode(Node);
						}
					}
					catch(...)
					{
						delete Group;
						throw 0;
					}

					return Group;
				}
				else if(cChar == '\"')	// We have a string value.
				{
					// Read the value (string).
					uiValueLength = 0;
					while(true)
					{
						if(!Reader->Read(cChar))
							throw 0;

						if(cChar == '\"')	// We found the end of the string.
							break;

						cValueBuffer[uiValueLength++] = cChar;
					}
					cValueBuffer[uiValueLength++] = '\0';

					return new CVMTStringNode(cNameBuffer, cValueBuffer);
				}
				else if((cChar >= '0' && cChar <= '9') || cChar == '.' || cChar == '+' || cChar == '-')	// We have a numeric value.
				{
					bool bInteger = cChar != '.';

					// Read the value (numeric).
					uiValueLength = 0;
					cValueBuffer[uiValueLength++] = cChar;
					while(true)
					{
						if(!Reader->Read(cChar))
							throw 0;

						if(isspace(cChar))	// We found the end of the number.
							break;

						if(cChar == '.')
						{
							if(bInteger)
								bInteger = false;
							else
								throw 0;
						}

						if((cChar < '0' || cChar > '9') && cChar != '.')
							throw 0;

						cValueBuffer[uiValueLength++] = cChar;
					}
					cValueBuffer[uiValueLength++] = '\0';

					if(bInteger)
					{
						return new CVMTIntegerNode(cNameBuffer, cValueBuffer);
					}
					else
					{
						return new CVMTSingleNode(cNameBuffer, cValueBuffer);
					}
				}
				else if(cChar == '/')	// We have a comment, scan past it.
				{
					if(!Reader->Read(cChar))
						throw 0;

					if(cChar != '/')
						throw 0;

					while(true)
					{
						if(!Reader->Read(cChar))
							throw 0;

						if(cChar == '\n')	// We found the end of the comment.
							break;
					}
				}
				else	// We have a problem. ;)
				{
					throw 0;
				}
			}
		}
		else if(cChar == '/')	// We have a comment, scan past it.
		{
			if(!Reader->Read(cChar))
				throw 0;

			if(cChar != '/')
				throw 0;

			while(true)
			{
				if(!Reader->Read(cChar))
					throw 0;

				if(cChar == '\n')	// We found the end of the comment.
					break;
			}
		}
		else if(cChar == '}' && bInGroup)	// We could have the end of a group (if we are looking for it).
		{
			return 0;
		}
		else	// We have a problem. ;)
		{
			throw 0;
		}
	}
}*/

bool CVMTFile::Save(IO::Writers::IWriter *writer, Diagnostics::CError &error) const {
    if (this->mRoot == nullptr) {
        VTFError_Set(error, "No material loaded.");
        return false;
    }

    if (!writer->Open(error))
        return false;

    this->Save(writer, this->mRoot, error);

    writer->Close();

    return true;
}

//
// Indent()
// Indents a line uiLevel tab sapces.
//
void CVMTFile::Indent(IO::Writers::IWriter *writer, const uint32_t level, Diagnostics::CError &error) const {
    for (uint32_t i = 0; i < level; i++) {
        writer->Write('\t', error);
    }
}

//
// Save()
// Saves a node to a file.
//
void CVMTFile::Save(IO::Writers::IWriter *writer, CVMTNode *node, Diagnostics::CError &error,
                    const uint32_t level) const {
    char cBuffer[2048];

    if (node->GetType() == NODE_TYPE_GROUP) {
        CVMTGroupNode *Group = static_cast<CVMTGroupNode *>(node);

        this->Indent(writer, level, error);
        sprintf(cBuffer, "\"%s\"\r\n", Group->GetName());
        writer->Write(cBuffer, (uint32_t) strlen(cBuffer), error);

        this->Indent(writer, level, error);
        sprintf(cBuffer, "{\r\n");
        writer->Write(cBuffer, (uint32_t) strlen(cBuffer), error);

        for (uint32_t i = 0; i < Group->GetNodeCount(); i++) {
            this->Save(writer, Group->GetNode(i), error, level + 1);
        }

        this->Indent(writer, level, error);
        sprintf(cBuffer, "}\r\n");
        writer->Write(cBuffer, (uint32_t) strlen(cBuffer), error);
    } else if (node->GetType() == NODE_TYPE_STRING) {
        CVMTStringNode *String = static_cast<CVMTStringNode *>(node);

        this->Indent(writer, level, error);
        sprintf(cBuffer, "\"%s\" \"%s\"\r\n", String->GetName(), String->GetValue());
        writer->Write(cBuffer, (uint32_t) strlen(cBuffer), error);
    } else if (node->GetType() == NODE_TYPE_INTEGER) {
        CVMTIntegerNode *Integer = static_cast<CVMTIntegerNode *>(node);

        this->Indent(writer, level, error);
        sprintf(cBuffer, "\"%s\" %d\r\n", Integer->GetName(), Integer->GetValue());
        writer->Write(cBuffer, (uint32_t) strlen(cBuffer), error);
    } else if (node->GetType() == NODE_TYPE_SINGLE) {
        CVMTSingleNode *Single = static_cast<CVMTSingleNode *>(node);

        this->Indent(writer, level, error);
        sprintf(cBuffer, "\"%s\" %f\r\n", Single->GetName(), Single->GetValue());
        writer->Write(cBuffer, (uint32_t) strlen(cBuffer), error);
    }
}

CVMTGroupNode *CVMTFile::GetRoot() const {
    return this->mRoot;
}

uint32_t CVMTFile::GetParseErrorLine() const {
    return this->mParseErrorLine;
}
