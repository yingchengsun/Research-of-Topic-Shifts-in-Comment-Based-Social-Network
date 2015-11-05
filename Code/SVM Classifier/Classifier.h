// Classifier.h: interface for the CClassifier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLASSIFIER_H__FA4DB8D8_AC36_44A8_884B_0D715575B7A1__INCLUDED_)
#define AFX_CLASSIFIER_H__FA4DB8D8_AC36_44A8_884B_0D715575B7A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CatalogList.h"
#include "ClassifierParam.h"
#include "WordList.h"
#include "Compute_Param.h"
#include "Compute_Prompt.h"
#include "Compute_Result.h"
#include "svm.h"
#include "BAYES.h"

struct sSortType
{
	char        word[40];
	double		dWeight;
	CWordNode	*pclsWordNode;
};

struct DocWeight
{
	double dWeight;
	long   lDocID;
};

struct DocCatalog
{
	CDocNode * pDocNode;
	short      nCataID;
};

struct QueryCatalog
{
	CQueryNode * pQueryNode;
	short        nCataID;
};

struct QueryWeight
{
	double qWeight;
	long lQueryID;
};

class CClassifier 
{
public:
	CClassifier();
	virtual ~CClassifier();
public: //训练时需要用到的公有成员方法
	//参数bGenDic=false 层次分类时使用, nType决定分类模型的类别
	bool Train(int nType=0,bool bFlag=true);
	void TrainSVM();
	void TrainBAYES();
	bool GenDic();
	///xby
	bool GenLogDic();
	void InitTrain();
	bool WriteModel(CString strFileName, int nType=0);
	void Evaluate(CString strPath);
	void CopyFile(char *pFileName, char *pSource, char *pTarget, char *pCatalog);
	long SaveResults(CCatalogList &cataList, CString strFileName, CStringArray *aryType=NULL);
	long SaveResults_Query(CCatalogList &cataList, CString strFileName, CStringArray *aryType=NULL);
private: //训练时需要用到的私有成员方法
	void Sort(struct sSortType *,int);
	void QuickSort(struct sSortType *,int,int);
	void GenSortBuf(CWordList& wordList,sSortType *psSortBuf,int nCatalog);
	void GenModel();
	void GenQueryModel();
	void FeatherSelection(CWordList& dstWordList);
	void FeatherWeight(CWordList& wordList);
public: //分类时需要用到的公有成员方法
	bool OpenModel(CString strFileName);
	void Prepare();
	void Prepare_Query();
	bool Classify();
	bool Classify_Query();
	bool ClassifySmart();
	bool KNNClassify(char *, CDocNode &, bool bFile=true, int nCmpType=0);
	long KNNClassify(CCatalogList&,int nCmpType=0);
	short KNNCategory(char *,CDocNode &,bool bFile=true, int nCmpType=0);
	short KNNCategory(char *pPath, bool bFile=true, int nCmpType=0);
	long SVMClassify(CCatalogList& cataList);
	long SVMClassify_Query(CCatalogList& cataList);
	bool SVMClassify(char *pPath, CDocNode &docNode, bool bFile=true);
	short SVMCategory(char *pPath, CDocNode &docNode, bool bFile=true);
	short SVMCategory(char *file, bool bFile=true);
	void SVMClassifyVectorFile(CString strFileName);
	void SVMClassifyVectorFile_Query(CString strFileName);
	short GetCategory(char *file, bool bFile=true);
	short GetCategory(char *path, CDocNode &docNode, bool bFile=true);
	bool Classify(char *path, CDocNode &docNode, bool bFile=true);
	long Classify(CCatalogList& cataList);
	long Classify_Query(CCatalogList& cataList);
	short SingleCategory(CDocNode &docNode);
	short SingleCategory_Query(CQueryNode &qryNode);
	bool MultiCategory(CDocNode &docNode, CArray<short,short>& aryResult, double dThreshold);
	bool MultiCategory_Query(CQueryNode &qryNode, CArray<short,short>& aryResult, double dThreshold);
private: //分类时需要用到的私有成员方法
	void ComputeWeight(bool bMult=false);
	void ComputeSimRatio(CDocNode &, int nCmpType=0);
	void Sort(DocWeight *,int);
	void QuickSort(DocWeight*, int, int);
private:  //分类时需要用到的私有成员变量
	//指向训练文本集的指针,用来加快读取速度
	DocCatalog *m_pDocs;
	//暂时保存当前测试文档和训练文档中每一篇文档的相似度
	DocWeight *m_pSimilarityRatio;
	//保存当前测试文档与每类的概率
	DocWeight *m_pProbability;
	//训练文档的个数
	long m_lDocNum;
	//训练文档的类别数
	short m_nClassNum;

	//指向训练文本集的指针,用来加快读取速度
	QueryCatalog *m_pQrys;
	//暂时保存当前测试文档和训练文档中每一篇文档的相似度
	QueryWeight *m_pQuerySimilarityRatio;
	//保存当前测试文档与每类的概率
	QueryWeight *m_pQueryProbability;
	//训练文档的个数
	long m_lQueryNum;
	//训练文档的类别数
	short m_nQueryClassNum;

public:
	void ComputePro(CDocNode &docNode);
	bool BAYESClassify(char *, CDocNode &, bool bFile=true);
	short BAYESCategory(char *pPath, CDocNode &, bool bFile=true);
	long BAYESClassify(CCatalogList& cataList);

	//分类模型文件头标识符
	int n_Type;
	static const DWORD dwModelFileID;
	CClassifierParam  m_paramClassifier;
	CWordList         m_lstTrainWordList;
	//训练时需要用到的,用来保存在没有进行特征选择之前训练集中所有的特征
	CWordList         m_lstWordList;
	CCatalogList      m_lstTrainCatalogList;
	CCatalogList      m_lstTestCatalogList;
	CSVM              m_theSVM;
	CBAYES			  m_theBAYES;
};

extern CClassifier theClassifier;
#endif // !defined(AFX_CLASSIFIER_H__FA4DB8D8_AC36_44A8_884B_0D715575B7A1__INCLUDED_)
