// Classifier.cpp: implementation of the CClassifier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextClassify.h"
#include "Classifier.h"
#include "WordSegment.h"
#include "Message.h"
#include <math.h>
#include <direct.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int  Choose_Way[4];    //0,nCS,nRS;1,UKS;2,Classifier;3,al 4 Length

CClassifier theClassifier;
const DWORD CClassifier::dwModelFileID=0xFFEFFFFF;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClassifier::CClassifier()
{
	n_Type=-1;
	m_pDocs=NULL;
	m_pSimilarityRatio=NULL;
	m_pProbability=NULL;
	m_lDocNum=0;
	m_nClassNum=0;
}

CClassifier::~CClassifier()
{

}

//����bGenDic=false������������ɨ���ĵ��õ�ѵ���ĵ�������������,һ���ڲ�η���ʱʹ��
//����nType������������ģ�͵����,nType=0����KNN������,nType=1����SVM������
bool CClassifier::Train(int nType, bool bFlag)
{
	this->n_Type=nType;
	CTime startTime;
	CTimeSpan totalTime;
	if(bFlag)
	{
		InitTrain();
		//�������к�ѡ��������䱣����m_lstWordList��
		//GenDic();
		GenLogDic();
	}
	CMessage::PrintStatusInfo("");

	if(m_lstWordList.GetCount()==0)
		return false;
	if(m_lstTrainCatalogList.GetCataNum()==0)
		return false;

	//����������б�m_lstTrainWordList
	m_lstTrainWordList.InitWordList();
	//Ϊ�������б�m_lstWordList�е�ÿ��������Ȩ
	CMessage::PrintInfo(_T("��ʼ�����ѡ��������ÿ��������������ֶȣ����Ժ�..."));
	startTime=CTime::GetCurrentTime();
	FeatherWeight(m_lstWordList);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�������ֶȼ����������ʱ")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

	//���������б�m_lstWordList��ѡ����������
	CMessage::PrintInfo(_T("��ʼ��������ѡ�����Ժ�..."));
	startTime=CTime::GetCurrentTime();
	FeatherSelection(m_lstTrainWordList);
    //Ϊ����������m_lstTrainWordList�е�ÿ����������һ��ID
	m_lstTrainWordList.IndexWord();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("����ѡ���������ʱ")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

//	���m_lstWordList,�ͷ���ռ�õĿռ�
	m_lstWordList.InitWordList();

	CMessage::PrintInfo("��ʼ�����ĵ����������Ժ�...");
	startTime=CTime::GetCurrentTime();
	//GenModel();//////////////////////////////////////////////////////////////////////// ����ѵ���ĵ�����������
	GenQueryModel();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�ĵ��������ɽ�������ʱ")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("O(��_��)O~");

	CMessage::PrintInfo("��ʼ�������ģ�ͣ����Ժ�...");
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo("ѡ��ѵ����ʽ...");
	WriteModel(m_paramClassifier.m_txtResultDir+"\\model.prj",nType);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�������ģ�ͽ�������ʱ")+totalTime.Format("%H:%M:%S"));

	//ѵ��SVM�����������ڱ���ѵ���ĵ����ĵ����������
	if(nType == 1)
	{
		CMessage::PrintInfo("��ʼѵ��SVM�����Ժ�...");
		m_lstTrainCatalogList.InitCatalogList(2); //ɾ���ĵ�������ռ�õĿռ�
		startTime=CTime::GetCurrentTime();
		TrainSVM();
		totalTime=CTime::GetCurrentTime()-startTime;
		CMessage::PrintInfo(_T("SVM������ѵ����������ʱ")+totalTime.Format("%H:%M:%S"));
		CMessage::PrintStatusInfo("");
	}
	//Ϊ��������׼��,�����ܽ��з���
	//Prepare();
	Prepare_Query();
	CMessage::PrintStatusInfo("");
	return TRUE;
}

void CClassifier::TrainSVM()
{
	CString str;
	CTime tmStart;
	CTimeSpan tmSpan;

	m_paramClassifier.m_strModelFile="model";
	for(int i=1;i<=m_lstTrainCatalogList.GetCataNum();i++)
	{
		tmStart=CTime::GetCurrentTime();
		str.Format("����ѵ����%d��SVM�����������Ժ�...",i);
		CMessage::PrintInfo(str);
		m_theSVM.com_pro.Setup();
		m_theSVM.com_param.trainfile=m_paramClassifier.m_txtResultDir+"\\train.txt";
		m_theSVM.com_param.modelfile.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.svm_learn_main(i);
		tmSpan=CTime::GetCurrentTime()-tmStart;
		str.Format("��%d��SVM������ѵ����ɣ���ʱ%s!",i,tmSpan.Format("%H:%M:%S"));
		CMessage::PrintInfo(str);
	}	
}

void CClassifier::TrainBAYES()
{
	/*
	CString str;
	CTime tmStart;
	CTimeSpan tmSpan;

	m_paramClassifier.m_strModelFile="model";
	for(int i=1;i<=m_lstTrainCatalogList.GetCataNum();i++)
	{
		tmStart=CTime::GetCurrentTime();
		str.Format("����ѵ����%d��SVM�����������Ժ�...",i);
		CMessage::PrintInfo(str);
		m_theSVM.com_param.trainfile=m_paramClassifier.m_txtResultDir+"\\train.txt";
		m_theSVM.com_param.modelfile.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.svm_learn_main(i);
		tmSpan=CTime::GetCurrentTime()-tmStart;
		str.Format("��%d��SVM������ѵ����ɣ���ʱ%s!",i,tmSpan.Format("%H:%M:%S"));
		CMessage::PrintInfo(str);
	}
	*/
}

// fill an array of CTrain::sSortType (train word length)
// nCatalog mean the value of element of the array is the weight
// of nCatalog(as an index of catalog) for each individual word
// if nCatalog==-1 then sum weight for all catalog
void CClassifier::GenSortBuf(CWordList& wordList,sSortType *psSortBuf,int nCatalog)
{
	int nTotalCata=m_lstTrainCatalogList.GetCataNum();
	int i;
	ASSERT(nCatalog<nTotalCata);
	long		lWordCount=0 ;
	POSITION pos_word = wordList.GetFirstPosition();
	CString str;
	while(pos_word!= NULL)  // for each word
	{
		CWordNode& wordnode = wordList.GetNext(pos_word,str);
		psSortBuf[lWordCount].pclsWordNode = &wordnode; 
		strcpy(psSortBuf[lWordCount].word,str);
		ASSERT(wordnode.m_nAllocLen==nTotalCata);
		if(nCatalog==-1)
		{
			psSortBuf[lWordCount].dWeight+=wordnode.m_dWeight;
		}
		else
			psSortBuf[lWordCount].dWeight=wordnode.m_pCataWeight[nCatalog];
		
		//������������ĸ���
		if(this->n_Type==2)
			for(i=0;i<nTotalCata;i++)
			{
				psSortBuf[lWordCount].pclsWordNode->m_pCataWeightPro[i]=wordnode.m_pCataWeightPro[i];
	//			CString strtemp;
	//			strtemp.Format("123 %f",psSortBuf[lWordCount].pclsWordNode->m_pCataWeightPro[i]);
	//			CMessage::PrintInfo(strtemp);
			}
			lWordCount++;
	}	
}


//��m_lstWordListѡ�����������Ӽ�,�浽dstWordList��
void CClassifier::FeatherSelection(CWordList& dstWordList)
{
	if(m_lstWordList.GetCount()<=0) return;
	dstWordList.InitWordList();
	m_lstWordList.IndexWord();

	sSortType	*psSortBuf;
	int			nDistinctWordNum = m_lstWordList.GetCount();
	psSortBuf = new sSortType[nDistinctWordNum ];  // the distinct number of the word 
	ASSERT(psSortBuf!=NULL);
	long lDocNum=m_lstTrainCatalogList.GetQueryNum();///////////
	//long lDocNum=m_lstTrainCatalogList.GetDocNum();
	for(int i=0;i<nDistinctWordNum ;i++)
	{
		psSortBuf[i].pclsWordNode = NULL;
		psSortBuf[i].dWeight	  = 0;
	}

	// catalog indivial selecting
	if(m_paramClassifier.m_nSelMode==CClassifierParam::nFSM_IndividualModel)
	{
		int nCatalogWordSize=m_paramClassifier.m_nWordSize;
		int nTotalCata=m_lstTrainCatalogList.GetCataNum();
		for(int i=0;i<nTotalCata;i++)
		{
			GenSortBuf(m_lstWordList,psSortBuf,i);//-1 mean sum all catalog
			Sort(psSortBuf,nDistinctWordNum-1);
			int nSelectWordNum=0;
			for(int j=0;j<nDistinctWordNum&&nSelectWordNum<nCatalogWordSize;j++)
			{
				CWordNode wordNode;
				if(m_paramClassifier.m_nWeightMode==CClassifierParam::nWM_TF_IDF)
					psSortBuf[i].pclsWordNode->ComputeWeight(lDocNum);
				else if(m_paramClassifier.m_nWeightMode==CClassifierParam::nWM_TF_IDF_DIFF)
					psSortBuf[i].pclsWordNode->ComputeWeight(lDocNum,true);
				else if(m_paramClassifier.m_nWeightMode==CClassifierParam::nWM_nCS_nRS)
					psSortBuf[i].pclsWordNode->ComputeWeight(lDocNum,true);///xby

				wordNode.m_dWeight=psSortBuf[j].pclsWordNode->m_dWeight;
				wordNode.m_lDocFreq=psSortBuf[j].pclsWordNode->m_lDocFreq;
				wordNode.m_lWordFreq=psSortBuf[j].pclsWordNode->m_lWordFreq;

				dstWordList.SetAt(psSortBuf[j].word,wordNode);
				nSelectWordNum++;
			}
		}
	}
	// total selecting
	else //if(m_paramClassifier.m_nSelMode==CClassifierParam::nFSM_GolbalMode)
	{
		int iWord=0;
		GenSortBuf(m_lstWordList,psSortBuf,-1);//-1 mean sum all catalog
		Sort(psSortBuf,nDistinctWordNum-1);
		
		int nSelectWordNum=m_paramClassifier.m_nWordSize;
		if (nSelectWordNum>nDistinctWordNum)
			nSelectWordNum=nDistinctWordNum;

		for(i=0;i<nSelectWordNum;i++)
		{
			CWordNode wordNode;
			wordNode.InitBuffer(m_lstTrainCatalogList.GetCataNum());
			if(m_paramClassifier.m_nWeightMode==CClassifierParam::nWM_TF_IDF)
				psSortBuf[i].pclsWordNode->ComputeWeight(lDocNum);
			else if(m_paramClassifier.m_nWeightMode==CClassifierParam::nWM_TF_IDF_DIFF)
				psSortBuf[i].pclsWordNode->ComputeWeight(lDocNum,true);
			wordNode.m_dWeight=psSortBuf[i].pclsWordNode->m_dWeight;
			wordNode.m_lDocFreq=psSortBuf[i].pclsWordNode->m_lDocFreq;
			wordNode.m_lWordFreq=psSortBuf[i].pclsWordNode->m_lWordFreq;

			if(this->n_Type==2)
	//			�������ڲ�ͬ���еĸ���
				for(int k=0;k<m_lstTrainCatalogList.GetCataNum();k++)
				{
					double abc = psSortBuf[i].pclsWordNode->m_pCataWeightPro[k];
					wordNode.m_pCataWeightPro[k] = psSortBuf[i].pclsWordNode->m_pCataWeightPro[k];
	//				CString strtemp;
	//				strtemp.Format("123 %f",wordNode.m_pCataWeightPro[k]);
	//				CMessage::PrintInfo(strtemp);
				}

			dstWordList.SetAt(psSortBuf[i].word,wordNode);
		}	
	}
	delete [] psSortBuf;
}

