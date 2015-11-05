// WordList.cpp: implementation of the CWordList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WordList.h"
#include "cataloglist.h"
#include "fstream.h"
#include "float.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

AFX_INLINE UINT AFXAPI HashKey(CString key)
{
	return HashKey((LPCTSTR)key);
}

void AFXAPI SerializeElements(CArchive& ar,CWordNode* pElements,int nCount)
{
	ASSERT(nCount==0||
		AfxIsValidAddress(pElements,nCount*sizeof(CWordNode)));	
	pElements->Serialize(ar);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWordList::CWordList()
{
	m_lstWordList.InitHashTable(2000);
}

CWordList::~CWordList()
{
}

CWordList& CWordList::operator =(CWordList &x)
{
	if(this==&x) return *this;
	CString str;
	POSITION pos=x.GetFirstPosition();
	while(pos!=NULL)
	{
		CWordNode& wordnodesrc=x.GetNext(pos,str);
		CWordNode& wordnodedst=Add(str);
		wordnodedst=wordnodesrc;
	}
	return *this;
}

void CWordList::InitWordList() // initialize the word list
{
	m_lstWordList.RemoveAll();
}

CWordNode& CWordList::Add(const CString str)
{
	CWordNode& wordnode=m_lstWordList[str];
	return wordnode;
}

CWordNode& CWordList::Add(const CString str, long docID)
{
 	CWordNode& wordnode=m_lstWordList[str];
	if(wordnode.m_lDocID!=docID) 
	{
		wordnode.m_lDocID=docID;
		wordnode.m_lDocFreq++;
	}
	wordnode.m_lWordFreq++;
	return wordnode;
}

CWordNode& CWordList::Add(const CString str, short cataID, long docID, int cataNum)
{
 	CWordNode& wordnode=m_lstWordList[str];
	wordnode.InitBuffer(cataNum);
	if(wordnode.m_lDocID!=docID) 
	{
		wordnode.m_lDocID=docID;
		wordnode.m_pCataDocFreq[cataID]++;
		wordnode.m_lDocFreq++;
	}
	wordnode.m_pCataWordFreq[cataID]++;
	wordnode.m_lWordFreq++;
	return wordnode;
}

bool CWordList::GetFromFile(CString strFileName) // get from mid information files WordList.mid
{
	InitWordList();
	CFile fIn;
	if(!fIn.Open(strFileName,CFile::modeRead) )
	{
		AfxMessageBox("无法打开文件"+strFileName+"!");
		return false;
	}
	CArchive ar(&fIn,CArchive::load);
	Serialize(ar);
	ar.Close();
	
	fIn.Close();
	return true;
}

//特征列表文件中每一行包含特征词和特征词的权重
//格式为feature weight，如果没有weight那么认为weight为1
bool CWordList::GetListFromFile(CString strFileName)
{
	InitWordList();
	FILE *fp;
	if((fp=fopen(strFileName,"r"))==NULL)
	{
		AfxMessageBox("无法打开文件"+strFileName+"!");
		return false;
	}
	char no[10],feature[MAX_PATH],line[MAX_PATH];
	float weight=1.0;
	int num=0;
	while(!feof(fp)&&fgets(line,MAX_PATH,fp))
	{
		if(sscanf(line,"%s %s %f",no,feature,&weight)>0)
		{
			if(weight<=0) weight=1.0;
			CWordNode &node=Add(feature);
			node.m_nWordID=num;
			node.m_dWeight=weight;
		}
		else
		{
			CString str;
			str.Format("文件的第%d行格式错误!",num+1);
			AfxMessageBox(str);
			fclose(fp);
			return false;
		}
		num++;
	}
	fclose(fp);
	return true;
}

void CWordList::DumpWordList(CString strFileName)
{
	FILE *stream;
	if( (stream  = fopen( strFileName, "w+" )) == NULL )
	{
		AfxMessageBox("无法创建文件i"+strFileName+"!");
		return;
	}

	POSITION	pos;
	CString     str;
	CWordNode   wordnode; 
	pos		  = GetFirstPosition();
	while(pos!=NULL)
	{
		wordnode=GetNext(pos, str);
		fprintf(stream,"%d %s %f\n",wordnode.m_nWordID,str,wordnode.m_dWeight);
	}
	fclose(stream);
}

void CWordList::DumpWordProList(CString strFileName,int CataNum)
{
	FILE *stream;
	if( (stream  = fopen( strFileName, "w+" )) == NULL )
	{
		AfxMessageBox("无法创建文件j"+strFileName+"!");
		return;
	}

	POSITION	pos;
	CString     str;
	CWordNode   wordnode; 
	pos		  = GetFirstPosition();
	fprintf(stream,"%d %d\n",m_lstWordList.GetCount(),CataNum);
	while(pos!=NULL)
	{
		wordnode=GetNext(pos, str);
		fprintf(stream,"%d",wordnode.m_nWordID);
		for(int i=0;i<CataNum;i++)
		{
			fprintf(stream," %f",wordnode.m_pCataWeightPro[i]);
		}
		fprintf(stream,"\n");
	}
	fclose(stream);
}

//建立词索引 
void CWordList::IndexWord()
{
	unsigned int i=0;
	CString str;
	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CWordNode& wordnode=GetNext(pos,str);
		wordnode.m_nWordID=i;
		i++;
	}
}

