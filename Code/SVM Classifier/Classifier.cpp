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

//参数bGenDic=false代表无需重新扫描文档得到训练文档集中所有特征,一般在层次分类时使用
//参数nType用来决定分类模型的类别,nType=0代表KNN分类器,nType=1代表SVM分类器
bool CClassifier::Train(int nType, bool bFlag)
{
	this->n_Type=nType;
	CTime startTime;
	CTimeSpan totalTime;
	if(bFlag)
	{
		InitTrain();
		//生成所有候选特征项，将其保存在m_lstWordList中
		//GenDic();
		GenLogDic();
	}
	CMessage::PrintStatusInfo("");

	if(m_lstWordList.GetCount()==0)
		return false;
	if(m_lstTrainCatalogList.GetCataNum()==0)
		return false;

	//清空特征项列表m_lstTrainWordList
	m_lstTrainWordList.InitWordList();
	//为特征项列表m_lstWordList中的每个特征加权
	CMessage::PrintInfo(_T("开始计算候选特征集中每个特征的类别区分度，请稍候..."));
	startTime=CTime::GetCurrentTime();
	FeatherWeight(m_lstWordList);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("特征区分度计算结束，耗时")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

	//从特征项列表m_lstWordList中选出最优特征
	CMessage::PrintInfo(_T("开始进行特征选择，请稍候..."));
	startTime=CTime::GetCurrentTime();
	FeatherSelection(m_lstTrainWordList);
    //为最优特征集m_lstTrainWordList中的每个特征建立一个ID
	m_lstTrainWordList.IndexWord();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("特征选择结束，耗时")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

//	清空m_lstWordList,释放它占用的空间
	m_lstWordList.InitWordList();

	CMessage::PrintInfo("开始生成文档向量，请稍候...");
	startTime=CTime::GetCurrentTime();
	//GenModel();//////////////////////////////////////////////////////////////////////// 生成训练文档的特征向量
	GenQueryModel();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("文档向量生成结束，耗时")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("O(∩_∩)O~");

	CMessage::PrintInfo("开始保存分类模型，请稍候...");
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo("选择训练方式...");
	WriteModel(m_paramClassifier.m_txtResultDir+"\\model.prj",nType);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("保存分类模型结束，耗时")+totalTime.Format("%H:%M:%S"));

	//训练SVM分类器必须在保存训练文档的文档向量后进行
	if(nType == 1)
	{
		CMessage::PrintInfo("开始训练SVM，请稍候...");
		m_lstTrainCatalogList.InitCatalogList(2); //删除文档向量所占用的空间
		startTime=CTime::GetCurrentTime();
		TrainSVM();
		totalTime=CTime::GetCurrentTime()-startTime;
		CMessage::PrintInfo(_T("SVM分类器训练结束，耗时")+totalTime.Format("%H:%M:%S"));
		CMessage::PrintStatusInfo("");
	}
	//为分类做好准备,否则不能进行分类
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
		str.Format("正在训练第%d个SVM分类器，请稍侯...",i);
		CMessage::PrintInfo(str);
		m_theSVM.com_pro.Setup();
		m_theSVM.com_param.trainfile=m_paramClassifier.m_txtResultDir+"\\train.txt";
		m_theSVM.com_param.modelfile.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.svm_learn_main(i);
		tmSpan=CTime::GetCurrentTime()-tmStart;
		str.Format("第%d个SVM分类器训练完成，耗时%s!",i,tmSpan.Format("%H:%M:%S"));
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
		str.Format("正在训练第%d个SVM分类器，请稍侯...",i);
		CMessage::PrintInfo(str);
		m_theSVM.com_param.trainfile=m_paramClassifier.m_txtResultDir+"\\train.txt";
		m_theSVM.com_param.modelfile.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.svm_learn_main(i);
		tmSpan=CTime::GetCurrentTime()-tmStart;
		str.Format("第%d个SVM分类器训练完成，耗时%s!",i,tmSpan.Format("%H:%M:%S"));
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
		
		//拷贝词属于类的概率
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


//从m_lstWordList选出最优特征子集,存到dstWordList中
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
	//			拷贝词在不同类中的概率
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
	int			N;		//总的文档数;
	int			N_c;	//C类的文档数
	int			N_ft;	//含有ft的文档数
	int			N_c_ft;	//C类中含有ft的文档数
