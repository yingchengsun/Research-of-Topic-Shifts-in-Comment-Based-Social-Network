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
public: //ѵ��ʱ��Ҫ�õ��Ĺ��г�Ա����
	//����bGenDic=false ��η���ʱʹ��, nType��������ģ�͵����
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
private: //ѵ��ʱ��Ҫ�õ���˽�г�Ա����
	void Sort(struct sSortType *,int);
	void QuickSort(struct sSortType *,int,int);
	void GenSortBuf(CWordList& wordList,sSortType *psSortBuf,int nCatalog);
	void GenModel();
	void GenQueryModel();
	void FeatherSelection(CWordList& dstWordList);
	void FeatherWeight(CWordList& wordList);
public: //����ʱ��Ҫ�õ��Ĺ��г�Ա����
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
private: //����ʱ��Ҫ�õ���˽�г�Ա����
	void ComputeWeight(bool bMult=false);
	void ComputeSimRatio(CDocNode &, int nCmpType=0);
	void Sort(DocWeight *,int);
	void QuickSort(DocWeight*, int, int);
private:  //����ʱ��Ҫ�õ���˽�г�Ա����
	//ָ��ѵ���ı�����ָ��,�����ӿ��ȡ�ٶ�
	DocCatalog *m_pDocs;
	//��ʱ���浱ǰ�����ĵ���ѵ���ĵ���ÿһƪ�ĵ������ƶ�
	DocWeight *m_pSimilarityRatio;
	//���浱ǰ�����ĵ���ÿ��ĸ���
	DocWeight *m_pProbability;
	//ѵ���ĵ��ĸ���
	long m_lDocNum;
	//ѵ���ĵ��������
	short m_nClassNum;

	//ָ��ѵ���ı�����ָ��,�����ӿ��ȡ�ٶ�
	QueryCatalog *m_pQrys;
	//��ʱ���浱ǰ�����ĵ���ѵ���ĵ���ÿһƪ�ĵ������ƶ�
	QueryWeight *m_pQuerySimilarityRatio;
	//���浱ǰ�����ĵ���ÿ��ĸ���
	QueryWeight *m_pQueryProbability;
	//ѵ���ĵ��ĸ���
	long m_lQueryNum;
	//ѵ���ĵ��������
	short m_nQueryClassNum;

public:
	void ComputePro(CDocNode &docNode);
	bool BAYESClassify(char *, CDocNode &, bool bFile=true);
	short BAYESCategory(char *pPath, CDocNode &, bool bFile=true);
	long BAYESClassify(CCatalogList& cataList);

	//����ģ���ļ�ͷ��ʶ��
	int n_Type;
	static const DWORD dwModelFileID;
	CClassifierParam  m_paramClassifier;
	CWordList         m_lstTrainWordList;
	//ѵ��ʱ��Ҫ�õ���,����������û�н�������ѡ��֮ǰѵ���������е�����
	CWordList         m_lstWordList;
	CCatalogList      m_lstTrainCatalogList;
	CCatalogList      m_lstTestCatalogList;
	CSVM              m_theSVM;
	CBAYES			  m_theBAYES;
};

extern CClassifier theClassifier;
#endif // !defined(AFX_CLASSIFIER_H__FA4DB8D8_AC36_44A8_884B_0D715575B7A1__INCLUDED_)
