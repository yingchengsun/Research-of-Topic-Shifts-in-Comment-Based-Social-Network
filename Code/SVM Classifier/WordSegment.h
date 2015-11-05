// WordSegment.h: interface for the CWordSeg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORDSEGMENT_H__18EFCCE0_4780_45CA_962A_8128DD2FC299__INCLUDED_)
#define AFX_WORDSEGMENT_H__18EFCCE0_4780_45CA_962A_8128DD2FC299__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include ".\\Utility\\Dictionary.h"
#include ".\\Segment\\Segment.h"
#include ".\\Tag\\Span.h"
#include ".\\Unknown\\UnknowWord.h"
#include <afxtempl.h>
#include "UTILITY\Dictionary.h"	// Added by ClassView

AFX_INLINE UINT AFXAPI HashKey(CString key);

class CWordSegment  
{
public:
	CWordSegment();
	virtual ~CWordSegment();
	bool InitWorgSegment(char *, int nLanguage=0);
	void FreeWordSegment();
	void SetSegSetting(unsigned short);
	unsigned short GetSegSetting();
	bool Segment(char*);
	bool isInStopWords(char*);
public:
	int SegmentFile(const char *source, const char * target, BOOL bStopWord=true, BOOL bTag=false);
	PWORD_RESULT GetWordSeg(int index);
	int GetSegmentCount();
	static const unsigned short uTag;  // word classify tag
	static const unsigned short uPlace; // Chinese place name
	static const unsigned short uPerson;  // chinese name
	static const unsigned short uTransPerson; // translation name, include translation
private:
	static int ParseFile(char *pBuffer, int nStart, int &nEnd);
private:
	//m_nInit>0 初始化了; m_nInit=1 分词初始化; m_nInit=2 英文初始化
	int m_nInit;
	CMap<CString,CString,int,int> m_lstStopWords;
	unsigned short m_nOperateType;
	double m_dSmoothingPara;
	char m_pNewSentence[MAX_PATH*10];
	static char m_pSentence[MAX_PATH*10];
	CSegment m_Seg;//Seg class
	CDictionary m_dictCore,m_dictBigram;//Core dictionary,bigram dictionary
	CSpan m_POSTagger;//POS tagger
	CUnknowWord m_uPerson,m_uTransPerson,m_uPlace;//Person recognition
};
extern CWordSegment g_wordSeg;
#endif // !defined(AFX_WORDSEGMENT_H__18EFCCE0_4780_45CA_962A_8128DD2FC299__INCLUDED_)