void CClassifier::FeatherWeight(CWordList& wordList)
{
// ------------------------------------------------------------------------------
//  based on document number model
	int			N;		//�ܵ��ĵ���;
	int			N_c;	//C����ĵ���
	int			N_ft;	//����ft���ĵ���
	int			N_c_ft;	//C���к���ft���ĵ���
// ------------------------------------------------------------------------------
//  based on word number model
	long		N_W;    //�ܵĴ���					m_lWordNum;
	long		N_W_C;  //C�����					CCatalogNode.m_lTotalWordNum;
	long		N_W_f_t; //f_t���ֵ��ܴ���	
	long		N_W_C_f_t;//C����f_t���ֵĴ���
// ------------------------------------------------------------------------------
	double		P_c_ft,P_c_n_ft,P_n_c_ft,P_n_c_n_ft;
	double		P_c,P_n_c;
	double		P_ft,P_n_ft;

// ------------------------------------------------------------------------------
	POSITION	pos_cata,pos_word;
	CString     strWord;

	// calculate the weight of each word to all catalog
	//N = m_lstTrainCatalogList.GetDocNum();
	N = m_lstTrainCatalogList.GetQueryNum();
	N_W = wordList.GetWordNum();

	int nTotalCata=m_lstTrainCatalogList.GetCataNum();
	pos_word = wordList.GetFirstPosition();
	while(pos_word!= NULL)  // for each word
	{
		CWordNode& wordnode = wordList.GetNext(pos_word,strWord);
		
		wordnode.m_dWeight=0;

		ASSERT(wordnode.m_pCataWeight);
		ASSERT(wordnode.m_pCataWeightPro);

		CMessage::PrintStatusInfo("����:"+strWord+"..."); 

		N_ft = wordnode.GetDocNum();  
		N_W_f_t = wordnode.GetWordNum();
		int nCataCount=0;
		pos_cata = m_lstTrainCatalogList.GetFirstPosition();
		
		while(pos_cata!=NULL)  // for each catalog 
		{
			CCatalogNode& catanode = m_lstTrainCatalogList.GetNext(pos_cata);
			//N_c  = catanode.GetDocNum();
			N_c= catanode.GetQueryNum();
			N_W_C  = catanode.m_lTotalWordNum;			
			N_c_ft = wordnode.GetCataDocNum(catanode.m_idxCata);
			N_W_C_f_t =wordnode.GetCataWordNum(catanode.m_idxCata);
			// calculation model 
			if(m_paramClassifier.m_nOpMode==CClassifierParam::nOpWordMode)//��Ƶͳ��ģʽ   
			{
				P_c	    = 1.0 * N_W_C /N_W;      //C��ft���ֵĸ���
				P_ft	= 1.0 * N_W_f_t/N_W;     //ft���ֵĸ���
				P_c_ft  = 1.0 * N_W_C_f_t/N_W;   //C����ft���ֵĸ���
			}
			else //if(m_paramClassifier.m_nOpMode==CClassifierParam::nOpDocMode)//�ĵ�Ƶ��ͳ��ģʽ
			{
				P_c			= 1.0 * N_c /N;		//C����ֵĸ���
				P_ft		= 1.0 * N_ft/N;		//����ft���ĵ����ֵĸ���
				P_c_ft		= 1.0 * N_c_ft/N;	//C���к���ft���ĵ��ĸ���
			}
			
			P_n_c		= 1 - P_c;
			P_n_ft		= 1 - P_ft;
			P_n_c_ft	= P_ft - P_c_ft;
			P_c_n_ft	= P_c - P_c_ft;
			P_n_c_n_ft  = P_n_ft - P_c_n_ft;

			wordnode.m_pCataWeight[nCataCount]=0;
			// feature selection model
			if(m_paramClassifier.m_nFSMode==CClassifierParam::nFS_XXMode)
			{
				// Right half of IG
				if ( (fabs(P_c * P_n_ft) > dZero) && ( fabs(P_c_n_ft) > dZero) ) 
				{
					wordnode.m_pCataWeight[nCataCount]+=P_c_n_ft * log( P_c_n_ft/(P_c * P_n_ft) );
				}
			}
			else if(m_paramClassifier.m_nFSMode==CClassifierParam::nFS_MIMode)
			{
				// Mutual Informaiton feature selection
				if ( (fabs(P_c * P_ft) > dZero) && (fabs(P_c_ft) > dZero) ) 
				{
					wordnode.m_pCataWeight[nCataCount]+= P_c * log( P_c_ft/(P_c * P_ft) );
				}
			}
			else if(m_paramClassifier.m_nFSMode==CClassifierParam::nFS_CEMode)
			{
				// Cross Entropy for text feature selection
				if ( (fabs(P_c * P_ft) > dZero) && (fabs(P_c_ft) > dZero) ) 
				{
					wordnode.m_pCataWeight[nCataCount]+= P_c_ft * log( P_c_ft/(P_c * P_ft) );
				}
			}
			else if(m_paramClassifier.m_nFSMode==CClassifierParam::nFS_X2Mode)
			{
				// X^2 Statistics feature selection
				if ( (fabs(P_n_c * P_ft * P_n_ft) > dZero) ) 
				{
					wordnode.m_pCataWeight[nCataCount]+= (P_c_ft * P_n_c_n_ft - P_n_c_ft * P_c_n_ft) * (P_c_ft * P_n_c_n_ft - P_n_c_ft * P_c_n_ft) / ( P_ft * P_n_c * P_n_ft);
				}
			}
			else if(m_paramClassifier.m_nFSMode==CClassifierParam::nFS_WEMode)
			{
				// Weight of Evielence for text feature selection
				double		odds_c_ft;
				double		odds_c;
				double		P_c_inv_ft=P_c_ft/P_ft;

				if( fabs(P_c_inv_ft) < dZero )
					odds_c_ft = 1.0 / ( N * N -1);
				else if ( fabs(P_c_inv_ft-1) < dZero )
					odds_c_ft = N * N -1;
				else
					odds_c_ft = P_c_inv_ft / (1.0 - P_c_inv_ft);

				if( fabs(P_c) < dZero )
					odds_c = 1.0 / ( N * N -1);
				else if ( fabs(P_c-1) < dZero )
					odds_c = N * N -1;
				else
					odds_c = P_c / (1.0 - P_c);
				if( fabs(odds_c) > dZero && fabs(odds_c_ft) > dZero )
				{
					wordnode.m_pCataWeight[nCataCount]+= P_c * P_ft * fabs( log(odds_c_ft / odds_c) );
				}
			}
			else //if(m_paramClassifier.m_nFSMode==CClassifierParam::nFS_IGMode) 
			{
				// Information gain feature selection
				if ( (fabs(P_c * P_n_ft) > dZero) && ( fabs(P_c_n_ft) > dZero) ) 
				{
					wordnode.m_pCataWeight[nCataCount]+=P_c_n_ft * log( P_c_n_ft/(P_c * P_n_ft) );
				}
				if ( (fabs(P_c * P_ft) > dZero) && (fabs(P_c_ft) > dZero) ) 
				{
					wordnode.m_pCataWeight[nCataCount]+= P_c_ft * log( P_c_ft/(P_c * P_ft) );
				}
			}
			wordnode.m_dWeight+=wordnode.m_pCataWeight[nCataCount];
			wordnode.m_pCataWeightPro[nCataCount] = 1.0 * (N_c_ft+1)/(2+N);//��str�������nCataCount�ĸ���
			
			//CString strtemp;
			//strtemp.Format("��%d�����У��ʵ�Ȩ����%lf",nCataCount,wordnode.m_pCataWeight[nCataCount]);
			//CMessage::PrintInfo(strtemp);
			
			nCataCount++;				
		}
		ASSERT(nCataCount==nTotalCata);
	}
	CMessage::PrintStatusInfo("");
}

//����ÿһƪѵ���ĵ�������ÿһά��Ȩ��
void CClassifier::ComputeWeight(bool bMult)
{
	long lWordNum=m_lstTrainWordList.GetCount();
	if(m_lstTrainWordList.GetCount()<=0) return;
	long lDocNum=m_lstTrainCatalogList.GetDocNum();
	if(lDocNum<=0) return;
	m_lstTrainWordList.ComputeWeight(lDocNum,bMult);

	double sum=0.0;
	int i=0;
	POSITION pos_cata = m_lstTrainCatalogList.GetFirstPosition();
	while(pos_cata != NULL)  // for each catalog 
	{
		//ȡ���б��е�ÿһ����
		CCatalogNode& cataNode = m_lstTrainCatalogList.GetNext(pos_cata);
		POSITION pos_doc  = cataNode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docNode=cataNode.GetNext(pos_doc);

			sum=0.0;
			for(i=0;i<docNode.m_nAllocLen;i++)
			{
				docNode.m_sWeightSet[i].s_dWeight*=docNode.m_sWeightSet[i].s_tfi;
				sum+=(docNode.m_sWeightSet[i].s_dWeight*docNode.m_sWeightSet[i].s_dWeight);
			}
			sum=sqrt(sum);
			for(i=0;i<docNode.m_nAllocLen;i++)
				docNode.m_sWeightSet[i].s_dWeight/=sum;

			CMessage::PrintStatusInfo("�����ĵ�"+docNode.m_strDocName+"����ÿһά��Ȩ��");
		}
	}
}

void CClassifier::QuickSort(sSortType *psData, int iLo,int iHi)
{
    int Lo, Hi;
	double Mid;
	sSortType	t;
    Lo = iLo;
    Hi = iHi;
    Mid = psData[(Lo + Hi)/2].dWeight;
    do
	{
		while(psData[Lo].dWeight > Mid) Lo++;
		while(psData[Hi].dWeight < Mid) Hi--;
		if(Lo <= Hi)
		{
			t = psData[Lo];
			psData[Lo]=psData[Hi];
			psData[Hi]=t;
			Lo++;
			Hi--;
		}
	}while(Hi>Lo);
    if(Hi > iLo) QuickSort(psData, iLo, Hi);
    if(Lo < iHi) QuickSort(psData, Lo, iHi);
}

void CClassifier::Sort(sSortType *psData,int nSize)
{
	QuickSort(psData,0,nSize);
}


// Give m_lstWordList & m_lstTrainCatalogList
// Output the present vector of each document;
// bFlag=false ��η����ʱ��ʹ��
void CClassifier::GenModel()
{
	CDocNode::AllocTempBuffer(m_lstTrainWordList.GetCount());
	POSITION pos_cata = m_lstTrainCatalogList.GetFirstPosition();
	while(pos_cata != NULL)  // for each catalog 
	{
		//ȡ���б��е�ÿһ����
		CCatalogNode& cataNode = m_lstTrainCatalogList.GetNext(pos_cata);
		POSITION pos_doc  = cataNode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docNode=cataNode.GetNext(pos_doc);
			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				docNode.ScanChineseWithDict(cataNode.m_strDirName.GetBuffer(0),m_lstTrainWordList);
			else
				docNode.ScanEnglishWithDict(cataNode.m_strDirName.GetBuffer(0),m_lstTrainWordList,m_paramClassifier.m_bStem);
			docNode.GenDocVector();
			CMessage::PrintStatusInfo("�����ĵ�"+docNode.m_strDocName+"���ĵ�����");
		}
	}
	CDocNode::DeallocTempBuffer();
}

