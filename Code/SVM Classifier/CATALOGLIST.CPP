// CatalogList.cpp: implementation of the CCatalogList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CatalogList.h"
#include "wordlist.h"
#include "direct.h"
#include "wordsegment.h"
#include "classifierparam.h"
#include "stemmer.h"
#include "classifier.h"
#include <math.h>
#include <memory.h> 
#include ".\\Utility\\Utility.h"
#include <ctype.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CCatalogList::m_nSaveMode;
char pTempStr[MAX_PATH];
char pWordTag[10];
sWeightNode *CDocNode::m_pTemp=NULL;                 //生成文档向量时需要使用的一块临时内存----newly added
int CDocNode::m_nAllocTempLen=0;
char CDocNode::m_pSentence[MAX_PATH*10];        //用来存放经过去掉空格回车等字母后的句子
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//文档结点类
CDocNode::CDocNode(const CDocNode& x):m_nAllocLen(0),m_sWeightSet(NULL),m_pResults(NULL),m_nClassNum(0)
{
	*this=x;
}
//拷贝
const CDocNode& CDocNode::operator=(const CDocNode& x)
{
	if(this==&x) return *this;
	m_strDocName=x.m_strDocName;
	m_idxDoc=x.m_idxDoc;
	if(x.m_sWeightSet!=NULL)
	{
		AllocBuffer(x.m_nAllocLen);
		memcpy(m_sWeightSet,x.m_sWeightSet,m_nAllocLen*sizeof(sWeightNode));
	}
	else
	{
		m_sWeightSet=NULL;
		m_nAllocLen=0;
	}

	if(x.m_pResults!=NULL)
	{
		AllocResultsBuffer(x.m_nClassNum);
		memcpy(m_pResults,x.m_pResults,m_nClassNum*sizeof(double));
	}
	else
	{
		m_pResults=NULL;
		m_nClassNum=0;
	}

	return *this;
}

CDocNode::CDocNode()
{
	m_sWeightSet = NULL;
	m_pResults   = NULL;
	m_nAllocLen=0;
	m_nClassNum=0;
	m_idxDoc=-1;
	m_nCataID=-1;
}

CDocNode::~CDocNode()
{
	DeallocBuffer();
	DeallocResultsBuffer();
}


///////
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//日志结点类
sWeightQueryNode *CQueryNode::m_pTemp=NULL;          //生成文档向量时需要使用的一块临时内存----newly added
int CQueryNode::m_nAllocTempLen=0;
char CQueryNode::m_pSentence[MAX_PATH*10];        //用来存放经过去掉空格回车等字母后的句子
CQueryNode::CQueryNode(const CQueryNode& x):m_nAllocLen(0),m_sWeightQuerySet(NULL),m_pResults(NULL),m_nClassNum(0)
{
	*this=x;
}
//拷贝
const CQueryNode& CQueryNode::operator=(const CQueryNode& x)
{
	if(this==&x) return *this;
	//m_strLogName=x.m_strLogName;
	m_idxQuery=x.m_idxQuery;
	if(x.m_sWeightQuerySet!=NULL)
	{
		AllocBuffer(x.m_nAllocLen);
		memcpy(m_sWeightQuerySet,x.m_sWeightQuerySet,m_nAllocLen*sizeof(m_sWeightQuerySet));
	}
	else
	{
		m_sWeightQuerySet=NULL;
		m_nAllocLen=0;
	}

	if(x.m_pResults!=NULL)
	{
		AllocResultsBuffer(x.m_nClassNum);
		memcpy(m_pResults,x.m_pResults,m_nClassNum*sizeof(double));
	}
	else
	{
		m_pResults=NULL;
		m_nClassNum=0;
	}

	m_idxQuery=x.m_idxQuery;
	m_nCataID=x.m_nCataID;
	m_nTotalSessionNum=x.m_nTotalSessionNum;
	m_Text=x.m_Text;

	m_nbelow=x.m_nbelow;         //计算nCS用
	m_nRank_below=x.m_nRank_below;    //计算nRS用

	nCS=x.nCS;
	nRS=x.nRS;
	Length=x.Length;

    UKS_N=x.UKS_N;
	UKS_I=x.UKS_I;
	UKS_T=x.UKS_T;

	return *this;
}

CQueryNode::CQueryNode()
{
	m_sWeightQuerySet = NULL;
	m_pResults   = NULL;
	m_nAllocLen=0;
	m_nClassNum=0;
	m_idxQuery=-1;
	m_nCataID=-1;
	m_nTotalSessionNum=0;
	m_Text="";

	m_nbelow=0.0;         //计算nCS用
	m_nRank_below=0.0;    //计算nRS用
    
	nCS=0.0;
	nRS=0.0;
	Length=0.0;

	UKS_N=0.0;
	UKS_I=0.0;
	UKS_T=0.0;
}

CQueryNode::~CQueryNode()
{
	DeallocBuffer();
	DeallocResultsBuffer();
}
void CQueryNode::Serialize(CArchive &ar)
{
	int nLen;
	if(ar.IsStoring())
	{
		ar<<m_idxQuery;
		//ar<<m_strLogName;
		if(CCatalogList::GetSaveMode()<=0)
		{
			ar<<m_nAllocLen;
			ar.Write((void*)m_sWeightQuerySet,m_nAllocLen*sizeof(m_sWeightQuerySet));
		}
	}
	else
	{
		ar>>m_idxQuery;
		//ar>>m_strLogName;
		if(CCatalogList::GetSaveMode()<=0)
		{
			ar>>nLen;
			AllocBuffer(nLen);
			ar.Read((void*)m_sWeightQuerySet,m_nAllocLen*sizeof(m_sWeightQuerySet));
		}
		else
		{
			m_nAllocLen=0;
			m_sWeightQuerySet=NULL;
		}
		m_nClassNum=0;
		m_pResults=NULL;
	}
}

//为权重分配空间
void CQueryNode::AllocTempBuffer(int nLen)
{
	if((nLen<=0)||(m_nAllocTempLen==nLen)) return;
	if(m_pTemp!=NULL)
	{
		delete []m_pTemp;
		m_pTemp=NULL;
	}
	m_pTemp=new sWeightQueryNode[nLen];
	m_nAllocTempLen=nLen;
}


//释放为权重分配的临时空间
void CQueryNode::DeallocTempBuffer()
{
	if(m_pTemp!=NULL)
	{
		delete []m_pTemp;
		m_pTemp=NULL;
	}
	m_nAllocTempLen=0;
}


//为权重分配临时空间
void CQueryNode::AllocBuffer(int nLen)
{
	if((nLen<=0)||(m_nAllocLen==nLen)) return;
	if(m_sWeightQuerySet!=NULL)
	{
		delete []m_sWeightQuerySet;
		m_sWeightQuerySet=NULL;
	}
	m_nAllocLen=nLen;
	m_sWeightQuerySet=new sWeightQueryNode[m_nAllocLen];
}


//释放为权重分配的空间
void CQueryNode::DeallocBuffer()
{
	if(m_sWeightQuerySet!=NULL)
	{
		delete []m_sWeightQuerySet;
		m_sWeightQuerySet=NULL;
	}
	m_nAllocLen=0;
}

//为分类结果分配空间
void CQueryNode::AllocResultsBuffer(short nLen)
{
	if((nLen<=0)||(m_nClassNum==nLen)) return;
	if(m_pResults!=NULL)
	{
		delete []m_pResults;
		m_pResults=NULL;
	}
	m_nClassNum=nLen;
	m_pResults=new double[m_nClassNum];
}


//释放为分类结果分配的空间
void CQueryNode::DeallocResultsBuffer()
{
	if(m_pResults!=NULL)
	{
		delete []m_pResults;
		m_pResults=NULL;
	}
	m_nClassNum=0;
}


///////////////////////////////////
CLogNode::CLogNode(const CLogNode& x)
{
	*this=x;
}
//拷贝
const CLogNode& CLogNode::operator=(const CLogNode& x)
{
	if(this==&x) return *this;
	//m_strLogName=x.m_strLogName;
	m_idxLog=x.m_idxLog;
	time=x.time;
	userID=x.userID;
	text=x.text;
	rank=x.rank;
	order=x.order;
	URL=x.URL;
	/*if(x.m_sWeightLogSet!=NULL)
	{
		AllocBuffer(x.m_nAllocLen);
		memcpy(m_sWeightLogSet,x.m_sWeightLogSet,m_nAllocLen*sizeof(m_sWeightLogSet));
	}
	else
	{
		m_sWeightLogSet=NULL;
		m_nAllocLen=0;
	}

	if(x.m_pResults!=NULL)
	{
		AllocResultsBuffer(x.m_nClassNum);
		memcpy(m_pResults,x.m_pResults,m_nClassNum*sizeof(double));
	}
	else
	{
		m_pResults=NULL;
		m_nClassNum=0;
	}*/

	return *this;
}

CLogNode::CLogNode()
{
	m_idxLog=-1;
	time="";
	userID=0.0;
	text="";
	rank=0;
	order=0.0;
	URL="";
}

CLogNode::~CLogNode()
{
	
}

void CLogNode::setTime(CString time){

	this->time=time;
}
CString CLogNode::getTime(){

	return this->time;
}
void CLogNode::setUserID(double userID){

	this->userID=userID;
}
double CLogNode::getUserID(){

	return this->userID;
}
void CLogNode::setText(CString text){

	this->text=text;
}
CString CLogNode::getText(){

	return this->text;
}

void CLogNode::setRank(int rank){

	this->rank=rank;
}
int CLogNode::getRank()
{
	return this->rank;
}
void CLogNode::setOrder(double order){

	this->order=order;
}
double CLogNode::getOrder()
{
	return this->order;
}
void CLogNode::setURL(CString URL){

	this->URL=URL;
}
CString CLogNode::getURL(){

	return this->URL;
}
bool CLogNode::operator <   (const CLogNode& l )   const //升序排序时必须写的函数
{
	if( time != l.time)

		 return time < l.time; 

	else //if(userID == l.userID) 		 
		
		return userID < l.userID; //return text < l.text;
	  
}

bool CLogNode::operator >   (const CLogNode& l )   const //降序排序时必须写的函数
{ 
	if(time != l.time){
		
		return time > l.time;
	} 
	else 
		return userID > l.userID; //return text > l.text;
}