// ------------------------------------------------------------------------------
//  based on word number model
	long		N_W;    //总的词数					m_lWordNum;
	long		N_W_C;  //C类词数					CCatalogNode.m_lTotalWordNum;
	long		N_W_f_t; //f_t出现的总次数	
	long		N_W_C_f_t;//C类中f_t出现的次数
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

		CMessage::PrintStatusInfo("特征:"+strWord+"..."); 

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
			if(m_paramClassifier.m_nOpMode==CClassifierParam::nOpWordMode)//词频统计模式   
			{
				P_c	    = 1.0 * N_W_C /N_W;      //C类ft出现的概率
				P_ft	= 1.0 * N_W_f_t/N_W;     //ft出现的概率
				P_c_ft  = 1.0 * N_W_C_f_t/N_W;   //C类中ft出现的概率
			}
			else //if(m_paramClassifier.m_nOpMode==CClassifierParam::nOpDocMode)//文档频率统计模式
			{
				P_c			= 1.0 * N_c /N;		//C类出现的概率
				P_ft		= 1.0 * N_ft/N;		//含有ft的文档出现的概率
				P_c_ft		= 1.0 * N_c_ft/N;	//C类中含有ft的文档的概率
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
			wordnode.m_pCataWeightPro[nCataCount] = 1.0 * (N_c_ft+1)/(2+N);//词str属于类别nCataCount的概率
			
			//CString strtemp;
			//strtemp.Format("第%d个类中，词的权重是%lf",nCataCount,wordnode.m_pCataWeight[nCataCount]);
			//CMessage::PrintInfo(strtemp);
			
			nCataCount++;				
		}
		ASSERT(nCataCount==nTotalCata);
	}
	CMessage::PrintStatusInfo("");
}

//计算每一篇训练文档向量的每一维的权重
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
		//取类列表中的每一个类
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

			CMessage::PrintStatusInfo("计算文档"+docNode.m_strDocName+"向量每一维的权重");
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
// bFlag=false 层次分类的时候使用
void CClassifier::GenModel()
{
	CDocNode::AllocTempBuffer(m_lstTrainWordList.GetCount());
	POSITION pos_cata = m_lstTrainCatalogList.GetFirstPosition();
	while(pos_cata != NULL)  // for each catalog 
	{
		//取类列表中的每一个类
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
			CMessage::PrintStatusInfo("生成文档"+docNode.m_strDocName+"的文档向量");
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
		//取类列表中的每一个类
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
			CMessage::PrintStatusInfo("生成查询"+qryNode.m_Text+"的查询向量");
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
	CMessage::PrintInfo(_T("分词程序初始化，请稍候..."));	
	if(!g_wordSeg.InitWorgSegment(theApp.m_strPath.GetBuffer(0),m_paramClassifier.m_nLanguageType))
	{
		CMessage::PrintError(_T("分词程序初始化失败!"));
		return false;
	}
	if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
		g_wordSeg.SetSegSetting(CWordSegment::uPlace);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("分词程序初始化结束，耗时")+totalTime.Format("%H:%M:%S"));


	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("开始扫描训练文档，请稍候..."));
	if(m_lstTrainCatalogList.BuildLib(m_paramClassifier.m_txtTrainDir)<=0)
	{
		CMessage::PrintError("训练文档的总数为0!");
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
			CMessage::PrintStatusInfo(_T("扫描文档")+docnode.m_strDocName);

			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				nCount=docnode.ScanChinese(catalognode.m_strDirName.GetBuffer(0),
							m_lstWordList,nCataNum,catalognode.m_idxCata);
			else
				nCount=docnode.ScanEnglish(catalognode.m_strDirName.GetBuffer(0),
							m_lstWordList,nCataNum,catalognode.m_idxCata,
							m_paramClassifier.m_bStem);
			if(nCount==0)
			{
				CMessage::PrintError("文件"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"无内容!");
				continue;
			}
			else if(nCount<0)
			{
				CMessage::PrintError("文件"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"无法打开!");
				continue;
			}
			catalognode.m_lTotalWordNum+=nCount;// information collection point
			//temp=nCount;
			//itoa(nCount,s,10);
			s.Format("%d",nCount);

			//CMessage::PrintInfo("文件"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"分词数："+s);
		}
	}
	g_wordSeg.FreeWordSegment();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("扫描训练文档结束，耗时")+totalTime.Format("%H:%M:%S"));
	return true;
}