void CClassifier::GenQueryModel()
{
	CQueryNode::AllocTempBuffer(m_lstTrainWordList.GetCount());
	POSITION pos_cata = m_lstTrainCatalogList.GetFirstPosition();
	while(pos_cata != NULL)  // for each catalog 
	{
		//ȡ���б��е�ÿһ����
		CCatalogNode& cataNode = m_lstTrainCatalogList.GetNext(pos_cata);
		POSITION pos_qry  = cataNode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			CQueryNode& qryNode=cataNode.GetNextQuery(pos_qry);
			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				qryNode.ScanChineseWithDict(m_lstTrainWordList);
			else
				qryNode.ScanEnglishWithDict(m_lstTrainWordList,m_paramClassifier.m_bStem);
			qryNode.GenQryVector();
			CMessage::PrintStatusInfo("���ɲ�ѯ"+qryNode.m_Text+"�Ĳ�ѯ����");
		}
	}
	CQueryNode::DeallocTempBuffer();
}
// generate original dictionary (the largest one)
// form train files
bool CClassifier::GenDic()
{
	m_lstWordList.InitWordList();
	CTime startTime;
	CTimeSpan totalTime;

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("�ִʳ����ʼ�������Ժ�..."));	
	if(!g_wordSeg.InitWorgSegment(theApp.m_strPath.GetBuffer(0),m_paramClassifier.m_nLanguageType))
	{
		CMessage::PrintError(_T("�ִʳ����ʼ��ʧ��!"));
		return false;
	}
	if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
		g_wordSeg.SetSegSetting(CWordSegment::uPlace);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�ִʳ����ʼ����������ʱ")+totalTime.Format("%H:%M:%S"));


	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("��ʼɨ��ѵ���ĵ������Ժ�..."));
	if(m_lstTrainCatalogList.BuildLib(m_paramClassifier.m_txtTrainDir)<=0)
	{
		CMessage::PrintError("ѵ���ĵ�������Ϊ0!");
		return false;
	}

	CString strFileName;
	POSITION pos_cata=m_lstTrainCatalogList.GetFirstPosition();
	int nCount,nCataNum; 
	CString s;
	nCataNum=m_lstTrainCatalogList.GetCataNum();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=m_lstTrainCatalogList.GetNext(pos_cata);
		POSITION pos_doc=catalognode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catalognode.GetNext(pos_doc);
			CMessage::PrintStatusInfo(_T("ɨ���ĵ�")+docnode.m_strDocName);

			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				nCount=docnode.ScanChinese(catalognode.m_strDirName.GetBuffer(0),
							m_lstWordList,nCataNum,catalognode.m_idxCata);
			else
				nCount=docnode.ScanEnglish(catalognode.m_strDirName.GetBuffer(0),
							m_lstWordList,nCataNum,catalognode.m_idxCata,
							m_paramClassifier.m_bStem);
			if(nCount==0)
			{
				CMessage::PrintError("�ļ�"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"������!");
				continue;
			}
			else if(nCount<0)
			{
				CMessage::PrintError("�ļ�"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"�޷���!");
				continue;
			}
			catalognode.m_lTotalWordNum+=nCount;// information collection point
			//temp=nCount;
			//itoa(nCount,s,10);
			s.Format("%d",nCount);

			//CMessage::PrintInfo("�ļ�"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"�ִ�����"+s);
		}
	}
	g_wordSeg.FreeWordSegment();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("ɨ��ѵ���ĵ���������ʱ")+totalTime.Format("%H:%M:%S"));
	return true;
}

bool CClassifier::GenLogDic()
{
	m_lstWordList.InitWordList();
	CTime startTime;
	CTimeSpan totalTime;

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("�ִʳ����ʼ�������Ժ�..."));	
	if(!g_wordSeg.InitWorgSegment(theApp.m_strPath.GetBuffer(0),m_paramClassifier.m_nLanguageType))
	{
		CMessage::PrintError(_T("�ִʳ����ʼ��ʧ��!"));
		return false;
	}
	if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
		g_wordSeg.SetSegSetting(CWordSegment::uPlace);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�ִʳ����ʼ����������ʱ")+totalTime.Format("%H:%M:%S"));


	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("��ʼɨ��ѵ���ĵ������Ժ�..."));
	if(m_lstTrainCatalogList.BuildLogLib(m_paramClassifier.m_txtTrainDir,m_paramClassifier.m_nUKS_k,m_paramClassifier.m_nURL)<=0)/////////xby
	{
		CMessage::PrintError("ѵ���ĵ�������Ϊ0!");
		return false;
	}

	CString strFileName;
	POSITION pos_cata=m_lstTrainCatalogList.GetFirstPosition();
	int nCount,nCataNum; 
	CString s;
	nCataNum=m_lstTrainCatalogList.GetCataNum();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=m_lstTrainCatalogList.GetNext(pos_cata);
		POSITION pos_qry=catalognode.GetFirstQueryPosition();///////////////////////////////////
		
		while(pos_qry!=NULL)
		{
			CQueryNode& querynode=catalognode.GetNextQuery(pos_qry);
			CMessage::PrintStatusInfo(_T("ɨ���ѯ")+querynode.m_idxQuery);

			/////����־�ļ��е�ÿ����־��¼

			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				nCount=querynode.ScanChineseText(querynode.m_Text,
							m_lstWordList,nCataNum,catalognode.m_idxCata);
			else
				nCount=querynode.ScanEnglishText(catalognode.m_strDirName.GetBuffer(0),
							m_lstWordList,nCataNum,catalognode.m_idxCata,
							m_paramClassifier.m_bStem);
			if(nCount==0)
			{
				CMessage::PrintError("��ѯ"+catalognode.m_strDirName+"\\"+querynode.m_Text+"������!");
				continue;
			}
			else if(nCount<0)
			{
				CMessage::PrintError("��ѯ"+catalognode.m_strDirName+"\\"+querynode.m_Text+"�޷���!");
				continue;
			}
			catalognode.m_lTotalWordNum+=nCount;// information collection point
			//temp=nCount;
			//itoa(nCount,s,10);
			s.Format("%d",nCount);

			//CMessage::PrintInfo("�ļ�"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"�ִ�����"+s);
		}
	}
	g_wordSeg.FreeWordSegment();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("ɨ��ѵ���ĵ���������ʱ")+totalTime.Format("%H:%M:%S"));
	return true;
}

void CClassifier::InitTrain()
{
	m_lstTrainWordList.InitWordList();
	m_lstTrainCatalogList.InitCatalogList();
	m_lstWordList.InitWordList();
}

//����nType������������ģ�͵����,nType=0����KNN������,nType=1����SVM������
bool CClassifier::WriteModel(CString strFileName, int nType)
{
	CFile fOut;
	if( !fOut.Open(strFileName,CFile::modeCreate | CFile::modeWrite) )
	{
		CMessage::PrintError("�޷������ļ�g"+strFileName+"!");
		return false;
	}
	int type[5];
		type[0]=m_paramClassifier.m_nCS;
		type[1]=m_paramClassifier.m_nRS;
		type[2]=m_paramClassifier.m_nUKS;
		type[3]=m_paramClassifier.m_Length;
		type[4]=m_paramClassifier.m_ClassifierW;


	CArchive ar(&fOut,CArchive::store);	
	if(nType==0)
	{
		m_lstTrainWordList.DumpToFile(m_paramClassifier.m_txtResultDir+"\\features.dat");
		m_lstTrainWordList.DumpWordList(m_paramClassifier.m_txtResultDir+"\\features.txt");
		m_lstTrainCatalogList.DumpToFile(m_paramClassifier.m_txtResultDir+"\\train.dat");
		m_lstTrainCatalogList.DumpQueryList(m_paramClassifier.m_txtResultDir+"\\train.txt",type);
		m_paramClassifier.DumpToFile(m_paramClassifier.m_txtResultDir+"\\params.dat");

		ar<<dwModelFileID;
		ar<<CString("params.dat");
		ar<<CString("features.dat");
		ar<<CString("train.dat");
	}
	else if(nType==1)
	{
	
		m_lstTrainWordList.DumpToFile(m_paramClassifier.m_txtResultDir+"\\features.dat");
		m_lstTrainWordList.DumpWordList(m_paramClassifier.m_txtResultDir+"\\features.txt");
		m_lstTrainCatalogList.DumpToFile(m_paramClassifier.m_txtResultDir+"\\train.dat",1);
		m_lstTrainCatalogList.DumpQueryList(m_paramClassifier.m_txtResultDir+"\\train.txt",type);
		m_paramClassifier.DumpToFile(m_paramClassifier.m_txtResultDir+"\\params.dat");
		
		m_theSVM.com_param.classifier_num=m_lstTrainCatalogList.GetCataNum();
		m_theSVM.com_param.trainfile="train.txt";
		m_theSVM.com_param.resultpath=m_paramClassifier.m_txtResultDir;
		m_theSVM.com_param.DumpToFile(m_paramClassifier.m_txtResultDir+"\\svmparams.dat");

		ar<<dwModelFileID;
		ar<<CString("params.dat");
		ar<<CString("features.dat");
		ar<<CString("train.dat");
		ar<<CString("svmparams.dat");
	}
	else if(nType==2)
	{
		m_lstTrainWordList.DumpToFile(m_paramClassifier.m_txtResultDir+"\\features.dat");
		m_lstTrainWordList.DumpWordList(m_paramClassifier.m_txtResultDir+"\\features.txt");
		m_lstTrainWordList.DumpWordProList(m_paramClassifier.m_txtResultDir+"\\WordPro.txt",m_lstTrainCatalogList.GetCataNum());

		m_lstTrainCatalogList.DumpToFile(m_paramClassifier.m_txtResultDir+"\\train.dat");
		m_lstTrainCatalogList.DumpQueryList(m_paramClassifier.m_txtResultDir+"\\train.txt",type);
		m_paramClassifier.DumpToFile(m_paramClassifier.m_txtResultDir+"\\params.dat");
		
		ar<<dwModelFileID;
		ar<<CString("params.dat");
		ar<<CString("features.dat");
		ar<<CString("train.dat");
		ar<<CString("WordPro.txt");
	}
	ar.Close();
	fOut.Close(); 
	return true;
}