//nMode<=0  删除所有文档信息
//nMode>0   只删除文档向量所占用的空间
void CCatalogNode::InitCatalogNode(int nMode)
{
	m_lTotalWordNum = 0;
	POSITION pos_doc=m_lstDocList.GetHeadPosition();
	while(pos_doc!=NULL)
	{
		CDocNode& docnode=m_lstDocList.GetNext(pos_doc);
		docnode.DeallocBuffer();
		docnode.DeallocResultsBuffer();
	}
	if(nMode>0) m_lstDocList.RemoveAll();
}

///xby
void CCatalogNode::InitCatalogNode_LogFileList(int nMode)
{
	//m_lTotalWordNum = 0;
	POSITION pos_log=m_lstLogFileList.GetHeadPosition();
	while(pos_log!=NULL)
	{
		CLogFileNode& logfilenode=m_lstLogFileList.GetNext(pos_log);
		//logfilenode.DeallocBuffer();
		//logfilenode.DeallocResultsBuffer();
	}
	if(nMode>0) m_lstLogFileList.RemoveAll();
}

///xby
void CCatalogNode::InitCatalogNode_QueryList(int nMode)
{
	m_lTotalWordNum = 2;
	POSITION pos_log=m_lstQueryList.GetHeadPosition();
	while(pos_log!=NULL)
	{
		CQueryNode& querynode=m_lstQueryList.GetNext(pos_log);
		querynode.DeallocBuffer();
		querynode.DeallocResultsBuffer();
	}
	if(nMode>0) m_lstQueryList.RemoveAll();
}

CCatalogNode::CCatalogNode(const CCatalogNode& x)
{
	*this=x;
}

//类节点构造
const CCatalogNode& CCatalogNode::operator = (const CCatalogNode& x)
{
	if(this==&x) return *this;
	m_lTotalWordNum = x.m_lTotalWordNum ;
	m_strCatalogName=x.m_strCatalogName;
	m_strDirName=x.m_strDirName;
	m_idxCata=x.m_idxCata;
	m_lstDocList.RemoveAll();
	POSITION pos = x.m_lstDocList.GetHeadPosition();
	while(pos!=NULL)
	{
		CDocNode& docnode=x.m_lstDocList.GetNext(pos);
		m_lstDocList.AddTail(docnode);
	}
	return *this;
}

const CCatalogNode& CCatalogNode::operator += (const CCatalogNode& x)
{
	if(this==&x) return *this;
	m_lTotalWordNum += x.m_lTotalWordNum ;
	m_strCatalogName=x.m_strCatalogName;
	m_strDirName=x.m_strDirName;
	m_idxCata=x.m_idxCata;
	POSITION pos = x.m_lstDocList.GetHeadPosition();
	while(pos!=NULL)
	{
		CDocNode& docnode=x.m_lstDocList.GetNext(pos);
		m_lstDocList.AddTail(docnode);
	}
	return *this;
}

CCatalogNode::CCatalogNode()
{
	m_idxCata=-1;
	m_lCurDocID=0;
	m_lTotalWordNum=0;
	InitCatalogNode();
	InitCatalogNode_LogFileList();
	InitCatalogNode_QueryList();
}

CCatalogNode::~CCatalogNode()
{
	InitCatalogNode();
	InitCatalogNode_LogFileList();
	InitCatalogNode_QueryList();
}


void CCatalogNode::SetStartDocID(long lDocID)
{
	m_lCurDocID=lDocID;
}

void CCatalogNode::SetStartLogFileID(long lLogFileID)
{
	m_lCurLogFileID=lLogFileID;
}

long CCatalogNode::ScanDirectory(CString strPath)
{
	if(_chdir(strPath))  // if can't find the dir
	{
		CString	csTmp = "目录";
		csTmp+=strPath;
		csTmp+="不存在!";
		AfxMessageBox(csTmp);
		return -1;
	}

	HANDLE hFinder;
	LPWIN32_FIND_DATA lpFindFileData;	
	lpFindFileData  = new WIN32_FIND_DATA;
	hFinder = ::FindFirstFile("*.*",lpFindFileData );
	while(::FindNextFile(hFinder,lpFindFileData))
	{
		if( !strcmp(lpFindFileData->cFileName,".") || !strcmp(lpFindFileData->cFileName,"..") )
			continue;

		if(!(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CDocNode docnode;
			docnode.m_strDocName=lpFindFileData->cFileName;
			docnode.m_idxDoc=m_lCurDocID++;
			docnode.m_nAllocLen=0;
			docnode.m_sWeightSet=NULL;
			docnode.m_nClassNum=0;
			docnode.m_pResults=NULL;
			AddDoc(docnode);
		}
	}
	delete	lpFindFileData;
	return m_lCurDocID;
}

long CCatalogNode::ScanLogDirectory(CString strPath,double k,bool url)
{
	
	double m1=k;
	bool m2=url;
	if(_chdir(strPath))  // if can't find the dir
	{
		CString	csTmp = "目录";
		csTmp+=strPath;
		csTmp+="不存在!";
		AfxMessageBox(csTmp);
		return -1;
	}
	int TotalFileNum=0;

	HANDLE hFinder;
	LPWIN32_FIND_DATA lpFindFileData;	
	lpFindFileData  = new WIN32_FIND_DATA;
	hFinder = ::FindFirstFile("*.*",lpFindFileData );
	while(::FindNextFile(hFinder,lpFindFileData))
	{
		if( !strcmp(lpFindFileData->cFileName,".") || !strcmp(lpFindFileData->cFileName,"..") )
			continue;

		if(!(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			//	//读日志文件到每个lognode
			CLogFileNode logfilenode;
			logfilenode.m_strLogFileName=lpFindFileData->cFileName;
			logfilenode.m_idxCata=m_lCurLogFileID;
			logfilenode.m_lTotalFileNum=TotalFileNum++;
			logfilenode.readLogFile();
			AddLogFile(logfilenode);
			//AfxMessageBox("啥毛病");
		}
	}

	ScanLogtoQuery(m1,m2);
	//处理每个logfile中的lognode，得到querynode

	int nCount=0,nQueryNum=0; 
	POSITION pos_qry=GetFirstQueryPosition();
	nQueryNum=GetQueryNum();
	while(pos_qry!=NULL)
	{
		CQueryNode &qrynode=GetNextQuery(pos_qry);
		//CMessage::PrintStatusInfo(_T("扫描日志记录"));
		qrynode.m_idxQuery=m_lCurQueryID++;
		qrynode.m_nAllocLen=0;
		qrynode.m_sWeightQuerySet=NULL;
		qrynode.m_nClassNum=0;
		qrynode.m_pResults=NULL;
		//AddQuery(qrynode);
	}			
	delete	lpFindFileData;
	return m_lCurQueryID;
}

CCatalogList::CCatalogList()
{
	m_nSaveMode=0;
}

CCatalogList::~CCatalogList()
{
}


void CCatalogList::DumpCataList(CString strFileName)
{
	FILE *stream;
	if( (stream  = fopen( strFileName, "w+" )) == NULL )
	{
		AfxMessageBox("无法创建文件a"+strFileName+"!");
		return;
	}

	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		fprintf(stream,"%d %s\n",catanode.m_idxCata,catanode.m_strCatalogName);
	}
	fclose(stream);
}

void CCatalogList::DumpDocList(CString strFileName)
{
	FILE *stream;
	if( (stream  = fopen( strFileName, "w+" )) == NULL )
	{
		AfxMessageBox("无法创建文件b"+strFileName+"!");
		return;
	}

	POSITION pos=GetFirstPosition();
	POSITION pos_doc;
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		pos_doc=catanode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catanode.GetNext(pos_doc);
			if(docnode.m_nAllocLen>0)
			{
				fprintf(stream,"%d",catanode.m_idxCata+1);
				for(int i=0;i<docnode.m_nAllocLen;i++)
				{
					if(fabs(docnode.m_sWeightSet[i].s_dWeight) > dZero)
					fprintf(stream," %d:%f",docnode.m_sWeightSet[i].s_idxWord+1,docnode.m_sWeightSet[i].s_dWeight);
				}
				fprintf(stream,"\n");
			}
		}
	}
	fclose(stream);
}

void CCatalogList::DumpQueryList(CString strFileName,int type[5])//
{
	FILE *stream;
	int sum=0;
	if( (stream  = fopen( strFileName, "w+" )) == NULL )
	{
		AfxMessageBox("无法创建文件c"+strFileName+"!");
		return;
	}

	POSITION pos=GetFirstPosition();
	POSITION pos_qry;
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		pos_qry=catanode.GetFirstQueryPosition();
		while(pos_qry!=NULL)
		{
			sum=0;
			CQueryNode& querynode=catanode.GetNextQuery(pos_qry);
			if(querynode.m_nAllocLen>0)
			{
				fprintf(stream,"%d",catanode.m_idxCata+1);
				////fprintf(stream,"%s",querynode.m_Text);
                if(type[0])
				{
				 fprintf(stream," %d:%f",++sum,querynode.nCS);//nCS值
				}
				if(type[1])
				{
			     fprintf(stream," %d:%lf",++sum,querynode.nRS);//nRS值
				}
				if(type[2])
				{
	             fprintf(stream," %d:%f",++sum,querynode.UKS_N);//UKS_N
				 fprintf(stream," %d:%f",++sum,querynode.UKS_I);//UKS_I
				 fprintf(stream," %d:%f",++sum,querynode.UKS_T);//UKS_T
				}
				if(type[3])
				{
				 fprintf(stream," %d:%lf",++sum,querynode.Length);//Length值

				}
				if(type[4])
				{
				 for(int i=0;i<querynode.m_nAllocLen;i++)
				 {
					if(fabs(querynode.m_sWeightQuerySet[i].s_dWeight) > dZero)
					fprintf(stream," %d:%f",querynode.m_sWeightQuerySet[i].s_idxWord+sum+1,querynode.m_sWeightQuerySet[i].s_dWeight);
				 }
				}
				fprintf(stream,"\n");
				/*
				 fprintf(stream," %d:%f",1,querynode.nCS);//nCS值
			     fprintf(stream," %d:%f",2,querynode.nRS);//nRS值
	             fprintf(stream," %d:%f",3,querynode.UKS_N);//UKS_N
				 fprintf(stream," %d:%f",4,querynode.UKS_I);//UKS_I
				 fprintf(stream," %d:%f",5,querynode.UKS_T);//UKS_T
				 for(int i=0;i<querynode.m_nAllocLen;i++)
				 {
					if(fabs(querynode.m_sWeightQuerySet[i].s_dWeight) > dZero)
					fprintf(stream," %d:%f",querynode.m_sWeightQuerySet[i].s_idxWord+6,querynode.m_sWeightQuerySet[i].s_dWeight);
				 }	
				 fprintf(stream,"\n");*/
				//}
			}
		}
	}
	fclose(stream);
}