bool CClassifier::GenLogDic()
{
	m_lstWordList.InitWordList();
	CTime startTime;
	CTimeSpan totalTime;

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("分词程序初始化，请稍候..."));	
	if(!g_wordSeg.InitWorgSegment(theApp.m_strPath.GetBuffer(0),m_paramClassifier.m_nLanguageType))
	{
		CMessage::PrintError(_T("分词程序初始化失败!"));
		return false;
	}
	if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
		g_wordSeg.SetSegSetting(CWordSegment::uPlace);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("分词程序初始化结束，耗时")+totalTime.Format("%H:%M:%S"));


	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("开始扫描训练文档，请稍候..."));
	if(m_lstTrainCatalogList.BuildLogLib(m_paramClassifier.m_txtTrainDir,m_paramClassifier.m_nUKS_k,m_paramClassifier.m_nURL)<=0)/////////xby
	{
		CMessage::PrintError("训练文档的总数为0!");
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
			CMessage::PrintStatusInfo(_T("扫描查询")+querynode.m_idxQuery);

			/////读日志文件中的每条日志记录

			if(m_paramClassifier.m_nLanguageType==CClassifierParam::nLT_Chinese)
				nCount=querynode.ScanChineseText(querynode.m_Text,
							m_lstWordList,nCataNum,catalognode.m_idxCata);
			else
				nCount=querynode.ScanEnglishText(catalognode.m_strDirName.GetBuffer(0),
							m_lstWordList,nCataNum,catalognode.m_idxCata,
							m_paramClassifier.m_bStem);
			if(nCount==0)
			{
				CMessage::PrintError("查询"+catalognode.m_strDirName+"\\"+querynode.m_Text+"无内容!");
				continue;
			}
			else if(nCount<0)
			{
				CMessage::PrintError("查询"+catalognode.m_strDirName+"\\"+querynode.m_Text+"无法打开!");
				continue;
			}
			catalognode.m_lTotalWordNum+=nCount;// information collection point
			//temp=nCount;
			//itoa(nCount,s,10);
			s.Format("%d",nCount);

			//CMessage::PrintInfo("文件"+catalognode.m_strDirName+"\\"+docnode.m_strDocName+"分词数："+s);
		}
	}
	g_wordSeg.FreeWordSegment();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("扫描训练文档结束，耗时")+totalTime.Format("%H:%M:%S"));
	return true;
}

void CClassifier::InitTrain()
{
	m_lstTrainWordList.InitWordList();
	m_lstTrainCatalogList.InitCatalogList();
	m_lstWordList.InitWordList();
}