bool CClassifier::OpenModel(CString strFileName)
{
	CFile fIn;
	if(!fIn.Open(strFileName,CFile::modeRead))
	{
		CMessage::PrintError("�޷����ļ�"+strFileName+"!") ;
		return false;
	}
	CTime startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("���ڴ򿪷���ģ���ļ������Ժ�..."));


	CArchive ar(&fIn,CArchive::load);
	CString str,strPath;
	DWORD dwFileID;
	
	//�����ļ���ʽ��ʶ��
	strPath=strFileName.Left(strFileName.ReverseFind('\\'));
	ar>>dwFileID;
	if(dwFileID!=dwModelFileID)
	{
		ar.Close();
		fIn.Close();
		CMessage::PrintError("����ģ���ļ��ĸ�ʽ����ȷ!");
		return false;
	}

	ar>>str;

	if(!m_paramClassifier.GetFromFile(strPath+"\\"+str))
	{
		CMessage::PrintError(_T("�޷���ѵ�������ļ�"+str+"!"));
		return false;
	}
	m_paramClassifier.m_txtResultDir=strPath;

	if(m_paramClassifier.m_nClassifierType==0)
	{
		ar>>str;
		m_lstTrainWordList.InitWordList();
		if(!m_lstTrainWordList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷�����������ļ�"+str+"!"));
			return false;
		}
		ar>>str;
		m_lstTrainCatalogList.InitCatalogList();
		if(!m_lstTrainCatalogList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷���ѵ���ĵ��б��ļ�"+str+"!"));
			return false;
		}
	}
	else if(m_paramClassifier.m_nClassifierType==1)
	{
		ar>>str;
		m_lstTrainWordList.InitWordList();
		if(!m_lstTrainWordList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷�����������ļ�"+str+"!"));
			return false;
		}
		//����SVM��������˵m_lstTrainCatalogList��ʵû��
		//�˴�������ֻ��Ϊ����CLeftViw����ʾĳЩͳ����Ϣʱʹ��
		ar>>str;
		m_lstTrainCatalogList.InitCatalogList();
		if(!m_lstTrainCatalogList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷���ѵ���ĵ��б��ļ�"+str+"!"));
			return false;
		}
		ar>>str;
		if(!m_theSVM.com_param.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷���SVMѵ�������ļ�"+str+"!"));
			return false;
		}
		m_theSVM.com_param.trainfile=strPath+"\\train.txt";
		m_theSVM.com_param.resultpath=strPath;
	}
	else if(m_paramClassifier.m_nClassifierType==2)
	{
		ar>>str;
		m_lstTrainWordList.InitWordList();
		if(!m_lstTrainWordList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷�����������ļ�"+str+"!"));
			return false;
		}
		ar>>str;
		m_lstTrainCatalogList.InitCatalogList();
		if(!m_lstTrainCatalogList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷���ѵ���ĵ��б��ļ�"+str+"!"));
			return false;
		}
		ar>>str;
		if(!m_lstTrainWordList.GetProFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("�޷������������������ļ�"+str+"!"));
			return false;
		}
	}
	ar.Close();
	fIn.Close();
	Prepare();
	CTimeSpan totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("����ģ���ļ��Ѿ��򿪣���ʱ")+totalTime.Format("%H:%M:%S")+"\r\n");
	
	str.Empty();
	m_paramClassifier.GetParamString(str);
	CMessage::PrintInfo(str);
	return true;	
}

bool CClassifier::Classify()
{
	m_lstTrainCatalogList.DumpCataList(m_paramClassifier.m_strResultDir+"\\classes.txt");
	CTime startTime;
	CTimeSpan totalTime;
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("����ɨ������ĵ������Ժ�O(��_��)O����~..."));
	if(m_paramClassifier.m_bEvaluation)
	{
		//BuildLib�������Ѿ������g_lstTestCatalogList�����Դ˴������ٶ����ʼ��
		m_lstTestCatalogList.BuildLib(m_paramClassifier.m_strTestDir);
		if(!m_lstTestCatalogList.BuildCatalogID(m_lstTrainCatalogList))
		{
			CMessage::PrintError("�����ļ��а������޷�ʶ������!");
			return false;
		}
		CMessage::PrintInfo(_T("O(��_��)O����~..."));
	}
	else
	{
		m_lstTestCatalogList.InitCatalogList();
		CCatalogNode catalognode;
		catalognode.m_strDirName=m_paramClassifier.m_strTestDir;
		catalognode.m_strCatalogName="�����ĵ�";
		catalognode.m_idxCata=-1;
		POSITION posTemp=m_lstTestCatalogList.AddCata(catalognode);
		CCatalogNode& cataTemp=m_lstTestCatalogList.GetAt(posTemp);
		cataTemp.SetStartDocID(0);
		cataTemp.ScanDirectory(m_paramClassifier.m_strTestDir);
	}
	//if(m_lstTestCatalogList.GetDocNum()<=0)
	if(m_lstTestCatalogList.GetQueryNum()<=0)
	{
		CMessage::PrintError("�����ļ�����Ϊ0!\r\n�������Ҫ�Է�������������ʱ,�����ĵ�������\"�����ĵ�Ŀ¼\"��,������������Ŀ¼��!");
		return false;
	}
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("ɨ������ĵ���������ʱ")+totalTime.Format("%H:%M:%S"));

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("���ڶԲ����ĵ����з��࣬���Ժ�..."));
	long lCorrect=0,lUnknown=0;
	lUnknown=Classify(m_lstTestCatalogList);


	lCorrect=SaveResults(m_lstTestCatalogList,m_paramClassifier.m_strResultDir+"\\results.txt");
	long lTotalNum=m_lstTestCatalogList.GetDocNum()-lUnknown;
	CString str;
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�����ĵ������������ʱ")+totalTime.Format("%H:%M:%S"));
	if (lUnknown>0) 
	{
		str.Format("�޷�������ĵ���%d:",lUnknown);
		CMessage::PrintInfo(str);
	}
	if(m_paramClassifier.m_bEvaluation&&lTotalNum>0&&lCorrect>0)
		str.Format("�����ĵ�����:%d,׼ȷ��:%f",m_lstTestCatalogList.GetDocNum(),(float)(lCorrect)/(float)(lTotalNum));
	else
		str.Format("�����ĵ�����:%d",m_lstTestCatalogList.GetDocNum());
	CMessage::PrintInfo(str);
	return true;
}

bool CClassifier::Classify_Query()
{
	m_lstTrainCatalogList.DumpCataList(m_paramClassifier.m_strResultDir+"\\classes.txt");
	CTime startTime;
	CTimeSpan totalTime;
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("����ɨ������ĵ������Ժ�O(��_��)O����~~..."));
	if(m_paramClassifier.m_bEvaluation)
	{
		//BuildLib�������Ѿ������g_lstTestCatalogList�����Դ˴������ٶ����ʼ��
		m_lstTestCatalogList.BuildLogLib(m_paramClassifier.m_strTestDir,m_paramClassifier.m_nUKS_k,m_paramClassifier.m_nURL);
		if(!m_lstTestCatalogList.BuildCatalogID(m_lstTrainCatalogList))
		{
			CMessage::PrintError("�����ļ��а������޷�ʶ������!");
			return false;
		}
		CMessage::PrintInfo(_T("O(��_��)O����~..."));
	}
	else
	{
		m_lstTestCatalogList.InitCatalogList();
		CCatalogNode catalognode;
		catalognode.m_strDirName=m_paramClassifier.m_strTestDir;
		catalognode.m_strCatalogName="�����ĵ�";
		catalognode.m_idxCata=-1;
		POSITION posTemp=m_lstTestCatalogList.AddCata(catalognode);
		CCatalogNode& cataTemp=m_lstTestCatalogList.GetAt(posTemp);
		cataTemp.SetStartDocID(0);
		cataTemp.ScanDirectory(m_paramClassifier.m_strTestDir);
	}
	//if(m_lstTestCatalogList.GetDocNum()<=0)
	if(m_lstTestCatalogList.GetQueryNum()<=0)
	{
		CMessage::PrintError("�����ļ�����Ϊ0!\r\n�������Ҫ�Է�������������ʱ,�����ĵ�������\"�����ĵ�Ŀ¼\"��,������������Ŀ¼��!");
		return false;
	}
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("ɨ������ĵ���������ʱ")+totalTime.Format("%H:%M:%S"));

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("���ڶԲ����ĵ����з��࣬���Ժ�..."));
	long lCorrect=0,lUnknown=0;
	lUnknown=Classify_Query(m_lstTestCatalogList);


	lCorrect=SaveResults_Query(m_lstTestCatalogList,m_paramClassifier.m_strResultDir+"\\results.txt");
	long lTotalNum=m_lstTestCatalogList.GetQueryNum()-lUnknown;
	CString str;
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�����ĵ������������ʱ")+totalTime.Format("%H:%M:%S"));
	if (lUnknown>0) 
	{
		str.Format("�޷�������ĵ���%d:",lUnknown);
		CMessage::PrintInfo(str);
	}
	if(m_paramClassifier.m_bEvaluation&&lTotalNum>0&&lCorrect>0)
		str.Format("�����ĵ�����:%d,׼ȷ��:%f",m_lstTestCatalogList.GetQueryNum(),(float)(lCorrect)/(float)(lTotalNum));
	else
		str.Format("�����ĵ�����:%d",m_lstTestCatalogList.GetQueryNum());
	CMessage::PrintInfo(str);
	return true;
}

//��Smart��ʽ���ĵ����з���
bool CClassifier::ClassifySmart()
{
	m_lstTrainCatalogList.DumpCataList(m_paramClassifier.m_strResultDir+"\\classes.txt");
	m_lstTestCatalogList.InitCatalogList();
	CCatalogNode catalognode;
	catalognode.m_strDirName=m_paramClassifier.m_strTestDir;
	catalognode.m_strCatalogName="�����ĵ�";
	catalognode.m_idxCata=-1;
	POSITION posTemp=m_lstTestCatalogList.AddCata(catalognode);
	CCatalogNode& cataTemp=m_lstTestCatalogList.GetAt(posTemp);

	FILE *stream1,*stream2;
	if( (stream1 = fopen( m_paramClassifier.m_strTestDir, "r" )) == NULL )
	{
		CMessage::PrintError("�޷����ļ�"+m_paramClassifier.m_strTestDir+"!");
		return false;
	}

	//�����SVM������,����Ҫ�Ƚ����в����ĵ�ת��Ϊ����,���浽�ļ�test.dat
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
	{
		m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.dat";
		if((stream2=fopen(m_theSVM.com_param.classifyfile,"w"))==NULL)
		{
			CMessage::PrintError("�޷����������ĵ������ļ�"+m_theSVM.com_param.classifyfile+"!");
			return false;
		}
	}

	CTime startTime;
	CTimeSpan totalTime;
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("���ڶԲ����ĵ����з��࣬���Ժ�..."));

	char fname[10],type[1024],line[4096],content[100*1024];
	//falg=1 ��һ�е��������ĵ������
	//flag=2 ��һ�е��������ĵ��ı���
	//flag=3 ��һ�е��������ĵ�������
	int flag=0,nCount,len,i;
	long lUnknown=0,lDocNum=0;
	CStringArray typeArray;
	CString strFileName,strCopyFile;
	bool bTitle=false; //�Ƿ��Ѿ���������
	double dThreshold=(double)m_paramClassifier.m_dThreshold/100.0;
	int nWordNum=m_lstTrainWordList.GetCount();
	while(!feof(stream1))
	{
		if(fgets(line,4096,stream1)==NULL) continue;
		if(line[0]=='.')
		{
			if(flag==3)
			{
				CDocNode doc;
				posTemp=cataTemp.AddDoc(doc);
				CDocNode& docnode=cataTemp.GetAt(posTemp);
				docnode.m_strDocName=fname;
				if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
				{
					nCount=KNNCategory(content,docnode,false);
				}
				else
				{
					if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
						nCount=docnode.ScanChineseStringWithDict(content,m_lstTrainWordList)-1;
					else
						nCount=docnode.ScanEnglishStringWithDict(content,m_lstTrainWordList,
											m_paramClassifier.m_bStem)-1;

					fprintf(stream2,"%d",1);
					for(i=0;i<nWordNum;i++)
					{
						if(docnode.m_pTemp[i].s_tfi!=0) 
							fprintf(stream2," %d:%f",i+1,docnode.m_pTemp[i].s_dWeight);
					}
					fprintf(stream2,"\n");
				}
				if(m_paramClassifier.m_bEvaluation) typeArray.Add(type);
				if(nCount<0) 
				{
					CMessage::PrintError("�޷�ʶ���ĵ�"+docnode.m_strDocName+"�����!");
					lUnknown++;
				}
				CMessage::PrintStatusInfo(_T("ɨ���ĵ�")+docnode.m_strDocName);
				flag=0;
				bTitle=false;
				fname[0]=0;
				type[0]=0;
				content[0]=0;
				lDocNum++;
			}
			switch(line[1])
			{
				case 'I':
					strcpy(fname,line+3);
					len=strlen(fname);
					if(fname[len-1]='\r') fname[len-1]='\0';
					break;
				case 'C':
					flag=1;
					break;
				case 'T':
					flag=2;
					break;
				case 'W':
					flag=3;
					break;
			}
		}
		else
		{
			switch(flag)
			{
				case 1:
					strcpy(type,line);
					break;
				case 2:
					if(!bTitle)
					{
						strcpy(content,line);
						bTitle=true;
					}
					else
						strcat(content,line);
					break;
				case 3:
					strcat(content,line);
					break;
			}
		}
	}
	fclose(stream1);
	CMessage::PrintStatusInfo("");
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM) 
	{
		fclose(stream2);
		startTime=CTime::GetCurrentTime();
		CMessage::PrintInfo(_T("����ʹ��SVM���������ĵ����з���,���Ժ�..."));
		m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.dat";
		SVMClassifyVectorFile(m_theSVM.com_param.classifyfile);
		totalTime=CTime::GetCurrentTime()-startTime;
		CMessage::PrintInfo(_T("SVM������̽�������ʱ")+totalTime.Format("%H:%M:%S"));
	}

	long lCorrect=SaveResults(m_lstTestCatalogList,m_paramClassifier.m_strResultDir+"\\results.txt",&typeArray);
	long lTotalNum=m_lstTestCatalogList.GetDocNum()-lUnknown;
	CString str;
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�����ĵ������������ʱ")+totalTime.Format("%H:%M:%S"));
	if (lUnknown>0) 
	{
		str.Format("�޷�������ĵ���%d:",lUnknown);
		CMessage::PrintInfo(str);
	}
	if(m_paramClassifier.m_bEvaluation&&lTotalNum>0&&lCorrect>0)
		str.Format("�����ĵ�����:%d,׼ȷ��:%f",m_lstTestCatalogList.GetDocNum(),(float)(lCorrect)/(float)(lTotalNum));
	else
		str.Format("�����ĵ�����:%d",m_lstTestCatalogList.GetDocNum());
	CMessage::PrintInfo(str);
	return true;
}