//nSaveMode<=0 保存文档的向量
//nSaveMode>0  不保存文档的向量
void CCatalogList::DumpToFile(CString strFileName, int nSaveMode)  // view the word list content
{
	CFile		fBinOut;
	if(!fBinOut.Open(strFileName,CFile::modeWrite | CFile::modeCreate))
	{
		AfxMessageBox("无法创建文件d"+strFileName+"!");
		return;
	}
	
	CArchive ar(&fBinOut,CArchive::store);
	CCatalogList::m_nSaveMode=nSaveMode;
	Serialize(ar);
	
	ar.Close();
	fBinOut.Close();
}

BOOL CCatalogList::GetFromFile(CString strFileName)  // view the word list content
{
	CFile	fBinOut;
	if(!fBinOut.Open(strFileName,CFile::modeRead))
	{
		AfxMessageBox("无法打开文件"+strFileName+"!");
		return FALSE;
	}

	CArchive ar(&fBinOut,CArchive::load);
	Serialize(ar);
	ar.Close();

	fBinOut.Close();
	return TRUE;
}
//处理文档库中的文档
long CCatalogList::BuildLib(CString strDirName)
{
	InitCatalogList();
	return ScanDirectory(strDirName);
}

//xby
long CCatalogList::BuildLogLib(CString strDirName,double k,bool url)
{
	double m1=k;
	bool m2=url;
	InitCatalogList();
	return ScanDirectory_Log(strDirName,m1,m2);
}

void CCatalogList::Serialize(CArchive &ar)
{
	if(ar.IsStoring()) ar<<m_nSaveMode;
	else ar>>m_nSaveMode;
	m_lstCatalogList.Serialize(ar);
}

void AFXAPI SerializeElements(CArchive& ar,CCatalogNode* pElements,int nCount)
{
	ASSERT(nCount==0||
		AfxIsValidAddress(pElements,nCount*sizeof(CCatalogNode)));
	pElements->Serialize(ar);
}

void AFXAPI SerializeElements(CArchive& ar,CDocNode* pElements,int nCount)
{
	ASSERT(nCount==0||
		AfxIsValidAddress(pElements,nCount*sizeof(CDocNode)));
	pElements->Serialize(ar);
}
//////xby
void AFXAPI SerializeElements(CArchive& ar,CLogNode* pElements,int nCount)
{
	ASSERT(nCount==0||
		AfxIsValidAddress(pElements,nCount*sizeof(CLogNode)));
	pElements->Serialize(ar);
}
void CDocNode::Serialize(CArchive &ar)
{
	int nLen;
	if(ar.IsStoring())
	{
		ar<<m_idxDoc;
		ar<<m_strDocName;
		if(CCatalogList::GetSaveMode()<=0)
		{
			ar<<m_nAllocLen;
			ar.Write((void*)m_sWeightSet,m_nAllocLen*sizeof(sWeightNode));
		}
	}
	else
	{
		ar>>m_idxDoc;
		ar>>m_strDocName;
		if(CCatalogList::GetSaveMode()<=0)
		{
			ar>>nLen;
			AllocBuffer(nLen);
			ar.Read((void*)m_sWeightSet,m_nAllocLen*sizeof(sWeightNode));
		}
		else
		{
			m_nAllocLen=0;
			m_sWeightSet=NULL;
		}
		m_nClassNum=0;
		m_pResults=NULL;
	}
}

void CCatalogNode::Serialize(CArchive &ar)
{
	if(ar.IsStoring())
	{
		ar<<m_idxCata;
		ar<<m_strDirName;
		ar<<m_lTotalWordNum;
		ar<<m_strCatalogName;
	}
	else
	{
		ar>>m_idxCata;
		ar>>m_strDirName;
		ar>>m_lTotalWordNum;
		ar>>m_strCatalogName;
	}
	m_lstDocList.Serialize(ar);
}

//为权重分配空间
void CDocNode::AllocTempBuffer(int nLen)
{
	if((nLen<=0)||(m_nAllocTempLen==nLen)) return;
	if(m_pTemp!=NULL)
	{
		delete []m_pTemp;
		m_pTemp=NULL;
	}
	m_pTemp=new sWeightNode[nLen];
	m_nAllocTempLen=nLen;
}


//释放为权重分配的临时空间
void CDocNode::DeallocTempBuffer()
{
	if(m_pTemp!=NULL)
	{
		delete []m_pTemp;
		m_pTemp=NULL;
	}
	m_nAllocTempLen=0;
}


//为权重分配临时空间
void CDocNode::AllocBuffer(int nLen)
{
	if((nLen<=0)||(m_nAllocLen==nLen)) return;
	if(m_sWeightSet!=NULL)
	{
		delete []m_sWeightSet;
		m_sWeightSet=NULL;
	}
	m_nAllocLen=nLen;
	m_sWeightSet=new sWeightNode[m_nAllocLen];
}


//释放为权重分配的空间
void CDocNode::DeallocBuffer()
{
	if(m_sWeightSet!=NULL)
	{
		delete []m_sWeightSet;
		m_sWeightSet=NULL;
	}
	m_nAllocLen=0;
}

//为分类结果分配空间
void CDocNode::AllocResultsBuffer(short nLen)
{
	if((nLen<=0)||(m_nClassNum==nLen)) return;
	if(m_pResults!=NULL)
	{
		delete []m_pResults;
		m_pResults=NULL;
	}
	m_nClassNum=nLen;
	m_pResults=new double[m_nClassNum];
}


//释放为分类结果分配的空间
void CDocNode::DeallocResultsBuffer()
{
	if(m_pResults!=NULL)
	{
		delete []m_pResults;
		m_pResults=NULL;
	}
	m_nClassNum=0;
}

int CDocNode::ScanChinese(char * pPath,CWordList& wordList,int nCataNum, short idxCata)
{
	CFile fin;
	char *buffer;
	strcpy(pTempStr,pPath);
	strcat(pTempStr,"\\");
	strcat(pTempStr,m_strDocName.GetBuffer(0));
	if(!fin.Open(pTempStr,CFile::modeRead))
		return -1;

	unsigned int flen=fin.GetLength();
	buffer=new char[flen+1];
	flen=fin.ReadHuge(buffer,flen);
	buffer[flen]='\0';
	fin.Close();
	int num=ScanChineseString(buffer,wordList,nCataNum,m_idxDoc,idxCata);
	delete[] buffer;
	return num;
}

int CDocNode::ScanChineseString(char * pPath,CWordList& wordList,int nCataNum, long idxDoc, short idxCata)
{
	char *buffer=pPath;
	int i,j,sum;
	char *w;
	//realcnt为文章中去掉停用词后剩下的总共词数
	//nStart为一个句子在buffer中的开始位置
	int nStart=0,nNewStart=0;
	bool flag=true;
	int nSentenceLen=0;
	int realcnt=0;
	while(buffer[nStart]!='\0')
	{
		flag=true;
		nSentenceLen=ParseFile(buffer,nStart,nNewStart);
		nStart=nNewStart;
		if(nSentenceLen==0) continue;
		if(m_pSentence[0]>0) //如果是一个英文单词
		{
			//如果英文单词的长度大于等于2,且不是数字
			if((nSentenceLen>=2)&&((m_pSentence[0]<'0')||(m_pSentence[0]>'9')))
			{
				wordList.Add(m_pSentence,idxCata,idxDoc,nCataNum);
				realcnt++;
			}
		}
		else //如果是汉字串
		{
			if(nSentenceLen%2!=0) continue;
			if(nSentenceLen<=6) //如果是单个汉字
			{
				wordList.Add(m_pSentence,idxCata,idxDoc,nCataNum);
				realcnt++;
			}
			else
			{
				g_wordSeg.Segment(m_pSentence);
				for(i=0;i<g_wordSeg.GetSegmentCount();i++)
				{
					sum=0;
					PWORD_RESULT pItem=g_wordSeg.GetWordSeg(i);
					while(pItem[sum].sWord[0]!=0) sum++;
				
					for(j=1;j<sum-1;j++)
					{
						w=pItem[j].sWord;
						if(w[0]=='\0'||g_wordSeg.isInStopWords(w)) flag=false;		
						if(flag)
						{
							wordList.Add(w,idxCata,idxDoc,nCataNum);
							realcnt++;
						}
						flag=true;/////xby
					}
				}
			}
		}
	}
	return realcnt;
}

int CDocNode::ScanEnglish(char *pPath, CWordList &wordList, int nCataNum, short idxCata, bool bStem)
{
	CFile fin;
	char *buffer;
	strcpy(pTempStr,pPath);
	strcat(pTempStr,"\\");
	strcat(pTempStr,m_strDocName.GetBuffer(0));
	if(!fin.Open(pTempStr,CFile::modeRead))
		return -1;

	int flen=fin.GetLength();
	buffer=new char[flen+1];
	flen=fin.ReadHuge(buffer,flen);
	buffer[flen]='\0';
	fin.Close();

	int num=ScanEnglishString(buffer,wordList,nCataNum,m_idxDoc,idxCata,bStem);
	delete[] buffer;
	return num;
}

int CDocNode::ScanEnglishString(char *pPath, CWordList &wordList, int nCataNum, long idxDoc, short idxCata, bool bStem)
{
	char *buffer=pPath;
	_strlwr(buffer);

	int nFilePos=0;
	int realcnt=0,wordLen=0;
	char c, *p;
	p=buffer;
	while(buffer[nFilePos]!='\0')
	{
		c=buffer[nFilePos];
		if(c==' '||c=='\r'||c=='\n'||
			(c>32&&c<=47)||(c>=58&&c<=64)||(c>=91&&c<=96)||(c>=123&&c<=127))
		{
			buffer[nFilePos]='\0';
			wordLen=buffer+nFilePos-p;
			if(wordLen>2)
			{
				if(bStem) theStemmer.stem(p,0,wordLen-1);
				wordList.Add(p,idxCata,idxDoc,nCataNum);
				realcnt++;
			}
			p=buffer+nFilePos+1;
		}
		nFilePos++;
	}
	return realcnt;
}