//参数nType用来决定分类模型的类别,nType=0代表KNN分类器,nType=1代表SVM分类器
bool CClassifier::WriteModel(CString strFileName, int nType)
{
	CFile fOut;
	if( !fOut.Open(strFileName,CFile::modeCreate | CFile::modeWrite) )
	{
		CMessage::PrintError("无法创建文件g"+strFileName+"!");
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
		CMessage::PrintError("无法打开文件"+strFileName+"!") ;
		return false;
	}
	CTime startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在打开分类模型文件，请稍候..."));


	CArchive ar(&fIn,CArchive::load);
	CString str,strPath;
	DWORD dwFileID;
	
	//读入文件格式标识符
	strPath=strFileName.Left(strFileName.ReverseFind('\\'));
	ar>>dwFileID;
	if(dwFileID!=dwModelFileID)
	{
		ar.Close();
		fIn.Close();
		CMessage::PrintError("分类模型文件的格式不正确!");
		return false;
	}

	ar>>str;

	if(!m_paramClassifier.GetFromFile(strPath+"\\"+str))
	{
		CMessage::PrintError(_T("无法打开训练参数文件"+str+"!"));
		return false;
	}
	m_paramClassifier.m_txtResultDir=strPath;

	if(m_paramClassifier.m_nClassifierType==0)
	{
		ar>>str;
		m_lstTrainWordList.InitWordList();
		if(!m_lstTrainWordList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开特征类表文件"+str+"!"));
			return false;
		}
		ar>>str;
		m_lstTrainCatalogList.InitCatalogList();
		if(!m_lstTrainCatalogList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开训练文档列表文件"+str+"!"));
			return false;
		}
	}
	else if(m_paramClassifier.m_nClassifierType==1)
	{
		ar>>str;
		m_lstTrainWordList.InitWordList();
		if(!m_lstTrainWordList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开特征类表文件"+str+"!"));
			return false;
		}
		//对于SVM分类起来说m_lstTrainCatalogList其实没用
		//此处读入它只是为了在CLeftViw中显示某些统计信息时使用
		ar>>str;
		m_lstTrainCatalogList.InitCatalogList();
		if(!m_lstTrainCatalogList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开训练文档列表文件"+str+"!"));
			return false;
		}
		ar>>str;
		if(!m_theSVM.com_param.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开SVM训练参数文件"+str+"!"));
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
			CMessage::PrintError(_T("无法打开特征类表文件"+str+"!"));
			return false;
		}
		ar>>str;
		m_lstTrainCatalogList.InitCatalogList();
		if(!m_lstTrainCatalogList.GetFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开训练文档列表文件"+str+"!"));
			return false;
		}
		ar>>str;
		if(!m_lstTrainWordList.GetProFromFile(strPath+"\\"+str))
		{
			CMessage::PrintError(_T("无法打开特征词类属概率文件"+str+"!"));
			return false;
		}
	}
	ar.Close();
	fIn.Close();
	Prepare();
	CTimeSpan totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("分类模型文件已经打开，耗时")+totalTime.Format("%H:%M:%S")+"\r\n");
	
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
	CMessage::PrintInfo(_T("正在扫描测试文档，请稍候O(∩_∩)O哈哈~..."));
	if(m_paramClassifier.m_bEvaluation)
	{
		//BuildLib方法中已经清空了g_lstTestCatalogList，所以此处无需再对其初始化
		m_lstTestCatalogList.BuildLib(m_paramClassifier.m_strTestDir);
		if(!m_lstTestCatalogList.BuildCatalogID(m_lstTrainCatalogList))
		{
			CMessage::PrintError("测试文件中包含有无法识别的类别!");
			return false;
		}
		CMessage::PrintInfo(_T("O(∩_∩)O哈哈~..."));
	}
	else
	{
		m_lstTestCatalogList.InitCatalogList();
		CCatalogNode catalognode;
		catalognode.m_strDirName=m_paramClassifier.m_strTestDir;
		catalognode.m_strCatalogName="测试文档";
		catalognode.m_idxCata=-1;
		POSITION posTemp=m_lstTestCatalogList.AddCata(catalognode);
		CCatalogNode& cataTemp=m_lstTestCatalogList.GetAt(posTemp);
		cataTemp.SetStartDocID(0);
		cataTemp.ScanDirectory(m_paramClassifier.m_strTestDir);
	}
	//if(m_lstTestCatalogList.GetDocNum()<=0)
	if(m_lstTestCatalogList.GetQueryNum()<=0)
	{
		CMessage::PrintError("测试文件总数为0!\r\n如果不需要对分类结果进行评价时,分类文档必须在\"分类文档目录\"下,而不是它的子目录下!");
		return false;
	}
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("扫描测试文档结束，耗时")+totalTime.Format("%H:%M:%S"));

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在对测试文档进行分类，请稍候..."));
	long lCorrect=0,lUnknown=0;
	lUnknown=Classify(m_lstTestCatalogList);


	lCorrect=SaveResults(m_lstTestCatalogList,m_paramClassifier.m_strResultDir+"\\results.txt");
	long lTotalNum=m_lstTestCatalogList.GetDocNum()-lUnknown;
	CString str;
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("测试文档分类结束，耗时")+totalTime.Format("%H:%M:%S"));
	if (lUnknown>0) 
	{
		str.Format("无法分类的文档数%d:",lUnknown);
		CMessage::PrintInfo(str);
	}
	if(m_paramClassifier.m_bEvaluation&&lTotalNum>0&&lCorrect>0)
		str.Format("测试文档总数:%d,准确率:%f",m_lstTestCatalogList.GetDocNum(),(float)(lCorrect)/(float)(lTotalNum));
	else
		str.Format("测试文档总数:%d",m_lstTestCatalogList.GetDocNum());
	CMessage::PrintInfo(str);
	return true;
}

