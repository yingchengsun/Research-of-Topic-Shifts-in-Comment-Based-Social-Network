// CatalogList.h: interface for the CCatalogList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CATALOGLIST_H__4CF04BF3_9DA2_11D3_8433_00C04F722981__INCLUDED_)
#define AFX_CATALOGLIST_H__4CF04BF3_9DA2_11D3_8433_00C04F722981__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wordlist.h"
#include <malloc.h>
#include "svm.h"

class CDocNode;
void AFXAPI SerializeElements(CArchive& ar,CDocNode* pElements,int nCount);

struct DocInfo{
	CDocNode *m_pDocNode;
	CString m_sDocName;
	CString m_sClsName;
	double m_dSimRatio;
};

//用来记录文档向量中每一维特征的权重
struct	sWeightNode
{
	int    s_idxWord;    //特征的ID
	short  s_tfi;        //特征在文档中出现的频次
	double s_dWeight;    //特征的权重
};

class CDocNode:public CObject
{
public:
	CDocNode();
	~CDocNode();
	const CDocNode& operator=(const CDocNode& x);
	static void AllocTempBuffer(int nLen);
	static void DeallocTempBuffer();
	void AllocBuffer(int nLen);
	void DeallocBuffer();
	void AllocResultsBuffer(short nLen);
	void DeallocResultsBuffer();
	//使用分词的方法形成文档的特征属性
	int ScanChinese(char *, CWordList&, int, short idxCata=-1);
	int ScanEnglish(char *, CWordList&, int, short idxCata=-1, bool bStem=false);
	static int ScanChineseString(char*, CWordList&, int, long, short idxCata=-1);
	static int ScanEnglishString(char*, CWordList&, int, long, short idxCata=-1, bool bStem=false);
	//根据词典wordList计算文档每一维的权重,形成文档的向量,将其保存到数组m_sWeightSet
	//这个方法要求词典wordList中每一个wordnode的m_dWeight的值都赋为此特征的反比文档频率
	int ScanChineseWithDict(char *,CWordList&);
	int ScanEnglishWithDict(char *,CWordList&, bool bStem=false);
	static int ScanChineseStringWithDict(char *pPath,CWordList& wordList);
	static int ScanEnglishStringWithDict(char *pPath, CWordList &wordList, bool bStem);
	int GenDocVector();
	int GenDocVector(DOC &doc);
	double ComputeSimilarityRatio();  //与存放在m_pTemp中的向量进行相似度的计算
	void Serialize(CArchive& ar);
	CDocNode(const CDocNode& x);
	int GetWordNum();
	BOOL IsZero();
public:
	double ComputeProbability(CWordList& wordList,int n);
	long m_idxDoc;                //文档标识
	CString	m_strDocName;         //文档名称
	int m_nAllocLen;              //文档向量的长度,即数组m_sWeightSet的长度
	sWeightNode	*m_sWeightSet;    //词权值列表
	short  m_nClassNum;           //代表训练文档中的类别总数,即数组m_pResults的大小
	double *m_pResults;           //文档与每个类别的相似度
	short  m_nCataID;             //代表当前文档的所属类别,由于只在分类时使用,所以序列化的时候不操作此属性
	static sWeightNode *m_pTemp;  //生成文档向量时需要使用的一块临时内存
	static int m_nAllocTempLen;   //临时内存的大小
private:
	static char m_pSentence[MAX_PATH*10];
private:
	bool IsNumber(char * p);
	static int ParseFile(char *, int, int &);
};
class CLogNode;
void AFXAPI SerializeElements(CArchive& ar,CLogNode* pElements,int nCount);
class CLogNode:public CObject
{
public:
	CLogNode();
	~CLogNode();
	const CLogNode& operator=(const CLogNode& x);
	CLogNode(const CLogNode& x);

	long m_idxLog;                //日志记录标识
private:
//private://访问时间\t用户ID\t[查询词]\t该URL在返回结果中的排名\t用户点击的顺序号\t用户点击的URL
//其中，用户ID是根据用户使用浏览器访问搜索引擎时的Cookie信息自动赋值，即同一次使用浏览器输入的不同查询对应同一个用户ID。
	CString time;
	double userID;
	CString text;
	int rank;
	double order;
	CString URL;
public:
	void setTime(CString time);
	CString getTime();
	void setUserID(double userID);
	double getUserID();
	void setText(CString text);
	CString getText();
	void setRank(int rank);
	int getRank();
	void setOrder(double order);
	double getOrder();
	void setURL(CString URL);
	CString getURL();
	bool operator <   (const CLogNode& l )   const; //升序排序时必须写的函数
	bool operator >   (const CLogNode& l )   const;//降序排序时必须写的函数
};