BOOL CDocNode::IsZero()
{
	ASSERT(m_sWeightSet!=NULL);
	for(int i=0;i<m_nAllocLen;i++)
		if(m_sWeightSet[i].s_dWeight!=0) return FALSE;
	return TRUE;
}

const CCatalogList& CCatalogList::operator = (const CCatalogList& x)
{
	if(this==&x) return *this;
	m_lstCatalogList.RemoveAll();
	POSITION pos = x.m_lstCatalogList.GetHeadPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=x.m_lstCatalogList.GetNext(pos);
		m_lstCatalogList.AddTail(catanode);
	}
	return *this;
}

const CCatalogList& CCatalogList::operator += (const CCatalogList& x)
{
	if(this==&x) return *this;
	POSITION pos = x.m_lstCatalogList.GetHeadPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=x.m_lstCatalogList.GetNext(pos);
		m_lstCatalogList.AddTail(catanode);
	}
	return *this;
}

//nMode<=0 删除所有信息
//nMode=1  删除所有文档,但保留类别(节点)
//nMode=2  只删除文档向量所占用的内存
//nMode=3  删除日志文件
//nMode=4  删除所有查询
void CCatalogList::InitCatalogList(int nMode)
{
	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		if(nMode==2) catanode.InitCatalogNode(1);
		else catanode.InitCatalogNode(0);
	}
	if(nMode<=0) m_lstCatalogList.RemoveAll();
}


long CCatalogList::GetDocNum()
{
	long i=0;
	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		i+=catanode.GetDocNum();
	}
	return i;
}

long CCatalogList::GetQueryNum()
{
	long i=0;
	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		i+=catanode.GetQueryNum();
	}
	return i;
}

int CCatalogList::GetCataNum()
{
	return m_lstCatalogList.GetCount();
}


POSITION CCatalogNode::AddDoc(CDocNode &docnode)
{
	return m_lstDocList.AddTail(docnode);
}

POSITION CCatalogNode::AddLogFile(CLogFileNode &logfilenode)
{
	return m_lstLogFileList.AddTail(logfilenode);
}

POSITION CCatalogNode::AddQuery(CQueryNode &qrynode)
{
	return m_lstQueryList.AddTail(qrynode);
}

bool CCatalogList::GetCataName(short idxCata,CString& strCataName)
{
	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos);
		if(idxCata==catanode.m_idxCata)
		{
			strCataName=catanode.m_strCatalogName;
			return true;
		}
	}
	return false;
}

bool CCatalogList::GetDocName(short idxCata, long idxDoc, CString &strDocName)
{
	POSITION pos_cata=GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos_cata);
		if(idxCata==catanode.m_idxCata)
		{
			POSITION pos_doc=catanode.GetFirstPosition();
			while(pos_doc!=NULL)
			{
				CDocNode& docnode=catanode.GetNext(pos_doc);
				if(idxDoc==docnode.m_idxDoc)
				{
					strDocName=docnode.m_strDocName;
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

POSITION CCatalogList::AddCata(CCatalogNode &catanode)
{
	return m_lstCatalogList.AddTail(catanode);
}

POSITION CCatalogList::GetFirstPosition()
{
	return m_lstCatalogList.GetHeadPosition();
}

CCatalogNode& CCatalogList::GetNext(POSITION &rPos)
{
	return m_lstCatalogList.GetNext(rPos);
}

POSITION CCatalogNode::GetFirstPosition()
{
	return m_lstDocList.GetHeadPosition();
}

CDocNode& CCatalogNode::GetNext(POSITION &rPos)
{
	return m_lstDocList.GetNext(rPos);
}

UINT CCatalogNode::GetDocNum()
{
    return m_lstDocList.GetCount();
}

///xby
POSITION CCatalogNode::GetFirstQueryPosition()
{
	return m_lstQueryList.GetHeadPosition();
}

CQueryNode& CCatalogNode::GetNextQuery(POSITION &rPos)
{
	return m_lstQueryList.GetNext(rPos);
}
CLogFileNode& CCatalogNode::GetNextLogFile(POSITION &rPos)
{
	return m_lstLogFileList.GetNext(rPos);
}

UINT CCatalogNode::GetQueryNum()
{
    return m_lstQueryList.GetCount();
}

UINT CCatalogNode::GetLogFileNum()///xby
{
    return m_lstLogFileList.GetCount();
}

UINT CLogFileNode::GetLogNum()///xby
{
	return m_lstLogList.GetCount();
}

//扫描目录构建类和文档节点
long CCatalogList::ScanDirectory(CString strPath)
{
	if(_chdir(strPath))  // if can't find the dir
	{
		CString	csTmp = "目录";
		csTmp+=strPath;
		csTmp+="不存在!";
		AfxMessageBox(csTmp);
		return -1;
	}

	long docNum=0;
	short idxCurCata=0;
	HANDLE hFinder;
	LPWIN32_FIND_DATA lpFindFileData;	
	lpFindFileData  = new WIN32_FIND_DATA;

	hFinder = ::FindFirstFile("*.*",lpFindFileData );
	while(::FindNextFile(hFinder,lpFindFileData))  // process the catalog dir;
	{
		if( !strcmp(lpFindFileData->cFileName,".") || !strcmp(lpFindFileData->cFileName,"..") )
			continue;

		if((lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CCatalogNode catalognode;
			catalognode.m_strCatalogName=lpFindFileData->cFileName;
			catalognode.m_strDirName=strPath+"\\"+catalognode.m_strCatalogName;
			catalognode.m_idxCata=idxCurCata++;
			POSITION posTemp=AddCata(catalognode);
			CCatalogNode& cataTemp=GetAt(posTemp);
			cataTemp.SetStartDocID(docNum);
			docNum=cataTemp.ScanDirectory(catalognode.m_strDirName);
		}
	}
	delete	lpFindFileData;
	return docNum;
}

///xby
//扫描目录构建类和文档节点
long CCatalogList::ScanDirectory_Log(CString strPath,double k,bool url)
{
	double m1=k;
	bool m2=url;
	if(_chdir(strPath))  // if can't find the dir
	{
		CString	csTmp = "目录";
		csTmp+=strPath;
		csTmp+="不存在!";
		AfxMessageBox(csTmp);
		return -1;
	}

	long logFileNum=0;
	short idxCurCata=0;
	HANDLE hFinder;
	LPWIN32_FIND_DATA lpFindFileData;	
	lpFindFileData  = new WIN32_FIND_DATA;

	hFinder = ::FindFirstFile("*.*",lpFindFileData );
	while(::FindNextFile(hFinder,lpFindFileData))  // process the catalog dir;
	{
		if( !strcmp(lpFindFileData->cFileName,".") || !strcmp(lpFindFileData->cFileName,"..") )
			continue;

		if((lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CCatalogNode catalognode;
			catalognode.m_strCatalogName=lpFindFileData->cFileName;
			catalognode.m_strDirName=strPath+"\\"+catalognode.m_strCatalogName;
			catalognode.m_idxCata=idxCurCata++;
			POSITION posTemp=AddCata(catalognode);
			CCatalogNode& cataTemp=GetAt(posTemp);
			cataTemp.SetStartLogFileID(logFileNum);
			logFileNum=cataTemp.ScanLogDirectory(catalognode.m_strDirName,m1,m2);
		}
	}
	delete	lpFindFileData;
	return logFileNum;
}

CCatalogNode CCatalogList::GetAt(POSITION pos) const
{
	return m_lstCatalogList.GetAt(pos);
}

CCatalogNode& CCatalogList::GetAt(POSITION pos)
{
	return m_lstCatalogList.GetAt(pos);
}
//根据标识获得类节点
CCatalogNode* CCatalogList::GetCata(short idxCata)
{
	POSITION pos_cata=GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos_cata);
		if(catanode.m_idxCata==idxCata)
			return &catanode;
	}
	return NULL;
}

//根据文档名获得文档
CDocNode* CCatalogList::GetDocByName(CString strDocName)
{
	POSITION pos_cata=GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos_cata);
		POSITION pos_doc=catanode.GetFirstPosition();
		while(pos_doc!=NULL)
		{
			CDocNode& docnode=catanode.GetNext(pos_doc);
			if(docnode.m_strDocName==strDocName)
				return &docnode;
		}
	}
	return NULL;
}
//根据类名获得类
CCatalogNode* CCatalogList::GetCataByName(CString strCataName)
{
	POSITION pos_cata=GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos_cata);
		if(catanode.m_strCatalogName==strCataName)
			return &catanode;
	}
	return NULL;
}

//根据类名获得类的ID
short CCatalogList::GetCataIDByName(CString strCataName)
{
	POSITION pos_cata=GetFirstPosition();
	while(pos_cata!=NULL)
	{
		CCatalogNode& catanode=GetNext(pos_cata);
		if(catanode.m_strCatalogName==strCataName)
			return catanode.m_idxCata;
	}
	return -1;
}

int CDocNode::GetWordNum()
{
	int s=0;
	for(int i=0;i<m_nAllocLen;i++)
	{
		s+=m_sWeightSet[i].s_tfi;
	}
	return s;
}

int CDocNode::GenDocVector()
{
	int i,nSum=0;
	for(i=0;i<m_nAllocTempLen;i++)
		if(m_pTemp[i].s_dWeight>dZero) nSum++;	
	AllocBuffer(nSum);
	nSum=0;
	for(i=0;i<m_nAllocTempLen;i++)
	{
		if(m_pTemp[i].s_dWeight>dZero)
		{
			m_sWeightSet[nSum].s_idxWord=m_pTemp[i].s_idxWord;
			m_sWeightSet[nSum].s_tfi=m_pTemp[i].s_tfi;
			m_sWeightSet[nSum].s_dWeight=m_pTemp[i].s_dWeight;
			nSum++;
		}
	}
	return nSum;
}

int CQueryNode::GenQryVector()
{
	int i,nSum=0;
	for(i=0;i<m_nAllocTempLen;i++)
		if(m_pTemp[i].s_dWeight>dZero) nSum++;	
	AllocBuffer(nSum);
	nSum=0;
	for(i=0;i<m_nAllocTempLen;i++)
	{
		if(m_pTemp[i].s_dWeight>dZero)
		{
			m_sWeightQuerySet[nSum].s_idxWord=m_pTemp[i].s_idxWord;
			m_sWeightQuerySet[nSum].s_tfi=m_pTemp[i].s_tfi;
			m_sWeightQuerySet[nSum].s_dWeight=m_pTemp[i].s_dWeight;
			nSum++;
		}
	}
	return nSum;
}