bool CClassifier::Classify_Query()
{
	m_lstTrainCatalogList.DumpCataList(m_paramClassifier.m_strResultDir+"\\classes.txt");
	CTime startTime;
	CTimeSpan totalTime;
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在扫描测试文档，请稍候O(∩_∩)O哈哈~~..."));
	if(m_paramClassifier.m_bEvaluation)
	{
		//BuildLib方法中已经清空了g_lstTestCatalogList，所以此处无需再对其初始化
		m_lstTestCatalogList.BuildLogLib(m_paramClassifier.m_strTestDir,m_paramClassifier.m_nUKS_k,m_paramClassifier.m_nURL);
		if(!m_lstTestCatalogList.BuildCatalogID(m_lstTrainCatalogList))
		{
			CMessage::PrintError("测试文件中包含有无法识别的类别!");
			return false;
		}
		CMessage::PrintInfo(_T("O(∩_∩)O哈哈~..."));
	}
	else
	{
		m_lstTestCatalogList.InitCatalogList();
		CCatalogNode catalognode;
		catalognode.m_strDirName=m_paramClassifier.m_strTestDir;
		catalognode.m_strCatalogName="测试文档";
		catalognode.m_idxCata=-1;
		POSITION posTemp=m_lstTestCatalogList.AddCata(catalognode);
		CCatalogNode& cataTemp=m_lstTestCatalogList.GetAt(posTemp);
		cataTemp.SetStartDocID(0);
		cataTemp.ScanDirectory(m_paramClassifier.m_strTestDir);
	}
	//if(m_lstTestCatalogList.GetDocNum()<=0)
	if(m_lstTestCatalogList.GetQueryNum()<=0)
	{
		CMessage::PrintError("测试文件总数为0!\r\n如果不需要对分类结果进行评价时,分类文档必须在\"分类文档目录\"下,而不是它的子目录下!");
		return false;
	}
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("扫描测试文档结束，耗时")+totalTime.Format("%H:%M:%S"));

	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在对测试文档进行分类，请稍候..."));
	long lCorrect=0,lUnknown=0;
	lUnknown=Classify_Query(m_lstTestCatalogList);


	lCorrect=SaveResults_Query(m_lstTestCatalogList,m_paramClassifier.m_strResultDir+"\\results.txt");
	long lTotalNum=m_lstTestCatalogList.GetQueryNum()-lUnknown;
	CString str;
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("测试文档分类结束，耗时")+totalTime.Format("%H:%M:%S"));
	if (lUnknown>0) 
	{
		str.Format("无法分类的文档数%d:",lUnknown);
		CMessage::PrintInfo(str);
	}
	if(m_paramClassifier.m_bEvaluation&&lTotalNum>0&&lCorrect>0)
		str.Format("测试文档总数:%d,准确率:%f",m_lstTestCatalogList.GetQueryNum(),(float)(lCorrect)/(float)(lTotalNum));
	else
		str.Format("测试文档总数:%d",m_lstTestCatalogList.GetQueryNum());
	CMessage::PrintInfo(str);
	return true;
}

//对Smart格式的文档进行分类
bool CClassifier::ClassifySmart()
{
	m_lstTrainCatalogList.DumpCataList(m_paramClassifier.m_strResultDir+"\\classes.txt");
	m_lstTestCatalogList.InitCatalogList();
	CCatalogNode catalognode;
	catalognode.m_strDirName=m_paramClassifier.m_strTestDir;
	catalognode.m_strCatalogName="测试文档";
	catalognode.m_idxCata=-1;
	POSITION posTemp=m_lstTestCatalogList.AddCata(catalognode);
	CCatalogNode& cataTemp=m_lstTestCatalogList.GetAt(posTemp);

	FILE *stream1,*stream2;
	if( (stream1 = fopen( m_paramClassifier.m_strTestDir, "r" )) == NULL )
	{
		CMessage::PrintError("无法打开文件"+m_paramClassifier.m_strTestDir+"!");
		return false;
	}

	//如果是SVM分类器,则需要先将所有测试文档转换为向量,保存到文件test.dat
	if(m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
	{
		m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.dat";
		if((stream2=fopen(m_theSVM.com_param.classifyfile,"w"))==NULL)
		{
			CMessage::PrintError("无法创建测试文档向量文件"+m_theSVM.com_param.classifyfile+"!");
			return false;
		}
	}

	CTime startTime;
	CTimeSpan totalTime;
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在对测试文档进行分类，请稍候..."));

	char fname[10],type[1024],line[4096],content[100*1024];
	//falg=1 下一行的内容是文档的类别
	//flag=2 下一行的内容是文档的标题
	//flag=3 下一行的内容是文档的内容
	int flag=0,nCount,len,i;
	long lUnknown=0,lDocNum=0;
	CStringArray typeArray;
	CString strFileName,strCopyFile;
	bool bTitle=false; //是否已经读出标题
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
					CMessage::PrintError("无法识别文档"+docnode.m_strDocName+"的类别!");
					lUnknown++;
				}
				CMessage::PrintStatusInfo(_T("扫描文档")+docnode.m_strDocName);
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
		CMessage::PrintInfo(_T("正在使用SVM分类器对文档进行分类,请稍候..."));
		m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.dat";
		SVMClassifyVectorFile(m_theSVM.com_param.classifyfile);
		totalTime=CTime::GetCurrentTime()-startTime;
		CMessage::PrintInfo(_T("SVM分类过程结束，耗时")+totalTime.Format("%H:%M:%S"));
	}

	long lCorrect=SaveResults(m_lstTestCatalogList,m_paramClassifier.m_strResultDir+"\\results.txt",&typeArray);
	long lTotalNum=m_lstTestCatalogList.GetDocNum()-lUnknown;
	CString str;
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("测试文档分类结束，耗时")+totalTime.Format("%H:%M:%S"));
	if (lUnknown>0) 
	{
		str.Format("无法分类的文档数%d:",lUnknown);
		CMessage::PrintInfo(str);
	}
	if(m_paramClassifier.m_bEvaluation&&lTotalNum>0&&lCorrect>0)
		str.Format("测试文档总数:%d,准确率:%f",m_lstTestCatalogList.GetDocNum(),(float)(lCorrect)/(float)(lTotalNum));
	else
		str.Format("测试文档总数:%d",m_lstTestCatalogList.GetDocNum());
	CMessage::PrintInfo(str);
	return true;
}

