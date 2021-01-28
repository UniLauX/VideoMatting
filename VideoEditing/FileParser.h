// FileParser.h: interface for the CFileParser class.
//
//////////////////////////////////////////////////////////////////////

#include <atlstr.h>

#if !defined(AFX_FILEPARSER_H__102D9673_F061_4B5D_8DE1_838D7DC65540__INCLUDED_)
#define AFX_FILEPARSER_H__102D9673_F061_4B5D_8DE1_838D7DC65540__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_LINE_VRML 10000
#define MAX_WORD_VRML 1000

class CFileParser  
{
public:
	CFileParser();
	virtual ~CFileParser();

protected :
	int m_SizeFile;

	char *m_pBuffer;

	int m_IndexBuffer;

	CString m_FileName;

	char m_pBufferLine[MAX_LINE_VRML];

	char m_pBufferWord[MAX_WORD_VRML];

public:
	void Free(void);

	virtual int Run(const char *filename);

	virtual int ReadFile(const char *filename);

	int ReadLine();

	int NextLine();

	int ReadWord();

	//寻找最近的匹配字符串，从当前位置开始，包括当前字符
	int OffsetToString(const char *string);
	int OffsetToString(const char* string,int start,int end);
};

#endif // !defined(AFX_FILEPARSER_H__102D9673_F061_4B5D_8DE1_838D7DC65540__INCLUDED_)