//���ĵ����з��࣬�����ĵ���ÿ���������ƶȣ�����ֵΪ����޷�ʶ����ĵ�����
long CClassifier::SVMClassify(CCatalogList &cataList)
{
	long lUnknown=0;
	FILE *stream;
	m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.dat";
	if((stream=fopen(m_theSVM.com_param.classifyfile,"w"))==NULL)
	{
		CMessage::PrintError("�޷����������ĵ������ļ�"+m_theSVM.com_param.classifyfile+"!");
		return 0;
	}
	
	CTime startTime;
	CTimeSpan totalTime;
	CString str;
	int nCount=0;
	long lWordNum=m_lstTrainWordList.GetCount();
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("�������ɲ����ĵ���������ʽ�����Ժ�..."));
	POSITION pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=cataList.GetNext(pos_cata);
		char *path=catalognode.m_strDirName.GetBuffer(0);
		POSITION pos_doc=catalognode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catalognode.GetNext(pos_doc);
			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				nCount=docnode.ScanChineseWithDict(path,m_lstTrainWordList);
			else
				nCount=docnode.ScanEnglishWithDict(path,m_lstTrainWordList,
											m_paramClassifier.m_bStem);

			fprintf(stream,"%d",catalognode.m_idxCata+1);
			for(int i=0;i<lWordNum;i++)
			{
				if(docnode.m_pTemp[i].s_tfi!=0) 
					fprintf(stream," %d:%f",i+1,docnode.m_pTemp[i].s_dWeight);
			}
			fprintf(stream,"\n");
			CMessage::PrintStatusInfo(_T("ɨ���ĵ�")+docnode.m_strDocName);
		
			if(nCount<0)
			{
				str="�޷�ʶ���ĵ�";
				str+=catalognode.m_strDirName;
				str+="\\"+docnode.m_strDocName+"�����!";
				lUnknown++;
				CMessage::PrintError(str);
			}
		}
	}
	CMessage::PrintStatusInfo(_T(""));
	fclose(stream);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�����ĵ����������ɽ�������ʱ")+totalTime.Format("%H:%M:%S"));
	
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("����ʹ��SVM���������ĵ����з���,���Ժ�..."));
	//Ϊÿƪ�ĵ��͸����������ƶ���������ڴ�
	pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=cataList.GetNext(pos_cata);
		POSITION pos_doc=catalognode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catalognode.GetNext(pos_doc);
			docnode.AllocResultsBuffer(m_nClassNum);
		}
	}
	SVMClassifyVectorFile(m_theSVM.com_param.classifyfile);
	CMessage::PrintStatusInfo(_T(""));

	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("SVM������̽�������ʱ")+totalTime.Format("%H:%M:%S"));
	return lUnknown;
}

//���ĵ����з��࣬�����ĵ���ÿ���������ƶȣ�����ֵΪ����޷�ʶ����ĵ�����
long CClassifier::SVMClassify_Query(CCatalogList &cataList)
{
	long lUnknown=0;
	FILE *stream;
	int sum=0;
	m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.txt";
	if((stream=fopen(m_theSVM.com_param.classifyfile,"w"))==NULL)
	{
		CMessage::PrintError("�޷����������ĵ������ļ�"+m_theSVM.com_param.classifyfile+"!");
		return 0;
	}
	
	CTime startTime;
	CTimeSpan totalTime;
	CString str;
	int nCount=0;
	long lWordNum=m_lstTrainWordList.GetCount();
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("�������ɲ����ĵ���������ʽ�����Ժ�..."));
	POSITION pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=cataList.GetNext(pos_cata);
		char *path=catalognode.m_strDirName.GetBuffer(0);
		POSITION pos_qry=catalognode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			sum=0;
			CQueryNode& qrynode=catalognode.GetNextQuery(pos_qry);
			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				nCount=qrynode.ScanChineseWithDict(m_lstTrainWordList);
			else
				nCount=qrynode.ScanEnglishWithDict(m_lstTrainWordList,
											m_paramClassifier.m_bStem);

			fprintf(stream,"%d",catalognode.m_idxCata+1);
			////fprintf(stream,"%s",qrynode.m_Text);
	           if(m_paramClassifier.m_nCS)
				{
				 fprintf(stream," %d:%f",++sum,qrynode.nCS);//nCSֵ
			   }
			   if(m_paramClassifier.m_nRS)
			   {
			     fprintf(stream," %d:%lf",++sum,qrynode.nRS);//nRSֵ
				}
			   
				if(m_paramClassifier.m_nUKS)
				{
	             fprintf(stream," %d:%f",++sum,qrynode.UKS_N);//UKS_N 
				 fprintf(stream," %d:%f",++sum,qrynode.UKS_I);//UKS_I
				 fprintf(stream," %d:%f",++sum,qrynode.UKS_T);//UKS_T
				}
				if(m_paramClassifier.m_Length)
			   {
			     fprintf(stream," %d:%lf",++sum,qrynode.Length);//Lengthֵ
				}
				if(m_paramClassifier.m_ClassifierW)
				{
				 for(int i=0;i<lWordNum;i++)
				 {
					if(qrynode.m_pTemp[i].s_tfi!=0) 
					fprintf(stream," %d:%f",i+sum+1,qrynode.m_pTemp[i].s_dWeight);
				 }
				}
				fprintf(stream,"\n");
			/*
			fprintf(stream," %d:%f",1,qrynode.nCS);//test��nCS
			fprintf(stream," %d:%f",2,qrynode.nRS);
			fprintf(stream," %d:%f",3,qrynode.UKS_N);
			fprintf(stream," %d:%f",4,qrynode.UKS_I);
			fprintf(stream," %d:%f",5,qrynode.UKS_T);
			for(int i=0;i<lWordNum;i++)
			{
				if(qrynode.m_pTemp[i].s_tfi!=0) 
					fprintf(stream," %d:%f",i+6,qrynode.m_pTemp[i].s_dWeight);
			}
			fprintf(stream,"\n");*/
			CMessage::PrintStatusInfo(_T("ɨ���ѯ")+qrynode.m_Text);
		
			if(nCount<0)
			{
				str="�޷�ʶ���ѯ";
				str+=catalognode.m_strDirName;
				str+="\\"+qrynode.m_Text+"�����!";
				lUnknown++;
				CMessage::PrintError(str);
			}
		}
	}
	CMessage::PrintStatusInfo(_T(""));
	fclose(stream);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("�����ĵ����������ɽ�������ʱ")+totalTime.Format("%H:%M:%S"));
	
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("����ʹ��SVM���������ĵ����з���,���Ժ�..."));
	//Ϊÿƪ�ĵ��͸����������ƶ���������ڴ�
	pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=cataList.GetNext(pos_cata);
		POSITION pos_qry=catalognode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			CQueryNode& qrynode=catalognode.GetNextQuery(pos_qry);
			qrynode.AllocResultsBuffer(m_nClassNum);
		}
	}
	SVMClassifyVectorFile_Query(m_theSVM.com_param.classifyfile);
	CMessage::PrintStatusInfo(_T(""));

	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("SVM������̽�������ʱ")+totalTime.Format("%H:%M:%S"));
	return lUnknown;
}

//ʹ��KNN�������ĵ����з��࣬�����ĵ���ÿ���������ƶ�
//����ֵΪ����޷�ʶ����ĵ�����
long CClassifier::KNNClassify(CCatalogList& cataList,int nCmpType)
{
	long docID=0,lUnknown=0;
	CString str;
	POSITION pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& cataNode=cataList.GetNext(pos_cata);
		POSITION pos_doc=cataNode.GetFirstPosition();
		char *path=cataNode.m_strDirName.GetBuffer(0);
		while(pos_doc!=NULL)
		{
			CDocNode& docNode=cataNode.GetNext(pos_doc);
			short id=KNNCategory(path, docNode, true, nCmpType);
			if(id==-1) 
			{
				str="�޷�ʶ���ĵ�";
				str+=cataNode.m_strDirName;
				str+="\\"+docNode.m_strDocName+"�����!";
				CMessage::PrintError(str);
				lUnknown++;
			}
			CMessage::PrintStatusInfo(_T("ɨ���ĵ�")+docNode.m_strDocName);
		}
	}
	return lUnknown;
}

//�����ĵ���ÿ���������ƶȣ��������ĵ����ƶ��������ID
//nCmpType�������ƶȵĲ�ͬ���㷽ʽ
short CClassifier::KNNCategory(char *pPath, CDocNode &docNode, bool bFile, int nCmpType)
{
	short nCataID=-1;
	if(KNNClassify(pPath,docNode,bFile,nCmpType)) nCataID=SingleCategory(docNode);
	return nCataID;
}

//���bFileΪ��,�����fileΪ�ļ����ļ�����(��������·��)
short CClassifier::KNNCategory(char *file, bool bFile, int nCmpType)
{
	CDocNode docNode;
	short id=-1;
	if(bFile)
	{
		char *fname=strrchr(file,'\\');
		if(fname==NULL) return -1;
		docNode.m_strDocName=(fname+1);

		char path[MAX_PATH];
		strncpy(path,file,fname-file);
		path[fname-file]=0;
		id=KNNCategory(path,docNode,bFile,nCmpType);
	}
	else
		id=KNNCategory(file,docNode,bFile,nCmpType);
	return id;
}