//对文档进行分类，计算文档和每个类别的相似度，返回值为类别无法识别的文档总数
long CClassifier::SVMClassify(CCatalogList &cataList)
{
	long lUnknown=0;
	FILE *stream;
	m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.dat";
	if((stream=fopen(m_theSVM.com_param.classifyfile,"w"))==NULL)
	{
		CMessage::PrintError("无法创建测试文档向量文件"+m_theSVM.com_param.classifyfile+"!");
		return 0;
	}
	
	CTime startTime;
	CTimeSpan totalTime;
	CString str;
	int nCount=0;
	long lWordNum=m_lstTrainWordList.GetCount();
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在生成测试文档的向量形式，请稍候..."));
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
			CMessage::PrintStatusInfo(_T("扫描文档")+docnode.m_strDocName);
		
			if(nCount<0)
			{
				str="无法识别文档";
				str+=catalognode.m_strDirName;
				str+="\\"+docnode.m_strDocName+"的类别!";
				lUnknown++;
				CMessage::PrintError(str);
			}
		}
	}
	CMessage::PrintStatusInfo(_T(""));
	fclose(stream);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("测试文档的向量生成结束，耗时")+totalTime.Format("%H:%M:%S"));
	
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在使用SVM分类器对文档进行分类,请稍候..."));
	//为每篇文档和各个类别的相似读数组分配内存
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
	CMessage::PrintInfo(_T("SVM分类过程结束，耗时")+totalTime.Format("%H:%M:%S"));
	return lUnknown;
}

//对文档进行分类，计算文档和每个类别的相似度，返回值为类别无法识别的文档总数
long CClassifier::SVMClassify_Query(CCatalogList &cataList)
{
	long lUnknown=0;
	FILE *stream;
	int sum=0;
	m_theSVM.com_param.classifyfile=m_paramClassifier.m_strResultDir+"\\test.txt";
	if((stream=fopen(m_theSVM.com_param.classifyfile,"w"))==NULL)
	{
		CMessage::PrintError("无法创建测试文档向量文件"+m_theSVM.com_param.classifyfile+"!");
		return 0;
	}
	
	CTime startTime;
	CTimeSpan totalTime;
	CString str;
	int nCount=0;
	long lWordNum=m_lstTrainWordList.GetCount();
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在生成测试文档的向量形式，请稍候..."));
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
				 fprintf(stream," %d:%f",++sum,qrynode.nCS);//nCS值
			   }
			   if(m_paramClassifier.m_nRS)
			   {
			     fprintf(stream," %d:%lf",++sum,qrynode.nRS);//nRS值
				}
			   
				if(m_paramClassifier.m_nUKS)
				{
	             fprintf(stream," %d:%f",++sum,qrynode.UKS_N);//UKS_N 
				 fprintf(stream," %d:%f",++sum,qrynode.UKS_I);//UKS_I
				 fprintf(stream," %d:%f",++sum,qrynode.UKS_T);//UKS_T
				}
				if(m_paramClassifier.m_Length)
			   {
			     fprintf(stream," %d:%lf",++sum,qrynode.Length);//Length值
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
			fprintf(stream," %d:%f",1,qrynode.nCS);//test的nCS
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
			CMessage::PrintStatusInfo(_T("扫描查询")+qrynode.m_Text);
		
			if(nCount<0)
			{
				str="无法识别查询";
				str+=catalognode.m_strDirName;
				str+="\\"+qrynode.m_Text+"的类别!";
				lUnknown++;
				CMessage::PrintError(str);
			}
		}
	}
	CMessage::PrintStatusInfo(_T(""));
	fclose(stream);
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("测试文档的向量生成结束，耗时")+totalTime.Format("%H:%M:%S"));
	
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("正在使用SVM分类器对文档进行分类,请稍候..."));
	//为每篇文档和各个类别的相似度数组分配内存
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
	CMessage::PrintInfo(_T("SVM分类过程结束，耗时")+totalTime.Format("%H:%M:%S"));
	return lUnknown;
}

//使用KNN方法对文档进行分类，计算文档和每个类别的相似度
//返回值为类别无法识别的文档总数
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
				str="无法识别文档";
				str+=cataNode.m_strDirName;
				str+="\\"+docNode.m_strDocName+"的类别!";
				CMessage::PrintError(str);
				lUnknown++;
			}
			CMessage::PrintStatusInfo(_T("扫描文档")+docNode.m_strDocName);
		}
	}
	return lUnknown;
}