class CQueryNode;

class CLogFileNode:public CObject
{
public:
	int readLogFile();
	void InitLogFileNode(int nMode=0);///xby

	CLogFileNode();
	~CLogFileNode();
	CLogFileNode(const CLogFileNode& x);
	const CLogFileNode& operator = (const CLogFileNode& x);
	const CLogFileNode& operator += (const CLogFileNode& x);
	POSITION AddLogNode(CLogNode& lognode);///xby
	//POSITION AddCata(CLogNode& lognode);
	void ComputeN(CList<CQueryNode,CQueryNode&>& QueryList ,int CS_n,int RS_n,double k,bool url);

	UINT GetLogNum();
	CLogNode GetFirstNode();
	CLogNode GetNextNode();


public:
	CString	m_strLogFileName;         //日志文件名称
	short m_idxCata;
	CString m_strDirName;
	long	m_lTotalFileNum;
private:
	long    m_lCurLogID;
	CList<CLogNode,CLogNode&>           m_lstLogList;///xby 日志记录列表

};

class CQueryNode;


struct QueryInfo{
	CQueryNode *m_pQueryNode;
	//CString m_sLogName;
	CString m_sClsName;
	double m_dSimRatio;
};

//用来记录日志每条查询记录中每一维特征的权重
struct	sWeightQueryNode
{
	int    s_idxWord;    //特征的ID
	short  s_tfi;        //特征在查询中出现的频次
	double s_dWeight;    //特征的权重
};

class CQueryNode:public CObject
{
public:
	long m_idxQuery;                        //查询标识
	int m_nAllocLen;                        //日志中查询向量的长度,即数组m_sWeightSet的长度
	sWeightQueryNode *m_sWeightQuerySet;    //词权值列表
	short  m_nClassNum;                     //代表训练查询中的类别总数,即数组m_pResults的大小
	double *m_pResults;                     //查询与每个类别的相似度
	short  m_nCataID;                       //代表当前查询的所属类别,由于只在分类时使用,所以序列化的时候不操作此属性
	static sWeightQueryNode *m_pTemp;       //生成查询向量时需要使用的一块临时内存
	static int m_nAllocTempLen;             //临时内存的大小

	CString m_Text;
	int m_nTotalSessionNum;
	long float m_nbelow;         //计算nCS用
	long float m_nRank_below;    //计算nRS用

	long double nCS;
	long double nRS;
	long double Length;

	long double UKS_N;
	long double UKS_I;
	long double UKS_T;


	void setText(CString text);
	void Reset();

	CQueryNode();
	~CQueryNode();
	const CQueryNode& operator=(const CQueryNode& x);
	CQueryNode(const CQueryNode& x);
	static void AllocTempBuffer(int nLen);
	static void DeallocTempBuffer();
	void AllocBuffer(int nLen);
	void DeallocBuffer();
	void AllocResultsBuffer(short nLen);
	void DeallocResultsBuffer();
	void Serialize(CArchive& ar);
	//使用分词的方法形成文档的特征属性
	int ScanChineseText(CString, CWordList&, int, short idxCata=-1);
	int ScanEnglishText(CString, CWordList&, int, short idxCata=-1, bool bStem=false);
	static int ScanChineseString(char *, CWordList&, int, long, short idxCata=-1);
	static int ScanEnglishString(char *, CWordList&, int, long, short idxCata=-1, bool bStem=false);
	//根据词典wordList计算文档每一维的权重,形成文档的向量,将其保存到数组m_sWeightSet
	//这个方法要求词典wordList中每一个wordnode的m_dWeight的值都赋为此特征的反比文档频率
	int ScanChineseWithDict(CWordList&);
	static int ScanChineseStringWithDict(char *pPath,CWordList& wordList);
	int ScanEnglishWithDict(CWordList&, bool bStem=false);
	static int ScanEnglishStringWithDict(char *pPath, CWordList &wordList, bool bStem);
	int GenQryVector();
	/*int GenDocVector();
	int GenDocVector(DOC &doc);
	double ComputeSimilarityRatio();  //与存放在m_pTemp中的向量进行相似度的计算
	int GetWordNum();
	BOOL IsZero();
public:
	double ComputeProbability(CWordList& wordList,int n);*/
private:
	//bool IsNumber(char * p);
	static int ParseFile(char *, int, int &);
	static char m_pSentence[MAX_PATH*10];
};