//�����ĵ�docNode��������ʽ�����÷���ComputeSimRatio���������ÿһ���������ƶ�
//����bFileΪ�����pPathΪdocNode��·��,���������Ҫ���з�����ĵ�������
//����nCmpType�������ƶȵĲ�ͬ���㷽ʽ
bool CClassifier::KNNClassify(char *pPath, CDocNode &docNode, bool bFile, int nCmpType)
{
	int nCount=0;
	if(bFile)
	{
		if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
			nCount=docNode.ScanChineseWithDict(pPath,m_lstTrainWordList);
		else
			nCount=docNode.ScanEnglishWithDict(pPath,m_lstTrainWordList,
											m_paramClassifier.m_bStem);
	}
	else
	{
		if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
			nCount=docNode.ScanChineseStringWithDict(pPath,m_lstTrainWordList);
		else
			nCount=docNode.ScanEnglishStringWithDict(pPath,m_lstTrainWordList,
											m_paramClassifier.m_bStem);
	}

	if((m_lDocNum>0)&&(nCount>0))
	{
		ComputeSimRatio(docNode,nCmpType);
		return true;
	}
	else
		return false;
}

//�õ����ĵ�docNode�����ƶȴ�����ֵdThreshold���������
//���û�д���ֵ��ֵ������򷵻����ƶ��������
bool CClassifier::MultiCategory(CDocNode &docNode, CArray<short,short>& aryResult, double dThreshold)
{
	double *pSimRatio=docNode.m_pResults;
	if(pSimRatio==NULL) return false;
	
	double dMax=pSimRatio[0];
	short nMax=0;
	bool bFound=false;
	aryResult.RemoveAll();
	for(short i=1;i<m_nClassNum;i++)
	{
		if(pSimRatio[i]>dMax)
		{
			dMax=pSimRatio[i];
			nMax=i;
		}
		if(pSimRatio[i]>dThreshold) 
		{
			aryResult.Add(i);
			bFound=true;
		}
	}
	if(!bFound) aryResult.Add(nMax);
	return true;
}

//�õ����ĵ�docNode�����ƶȴ�����ֵdThreshold���������
//���û�д���ֵ��ֵ������򷵻����ƶ��������
bool CClassifier::MultiCategory_Query(CQueryNode &qryNode, CArray<short,short>& aryResult, double dThreshold)
{
	double *pSimRatio=qryNode.m_pResults;
	if(pSimRatio==NULL) return false;
	
	double dMax=pSimRatio[0];
	short nMax=0;
	bool bFound=false;
	aryResult.RemoveAll();
	for(short i=1;i<m_nClassNum;i++)
	{
		if(pSimRatio[i]>dMax)
		{
			dMax=pSimRatio[i];
			nMax=i;
		}
		if(pSimRatio[i]>dThreshold) 
		{
			aryResult.Add(i);
			bFound=true;
		}
	}
	if(!bFound) aryResult.Add(nMax);
	return true;
}

//�����ĵ�docNode��ÿһ���������ƶ�
//nCmpType�������ƶȵĲ�ͬ���㷽ʽ
void CClassifier::ComputeSimRatio(CDocNode &docNode,int nCmpType)
{
	//�����ĵ���ѵ������ÿһƪ�ĵ������ƶ�
	int i;
	long k;
	for(i=0;i<m_lDocNum;i++)
	{
		m_pSimilarityRatio[i].dWeight=m_pDocs[i].pDocNode->ComputeSimilarityRatio();
		m_pSimilarityRatio[i].lDocID=i;
	}
	//�������ĵ���ѵ���ĵ������ĵ������ƶȽ��н�������
	Sort(m_pSimilarityRatio,m_lDocNum-1);
	docNode.AllocResultsBuffer(m_nClassNum);
	double *pSimRatio=docNode.m_pResults;
	for(i=0;i<m_nClassNum;i++) pSimRatio[i]=0;
	if(nCmpType<=0)
	{
		//����������ĵ���k������ÿ������е���Ŀ
		for(i=0;i<m_paramClassifier.m_nKNN;i++)
		{
			k=m_pSimilarityRatio[i].lDocID;
			k=m_pDocs[k].nCataID;
			pSimRatio[k]+=1;
		}
		//����"�����ĵ���k������ĳ������е���Ŀ/k"�õ������ĵ�������������ƶ�
		for(i=0;i<m_nClassNum;i++)
			pSimRatio[i]/=m_paramClassifier.m_nKNN;
	}
	else if(nCmpType==1)
	{
		for(i=0;i<m_paramClassifier.m_nKNN;i++)
		{
			k=m_pSimilarityRatio[i].lDocID;
			k=m_pDocs[k].nCataID;
			pSimRatio[k]+=m_pSimilarityRatio[i].dWeight;
		}
	}
}

bool CClassifier::SVMClassify(char *pPath, CDocNode &docNode, bool bFile)
{
	int nCount=0;
	if(bFile)
	{
		if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
			nCount=docNode.ScanChineseWithDict(pPath,m_lstTrainWordList);
		else
			nCount=docNode.ScanEnglishWithDict(pPath,m_lstTrainWordList,
											m_paramClassifier.m_bStem);
	}
	else
	{
		if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
			nCount=docNode.ScanChineseStringWithDict(pPath,m_lstTrainWordList);
		else
			nCount=docNode.ScanEnglishStringWithDict(pPath,m_lstTrainWordList,
											m_paramClassifier.m_bStem);
	}

	if((m_lDocNum>0)&&(nCount>0))
	{
		DOC doc;
		CString str;
		docNode.GenDocVector(doc);
		docNode.AllocResultsBuffer(m_nClassNum);
		for(int i=0;i<m_nClassNum;i++)
		{
			str.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i+1);
			theClassifier.m_theSVM.com_param.modelfile=str;
			docNode.m_pResults[i]=theClassifier.m_theSVM.svm_classify(doc);
		}
		free(doc.words);
		return true;
	}
	else
		return false;
}



void CClassifier::Prepare()
{
	CTime startTime;
	CTimeSpan totalTime;

	if(m_pDocs!=NULL)
	{
		m_lDocNum=0;
		free(m_pDocs);
		m_pDocs=NULL;
	}
	if(m_pSimilarityRatio!=NULL)
	{
		m_lDocNum=0;
		delete[] m_pSimilarityRatio;
		m_pSimilarityRatio=NULL;
	}
	if(m_pProbability!=NULL)
	{
		m_nClassNum=0;
		delete[] m_pProbability;
		m_pProbability=NULL;
	}

	m_nClassNum=m_lstTrainCatalogList.GetCataNum();
	m_lDocNum=m_lstTrainCatalogList.GetDocNum();
	if(m_paramClassifier.m_nKNN>m_lDocNum) m_paramClassifier.m_nKNN=m_lDocNum;
	m_pSimilarityRatio=new DocWeight[m_lDocNum];
	m_pProbability=new DocWeight[m_nClassNum];
	m_pDocs=(DocCatalog*)malloc(sizeof(DocCatalog)*m_lDocNum);
	long num=0;
	POSITION pos_cata = m_lstTrainCatalogList.GetFirstPosition();
	while(pos_cata != NULL)  // for each catalog 
	{
		CCatalogNode& catanode = m_lstTrainCatalogList.GetNext(pos_cata);
		short idxCata=catanode.m_idxCata;
		POSITION pos_doc  = catanode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catanode.GetNext(pos_doc);
			m_pDocs[num].pDocNode=&docnode;
			m_pDocs[num].nCataID=idxCata;
			num++;
		}
	}
	CDocNode::AllocTempBuffer(m_lstTrainWordList.GetCount());
}

void CClassifier::Prepare_Query()
{
	CTime startTime;
	CTimeSpan totalTime;

	if(m_pQrys!=NULL)
	{
		m_lQueryNum=0;
		free(m_pQrys);
		m_pQrys=NULL;
	}
	if(m_pQuerySimilarityRatio!=NULL)
	{
		m_lQueryNum=0;
		delete[] m_pQuerySimilarityRatio;
		m_pQuerySimilarityRatio=NULL;
	}
	if(m_pQueryProbability!=NULL)
	{
		m_nClassNum=0;
		delete[] m_pQueryProbability;
		m_pQueryProbability=NULL;
	}

	m_nClassNum=m_lstTrainCatalogList.GetCataNum();
	m_lQueryNum=m_lstTrainCatalogList.GetQueryNum();
	if(m_paramClassifier.m_nKNN>m_lQueryNum) m_paramClassifier.m_nKNN=m_lQueryNum;
	m_pQuerySimilarityRatio=new QueryWeight[m_lQueryNum];
	m_pQueryProbability=new QueryWeight[m_nClassNum];
	m_pQrys=(QueryCatalog*)malloc(sizeof(QueryCatalog)*m_lQueryNum);
	long num=0;
	POSITION pos_cata = m_lstTrainCatalogList.GetFirstPosition();
	while(pos_cata != NULL)  // for each catalog 
	{
		CCatalogNode& catanode = m_lstTrainCatalogList.GetNext(pos_cata);
		short idxCata=catanode.m_idxCata;
		POSITION pos_qry  = catanode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			CQueryNode& qrynode=catanode.GetNextQuery(pos_qry);
			m_pQrys[num].pQueryNode=&qrynode;
			m_pQrys[num].nCataID=idxCata;
			num++;
		}
	}
	CQueryNode::AllocTempBuffer(m_lstTrainWordList.GetCount());
}

void CClassifier::Sort(DocWeight *pData,int nSize)
{
	QuickSort(pData,0,nSize);
}

void CClassifier::QuickSort(DocWeight *psData, int iLo,int iHi)
{
    int Lo, Hi;
	double Mid;
	DocWeight	t;
    Lo = iLo;
    Hi = iHi;
    Mid = psData[(Lo + Hi)/2].dWeight;
    do
	{
		while(psData[Lo].dWeight > Mid) Lo++;
		while(psData[Hi].dWeight < Mid) Hi--;
		if(Lo <= Hi)
		{
			t = psData[Lo];
			psData[Lo]=psData[Hi];
			psData[Hi]=t;
			Lo++;
			Hi--;
		}
	}while(Hi>Lo);
    if(Hi > iLo) QuickSort(psData, iLo, Hi);
    if(Lo < iHi) QuickSort(psData, Lo, iHi);
}