int CDocNode::GenDocVector(DOC &doc)
{
	if(m_pTemp==NULL||m_nAllocTempLen<=0) return -1;
	
	int i,nSum=0;
	for(i=0;i<CDocNode::m_nAllocTempLen;i++)
	{
		if(CDocNode::m_pTemp[i].s_dWeight>0) nSum++;
	}

	doc.words=(SVM_WORD *)malloc(sizeof(SVM_WORD)*(nSum+12));
	nSum=0;
	for(i=0;i<CDocNode::m_nAllocTempLen;i++)
	{
		if(CDocNode::m_pTemp[i].s_dWeight>0)
		{
			//DOC的特征ID从1开始
			(doc.words[nSum]).wnum=i+1;
			(doc.words[nSum]).weight=CDocNode::m_pTemp[i].s_dWeight;
			nSum++;
		}
	}
	(doc.words[nSum]).wnum=0;
	doc.docnum=-1;
	return nSum;
}

int CDocNode::ScanChineseStringWithDict(char *pPath,CWordList& wordList)
{
	char *buffer=pPath;
	int i,j,k;
	short l,n;
	char gram[12];
	//realcnt为文章中去掉停用词后剩下的总共词数
	//nStart为一个句子在buffer中的开始位置
	int realcnt=0,nStart=0,nNewStart=0;
	//句子的长度
	int nSentenceLen=0;
	memset(m_pTemp,0,sizeof(sWeightNode)*m_nAllocTempLen);
	CWordNode wordNode;
	while(buffer[nStart]!='\0')
	{
		nSentenceLen=ParseFile(buffer,nStart,nNewStart);
		nStart=nNewStart;
		if(nSentenceLen==0) continue;
		if(m_pSentence[0]>0) //如果是一个英文单词
		{
			//如果英文单词的长度大于等于2,且不是数字
			if((nSentenceLen>=2)&&((m_pSentence[0]<'0')||(m_pSentence[0]>'9')))
			{
				if(wordList.Lookup(m_pSentence,wordNode))
				{
					m_pTemp[wordNode.m_nWordID].s_idxWord=wordNode.m_nWordID;
					m_pTemp[wordNode.m_nWordID].s_tfi+=1;
					m_pTemp[wordNode.m_nWordID].s_dWeight=wordNode.m_dWeight;
					realcnt++;
				}
			}
		}
		else //如果是汉字串
		{
			if(nSentenceLen%2!=0) continue;
			//倒着扫描句子，这样分词的准确率高一些
			i=nSentenceLen;
			while(i>0)
			{
				//最长扫描5个汉字的单词
				if(i>10) k=10;
				else k=i;
				for(j=k;j>0;j=j-2)
				{
					//将Gram项拷贝到gram中
					n=0;
					for(l=j;l>0;l--) gram[n++]=m_pSentence[i-l];
					gram[n]='\0';
					if(wordList.Lookup(gram,wordNode))
					{
						m_pTemp[wordNode.m_nWordID].s_idxWord=wordNode.m_nWordID;
						m_pTemp[wordNode.m_nWordID].s_tfi+=1;
						m_pTemp[wordNode.m_nWordID].s_dWeight=wordNode.m_dWeight;
						realcnt++;
						i=i-j+2;
						break;
					}
				}
				i=i-2;
			}
		}
	}
	//对文档向量中的每一维进行加权
	if(realcnt>0)
	{
		double sum=0;
		for(i=0;i<m_nAllocTempLen;i++)
		{
			m_pTemp[i].s_dWeight*=m_pTemp[i].s_tfi;
			sum+=(m_pTemp[i].s_dWeight*m_pTemp[i].s_dWeight);
		}
		sum=sqrt(sum);
		for(i=0;i<m_nAllocTempLen;i++)
			m_pTemp[i].s_dWeight/=sum;
	}
	return realcnt;
}

int CQueryNode::ScanChineseStringWithDict(char *pPath,CWordList& wordList)
{
	char *buffer=pPath;
	int i,j,k;
	short l,n;
	char gram[12];
	//realcnt为文章中去掉停用词后剩下的总共词数
	//nStart为一个句子在buffer中的开始位置
	int realcnt=0,nStart=0,nNewStart=0;
	//句子的长度
	int nSentenceLen=0;
	memset(m_pTemp,0,sizeof(sWeightQueryNode)*m_nAllocTempLen);
	CWordNode wordNode;
	while(buffer[nStart]!='\0')
	{
		nSentenceLen=ParseFile(buffer,nStart,nNewStart);
		nStart=nNewStart;
		if(nSentenceLen==0) continue;
		if(m_pSentence[0]>0) //如果是一个英文单词
		{
			//如果英文单词的长度大于等于2,且不是数字
			if((nSentenceLen>=2)&&((m_pSentence[0]<'0')||(m_pSentence[0]>'9')))
			{
				if(wordList.Lookup(m_pSentence,wordNode))
				{
					m_pTemp[wordNode.m_nWordID].s_idxWord=wordNode.m_nWordID;
					m_pTemp[wordNode.m_nWordID].s_tfi+=1;
					m_pTemp[wordNode.m_nWordID].s_dWeight=wordNode.m_dWeight;
					realcnt++;
				}
			}
		}
		else //如果是汉字串
		{
			if(nSentenceLen%2!=0) continue;
			//倒着扫描句子，这样分词的准确率高一些
			i=nSentenceLen;
			while(i>0)
			{
				//最长扫描5个汉字的单词
				if(i>10) k=10;
				else k=i;
				for(j=k;j>0;j=j-2)
				{
					//将Gram项拷贝到gram中
					n=0;
					for(l=j;l>0;l--) gram[n++]=m_pSentence[i-l];
					gram[n]='\0';
					if(wordList.Lookup(gram,wordNode))
					{
						m_pTemp[wordNode.m_nWordID].s_idxWord=wordNode.m_nWordID;
						m_pTemp[wordNode.m_nWordID].s_tfi+=1;
						m_pTemp[wordNode.m_nWordID].s_dWeight=wordNode.m_dWeight;
						realcnt++;
						i=i-j+2;
						break;
					}
				}
				i=i-2;
			}
		}
	}
	//对文档向量中的每一维进行加权
	if(realcnt>0)
	{
		double sum=0;
		for(i=0;i<m_nAllocTempLen;i++)
		{
			m_pTemp[i].s_dWeight*=m_pTemp[i].s_tfi;
			sum+=(m_pTemp[i].s_dWeight*m_pTemp[i].s_dWeight);
		}
		sum=sqrt(sum);
		for(i=0;i<m_nAllocTempLen;i++)
			m_pTemp[i].s_dWeight/=sum;
	}
	return realcnt;
}


int CDocNode::ScanChineseWithDict(char *pPath,CWordList& wordList)
{
	CFile fin;
	char *buffer;
	strcpy(pTempStr,pPath);
	strcat(pTempStr,"\\");
	strcat(pTempStr,m_strDocName.GetBuffer(0));
	if(!fin.Open(pTempStr,CFile::modeRead))
		return -1;

	int flen=fin.GetLength();
	buffer=new char[flen+1];
	flen=fin.ReadHuge(buffer,flen);
	buffer[flen]='\0';
	fin.Close();
	int num=ScanChineseStringWithDict(buffer,wordList);
	delete[] buffer;
	return num;
}

int CQueryNode::ScanChineseWithDict(CWordList& wordList)
{
	char *buffer;	
	unsigned int tlen=m_Text.GetLength();
	buffer=new char[tlen+1];

	CString str = _T(m_Text);
	int len = str.GetLength()*sizeof(TCHAR);
	memcpy(buffer, str, len);
	
	m_Text.GetBuffer(m_Text.GetLength());

	//tlen=fin.ReadHuge(buffer,flen);
	buffer[tlen]='\0';
	int num=ScanChineseStringWithDict(buffer,wordList);
	delete[] buffer;
	return num;
}


int CDocNode::ScanEnglishStringWithDict(char *pPath, CWordList &wordList, bool bStem)
{
	char *buffer=pPath;
	_strlwr(buffer);

	int nFilePos=0;
	int realcnt=0,wordLen=0;
	char c, *p;
	p=buffer;
	memset(m_pTemp,0,sizeof(sWeightNode)*m_nAllocTempLen);
	CWordNode wordNode;
	while(buffer[nFilePos]!='\0')
	{
		c=buffer[nFilePos];
		if(c==' '||c=='\r'||c=='\n'||
			(c>32&&c<=47)||(c>=58&&c<=64)||(c>=91&&c<=96)||(c>=123&&c<=127))
		{
			buffer[nFilePos]='\0';
			wordLen=buffer+nFilePos-p;
			if(wordLen>2)
			{
				if(bStem) theStemmer.stem(p,0,wordLen-1);
				if(wordList.Lookup(p,wordNode))
				{
					m_pTemp[wordNode.m_nWordID].s_idxWord=wordNode.m_nWordID;
					m_pTemp[wordNode.m_nWordID].s_tfi+=1;
					m_pTemp[wordNode.m_nWordID].s_dWeight=wordNode.m_dWeight;
					realcnt++;
				}
			}
			p=buffer+nFilePos+1;
		}
		nFilePos++;
	}

	//对文档向量中的每一维进行加权
	int i;
	if(realcnt>0)
	{
		double sum=0;
		for(i=0;i<m_nAllocTempLen;i++)
		{
			m_pTemp[i].s_dWeight*=m_pTemp[i].s_tfi;
			sum+=(m_pTemp[i].s_dWeight*m_pTemp[i].s_dWeight);
		}
		sum=sqrt(sum);
		for(i=0;i<m_nAllocTempLen;i++)
			m_pTemp[i].s_dWeight/=sum;
	}
	return realcnt;
}

