// WordList.h: interface for the CWordList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORDLIST_H__D7A246C0_9CD0_11D3_BAA2_0000250394FE__INCLUDED_)
#define AFX_WORDLIST_H__D7A246C0_9CD0_11D3_BAA2_0000250394FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#define CataMax 200

class CWordNode;
void AFXAPI SerializeElements(CArchive& ar,CWordNode* pElements,int nCount);

class CWordNode
{
public:
	CWordNode();
	~CWordNode();
	CWordNode& operator = (const CWordNode& x);
	void AllocBuffer(int nLen);
	void DeallocBuffer();
	void Serialize(CArchive& ar);
public:
	void Copy(CWordNode& wordNode);
	void InitBuffer(int nLen);
	long GetWordNum();
	long GetDocNum();
	long GetCataWordNum(int cataID);
	long GetCataDocNum(int cataID);
	int  MaxWeightIndex();
	void ComputeWeight(long sum, bool bMult=false);
	//在特征选择的时候用来保存特征的类别区分度
	//在特征选择完成之后，它用来保存每个特征的"log(训练文档总数/特征的文档频率)"值
	double m_dWeight;
	short m_nAllocLen;
	int m_nWordID;           //特征的ID
	double *m_pCataWeight;   //特征对于每个类别的区分度
	double *m_pCataWeightPro;	 //特征属于每个类别的概率
	//特征在整个文档集中的文档频率,实际就是函数GetDocNum()返回的值
	//如果特征不是从训练文档集中选择得到的，就无法使用GetDocNum()得到特征的文档频率
	//所以，此处使用m_lDocFreq来记录特征的文档频率
	long m_lDocFreq;         //特征在整个文档集中的文档频率
	//特征在整个文档集中的词频,实际就是函数GetWordNum()返回的值
	//如果特征不是从训练文档集中选择得到的，就无法使用GetWordNum()得到特征的词频
	//所以，此处使用m_lWordFreq来记录特征的词频
	long m_lWordFreq;        //特征在整个文档集中的词频
	long *m_pCataDocFreq;    //特征在每一个类别中的文档频率
	long *m_pCataWordFreq;   //特征在每一个类别中的词频
	long m_lDocID;           //得到特征的文档频率的时候用到
};

AFX_INLINE UINT AFXAPI HashKey(CString key);
typedef CTypedPtrList<CPtrList,CWordNode*> CPWordList;

class CWordList  
{
public:
	CWordNode& Add(const CString str,  long docID);
	bool GetListFromFile(CString strFileName);
	CString GetWordByID(long wordID);
	CWordList();
	CWordList& operator =(CWordList &x);
	virtual ~CWordList();

	void Serialize(CArchive& ar);
	void DumpToFile(CString strFileName);
	void DumpWordList(CString strFileName);
	void DumpWordProList(CString strFileName,int CataNum);
	bool GetFromFile(CString strFileName);

	void InitWordList();
	//如果词表中不存在这个词,则加入词表
	CWordNode& Add(const CString);
	//如果词表中不存在这个词,则加入词表,并且根据docID决定文档频率是否加1
	CWordNode& Add(const CString, short, long, int);
	void SetAt(const CString,CWordNode&);
	//获得词表中词的总数
	int  GetCount();
	long GetWordNum();
	BOOL Lookup(CString str, CWordNode &wordNode);

	CWordNode& GetNext(POSITION &, CString &);
	POSITION GetFirstPosition(void);
	void IndexWord();
	//计算每个词的反比文档频率
	void ComputeWeight(long sum, bool bMult=false);
public :
	bool GetProFromFile(CString strFileName);
	CWordNode& GetWordProByID(POSITION& pos,int j);
	double GetWordProByID(POSITION &pos,long wordID,int classnum);
	CMap<CString,CString,CWordNode,CWordNode&> m_lstWordList;
};
#endif // !defined(AFX_WORDLIST_H__D7A246C0_9CD0_11D3_BAA2_0000250394FE__INCLUDED_)