//计算每个特征的权重, 参数sum代表文档集中的文档总数
//参数bMult含义与类CWordNode中函数ComputeWeight相同
void CWordList::ComputeWeight(long sum, bool bMult)
{
	if(sum<=0) return;
	CString str;
	POSITION pos=GetFirstPosition();
	while(pos!=NULL)
	{
		CWordNode& wordNode=GetNext(pos,str);
		wordNode.ComputeWeight(sum,bMult);
	}
}

void CWordList::DumpToFile(CString strFileName)
{
	CFile		fBinOut;
	if(!fBinOut.Open(strFileName,CFile::modeWrite | CFile::modeCreate) )
	{
		AfxMessageBox("无法创建文件k"+strFileName+"!");
		return;
	}
	CArchive ar(&fBinOut,CArchive::store);
	Serialize(ar);
	ar.Close();
	fBinOut.Close();
}


POSITION CWordList::GetFirstPosition()
{
	return m_lstWordList.GetStartPosition();
}

CWordNode& CWordList::GetNext(POSITION& pos, CString &str)
{
	CWordNode node;
	m_lstWordList.GetNextAssoc(pos,str,node);
	return m_lstWordList[str];
}


int CWordList::GetCount()
{
	return m_lstWordList.GetCount();
}

long CWordList::GetWordNum()
{
	long n=0;
	POSITION pos=GetFirstPosition();
	CString strWord;
	while(pos!=NULL)
	{
		CWordNode& wordnode=GetNext(pos,strWord);
		n+=wordnode.GetWordNum();
	}
	return n;
}

void CWordList::Serialize(CArchive &ar)
{
	m_lstWordList.Serialize(ar);
}

BOOL CWordList::Lookup(CString str, CWordNode &wordNode)
{
	return m_lstWordList.Lookup(str,wordNode);
}

CString CWordList::GetWordByID(long wordID)
{
	CString str;
	POSITION pos=GetFirstPosition();
	bool bFound=false;
	while(pos!=NULL)
	{
		CWordNode& wordnode=GetNext(pos,str);
		if(wordnode.m_nWordID==wordID)
		{
			bFound=true;
			break;
		}
	}
	if(bFound) return str;
	else return "";
}

void CWordList::SetAt(CString str,CWordNode& node)
{
	m_lstWordList.SetAt(str,node);
}

void CWordNode::Serialize(CArchive &ar)
{
	if(ar.IsStoring())
	{
		ar<<m_nWordID;
		ar<<m_dWeight;
		ar<<m_lDocFreq;
		ar<<m_lWordFreq;
	}
	else
	{
		ar>>m_nWordID;
		ar>>m_dWeight;
		ar>>m_lDocFreq;
		ar>>m_lWordFreq;
	}
}