int CQueryNode::ScanEnglishStringWithDict(char *pPath, CWordList &wordList, bool bStem)
{
	char *buffer=pPath;
	_strlwr(buffer);

	int nFilePos=0;
	int realcnt=0,wordLen=0;
	char c, *p;
	p=buffer;
	memset(m_pTemp,0,sizeof(sWeightNode)*m_nAllocTempLen);
	CWordNode wordNode;
	while(buffer[nFilePos]!='\0')
	{
		c=buffer[nFilePos];
		if(c==' '||c=='\r'||c=='\n'||
			(c>32&&c<=47)||(c>=58&&c<=64)||(c>=91&&c<=96)||(c>=123&&c<=127))
		{
			buffer[nFilePos]='\0';
			wordLen=buffer+nFilePos-p;
			if(wordLen>2)
			{
				if(bStem) theStemmer.stem(p,0,wordLen-1);
				if(wordList.Lookup(p,wordNode))
				{
					m_pTemp[wordNode.m_nWordID].s_idxWord=wordNode.m_nWordID;
					m_pTemp[wordNode.m_nWordID].s_tfi+=1;
					m_pTemp[wordNode.m_nWordID].s_dWeight=wordNode.m_dWeight;
					realcnt++;
				}
			}
			p=buffer+nFilePos+1;
		}
		nFilePos++;
	}

	//对文档向量中的每一维进行加权
	int i;
	if(realcnt>0)
	{
		double sum=0;
		for(i=0;i<m_nAllocTempLen;i++)
		{
			m_pTemp[i].s_dWeight*=m_pTemp[i].s_tfi;
			sum+=(m_pTemp[i].s_dWeight*m_pTemp[i].s_dWeight);
		}
		sum=sqrt(sum);
		for(i=0;i<m_nAllocTempLen;i++)
			m_pTemp[i].s_dWeight/=sum;
	}
	return realcnt;
}

int CDocNode::ScanEnglishWithDict(char *pPath, CWordList &wordList, bool bStem)
{
	CFile fin;
	char *buffer;
	strcpy(pTempStr,pPath);
	strcat(pTempStr,"\\");
	strcat(pTempStr,m_strDocName.GetBuffer(0));
	if(!fin.Open(pTempStr,CFile::modeRead))
		return -1;

	int flen=fin.GetLength();
	buffer=new char[flen+1];
	flen=fin.ReadHuge(buffer,flen);
	buffer[flen]='\0';
	fin.Close();
	int num=ScanEnglishStringWithDict(buffer,wordList,bStem);
	delete[] buffer;
	return num;
}

int CQueryNode::ScanEnglishWithDict(CWordList &wordList, bool bStem)
{
	char *buffer;	
	unsigned int tlen=m_Text.GetLength();
	buffer=new char[tlen+1];

	CString str = _T(m_Text);
	int len = str.GetLength()*sizeof(TCHAR);
	memcpy(buffer, str, len);
	
	m_Text.GetBuffer(m_Text.GetLength());

	//tlen=fin.ReadHuge(buffer,flen);
	buffer[tlen]='\0';
	int num=ScanEnglishStringWithDict(buffer,wordList,bStem);
	delete[] buffer;
	return num;
}

double CDocNode::ComputeSimilarityRatio()
{
	double sum=0.0;
	for(int i=0;i<m_nAllocLen;i++)
		sum+=m_sWeightSet[i].s_dWeight*m_pTemp[m_sWeightSet[i].s_idxWord].s_dWeight;
	return sum;
}


//对pBuffer中的文字进行处理，得到一个句子的结束位置nEnd
//并且将去掉空格回车等字母后的句子放入变量m_pSentence
int CDocNode::ParseFile(char *pBuffer, int nStart, int &nEnd)
{
	//nSum为得到的句子包含的字节数
	int nCurrent,nSum=0;
	byte bChar[2];
	//是否为中文句子
	bool bChinese=true;
	nCurrent=nStart;
	//去掉句首的全角和半角空格
	//判断句子是以中文字母开头，还是英文字母开头
	while(pBuffer[nCurrent]!='\0')
	{
		bChar[0]=pBuffer[nCurrent];
		if(bChar[0]==' '||bChar[0]=='\r'||bChar[0]=='\n')
			nCurrent++;
		else if(bChar[0]==0xA1)
		{
			bChar[1]=pBuffer[nCurrent+1];
			if(bChar[1]==0xA1) 
				nCurrent+=2;
			else
			{
				bChinese=true;
				break;
			}
		}
		else if(pBuffer[nCurrent]>0)
		{
			bChinese=false;
			break;
		}
		else
		{
			bChinese=true;
			break;
		}
	}

	while(pBuffer[nCurrent]!='\0')
	{
		bChar[0]=pBuffer[nCurrent];
		if(bChar[0]>127)
		{	
			if(!bChinese) break;
			nCurrent++;
			bChar[1]=pBuffer[nCurrent];
			//0xA1A1为全角的空格
			if((bChar[0]!=0xA1)||(bChar[1]!=0xA1))
			{
				//如果为"的"字,或为标点符号或其它全角字母
				if(((bChar[0]==0xB5)&&(bChar[1]==0xC4))||
					((bChar[0]==0xA1)&&(bChar[1]>0xA1)&&(bChar[1]<=0xFE))||
					((bChar[0]==0xA2)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xFC))||
					((bChar[0]==0xA3)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xFE))||
					((bChar[0]==0xA4)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF3))||
					((bChar[0]==0xA5)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF6))||
					((bChar[0]==0xA6)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF5))||
					((bChar[0]==0xA7)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF1))||
					((bChar[0]==0xA8)&&(bChar[1]>=0x40)&&(bChar[1]<=0xE9))||
					((bChar[0]==0xA9)&&(bChar[1]>=0x40)&&(bChar[1]<=0xEF))||
					((bChar[0]==0xAA)&&(bChar[1]==0xA5)))
				{
					nCurrent++;
					break;
				}
				//如果后半部分小于0x80，代表是一个错误的汉字
				else if(bChar[1]>=0x80)
				{
					m_pSentence[nSum]=pBuffer[nCurrent-1];
					nSum++;
					m_pSentence[nSum]=pBuffer[nCurrent];
					nSum++;
				}
			}
		}
		else
		{
			if(bChar[0]==' '||bChar[0]=='\r'||bChar[0]=='\n')
			{
				if(!bChinese)
				{
					nCurrent++;
					break;
				}
			}
			else
			{
				//if(bChar[0]=='!'||bChar[0]=='?'||bChar[0]==':'||
				//	bChar[0]==';'||bChar[0]=='.')
				if((bChar[0]>32&&bChar[0]<=47)||(bChar[0]>=58&&bChar[0]<=64)||
					(bChar[0]>=91&&bChar[0]<=96)||(bChar[0]>=123&&bChar[0]<=127))
				{
					nCurrent++;
					break;
				}
				else if(bChinese) break;
				else
				{
					m_pSentence[nSum]=pBuffer[nCurrent];
					nSum++;
				}
			}
		}
		nCurrent++;
	}
	m_pSentence[nSum]='\0';
	nEnd=nCurrent;
	return nSum;
}

bool CCatalogList::BuildCatalogID(CCatalogList & catalogList)
{
	POSITION pos=GetFirstPosition();
	CString strCatalogName;
	short cataID;
	while(pos!=NULL)
	{
		CCatalogNode& cataNode=GetNext(pos);
		cataID=catalogList.GetCataIDByName(cataNode.m_strCatalogName);
		if(cataID<0) return false;
		cataNode.m_idxCata=cataID;
	}
	return true;
}

bool CDocNode::IsNumber(char *p)
{
	int i=0;
	while(p[i]!='\0'&&p[i]>='0'&&p[i]<='9') i++;
	if(p[i]=='\0') return true;
	else return false;
}

//函数将smart格式的类别字符串line转换一个类别ID数组
//函数的返回值为字符串line中包含无法识别的类别总数
short CCatalogList::GetCataIDArrayFromString(char * line, CArray<short, short> &aryCataID)
{
	int pos=0,id,d;
	short result=0;
	char type[MAX_PATH];

	aryCataID.RemoveAll();
	while(sscanf(line+pos,"%s %d",type,&d) != EOF)
	{
		id=GetCataIDByName(type);
		if(id>=0)
		{
			aryCataID.Add(id);
			while(line[pos]!=';'&&line[pos]!='\r'&&line[pos]!='\0') 
				pos++;
			if(line[pos]==';') pos=pos+2;
		}
		else result++;
	}
	return result;
}

CDocNode& CCatalogNode::GetAt(POSITION position)
{
	return m_lstDocList.GetAt(position);
}

int CCatalogList::GetSaveMode()
{
	return CCatalogList::m_nSaveMode;
}


double CDocNode::ComputeProbability(CWordList &wordlist,int n)
{
	/*
	double sum=0.0;
	for(int i=0;i<m_nAllocTempLen;i++)
	{
		int j=m_pTemp[i].s_idxWord;
		CString str = wordlist.GetWordByID(i);
		CWordNode &wordnode = wordlist.m_lstWordList[str];
		sum+= wordnode.m_pCataWeightPro[n];
	}
	return sum;
	*/
	return m_nAllocLen;
}
/*
long CCatalogList::ScanDirectory_Log(CString strLogDirName)
{
	if(_chdir(strLogDirName))  // if can't find the dir
	{
		CString	csTmp = "目录";
		csTmp+=strLogDirName;
		csTmp+="不存在!";
		AfxMessageBox(csTmp);
		return -1;
	}

	CString time,userID,text,order,URL;//日志属性
	int rank;
	CString temp,fileName1,fileName2;

	/*读取6月1号的日志
	for(int k=1;k<2;k++){
		//sprintf(filename,"access_log.2008060%d.decode.filter",k);
		//fin.open(filename);
		fin.open("result_sort_hash2.txt",ios::in);
		
		while(fin>>time>>userID>>text>>rank>>order>>URL){
			log1.setTime(time);
			log1.setUserID(userID);
			log1.setText(text);
			log1.setRank(rank);
			log1.setOrder(order);
			log1.setURL(URL);
			coll.push_back(log1);
		}		
		fin.close();
	}

	long LogNum=0;
	short idxCurCata=0;

	HANDLE hFinder;
	LPWIN32_FIND_DATA lpFindFileData;	
	lpFindFileData  = new WIN32_FIND_DATA;
	hFinder = ::FindFirstFile(strLogDirName+"\\*.*",lpFindFileData );
	while(::FindNextFile(hFinder,lpFindFileData))
	{
		if( !strcmp(lpFindFileData->cFileName,".") || !strcmp(lpFindFileData->cFileName,"..") )
			continue;

		if(!(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CCatalogNode catalognode;
			////找到日志结点对应的日志文件
			//CLogNode& lognode=catalognode.GetFirstPosition();
			catalognode.m_strCatalogName=lpFindFileData->cFileName;

			//读日志文件到每个lognode

			CLogNode lognode;
			lognode.m_strLogName=lpFindFileData->cFileName;
			lognode.m_idxLog=idxCurCata++;
			lognode.m_nAllocLen=0;
			lognode.m_sWeightSet=NULL;
			lognode.m_nClassNum=0;
			lognode.m_pResults=NULL;
			catalognode.AddLog(lognode);

			////
			catalognode.m_idxCata=idxCurCata++;
			POSITION posTemp=AddCata(catalognode);

			//CCatalogNode catalognode;
			catalognode.m_strCatalogName=lpFindFileData->cFileName;
			catalognode.m_strDirName=strPath+"\\"+catalognode.m_strCatalogName;
			catalognode.m_idxCata=idxCurCata++;
			POSITION posTemp=AddCata(catalognode);
			CCatalogNode& cataTemp=GetAt(posTemp);
			cataTemp.SetStartDocID(docNum);
			docNum=cataTemp.ScanDirectory(catalognode.m_strDirName);
		}
	}
	delete	lpFindFileData;
	return docNum;
	
	CString strTarget,strSource;
		

	return 0;
}*/



