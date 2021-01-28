// FileParser.cpp: implementation of the CFileParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileParser::CFileParser()
{
	m_pBuffer = NULL;
	m_pBufferWord[0] = '\0';
	m_pBufferLine[0] = '\0';
	m_IndexBuffer = 0;
}

CFileParser::~CFileParser()
{
	Free();
}

void CFileParser::Free(void)
{
	if(m_pBuffer != NULL)
		delete [] m_pBuffer;
	m_IndexBuffer = 0;
}

int CFileParser::Run(const char *filename)
{
	return 1;
}

//********************************************
// ReadFile
//********************************************
//##ModelId=41679FDC039B
int CFileParser::ReadFile(const char *filename)
{
	m_FileName = filename;

	CFile file;
	CFileException ex;

	// Opening
	TRACE("  opening...");
	if(!file.Open(filename, CFile::modeRead | CFile::typeBinary,&ex))
	{
		#ifdef _DEBUG
		  afxDump << "File could not be opened " << ex.m_cause << "\n";
		#endif
		TRACE("unable to open file for reading\n");
		return 0;
	}
	TRACE("ok\n");

	// Size file
	m_SizeFile = file.GetLength();
	TRACE("  length : %d bytes\n",m_SizeFile);

	// Alloc
	TRACE("  alloc...");
	m_pBuffer = new char[m_SizeFile];
	if(m_pBuffer == NULL)
	{
		TRACE("insuffisant memory\n");
		return 0;
	}
	TRACE("ok\n");

	// Reading
	TRACE("  reading...");
	TRY
	{
		file.Read(m_pBuffer,m_SizeFile);
	}
	CATCH(CFileException, e)
	{
		#ifdef _DEBUG
				afxDump << "Error during reading " << e->m_cause << "\n";
		#endif
		TRACE("error during reading\n");
		file.Close();
		return 0;
	}
	END_CATCH
	TRACE("ok\n");

	// Closing
	file.Close();

	return 1;
}
//********************************************
// ReadLine
// eol : '\n'
// eos : '\0'
//********************************************
int CFileParser::ReadLine()
{
	m_pBufferLine[0] = '\0';
	int i=0;
	do
		m_pBufferLine[i++] = m_pBuffer[m_IndexBuffer++];
	while(m_pBuffer[m_IndexBuffer-1] != '\n' && 
		    i < MAX_LINE_VRML &&
				m_IndexBuffer < m_SizeFile);

	m_pBufferLine[i-1] = '\0';

	return 1;
}

int CFileParser::NextLine()
{
	int i=0;
	int tmpIndex;

	//寻找最近的'\n'，从当前开始，包括当前字符
	while(m_IndexBuffer<m_SizeFile&&m_pBuffer[m_IndexBuffer++]!='\n')
		;
	if(m_IndexBuffer>=m_SizeFile){
		m_IndexBuffer = m_SizeFile-1;
		return 0;
	}
	
	//进入下一行
	tmpIndex = m_IndexBuffer;
	m_pBufferLine[0] = '\0';
	
	do
		m_pBufferLine[i++] = m_pBuffer[m_IndexBuffer++];
	while(m_pBuffer[m_IndexBuffer-1] != '\n' && 
		    i < MAX_LINE_VRML &&
				m_IndexBuffer < m_SizeFile);
	m_pBufferLine[i-1] = '\0';
	m_IndexBuffer = tmpIndex;
	
	return 1;
}

int CFileParser::ReadWord()
{
	m_pBufferWord[0] = '\0';
	int i=0;

	// Jump to next valid character
	while((m_pBuffer[m_IndexBuffer] == '\n' || 
		     m_pBuffer[m_IndexBuffer] == '\t' || 
		     m_pBuffer[m_IndexBuffer] == '\r' || 
		     m_pBuffer[m_IndexBuffer] == ' ') &&
				 m_IndexBuffer < m_SizeFile)
		m_IndexBuffer++;

	// Check eof
	if(m_IndexBuffer >= m_SizeFile)
		return 0;

	do
		m_pBufferWord[i++] = m_pBuffer[m_IndexBuffer++];
	while(m_pBuffer[m_IndexBuffer-1] != '\n' && 
		    m_pBuffer[m_IndexBuffer-1] != '\t' && 
		    m_pBuffer[m_IndexBuffer-1] != '\r' && 
		    m_pBuffer[m_IndexBuffer-1] != ' ' && 
		    i < MAX_WORD_VRML &&
				m_IndexBuffer < m_SizeFile);

	m_pBufferWord[i-1] = '\0';

	return 1;
}

//********************************************
// OffsetToString
//********************************************
//##ModelId=41679FDC03C8
//寻找字符串：找到，m_IndexBuffer指向第一个字符所在位置，返回1；没找到，返回0
int CFileParser::OffsetToString(const char *string)
{
	while(m_IndexBuffer < m_SizeFile)
	{
		ReadLine();
		char *adr = strstr(m_pBufferLine,string);
		if(strstr(m_pBufferLine,string) != NULL)
		{
			m_IndexBuffer = m_IndexBuffer - strlen(m_pBufferLine) - 1 + (adr-m_pBufferLine);
			ASSERT(m_IndexBuffer >= 0);
			return 1;
		}
	}
	return 0;
}

int CFileParser::OffsetToString(const char* string,int start,int end)
{
	m_IndexBuffer = start;
	while(m_IndexBuffer < m_SizeFile&&m_IndexBuffer<=end)
	{
		ReadLine();
		char *adr = strstr(m_pBufferLine,string);
		if(strstr(m_pBufferLine,string) != NULL)
		{
			m_IndexBuffer = m_IndexBuffer - strlen(m_pBufferLine) - 1 + (adr-m_pBufferLine);
			ASSERT(m_IndexBuffer >= 0);
			return 1;
		}
	}
	return 0;
}
//int CFileParser::OffsetToStringBegin(char *string)
//{
//	while(m_IndexBuffer < m_SizeFile)
//	{
//		ReadLine();
//		char *adr = strstr(m_pBufferLine,string);
//		if(strstr(m_pBufferLine,string) != NULL)
//		{
//			m_IndexBuffer = m_IndexBuffer - strlen(m_pBufferLine) - 1 + (adr-m_pBufferLine);
//			ASSERT(m_IndexBuffer >= 0);
//			return 1;
//		}
//	}
//	return 0;
//}