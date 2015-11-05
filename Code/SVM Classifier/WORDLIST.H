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
	//������ѡ���ʱ����������������������ֶ�
	//������ѡ�����֮������������ÿ��������"log(ѵ���ĵ�����/�������ĵ�Ƶ��)"ֵ
	double m_dWeight;
	short m_nAllocLen;
	int m_nWordID;           //������ID
	double *m_pCataWeight;   //��������ÿ���������ֶ�
	double *m_pCataWeightPro;	 //��������ÿ�����ĸ���
	//�����������ĵ����е��ĵ�Ƶ��,ʵ�ʾ��Ǻ���GetDocNum()���ص�ֵ
	//����������Ǵ�ѵ���ĵ�����ѡ��õ��ģ����޷�ʹ��GetDocNum()�õ��������ĵ�Ƶ��
	//���ԣ��˴�ʹ��m_lDocFreq����¼�������ĵ�Ƶ��
	long m_lDocFreq;         //�����������ĵ����е��ĵ�Ƶ��
	//�����������ĵ����еĴ�Ƶ,ʵ�ʾ��Ǻ���GetWordNum()���ص�ֵ
	//����������Ǵ�ѵ���ĵ�����ѡ��õ��ģ����޷�ʹ��GetWordNum()�õ������Ĵ�Ƶ
	//���ԣ��˴�ʹ��m_lWordFreq����¼�����Ĵ�Ƶ
	long m_lWordFreq;        //�����������ĵ����еĴ�Ƶ
	long *m_pCataDocFreq;    //������ÿһ������е��ĵ�Ƶ��
	long *m_pCataWordFreq;   //������ÿһ������еĴ�Ƶ
	long m_lDocID;           //�õ��������ĵ�Ƶ�ʵ�ʱ���õ�
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
	//����ʱ��в����������,�����ʱ�
	CWordNode& Add(const CString);
	//����ʱ��в����������,�����ʱ�,���Ҹ���docID�����ĵ�Ƶ���Ƿ��1
	CWordNode& Add(const CString, short, long, int);
	void SetAt(const CString,CWordNode&);
	//��ôʱ��дʵ�����
	int  GetCount();
	long GetWordNum();
	BOOL Lookup(CString str, CWordNode &wordNode);

	CWordNode& GetNext(POSITION &, CString &);
	POSITION GetFirstPosition(void);
	void IndexWord();
	//����ÿ���ʵķ����ĵ�Ƶ��
	void ComputeWeight(long sum, bool bMult=false);
public :
	bool GetProFromFile(CString strFileName);
	CWordNode& GetWordProByID(POSITION& pos,int j);
	double GetWordProByID(POSITION &pos,long wordID,int classnum);
	CMap<CString,CString,CWordNode,CWordNode&> m_lstWordList;
};
#endif // !defined(AFX_WORDLIST_H__D7A246C0_9CD0_11D3_BAA2_0000250394FE__INCLUDED_)