//�����������浽�ļ�strFileName��,������ȷ������ĵ�����
//������������Ҫ�󿽱��ļ���������Ŀ¼����ִ�п�������
//����typeArrayֻ���ڶ������,����Ҫ�������۵�ʱ��Ż��õ�
long CClassifier::SaveResults(CCatalogList &cataList, CString strFileName, CStringArray *aryType)
{
	FILE *stream;
	if( (stream = fopen(strFileName, "w+" )) == NULL )
	{
		CMessage::PrintError("�޷������������ļ�"+strFileName+"!");
		return -1;
	}

	CString str1,str2;
	long lCorrect=0;
	long docID=0;
	int i;
	char path[MAX_PATH];
	CArray<short,short> aryResult;
	CArray<short,short> aryAnswer;
	double dThreshold=(double)m_paramClassifier.m_dThreshold/100.0;

	POSITION pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& cataNode=cataList.GetNext(pos_cata);
		short id=cataNode.m_idxCata;
		strcpy(path,cataNode.m_strDirName.GetBuffer(0));
		POSITION pos_doc=cataNode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docNode=cataNode.GetNext(pos_doc);
			if(docNode.m_nCataID<0) continue;
			str1.Empty();
			str2.Empty();
			//����Ƕ������
			if(m_paramClassifier.m_nClassifyType==CClassifierParam::nFT_Multi)
			{
				MultiCategory(docNode,aryResult,dThreshold);
				//�����Ҫ��������������������Ŀ¼
				if(m_paramClassifier.m_bCopyFiles)
				{
					for(i=0;i<aryResult.GetSize();i++)
					{
						m_lstTrainCatalogList.GetCataName(aryResult[i],str1);
						str2=str2+str1+",";
						if(m_paramClassifier.m_bCopyFiles)
							CopyFile(docNode.m_strDocName.GetBuffer(0),path,
							m_paramClassifier.m_strResultDir.GetBuffer(0),str1.GetBuffer(0));
					}
					str2.SetAt(str2.GetLength()-1,' ');
				}
				//�����Ҫ�Է�������������
				if(m_paramClassifier.m_bEvaluation)
				{
					m_lstTrainCatalogList.GetCataIDArrayFromString(aryType->GetAt(docID).GetBuffer(0),aryAnswer);
					//�õ����ַ���
					for(i=0;i<aryAnswer.GetSize();i++)
					{
						str1.Format("%d",aryAnswer[i]);
						str2+=(str1+",");
					}
					str2.SetAt(str2.GetLength()-1,' ');
					fprintf(stream,"%d %s %s",docID,docNode.m_strDocName,str2);

					//�õ��������ַ���
					str2.Empty();
					for(i=0;i<aryResult.GetSize();i++)
					{
						str1.Format("%d",aryResult[i]);
						str2+=(str1+",");
					}
					str2=str2.Left(str2.GetLength()-1);
					fprintf(stream,"%s\n",str2);
				}
				else
				{
					if(str2.IsEmpty())
					{
						for(i=0;i<aryResult.GetSize();i++)
						{
							m_lstTrainCatalogList.GetCataName(aryResult[i],str1);
							str2=str2+str1+",";
						}
						str2.SetAt(str2.GetLength()-1,' ');
					}
					fprintf(stream,"%d\t%s\t\t%s\n",docID,docNode.m_strDocName,str2);
				}
			}
			//����ǵ������
			else
			{
				//�����Ҫ��������������������Ŀ¼
				if(m_paramClassifier.m_bCopyFiles)
				{
					m_lstTrainCatalogList.GetCataName(docNode.m_nCataID,str1);
					CopyFile(docNode.m_strDocName.GetBuffer(0),
						cataNode.m_strDirName.GetBuffer(0),
						m_paramClassifier.m_strResultDir.GetBuffer(0),
						str1.GetBuffer(0));
				}
				//�����Ҫ�Է�������������
				if(m_paramClassifier.m_bEvaluation)
				{
					if(docNode.m_nCataID==id) lCorrect++;
					fprintf(stream,"%d %s %d %d\n",docID,docNode.m_strDocName,
						cataNode.m_idxCata,docNode.m_nCataID);
				}
				else
				{
					if(str1.IsEmpty()) m_lstTrainCatalogList.GetCataName(docNode.m_nCataID,str1);
					fprintf(stream,"%d\t%s\t\t%s\n",docID,docNode.m_strDocName,str1);
				}
			}
			docID++;
		}
	}
	fclose(stream);
	return lCorrect;
}

//�����������浽�ļ�strFileName��,������ȷ������ĵ�����
//������������Ҫ�󿽱��ļ���������Ŀ¼����ִ�п�������
//����typeArrayֻ���ڶ������,����Ҫ�������۵�ʱ��Ż��õ�
long CClassifier::SaveResults_Query(CCatalogList &cataList, CString strFileName, CStringArray *aryType)
{
	FILE *stream;
	if( (stream = fopen(strFileName, "w+" )) == NULL )
	{
		CMessage::PrintError("�޷������������ļ�"+strFileName+"!");
		return -1;
	}

	CString str1,str2;
	long lCorrect=0;
	long qryID=0;
	int i;
	char path[MAX_PATH];
	CArray<short,short> aryResult;
	CArray<short,short> aryAnswer;
	double dThreshold=(double)m_paramClassifier.m_dThreshold/100.0;

	POSITION pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& cataNode=cataList.GetNext(pos_cata);
		short id=cataNode.m_idxCata;
		strcpy(path,cataNode.m_strDirName.GetBuffer(0));
		POSITION pos_qry=cataNode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			CQueryNode& qryNode=cataNode.GetNextQuery(pos_qry);
			if(qryNode.m_nCataID<0) continue;
			str1.Empty();
			str2.Empty();
			//����Ƕ������
			if(m_paramClassifier.m_nClassifyType==CClassifierParam::nFT_Multi)
			{
				MultiCategory_Query(qryNode,aryResult,dThreshold);
				//�����Ҫ��������������������Ŀ¼
				if(m_paramClassifier.m_bCopyFiles)
				{
					for(i=0;i<aryResult.GetSize();i++)
					{
						m_lstTrainCatalogList.GetCataName(aryResult[i],str1);
						str2=str2+str1+",";
						if(m_paramClassifier.m_bCopyFiles)
							CopyFile(qryNode.m_Text.GetBuffer(0),path,
							m_paramClassifier.m_strResultDir.GetBuffer(0),str1.GetBuffer(0));
					}
					str2.SetAt(str2.GetLength()-1,' ');
				}
				//�����Ҫ�Է�������������
				if(m_paramClassifier.m_bEvaluation)
				{
					m_lstTrainCatalogList.GetCataIDArrayFromString(aryType->GetAt(qryID).GetBuffer(0),aryAnswer);
					//�õ����ַ���
					for(i=0;i<aryAnswer.GetSize();i++)
					{
						str1.Format("%d",aryAnswer[i]);
						str2+=(str1+",");
					}
					str2.SetAt(str2.GetLength()-1,' ');
					fprintf(stream,"%d %s %s",qryID,qryNode.m_Text,str2);

					//�õ��������ַ���
					str2.Empty();
					for(i=0;i<aryResult.GetSize();i++)
					{
						str1.Format("%d",aryResult[i]);
						str2+=(str1+",");
					}
					str2=str2.Left(str2.GetLength()-1);
					fprintf(stream,"%s\n",str2);
				}
				else
				{
					if(str2.IsEmpty())
					{
						for(i=0;i<aryResult.GetSize();i++)
						{
							m_lstTrainCatalogList.GetCataName(aryResult[i],str1);
							str2=str2+str1+",";
						}
						str2.SetAt(str2.GetLength()-1,' ');
					}
					fprintf(stream,"%d\t%s\t\t%s\n",qryID,qryNode.m_Text,str2);
				}
			}
			//����ǵ������
			else
			{
				//�����Ҫ��������������������Ŀ¼
				if(m_paramClassifier.m_bCopyFiles)
				{
					m_lstTrainCatalogList.GetCataName(qryNode.m_nCataID,str1);
					CopyFile(qryNode.m_Text.GetBuffer(0),
						cataNode.m_strDirName.GetBuffer(0),
						m_paramClassifier.m_strResultDir.GetBuffer(0),
						str1.GetBuffer(0));
				}
				//�����Ҫ�Է�������������
				if(m_paramClassifier.m_bEvaluation)
				{
					if(qryNode.m_nCataID==id) lCorrect++;
					fprintf(stream,"%d %s %d %d\n",qryID,qryNode.m_Text,
						cataNode.m_idxCata,qryNode.m_nCataID);
				}
				else
				{
					if(str1.IsEmpty()) m_lstTrainCatalogList.GetCataName(qryNode.m_nCataID,str1);
					fprintf(stream,"%d\t%s\t\t%s\n",qryID,qryNode.m_Text,str1);
				}
			}
			qryID++;
		}
	}
	fclose(stream);
	return lCorrect;
}

void CClassifier::CopyFile(char *pFileName, char *pSource, char *pTarget, char *pCatalog)
{
	char targetFile[MAX_PATH];
	strcpy(targetFile,pTarget);
	strcat(targetFile,"\\");
	strcat(targetFile,pCatalog);
	if(_chdir(targetFile)<0)
		if(_mkdir(targetFile)<0) return;
	char sourceFile[MAX_PATH];
	strcpy(sourceFile,pSource);
	strcat(sourceFile,"\\");
	strcat(sourceFile,pFileName);
	strcat(targetFile,"\\");
	strcat(targetFile,pFileName);
	::CopyFile(sourceFile,targetFile,false);
}

void CClassifier::Evaluate(CString strPath)
{
	CString strFileName=strPath;
	strFileName=strFileName+"\\multieval.exe ";
	strFileName=strFileName+theClassifier.m_paramClassifier.m_strResultDir+"\\classes.txt ";
	strFileName=strFileName+theClassifier.m_paramClassifier.m_strResultDir+"\\results.txt";
	if(WinExec(strFileName,SW_SHOWNORMAL)<32)
		AfxMessageBox("������������򲻴���!");
}

short CClassifier::SingleCategory(CDocNode &docNode)
{
	short nCataID=-1;
	double *pSimRatio=docNode.m_pResults;
	//�õ��ĵ����������nMaxCatID
	double dMaxNum=pSimRatio[0];
	nCataID=0;
	for(int i=1;i<m_nClassNum;i++)
	{
		if(pSimRatio[i]>dMaxNum)
		{
			dMaxNum=pSimRatio[i];
			nCataID=i;
		}
	}
	docNode.m_nCataID=nCataID;
	return nCataID;
}

short CClassifier::SingleCategory_Query(CQueryNode &qryNode)
{
	short nCataID=-1;
	double *pSimRatio=qryNode.m_pResults;
	//�õ��ĵ����������nMaxCatID
	double dMaxNum=pSimRatio[0];
	nCataID=0;
	for(int i=1;i<m_nClassNum;i++)
	{
		if(pSimRatio[i]>dMaxNum)
		{
			dMaxNum=pSimRatio[i];
			nCataID=i;
		}
	}
	qryNode.m_nCataID=nCataID;
	return nCataID;
}


short CClassifier::SVMCategory(char *pPath, CDocNode &docNode, bool bFile)
{
	short nCataID=-1;
	if(SVMClassify(pPath,docNode,bFile)) nCataID=SingleCategory(docNode);
	return nCataID;
}

short CClassifier::SVMCategory(char *file, bool bFile)
{
	CDocNode docNode;
	short id=-1;
	if(bFile)
	{
		char *fname=strrchr(file,'\\');
		if(fname==NULL) return -1;
		docNode.m_strDocName=(fname+1);

		char path[MAX_PATH];
		strncpy(path,file,fname-file);
		path[fname-file]=0;
		id=SVMCategory(path,docNode,bFile);
	}
	else
		id=SVMCategory(file,docNode,bFile);
	return id;
}

void CClassifier::SVMClassifyVectorFile(CString strFileName)
{
	//Ϊ�˼������������������ÿ��������������������
	CTime startTime;
	CTimeSpan totalTime;
	CString str;
	long num=m_lstTestCatalogList.GetDocNum(),lDocNum=0;
	double *fpWeight=new double[num];
	POSITION pos_doc, pos_cata;

	m_theSVM.com_param.classifyfile=strFileName;
	for(int i=1;i<=m_nClassNum;i++)
	{
		memset(fpWeight,0,sizeof(double)*num);
		startTime=CTime::GetCurrentTime();
		str.Format("����ʹ�õ�%d��SVM���������ĵ����з���,���Ժ�...",i);
		CMessage::PrintInfo(str);
		str.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.com_param.modelfile=str;
		m_theSVM.svm_classify(i,fpWeight);
		//���ĵ��͵�ǰ�������ƶȸ���m_pResults[i-1]
		lDocNum=0;
		pos_cata=m_lstTestCatalogList.GetFirstPosition();
		while(pos_cata!=NULL)
		{
			CCatalogNode& catalognode=m_lstTestCatalogList.GetNext(pos_cata);
			pos_doc=catalognode.GetFirstPosition();
			while(pos_doc!=NULL)
			{
				CDocNode& docnode=catalognode.GetNext(pos_doc);
				docnode.AllocResultsBuffer(m_nClassNum);
				docnode.m_pResults[i-1]=fpWeight[lDocNum];
				lDocNum++;
			}
		}
		totalTime=CTime::GetCurrentTime()-startTime;
		str.Format("��%d��SVM�������������,��ʱ",i);
		CMessage::PrintInfo(str+totalTime.Format("%H:%M:%S"));
	}
	delete[] fpWeight;

	//������ĵ������ƶ��������
	pos_cata=m_lstTestCatalogList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=m_lstTestCatalogList.GetNext(pos_cata);
		pos_doc=catalognode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catalognode.GetNext(pos_doc);
			docnode.m_nCataID=SingleCategory(docnode);
		}
	}
}

