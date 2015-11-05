// ClassifierParam.cpp: implementation of the CClassifierParam class.
//
//////////////////////////////////////////////////////////////////////
//分类器参数
#include "stdafx.h"
#include "ClassifierParam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const int CClassifierParam::nOpDocMode  = 0;   // based on document number model
const int CClassifierParam::nOpWordMode = 1;   // based on word number model

const int CClassifierParam::nFS_IGMode  = 0;   // Information gain feature selection
const int CClassifierParam::nFS_MIMode  = 1;   // Mutual Informaiton feature selection
const int CClassifierParam::nFS_CEMode  = 2;   // Cross Entropy for text feature selection
const int CClassifierParam::nFS_X2Mode  = 3;   // X^2 Statistics feature selection
const int CClassifierParam::nFS_WEMode  = 4;   // Weight of Evielence for text feature selection
const int CClassifierParam::nFS_XXMode  = 5;   // Right half of IG

const int CClassifierParam::nFSM_GolbalMode=0; // 全局选
const int CClassifierParam::nFSM_IndividualModel=1; // 单独选

const int CClassifierParam::nCT_Unknown=-1;    // Unknown
const int CClassifierParam::nCT_KNN=0;         // KNN
const int CClassifierParam::nCT_SVM=1;         // SVM
const int CClassifierParam::nCT_BAYES=2;	   //BAYES	

const int CClassifierParam::nLT_Chinese=0;     // Chinese
const int CClassifierParam::nLT_English=1;     // English

const int CClassifierParam::nDF_Directory=0;   // Directory
const int CClassifierParam::nDF_Smart=1;       // Smart

const int CClassifierParam::nFT_Single=0;      // Single Classification
const int CClassifierParam::nFT_Multi=1;       // Multiple Classification

const int CClassifierParam::nWM_TF_IDF=0;      // TF*IDF
const int CClassifierParam::nWM_TF_DIFF=3;     // TF*DIFF
const int CClassifierParam::nWM_TF_IDF_DIFF=2; // TF*IDF*DIFF

const int CClassifierParam::nWM_nCS_nRS=1;//xby

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClassifierParam::CClassifierParam()
{
	//训练时需要使用的参数
	m_txtTrainDir = _T("");
	m_txtResultDir = _T("");
	m_nFSMode = CClassifierParam::nFS_IGMode;
	m_nWordSize = 1000;
	m_nUKS_k=0.6;
	m_nSelMode=CClassifierParam::nFSM_GolbalMode;
	m_nOpMode=CClassifierParam::nOpDocMode;
	m_nLanguageType=CClassifierParam::nLT_Chinese;
	m_bStem=FALSE;
	m_nWeightMode=0;
	m_nCS=FALSE;
	m_nRS=FALSE;
	m_Length=FALSE;
	m_nUKS=FALSE;
	m_nURL=FALSE;
	m_ClassifierW=FALSE;
	//分类时需要使用的参数
	m_nClassifyType=0;
	m_bEvaluation=TRUE;
	m_bCopyFiles=FALSE;
	m_strTestDir=_T("");
	m_strResultDir=_T("");
	m_strModelFile=_T("");
	m_nDocFormat=CClassifierParam::nDF_Directory;
	m_nKNN=35;
	m_dThreshold=60;
	m_nClassifierType=-1;   
}

CClassifierParam::~CClassifierParam()
{

}

void CClassifierParam::Serialize(CArchive &ar)
{
	if(ar.IsStoring())
	{
		//训练时需要使用的参数
		ar<<m_txtTrainDir;
		ar<<m_txtResultDir;
		ar<<m_nFSMode;
		ar<<m_nWordSize;
		ar<<m_nUKS_k;
		ar<<m_nSelMode;
		ar<<m_nOpMode;
		ar<<m_nLanguageType;
		ar<<m_bStem;
		ar<<m_nCS;
		ar<<m_nRS;
		ar<<m_Length;
		ar<<m_nUKS;
		ar<<m_nURL;
		ar<<m_ClassifierW;
		ar<<m_nWeightMode;
		//分类时需要使用的参数
		ar<<m_nClassifyType;
		ar<<m_bEvaluation;
		ar<<m_bCopyFiles;
		ar<<m_strTestDir;
		ar<<m_strResultDir;
		ar<<m_strModelFile;
		ar<<m_nDocFormat;
		ar<<m_nKNN;
		ar<<m_dThreshold;
		ar<<m_nClassifierType;
	}
	else
	{
		//训练时需要使用的参数
		ar>>m_txtTrainDir;
		ar>>m_txtResultDir;
		ar>>m_nFSMode;
		ar>>m_nWordSize;
		ar>>m_nUKS_k;
		ar>>m_nURL;
		ar>>m_nSelMode;
		ar>>m_nOpMode;
		ar>>m_nLanguageType;
		ar>>m_bStem;
		ar>>m_nCS;
		ar>>m_nRS;
		ar>>m_Length;
		ar>>m_nUKS;
		ar>>m_ClassifierW;
		ar>>m_nWeightMode;
		//分类时需要使用的参数
		ar>>m_nClassifyType;
		ar>>m_bEvaluation;
		ar>>m_bCopyFiles;
		ar>>m_strTestDir;
		ar>>m_strResultDir;
		ar>>m_strModelFile;
		ar>>m_nDocFormat;
		ar>>m_nKNN;
		ar>>m_dThreshold;
		ar>>m_nClassifierType;
	}
}