CWordNode::CWordNode()
{ 
	m_dWeight=0.0;
	m_nAllocLen=0;
	m_pCataWeight=NULL;
	m_pCataWeightPro=NULL;
	m_pCataDocFreq=NULL;
	m_pCataWordFreq=NULL;
	m_lDocFreq=0;
	m_lWordFreq=0;
	m_nWordID=-1;
	m_lDocID=-1;
}

CWordNode::~CWordNode()
{
	DeallocBuffer();
}

CWordNode& CWordNode::operator = (const CWordNode& x)
{
	if(this==&x) return *this;
	m_nWordID=x.m_nWordID;
	m_dWeight=x.m_dWeight;
	m_lDocID=x.m_lDocID;
	m_nAllocLen=x.m_nAllocLen;
	m_lDocFreq=x.m_lDocFreq;
	m_lWordFreq=x.m_lWordFreq;
	AllocBuffer(x.m_nAllocLen);
	if(x.m_pCataWeight!=NULL)
		memcpy(m_pCataWeight,x.m_pCataWeight,m_nAllocLen*sizeof(double));
	if(x.m_pCataWeightPro!=NULL)
		memcpy(m_pCataWeightPro,x.m_pCataWeightPro,m_nAllocLen*sizeof(double));
	if(x.m_pCataDocFreq!=NULL)
		memcpy(m_pCataDocFreq,x.m_pCataDocFreq,m_nAllocLen*sizeof(long));
	if(x.m_pCataWordFreq!=NULL)
		memcpy(m_pCataWordFreq,x.m_pCataWordFreq,m_nAllocLen*sizeof(long));
	return *this;
}

void CWordNode::DeallocBuffer()
{
	if(m_pCataWeight!=NULL)
	{
		delete []m_pCataWeight;
		m_pCataWeight=NULL;
	}

	if(m_pCataWeightPro!=NULL)
	{
		delete []m_pCataWeightPro;
		m_pCataWeightPro=NULL;
	}


	if(m_pCataDocFreq!=NULL)
	{
		delete []m_pCataDocFreq;
		m_pCataDocFreq=NULL;
	}
	if(m_pCataWordFreq!=NULL)
	{
		delete []m_pCataWordFreq;
		m_pCataWordFreq=NULL;
	}
	m_nAllocLen=0;
}

void CWordNode::InitBuffer(int nLen)
{
	if(nLen<=0) return;
	if(m_nAllocLen<=0&&m_pCataWeight==NULL&&m_pCataWeightPro==NULL&&
		m_pCataDocFreq==NULL&&m_pCataWordFreq==NULL)
	{
		m_nAllocLen=nLen;
		m_pCataWeight=new double[m_nAllocLen];
		memset(m_pCataWeight,0,sizeof(double)*m_nAllocLen);
		m_pCataWeightPro=new double[m_nAllocLen];
		memset(m_pCataWeightPro,0,sizeof(double)*m_nAllocLen);
		m_pCataDocFreq=new long[m_nAllocLen];
		memset(m_pCataDocFreq,0,sizeof(long)*m_nAllocLen);
		m_pCataWordFreq=new long[m_nAllocLen];
		memset(m_pCataWordFreq,0,sizeof(long)*m_nAllocLen);
	}
}


void CWordNode::AllocBuffer(int nLen)
{
	if(nLen<=0) return;
	DeallocBuffer();
	m_nAllocLen=nLen;
	m_pCataWeight=new double[m_nAllocLen];
	m_pCataWeightPro=new double[m_nAllocLen];
	m_pCataDocFreq=new long[m_nAllocLen];
	m_pCataWordFreq=new long[m_nAllocLen];
}