void CClassifier::SVMClassifyVectorFile_Query(CString strFileName)
{
	//Ϊ�˼������������������ÿ��������������������
	CTime startTime;
	CTimeSpan totalTime;
	CString str;
	long num=m_lstTestCatalogList.GetQueryNum(),lDocNum=0;
	double *fpWeight=new double[num];
	POSITION pos_qry, pos_cata;

	m_theSVM.com_param.classifyfile=strFileName;
	for(int i=1;i<=m_nClassNum;i++)
	{
		memset(fpWeight,0,sizeof(double)*num);
		startTime=CTime::GetCurrentTime();
		str.Format("����ʹ�õ�%d��SVM���������ĵ����з���,���Ժ�...",i);
		CMessage::PrintInfo(str);
		str.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.com_param.modelfile=str;
		m_theSVM.svm_classify(i,fpWeight);
		//���ĵ��͵�ǰ�������ƶȸ���m_pResults[i-1]
		lDocNum=0;
		pos_cata=m_lstTestCatalogList.GetFirstPosition();
		while(pos_cata!=NULL)
		{
			CCatalogNode& catalognode=m_lstTestCatalogList.GetNext(pos_cata);
			pos_qry=catalognode.GetFirstQueryPosition();
			while(pos_qry!=NULL)
			{
				CQueryNode& qrynode=catalognode.GetNextQuery(pos_qry);
				qrynode.AllocResultsBuffer(m_nClassNum);
				qrynode.m_pResults[i-1]=fpWeight[lDocNum];
				lDocNum++;
			}
		}
		totalTime=CTime::GetCurrentTime()-startTime;
		str.Format("��%d��SVM�������������,��ʱ",i);
		CMessage::PrintInfo(str+totalTime.Format("%H:%M:%S"));
	}
	delete[] fpWeight;

	//������ĵ������ƶ��������
	pos_cata=m_lstTestCatalogList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catalognode=m_lstTestCatalogList.GetNext(pos_cata);
		pos_qry=catalognode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			CQueryNode& qrynode=catalognode.GetNextQuery(pos_qry);
			qrynode.m_nCataID=SingleCategory_Query(qrynode);
		}
	}
}


//�����ĵ�docNode����ÿһ�����ĸ���
void CClassifier::ComputePro(CDocNode &docNode)
{
	//�����ĵ���ѵ������ÿһ���ĵ��ĸ���
	int i,j,l,FeaNum=0;
	long k;
	
	int			N;				//�ܵ��ĵ���;
	int			N_c;			//C����ĵ���
	int			N_Cata;			//������
	N = m_lstTrainCatalogList.GetDocNum();
	
	POSITION	pos_cata;
	CString     strWord;

	// calculate the weight of each word to all catalog
	N = m_lstTrainCatalogList.GetDocNum();
	N_Cata=m_lstTrainCatalogList.GetCataNum();
	
	docNode.AllocResultsBuffer(m_nClassNum);

	int nCataCount=0;
	double ClassPro=0.0;
	pos_cata = m_lstTrainCatalogList.GetFirstPosition();

	i=m_lstTrainWordList.GetCount();
	POSITION pos=m_lstTrainWordList.GetFirstPosition();


	for(l=0;l<m_nClassNum;l++)
	{
		CCatalogNode& catanode = m_lstTrainCatalogList.GetNext(pos_cata);
		N_c  = catanode.GetDocNum();
		m_pProbability[l].dWeight=(1.0+N_c)/(N_Cata+N);
		m_pProbability[l].lDocID=l;
	}

	for(j=0;j<i;j++)
	{
		if(docNode.m_pTemp[j].s_tfi!=0) 
		{
			FeaNum++;
			CWordNode &WordProNode = m_lstTrainWordList.GetWordProByID(pos,j);
			for(l=0;l<m_nClassNum;l++)
			{
				m_pProbability[l].dWeight+=WordProNode.m_pCataWeightPro[l];
//				CString strtemp;
//				strtemp.Format("%lf  %d",m_pProbability[s].dWeight,s);
//				CMessage::PrintInfo(strtemp);
			}
		}
	}

	for(l=0;l<m_nClassNum;l++)
	{
		m_pProbability[l].dWeight/=FeaNum;
		if(ClassPro<m_pProbability[l].dWeight)
		{
			ClassPro = m_pProbability[l].dWeight;
			docNode.m_nCataID = l;
		}
	}

/*
	while(pos_cata!=NULL)  // for each catalog 
	{
		docNode.m_pResults[nCataCount]=0;
		CCatalogNode& catanode = m_lstTrainCatalogList.GetNext(pos_cata);
		N_c  = catanode.GetDocNum();

		m_pProbability[nCataCount].dWeight=(1.0+N_c)/(N_Cata+N);
		m_pProbability[nCataCount].lDocID=nCataCount;

//				for(int i=0;i<lWordNum;i++)
//			{
//				if(docnode.m_pTemp[i].s_tfi!=0) 
//					fprintf(stream," %d:%f",i+1,docnode.m_pTemp[i].s_dWeight);
//			}	

//		m_pProbability[nCataCount].dWeight+=docNode.ComputeProbability(m_lstTrainWordList,nCataCount);
//		CString strttemp;
//		strttemp.Format("%lf",m_pProbability[nCataCount].dWeight);
//		CMessage::PrintInfo(strttemp);
		POSITION pos=m_lstTrainWordList.GetFirstPosition();
		for(j=0;j<i;j++)
		{
			if(docNode.m_pTemp[j].s_tfi!=0) 
			{
				m_pProbability[nCataCount].dWeight+=m_lstTrainWordList.GetWordProByID(pos,j,nCataCount);
//				CString str = m_lstTrainWordList.GetWordByID(j);
//				m_pProbability[nCataCount].dWeight+=1;
//				CWordNode &wordnode = m_lstTrainWordList.m_lstWordList[str];
//				m_pProbability[nCataCount].dWeight+= wordnode.m_pCataWeightPro[nCataCount];
			}
		
//			i=docNode.m_sWeightSet[nCataCount].s_idxWord;
//			CString str = m_lstTrainWordList.GetWordByID(i);
//			CWordNode &wordnode = m_lstTrainWordList.m_lstWordList[str];
//			m_pProbability[nCataCount].dWeight*= wordnode.m_pCataWeightPro[nCataCount];
			
		}
		if(ClassPro<m_pProbability[nCataCount].dWeight)
		{
			ClassPro = m_pProbability[nCataCount].dWeight;
			docNode.m_nCataID = nCataCount;
//			CString strtemp;
//			strtemp.Format("%d  %lf  %lf",nCataCount,(1.0+N_c)/(N_Cata+N),ClassPro);
//			CMessage::PrintInfo(strtemp);	
		}
		nCataCount++;
	}
*/
}


//�����ĵ�docNode��������ʽ�����÷���ComputePro���������ÿһ���������ƶ�
//����bFileΪ�����pPathΪdocNode��·��,���������Ҫ���з�����ĵ�������
//����nCmpType�������ƶȵĲ�ͬ���㷽ʽ
bool CClassifier::BAYESClassify(char *pPath, CDocNode &docNode, bool bFile)
{
	int nCount=0;
	if(bFile)
	{
		if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
			nCount=docNode.ScanChineseWithDict(pPath,m_lstTrainWordList);
		else
			nCount=docNode.ScanEnglishWithDict(pPath,m_lstTrainWordList,
											m_paramClassifier.m_bStem);
	}
	else
	{
		if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
			nCount=docNode.ScanChineseStringWithDict(pPath,m_lstTrainWordList);
		else
			nCount=docNode.ScanEnglishStringWithDict(pPath,m_lstTrainWordList,
											m_paramClassifier.m_bStem);
	}

	if((m_lDocNum>0)&&(nCount>0))
	{
		ComputePro(docNode);
		return true;
	}
	else
		return false;
}


//�����ĵ���ÿ���������ƶȣ��������ĵ����ƶ��������ID
short CClassifier::BAYESCategory(char *pPath, CDocNode &docNode, bool bFile)
{
	BAYESClassify(pPath,docNode,bFile);
	return docNode.m_nCataID;
}

long CClassifier::BAYESClassify(CCatalogList &cataList)
{
	long docID=0,lUnknown=0;
	CString str;
	POSITION pos_cata=cataList.GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& cataNode=cataList.GetNext(pos_cata);
		POSITION pos_doc=cataNode.GetFirstPosition();
		char *path=cataNode.m_strDirName.GetBuffer(0);
		while(pos_doc!=NULL)
		{
			CDocNode& docNode=cataNode.GetNext(pos_doc);
			short id=BAYESCategory(path, docNode, true);
			if(id==-1) 
			{
				str="�޷�ʶ���ĵ�";
				str+=cataNode.m_strDirName;
				str+="\\"+docNode.m_strDocName+"�����!";
				CMessage::PrintError(str);
				lUnknown++;
			}
			CMessage::PrintStatusInfo(_T("ɨ���ĵ�")+docNode.m_strDocName);
		}
	}
	return lUnknown;
}





short CClassifier::GetCategory(char *file, bool bFile)
{
	short result=-1;
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
		result=KNNCategory(file,bFile);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
		result=SVMCategory(file,bFile);
	return result;
}

short CClassifier::GetCategory(char *path, CDocNode &docNode, bool bFile)
{
	short result=-1;
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
		result=KNNCategory(path,docNode,bFile);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
		result=SVMCategory(path,docNode,bFile);
	return result;
}

bool CClassifier::Classify(char *path, CDocNode &docNode, bool bFile)
{
	bool result=false;
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
		result=KNNClassify(path,docNode,bFile);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
		result=SVMClassify(path,docNode,bFile);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_BAYES)
		result=BAYESClassify(path,docNode,bFile);
	return result;
}

long CClassifier::Classify(CCatalogList &cataList)
{
	long lUnknown=0;
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
		lUnknown=KNNClassify(m_lstTestCatalogList);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
		lUnknown=SVMClassify(m_lstTestCatalogList);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_BAYES)
		lUnknown=BAYESClassify(m_lstTestCatalogList);
	else 
		CMessage::PrintError("�޷�ȷ��������������!");
	return lUnknown;
}

long CClassifier::Classify_Query(CCatalogList &cataList)
{
	long lUnknown=0;
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
		lUnknown=KNNClassify(m_lstTestCatalogList);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
		lUnknown=SVMClassify_Query(m_lstTestCatalogList);
	else if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_BAYES)
		lUnknown=BAYESClassify(m_lstTestCatalogList);
	else 
		CMessage::PrintError("�޷�ȷ��������������!");
	return lUnknown;
}