//计算文档和每个类别的相似度，返回与文档相似度最大的类别ID
//nCmpType代表相似度的不同计算方式
short CClassifier::KNNCategory(char *pPath, CDocNode &docNode, bool bFile, int nCmpType)
{
	short nCataID=-1;
	if(KNNClassify(pPath,docNode,bFile,nCmpType)) nCataID=SingleCategory(docNode);
	return nCataID;
}

//如果bFile为真,则参数file为文件的文件名称(包括它的路径)
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

//生成文档docNode的向量形式，调用方法ComputeSimRatio计算器其和每一个类别的相似度
//参数bFile为真代表pPath为docNode的路径,否则代表需要进行分类的文档的内容
//参数nCmpType代表相似度的不同计算方式
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

//得到与文档docNode的相似度大于阈值dThreshold的所有类别
//如果没有大于值阈值的类别，则返回相似度最大的类别
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

//得到与文档docNode的相似度大于阈值dThreshold的所有类别
//如果没有大于值阈值的类别，则返回相似度最大的类别
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

//计算文档docNode和每一个类别的相似度
//nCmpType代表相似度的不同计算方式
void CClassifier::ComputeSimRatio(CDocNode &docNode,int nCmpType)
{
	//计算文档与训练集中每一篇文档的相似度
	int i;
	long k;
	for(i=0;i<m_lDocNum;i++)
	{
		m_pSimilarityRatio[i].dWeight=m_pDocs[i].pDocNode->ComputeSimilarityRatio();
		m_pSimilarityRatio[i].lDocID=i;
	}
	//将测试文档与训练文档集中文档的相似度进行降序排序
	Sort(m_pSimilarityRatio,m_lDocNum-1);
	docNode.AllocResultsBuffer(m_nClassNum);
	double *pSimRatio=docNode.m_pResults;
	for(i=0;i<m_nClassNum;i++) pSimRatio[i]=0;
	if(nCmpType<=0)
	{
		//计算出测试文档的k近邻在每个类别中的数目
		for(i=0;i<m_paramClassifier.m_nKNN;i++)
		{
			k=m_pSimilarityRatio[i].lDocID;
			k=m_pDocs[k].nCataID;
			pSimRatio[k]+=1;
		}
		//按照"测试文档的k近邻在某个类别中的数目/k"得到测试文档和这个类别的相似度
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

//将分类结果保存到文件strFileName中,返回正确分类的文档总数
//如果分类参数中要求拷贝文件到结果类别目录，则执行拷贝操作
//参数typeArray只有在多类分类,且需要进行评价的时候才会用到
long CClassifier::SaveResults(CCatalogList &cataList, CString strFileName, CStringArray *aryType)
{
	FILE *stream;
	if( (stream = fopen(strFileName, "w+" )) == NULL )
	{
		CMessage::PrintError("无法创建分类结果文件"+strFileName+"!");
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
			//如果是多类分类
			if(m_paramClassifier.m_nClassifyType==CClassifierParam::nFT_Multi)
			{
				MultiCategory(docNode,aryResult,dThreshold);
				//如果需要将分类结果拷贝到分类结果目录
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
				//如果需要对分类结果进行评价
				if(m_paramClassifier.m_bEvaluation)
				{
					m_lstTrainCatalogList.GetCataIDArrayFromString(aryType->GetAt(docID).GetBuffer(0),aryAnswer);
					//得到答案字符串
					for(i=0;i<aryAnswer.GetSize();i++)
					{
						str1.Format("%d",aryAnswer[i]);
						str2+=(str1+",");
					}
					str2.SetAt(str2.GetLength()-1,' ');
					fprintf(stream,"%d %s %s",docID,docNode.m_strDocName,str2);

					//得到分类结果字符串
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
			//如果是单类分类
			else
			{
				//如果需要将分类结果拷贝到分类结果目录
				if(m_paramClassifier.m_bCopyFiles)
				{
					m_lstTrainCatalogList.GetCataName(docNode.m_nCataID,str1);
					CopyFile(docNode.m_strDocName.GetBuffer(0),
						cataNode.m_strDirName.GetBuffer(0),
						m_paramClassifier.m_strResultDir.GetBuffer(0),
						str1.GetBuffer(0));
				}
				//如果需要对分类结果进行评价
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

//将分类结果保存到文件strFileName中,返回正确分类的文档总数
//如果分类参数中要求拷贝文件到结果类别目录，则执行拷贝操作
//参数typeArray只有在多类分类,且需要进行评价的时候才会用到
long CClassifier::SaveResults_Query(CCatalogList &cataList, CString strFileName, CStringArray *aryType)
{
	FILE *stream;
	if( (stream = fopen(strFileName, "w+" )) == NULL )
	{
		CMessage::PrintError("无法创建分类结果文件"+strFileName+"!");
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
			//如果是多类分类
			if(m_paramClassifier.m_nClassifyType==CClassifierParam::nFT_Multi)
			{
				MultiCategory_Query(qryNode,aryResult,dThreshold);
				//如果需要将分类结果拷贝到分类结果目录
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
				//如果需要对分类结果进行评价
				if(m_paramClassifier.m_bEvaluation)
				{
					m_lstTrainCatalogList.GetCataIDArrayFromString(aryType->GetAt(qryID).GetBuffer(0),aryAnswer);
					//得到答案字符串
					for(i=0;i<aryAnswer.GetSize();i++)
					{
						str1.Format("%d",aryAnswer[i]);
						str2+=(str1+",");
					}
					str2.SetAt(str2.GetLength()-1,' ');
					fprintf(stream,"%d %s %s",qryID,qryNode.m_Text,str2);

					//得到分类结果字符串
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
			//如果是单类分类
			else
			{
				//如果需要将分类结果拷贝到分类结果目录
				if(m_paramClassifier.m_bCopyFiles)
				{
					m_lstTrainCatalogList.GetCataName(qryNode.m_nCataID,str1);
					CopyFile(qryNode.m_Text.GetBuffer(0),
						cataNode.m_strDirName.GetBuffer(0),
						m_paramClassifier.m_strResultDir.GetBuffer(0),
						str1.GetBuffer(0));
				}
				//如果需要对分类结果进行评价
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
		AfxMessageBox("分类结果评测程序不存在!");
}

short CClassifier::SingleCategory(CDocNode &docNode)
{
	short nCataID=-1;
	double *pSimRatio=docNode.m_pResults;
	//得到文档的所属类别nMaxCatID
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
	//得到文档的所属类别nMaxCatID
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
	//为了计算分类结果，用来保存每个分类器分类结果的数组
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
		str.Format("正在使用第%d个SVM分类器对文档进行分类,请稍候...",i);
		CMessage::PrintInfo(str);
		str.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.com_param.modelfile=str;
		m_theSVM.svm_classify(i,fpWeight);
		//将文档和当前类别的相似度赋给m_pResults[i-1]
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
		str.Format("第%d个SVM分类器分类结束,耗时",i);
		CMessage::PrintInfo(str+totalTime.Format("%H:%M:%S"));
	}
	delete[] fpWeight;

	//计算和文档的相似度最大的类别
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
	//为了计算分类结果，用来保存每个分类器分类结果的数组
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
		str.Format("正在使用第%d个SVM分类器对文档进行分类,请稍候...",i);
		CMessage::PrintInfo(str);
		str.Format("%s\\%s%d.mdl",m_paramClassifier.m_txtResultDir,m_paramClassifier.m_strModelFile,i);
		m_theSVM.com_param.modelfile=str;
		m_theSVM.svm_classify(i,fpWeight);
		//将文档和当前类别的相似度赋给m_pResults[i-1]
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
		str.Format("第%d个SVM分类器分类结束,耗时",i);
		CMessage::PrintInfo(str+totalTime.Format("%H:%M:%S"));
	}
	delete[] fpWeight;

	//计算和文档的相似度最大的类别
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


//计算文档docNode属于每一个类别的概率
void CClassifier::ComputePro(CDocNode &docNode)
{
	//计算文档与训练集中每一类文档的概率
	int i,j,l,FeaNum=0;
	long k;
	
	int			N;				//总的文档数;
	int			N_c;			//C类的文档数
	int			N_Cata;			//总类数
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


//生成文档docNode的向量形式，调用方法ComputePro计算器其和每一个类别的相似度
//参数bFile为真代表pPath为docNode的路径,否则代表需要进行分类的文档的内容
//参数nCmpType代表相似度的不同计算方式
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


//计算文档和每个类别的相似度，返回与文档相似度最大的类别ID
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
				str="无法识别文档";
				str+=cataNode.m_strDirName;
				str+="\\"+docNode.m_strDocName+"的类别!";
				CMessage::PrintError(str);
				lUnknown++;
			}
			CMessage::PrintStatusInfo(_T("扫描文档")+docNode.m_strDocName);
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
		CMessage::PrintError("无法确定分类器的类型!");
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
		CMessage::PrintError("无法确定分类器的类型!");
	return lUnknown;
}