class CCatalogNode;
class CCatalogList;
void AFXAPI SerializeElements(CArchive& ar,CCatalogNode* pElements,int nCount);

class CCatalogNode
{
public:
	CCatalogNode();
	~CCatalogNode();
	CCatalogNode(const CCatalogNode& x);
	const CCatalogNode& operator = (const CCatalogNode& x);
	const CCatalogNode& operator += (const CCatalogNode& x);
public:
	void InitCatalogNode(int nMode=0);
	void InitCatalogNode_QueryList(int nMode=0);////xby
	void InitCatalogNode_LogFileList(int nMode=0);///xby
	void SetStartDocID(long lDocID);
	void SetStartLogFileID(long lLogID);//xby

	void ScanLogtoQuery(double k,bool url);///xby 扫描得到查询信息


	CDocNode& GetNext(POSITION& rPos);
	CLogFileNode& GetNextLogFile(POSITION& rPos);///xby
	CQueryNode& GetNextQuery(POSITION& rPos);///xby

	POSITION GetFirstPosition();
	POSITION GetFirstLogFilePosition();///xby
	POSITION GetFirstQueryPosition();///xby

	POSITION AddDoc(CDocNode& docnode);
	POSITION AddQuery(CQueryNode& qrynode);///xby
	POSITION AddLogFile(CLogFileNode& logfilenode);//xby

	void Serialize(CArchive& ar);

	UINT GetDocNum();
	UINT GetQueryNum();
	UINT GetLogFileNum();

	//扫描路径pPath下的所有文档，将其添加到当前类节点中
	long ScanDirectory(CString);	
	long ScanLogDirectory(CString strPath,double k,bool url);//////xby

public:
	CDocNode& GetAt( POSITION position );
	short m_idxCata;
	CString	m_strCatalogName;
	CString m_strDirName;
	long	m_lTotalWordNum;
	long    m_lTotalQueryNum;
private:
	CList<CDocNode,CDocNode&>			m_lstDocList;
	CList<CQueryNode,CQueryNode&>       m_lstQueryList;///xby
	CList<CLogFileNode,CLogFileNode&>   m_lstLogFileList;///xby
	long    m_lCurDocID;
	long    m_lCurLogFileID;
	long    m_lCurQueryID;

};

class CCatalogList  
{
public:
	CCatalogList();
	virtual ~CCatalogList();
	const CCatalogList& operator = (const CCatalogList& x);
	const CCatalogList& operator += (const CCatalogList& x);
	void InitCatalogList(int nMode=0);

	void DumpToFile (CString strFileName, int nSaveMode=0);
	BOOL GetFromFile(CString strFileName);
	void DumpDocList(CString strFileName);
	void DumpQueryList(CString strFileName,int type[5]);
	long BuildLib(CString	strDirName);

	//xby
	long BuildLogLib(CString strLogFileName,double k,bool url);

public:
	static int GetSaveMode();
	void DumpCataList(CString strFileName);
	bool BuildCatalogID(CCatalogList &);
	CCatalogNode* GetCataByName(CString strCataName);
	short GetCataIDByName(CString strCataName);
	short GetCataIDArrayFromString(char * line, CArray<short, short> &aryCataID);
	CDocNode* GetDocByName(CString strDocName);
	CCatalogNode* GetCata(short idxCata);
	CCatalogNode GetAt(POSITION pos) const;
	CCatalogNode& GetAt(POSITION pos);
	CCatalogNode& GetNext(POSITION& rPos);
	POSITION GetFirstPosition();
	POSITION AddCata(CCatalogNode& catanode);
	bool GetDocName(short idxCata,long idxDoc,CString& strDocName);
	bool GetCataName(short idxCata,CString& strCataName);
	int  GetCataNum();
	long GetDocNum();
	long GetQueryNum();
private:
	void Serialize(CArchive& ar);
	long ScanDirectory(CString strDirName);
	long ScanDirectory_Log(CString strPath,double k,bool url);
private:
	static int m_nSaveMode;  //0 保存文档向量, 1 不保存文档向量
	CList<CCatalogNode,CCatalogNode&>	m_lstCatalogList;
};

#endif // !defined(AFX_CATALOGLIST_H__4CF04BF3_9DA2_11D3_8433_00C04F722981__INCLUDED_)