void CClassifierParam::DumpToFile(CString strFileName)
{
	CFile		fBinOut;
	if(!fBinOut.Open(strFileName,CFile::modeWrite | CFile::modeCreate) )
	{
		AfxMessageBox( "无法创建文件f"+strFileName+"!") ;
		return;
	}
	CArchive ar(&fBinOut,CArchive::store);
	Serialize(ar);
	ar.Close();
	fBinOut.Close();
}

bool CClassifierParam::GetFromFile(CString strFileName)
{
	CFile		fBinIn;
	if(!fBinIn.Open(strFileName,CFile::modeRead) )
	{
		AfxMessageBox( "无法打开文件"+strFileName+"!");
		return false;
	}
	CArchive ar(&fBinIn,CArchive::load);
	Serialize(ar);
	ar.Close();
	
	fBinIn.Close();
	return true;
}

void CClassifierParam::GetParamString(CString &strParam)
{
	strParam ="Traning Document Directory:\t\t"+m_txtTrainDir+"\r\n";
	strParam+="Traning Result Directory:\t\t"+m_txtResultDir+"\r\n";
	
	strParam+="Probabilistic estimation methods:\t\t";
	if(m_nOpMode==CClassifierParam::nOpDocMode)
		strParam+="基于文档统计\r\n";
	else if(m_nOpMode==CClassifierParam::nOpWordMode)
		strParam+="基于词频统计\r\n";
	else
		strParam+="未知\r\n";
	
	strParam+="Feature selection method:\t\t";
	if(m_nFSMode==CClassifierParam::nFS_IGMode)
		strParam+="Information Gain\r\n";
	else if(m_nFSMode==CClassifierParam::nFS_MIMode)
		strParam+="Mutual Information\r\n";
	else if(m_nFSMode==CClassifierParam::nFS_CEMode)
		strParam+="Expected Cross Entropy\r\n";
	else if(m_nFSMode==CClassifierParam::nFS_X2Mode)
		strParam+="X^2 Statistics\r\n";
	else if(m_nFSMode==CClassifierParam::nFS_WEMode)
		strParam+="Text weight of evidence\r\n";
	else if(m_nFSMode==CClassifierParam::nFS_XXMode)
		strParam+="Right information gain\r\n";
	else
		strParam+="未知\r\n";

	strParam+="特征选择方式:\t\t";
	if(m_nSelMode==CClassifierParam::nFSM_GolbalMode)
		strParam+="全局选取\r\n";
	else if(m_nSelMode==CClassifierParam::nFSM_IndividualModel)
		strParam+="按类别单独选取\r\n";
	else
		strParam+="未知\r\n";

	strParam+="文档语言种类:\t\t";
	if(m_nLanguageType==CClassifierParam::nLT_Chinese)
		strParam+="中文\r\n";
	else if(m_nLanguageType==CClassifierParam::nLT_English)
	{
		strParam+="英文\r\n";
		if(m_bStem)
			strParam+="是否词干抽取:\t\t是\r\n";
		else
			strParam+="是否词干抽取:\t\t否\r\n";
	}
	else
		strParam+="未知\r\n";

	strParam+="特征加权算法:\t\t";
	if(m_nWeightMode==CClassifierParam::nWM_TF_IDF)
		strParam+="TF*IDF\r\n";
	else if(m_nWeightMode==CClassifierParam::nWM_TF_DIFF)
		strParam+="TF*特征评估函数值\r\n";
	else if(m_nWeightMode==CClassifierParam::nWM_TF_IDF_DIFF)
		strParam+="TF*IDF*特征评估函数值\r\n";
	else if(m_nWeightMode==CClassifierParam::nWM_nCS_nRS)
		strParam+="nCS_nRS*特征评估函数值\r\n";//xby
	else
		strParam+="未知\r\n";

	CString strWordSize;
	strWordSize.Format("特征空间维数:\t\t%d\r\n",m_nWordSize);
	strParam+=strWordSize;

	if(m_nClassifierType==CClassifierParam::nCT_KNN)
		strParam+="分类器类型: \t\tKNN\r\n";
	else if(m_nClassifierType==CClassifierParam::nCT_SVM)
		strParam+="分类器类型: \t\tSVM\r\n";
	else if(m_nClassifierType==CClassifierParam::nCT_BAYES)
		strParam+="分类器类型: \t\tBAYES\r\n";
	else
		strParam="请先打开一个分类模型文件!";
}