int CLogFileNode::readLogFile()
{
	FILE *fp;
	if((fp=fopen(m_strLogFileName,"r"))==NULL)
	{
		AfxMessageBox("无法打开文件"+m_strLogFileName+"!");
		return false;
	}
	char no[10],feature[MAX_PATH],line[MAX_PATH];
	//CString time,userID,text,URL;
	double userID,order;
	int rank;
	float weight=1.0;
	unsigned int num=0;
	char time[20],text[100000],URL[10000];

	while(!feof(fp)&&fgets(line,MAX_PATH,fp))
	{
		//ID,field,layer,length,sim,tfidf
		if(sscanf(line,"%s %s %d %lf %lf %s",time,URL,&rank,&order,&userID,text)>0)
		//if(sscanf(line,"%s %s %s %d %d %s",time,userID,text,&rank,&order,URL)>0)
		{
			CLogNode lognode;
			lognode.setTime(time);                                                                                                                                                                                                                                                                                                                                                                                                          			lognode.setUserID(userID);
			lognode.setText(text);
			lognode.setRank(rank);
			lognode.setOrder(order);
			lognode.setURL(URL);
			AddLogNode(lognode);
		}
		/*else
		{
			CString str;
			str.Format("文件的第%d行格式错误!",num+1);
			AfxMessageBox(str);
			fclose(fp);
			return false;
		}*/
		num++;		
	}
	fclose(fp);
}

POSITION CLogFileNode::AddLogNode(CLogNode& lognode)///xby
{
	return m_lstLogList.AddTail(lognode);
}

//类节点构造
const CLogFileNode& CLogFileNode::operator = (const CLogFileNode& x)
{
	if(this==&x) return *this;
	m_lTotalFileNum = x.m_lTotalFileNum ;
	m_strLogFileName=x.m_strLogFileName;
	m_strDirName=x.m_strDirName;
	m_idxCata=x.m_idxCata;
	m_lstLogList.RemoveAll();
	POSITION pos = x.m_lstLogList.GetHeadPosition();
	while(pos!=NULL)
	{
		CLogNode& lognode=x.m_lstLogList.GetNext(pos);
		m_lstLogList.AddTail(lognode);
	}
	return *this;
}

const CLogFileNode& CLogFileNode::operator += (const CLogFileNode& x)
{
	if(this==&x) return *this;
	m_lTotalFileNum += x.m_lTotalFileNum ;
	m_strLogFileName=x.m_strLogFileName;
	m_strDirName=x.m_strDirName;
	m_idxCata=x.m_idxCata;
	POSITION pos = x.m_lstLogList.GetHeadPosition();
	while(pos!=NULL)
	{
		CLogNode& lognode=x.m_lstLogList.GetNext(pos);
		m_lstLogList.AddTail(lognode);
	}
	return *this;
}

CLogFileNode::CLogFileNode()
{
	m_idxCata=-1;
	m_lCurLogID=0;
	m_lTotalFileNum=0;
	//InitLogFileNode();
}

CLogFileNode::~CLogFileNode()
{
	InitLogFileNode();
}

///xby
void CLogFileNode::InitLogFileNode(int nMode)
{
	m_lTotalFileNum = 0;
	POSITION pos_log=m_lstLogList.GetHeadPosition();
	while(pos_log!=NULL)
	{
		CLogNode& lognode=m_lstLogList.GetNext(pos_log);
		//lognode.DeallocBuffer();
		//lognode.DeallocResultsBuffer();
	}
	if(nMode>0) m_lstLogList.RemoveAll();
}

CLogFileNode::CLogFileNode(const CLogFileNode& x)
{
	*this=x;
}

CLogNode CLogFileNode::GetFirstNode()
{
	return m_lstLogList.GetHead();
}

void CCatalogNode::ScanLogtoQuery(double k,bool url)///xby 扫描得到查询信息 //处理每个lognode，得到querynode
{
	//m_lstQueryList
    double m1=k;
	bool m2=url;
	m_lTotalQueryNum = 0;
	POSITION pos_logfile=m_lstLogFileList.GetHeadPosition();
	while(pos_logfile!=NULL)
	{		
		CLogFileNode& logfilenode=m_lstLogFileList.GetNext(pos_logfile);
		logfilenode.ComputeN(m_lstQueryList,2,5,m1,m2);
	}
}

char Change(char word)
{
 if(islower(word))
   return toupper(word);
 if(isupper(word))
	 return tolower(word);
 char c=' ';
 return c;

}

int Compare(int type,char *URL,char Keyword[])   //查询URL中是否含有keyword,type=0倒序
{
	int i=0,j=0,m=0,sum=0;
	if(type==0)
	{
	  while(URL[i])
		  i++;
	  m=strlen(Keyword);
	  m--;
	  i--;
      for(;(m>=0)&&(i>=0);i--,m--)
	  {
        if((URL[i]!=Keyword[m])&&(URL[i]!=Change(Keyword[m])))
			return 0;
	  }
	  if(m<0)
	    return 1;
	}
	while(URL[j])
	{
	 i=j;
	 m=0;
	 for(;;i++,m++)
	 {	 /*CString str;
		 str.Format("%c",URL[i]);
		AfxMessageBox(str);*/
		 if(Keyword[m]=='\0')
			 sum++;
		 else if(URL[i]=='\0')
			     break;
		 else if((URL[i]!=Keyword[m])&&(URL[i]!=Change(Keyword[m])))
		     break;
	 }
	 j++;
	}
	return sum;
}

//type=0导航类，type=1信息类，type=2事务类
int Seek(int type,char *URL)
{ 
	
	int i=0,count=0;//cout<<URL<<endl;
    if(type==0)  //导航类地址比较
  {
	    char keyword[][20]={"Health"};
	  //char keyword[][20]={".jsp",".cn",".com",".com/",".cn/","biz?","cpc?",".asp"};
	  //for(i=1;i>0;i++)
	  //{
		 if(Compare(0,URL,keyword[0]))
		    return 1;
	  //}
  }
       else  if(type==1)//信息类地址比较
	  {
	   //char keyword[][20]={//"wiki","baike","ask","nc",".shtml","com",".htm",
			  //"com",".htm","shtml"
			  //"click.","cpc","index","aspx","cn","news",
			  //"zhidao.daidu",".asp","edu",".net","gov.","qq"
		 // };
		 char keywords[][20]={"Sports"};
		 if(Compare(1,URL,keywords[0]))
		    return 1;
	      //char keywords[][20]={"cn",".htm","shtml"};
	      //int k=0;
		  //for(k=1;k>0;k--)
		      //count=count+Compare(1,URL,keyword[k]);
          //if(count>=2)
			     //return 1;
		  //for(k=2;k>=0;k--)
			  //count=count+Compare(1,URL,keywords[k]);
          //if(count>=2)
			    // return 1;
	  }
	    else if(type==2)//事务类地址比较
		{
			char keywords[][20]={"Politic"};
				 /*char keywords[][20]={"soft","ent.","wav","song",
					  "avi.","mp3","music","movie","game","wma","play",
					  "youxi","zol.","tv","view","php","iask","file",
					  "film","show","editor","flash","down","video.",
                      "article","read","sing","youku","tudou","qq","duote",
					  "sohu","ezhun"};
			   for(int p=32;p>=0;p--)
			   {
				   if(Compare(2,URL,keywords[p])>=1)
					   count++;
			   }
			   if(count>=2)*/
			if(Compare(2,URL,keywords[0]))
				   return 1;
		
		}
	return 0;
}



void CLogFileNode::ComputeN(CList<CQueryNode,CQueryNode&>& QueryList ,int CS_n,int RS_n,double k,bool url)
{
	
	//CString session= "";
	CString query=""; 
	int query_num=0;
	int pre=0,cur=0;
    
	int S=2000;
	double UKS_i[2000]={0.0},UKS_n[2000]={0.0},UKS_t[2000]={0.0};
	double sum_i=0;
	double sum_n=0;
	double sum_t=0;
	double field1=0;
	double field2=0;
	double field3=0;

	int intLength;
    int sum_all=0;

	FILE *stream;
	if( (stream  = fopen( "E:\\res2.txt", "w+" )) == NULL )
	{
		CString str;
		str.Format("无法创建文件e");
		AfxMessageBox(str);
		return;
	}

	POSITION pos_query=QueryList.GetHeadPosition();
	while(pos_query!=NULL)
	{		
		CQueryNode& querynode=QueryList.GetNext(pos_query);
		//querynode.setTotalSessionNum();	
	}
	CQueryNode querynode;

	bool cur_flag=true,pre_flag=true;
	////计算nCS和nRS
	if(GetLogNum()!=0)
	{
		//nCS所需变量
		query_num++;
		querynode.setText(GetFirstNode().getText());
		querynode.m_nTotalSessionNum++;
		cur++;

	}
	//QueryList.AddTail(querynode);

	POSITION pos,pre_pos,temp_pos;

  
	CQueryNode& pre_querynode=querynode;
	int as=GetLogNum();
	for (int  i=0; (pos!=NULL)&&(i<GetLogNum()); i++ )
	{
	
		pos=m_lstLogList.FindIndex(i);

		CString str2=m_lstLogList.GetAt(pos).getURL();
		char *URL=str2.GetBuffer(0);
		field1=Seek(0,URL);
		field2=Seek(1,URL);
		field3=Seek(2,URL);
		
        
			pre_querynode.nCS=m_lstLogList.GetAt(pos).getRank();
			pre_querynode.nRS=m_lstLogList.GetAt(pos).getOrder();//length
			pre_querynode.Length=m_lstLogList.GetAt(pos).getUserID();//sim
			//pre_querynode.Length=(long double)intLength;

			pre_querynode.UKS_N=field1;
			pre_querynode.UKS_I=field2;
			pre_querynode.UKS_T=field3;
					
			//开始下一个query

			QueryList.AddTail(pre_querynode);	//结束前一个query
			fprintf(stream,"%lf",pre_querynode.Length);
			//fprintf(stream," %s\n %-5.2f,%f,%f,%f,%f,%d",pre_querynode.m_Text,pre_querynode.Length,pre_querynode.nCS,pre_querynode.nRS,pre_querynode.UKS_N,pre_querynode.UKS_I,pre_querynode.UKS_T,pre_querynode.m_nTotalSessionNum);
			fprintf(stream,"\n");

			pre_querynode.Reset();
			pre_querynode.m_nTotalSessionNum++;                  //新的query
			pre_querynode.setText(m_lstLogList.GetAt(pos).getText());
		
	}

	fprintf(stream,"query, Length, nCS, nRS, UKS_N, UKS_I, UKS_T, total\n");
	

	/*for( i=0;i<GetLogNum();i++)
	{
		pos=QueryList.FindIndex(i); 
		CQueryNode& temp_querynode=QueryList.GetAt(pos);
	
		 //fprintf(stream," %s\n,%f,%f,%f,%f,%f,%f,%d",temp_querynode.m_Text,temp_querynode.Length,temp_querynode.nCS,temp_querynode.nRS,temp_querynode.UKS_N,temp_querynode.UKS_I,temp_querynode.UKS_T,temp_querynode.m_nTotalSessionNum);
		fprintf(stream,"%f",temp_querynode.Length);
		 fprintf(stream,"\n");
	}*/

	fclose(stream);
	//return 1;

}