//用于计算特征的权重，参数sum代表文档集中的文档总数
//如果bMult=true且m_dWeight大于0, 则将特征的反比文档频率乘上m_dWeight原来的值, 再保存到成员变量m_dWeight中
//否则, 将特征的反比文档频率值保存到成员变量m_dWeight中
void CWordNode::ComputeWeight(long sum, bool bMult)
{
	long docFreq=GetDocNum();
	if(docFreq<=0&&sum<=0)
	{
		m_dWeight=0.0;
		return;
	}
	double weight=log((double)sum/(double)docFreq);
	if(bMult&&m_dWeight>dZero)
		m_dWeight*=weight;
	else
		m_dWeight=weight;
}

long CWordNode::GetCataDocNum(int cataID)
{
	return m_pCataDocFreq[cataID];
}

long CWordNode::GetCataWordNum(int cataID)
{
	return m_pCataWordFreq[cataID];	
}

long CWordNode::GetDocNum()
{
	long sum=0;
	if(m_nAllocLen>0)
	{
		for(int i=0;i<m_nAllocLen;i++)
			sum+=m_pCataDocFreq[i];
	}
	else sum=m_lDocFreq;
	return sum;
}

long CWordNode::GetWordNum()
{
	long sum=0;
	if(m_nAllocLen>0)
	{
		for(int i=0;i<m_nAllocLen;i++)
			sum+=m_pCataWordFreq[i];
	}
	else sum=m_lWordFreq;
	return sum;
}

int CWordNode::MaxWeightIndex()
{
	int idx=-1;
	double nMax=-DBL_MAX;
	for(int i=0;i<m_nAllocLen;i++)
	{
		if(m_pCataWeight[i]>nMax)
		{
			nMax=m_pCataWeight[i];
			idx=i;
		}
	}
	return idx;
}

//此函数暂且只在层次分类中用到,函数名称和其实现的功能看起来有点不同
void CWordNode::Copy(CWordNode &wordNode)
{
	m_dWeight=wordNode.m_dWeight;
	m_nAllocLen=0;
	m_nWordID=wordNode.m_nWordID;
	m_lDocFreq=wordNode.m_lDocFreq;
	m_pCataWeight=NULL;
	m_pCataWeightPro=NULL;
	m_pCataDocFreq=NULL;
	m_pCataWordFreq=NULL;
	m_lDocID=0;
}


//DEL CWordNode& CWordList::GetWordNodeByID(long wordID)
//DEL {
//DEL 
//DEL }

double CWordList::GetWordProByID(POSITION &pos,long wordID,int classnum)
{
	CString str;
	double pro;
//	POSITION pos=GetFirstPosition();
	bool bFound=false;
	while(pos!=NULL)
	{
		CWordNode& wordnode=GetNext(pos,str);
		if(wordnode.m_nWordID==wordID)
		{
			bFound=true;
			pro = wordnode.m_pCataWeightPro[classnum];
			break;
		}
	}
	if(bFound) return pro;
	else return 0.0;
}

CWordNode& CWordList::GetWordProByID(POSITION &pos, int j)
{
	CString str;
	bool bFound=false;
	while(pos!=NULL)
	{	
		CWordNode& wordnode=GetNext(pos,str);
		if(wordnode.m_nWordID==j)
		{
			bFound=true;
			break;
		}
	}
	if(bFound) return m_lstWordList[str];
}

bool CWordList::GetProFromFile(CString strFileName)
{
	FILE *stream;
	if( (stream  = fopen( strFileName, "r" )) == NULL )
	{
		AfxMessageBox("无法打开文件"+strFileName+"!");
		return false;
	}
	int m,n,i;
	fscanf(stream,"%d %d\n",&m,&n);
	
	POSITION	pos;
	CString     str;
	pos		  = GetFirstPosition();
	while(pos!=NULL)
	{
		CWordNode& wordnode=GetNext(pos, str);
		wordnode.InitBuffer(n);
		fscanf(stream,"%d",&wordnode.m_nWordID);
		for(i=0;i<n;i++)
		{
			fscanf(stream,"%lf",&wordnode.m_pCataWeightPro[i]);
		}
	}

	fclose(stream);
	return true;
}