void CQueryNode::setText(CString text)
{
	this->m_Text=text;
}

void CQueryNode::Reset()
{
	m_sWeightQuerySet = NULL;
	m_pResults   = NULL;
	m_nAllocLen=0;
	m_nClassNum=0;
	m_idxQuery=-1;
	m_nCataID=-1;
	m_nTotalSessionNum=0;
	m_Text="";

	m_nbelow=0.0;         //计算nCS用
	m_nRank_below=0.0;    //计算nRS用

	nCS=0.0;
	nRS=0.0;
	Length=0.0;

	UKS_N=0.0;
	UKS_I=0.0;
	UKS_T=0.0;
}


int CQueryNode::ScanChineseText(CString text,CWordList& wordList,int nCataNum, short idxCata)
{
	char *buffer;	
	unsigned int tlen=text.GetLength();
	buffer=new char[tlen+1];

	CString str = _T(text);
	int len = str.GetLength()*sizeof(TCHAR);
	memcpy(buffer, str, len);
	
	text.GetBuffer(text.GetLength());

	//tlen=fin.ReadHuge(buffer,flen);
	buffer[tlen]='\0';
	//fin.Close();
	int num=ScanChineseString(buffer,wordList,nCataNum,m_idxQuery,idxCata);
	//int num=0;
	delete[] buffer;
	return num;
}

int CQueryNode::ScanChineseString(char* pPath,CWordList& wordList,int nCataNum, long idxQuery, short idxCata)
{
	char *buffer=pPath;
	int i,j,sum;
	char *w;
	//realcnt为文章中去掉停用词后剩下的总共词数
	//nStart为一个句子在buffer中的开始位置
	int nStart=0,nNewStart=0;
	bool flag=true;
	int nSentenceLen=0;
	int realcnt=0;
	while(buffer[nStart]!='\0')
	{
		flag=true;
		nSentenceLen=ParseFile(buffer,nStart,nNewStart);
		nStart=nNewStart;
		if(nSentenceLen==0) continue;
		if(m_pSentence[0]>0) //如果是一个英文单词
		{
			//如果英文单词的长度大于等于2,且不是数字
			if((nSentenceLen>=2)&&((m_pSentence[0]<'0')||(m_pSentence[0]>'9')))
			{
				wordList.Add(m_pSentence,idxCata,idxQuery,nCataNum);
				realcnt++;
			}
		}
		else //如果是汉字串
		{
			if(nSentenceLen%2!=0) continue;
			if(nSentenceLen<=6) //如果是单个汉字
			{
				wordList.Add(m_pSentence,idxCata,idxQuery,nCataNum);
				realcnt++;
			}
			else
			{
				g_wordSeg.Segment(m_pSentence);
				for(i=0;i<g_wordSeg.GetSegmentCount();i++)
				{
					sum=0;
					PWORD_RESULT pItem=g_wordSeg.GetWordSeg(i);
					while(pItem[sum].sWord[0]!=0) sum++;
				
					for(j=1;j<sum-1;j++)
					{
						w=pItem[j].sWord;
						if(w[0]=='\0'||g_wordSeg.isInStopWords(w)) flag=false;		
						if(flag)
						{
							wordList.Add(w,idxCata,idxQuery,nCataNum);
							realcnt++;
						}
						flag=true;/////xby
					}
				}
			}
		}
	}
	return realcnt;
}





int CQueryNode::ScanEnglishText(CString text, CWordList &wordList, int nCataNum, short idxCata, bool bStem)
{
	char *buffer;	
	unsigned int tlen=text.GetLength();
	buffer=new char[tlen+1];

	CString str = _T(text);
	int len = str.GetLength()*sizeof(TCHAR);
	memcpy(buffer, str, len);
	
	text.GetBuffer(text.GetLength());

	//tlen=fin.ReadHuge(buffer,flen);
	buffer[tlen]='\0';


	int num=ScanEnglishString(buffer,wordList,nCataNum,m_idxQuery,idxCata,bStem);
	delete[] buffer;
	return num;

}

int CQueryNode::ScanEnglishString(char *pPath, CWordList &wordList, int nCataNum, long idxDoc, short idxCata, bool bStem)
{
	char *buffer=pPath;
	_strlwr(buffer);

	int nFilePos=0;
	int realcnt=0,wordLen=0;
	char c, *p;
	p=buffer;
	while(buffer[nFilePos]!='\0')
	{
		c=buffer[nFilePos];
		if(c==' '||c=='\r'||c=='\n'||
			(c>32&&c<=47)||(c>=58&&c<=64)||(c>=91&&c<=96)||(c>=123&&c<=127))
		{
			buffer[nFilePos]='\0';
			wordLen=buffer+nFilePos-p;
			if(wordLen>2)
			{
				if(bStem) theStemmer.stem(p,0,wordLen-1);
				wordList.Add(p,idxCata,idxDoc,nCataNum);
				realcnt++;
			}
			p=buffer+nFilePos+1;
		}
		nFilePos++;
	}
	return realcnt;
	return 0;
}

//对pBuffer中的文字进行处理，得到一个句子的结束位置nEnd
//并且将去掉空格回车等字母后的句子放入变量m_pSentence
int CQueryNode::ParseFile(char *pBuffer, int nStart, int &nEnd)
{
	//nSum为得到的句子包含的字节数
	int nCurrent,nSum=0;
	byte bChar[2];
	//是否为中文句子
	bool bChinese=true;
	nCurrent=nStart;
	//去掉句首的全角和半角空格
	//判断句子是以中文字母开头，还是英文字母开头
	while(pBuffer[nCurrent]!='\0')
	{
		bChar[0]=pBuffer[nCurrent];
		if(bChar[0]==' '||bChar[0]=='\r'||bChar[0]=='\n')
			nCurrent++;
		else if(bChar[0]==0xA1)
		{
			bChar[1]=pBuffer[nCurrent+1];
			if(bChar[1]==0xA1) 
				nCurrent+=2;
			else
			{
				bChinese=true;
				break;
			}
		}
		else if(pBuffer[nCurrent]>0)
		{
			bChinese=false;
			break;
		}
		else
		{
			bChinese=true;
			break;
		}
	}

	while(pBuffer[nCurrent]!='\0')
	{
		bChar[0]=pBuffer[nCurrent];
		if(bChar[0]>127)
		{	
			if(!bChinese) break;
			nCurrent++;
			bChar[1]=pBuffer[nCurrent];
			//0xA1A1为全角的空格
			if((bChar[0]!=0xA1)||(bChar[1]!=0xA1))
			{
				//如果为"的"字,或为标点符号或其它全角字母
				if(((bChar[0]==0xB5)&&(bChar[1]==0xC4))||
					((bChar[0]==0xA1)&&(bChar[1]>0xA1)&&(bChar[1]<=0xFE))||
					((bChar[0]==0xA2)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xFC))||
					((bChar[0]==0xA3)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xFE))||
					((bChar[0]==0xA4)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF3))||
					((bChar[0]==0xA5)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF6))||
					((bChar[0]==0xA6)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF5))||
					((bChar[0]==0xA7)&&(bChar[1]>=0xA1)&&(bChar[1]<=0xF1))||
					((bChar[0]==0xA8)&&(bChar[1]>=0x40)&&(bChar[1]<=0xE9))||
					((bChar[0]==0xA9)&&(bChar[1]>=0x40)&&(bChar[1]<=0xEF))||
					((bChar[0]==0xAA)&&(bChar[1]==0xA5)))
				{
					nCurrent++;
					break;
				}
				//如果后半部分小于0x80，代表是一个错误的汉字
				else if(bChar[1]>=0x80)
				{
					m_pSentence[nSum]=pBuffer[nCurrent-1];
					nSum++;
					m_pSentence[nSum]=pBuffer[nCurrent];
					nSum++;
				}
			}
		}
		else
		{
			if(bChar[0]==' '||bChar[0]=='\r'||bChar[0]=='\n')
			{
				if(!bChinese)
				{
					nCurrent++;
					break;
				}
			}
			else
			{
				//if(bChar[0]=='!'||bChar[0]=='?'||bChar[0]==':'||
				//	bChar[0]==';'||bChar[0]=='.')
				if((bChar[0]>32&&bChar[0]<=47)||(bChar[0]>=58&&bChar[0]<=64)||
					(bChar[0]>=91&&bChar[0]<=96)||(bChar[0]>=123&&bChar[0]<=127))
				{
					nCurrent++;
					break;
				}
				else if(bChinese) break;
				else
				{
					m_pSentence[nSum]=pBuffer[nCurrent];
					nSum++;
				}
			}
		}
		nCurrent++;
	}
	m_pSentence[nSum]='\0';
	nEnd=nCurrent;
	return nSum;
}