// WordSegment.cpp: implementation of the CWordSegment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WordSegment.h"
#include ".\\Utility\\Utility.h"

const unsigned short CWordSegment::uTag = 0x0001;  // word classify tag
const unsigned short CWordSegment::uPlace = 0x0002; // Chinese place name
const unsigned short CWordSegment::uPerson = 0x0004;  // chinese name
const unsigned short CWordSegment::uTransPerson = 0x0008; // translation name, include translation

char CWordSegment::m_pSentence[MAX_PATH*10];        //用来存放经过去掉空格回车等字母后的句子
CWordSegment g_wordSeg;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
AFX_INLINE UINT AFXAPI HashKey(CString key)
{
	return HashKey((LPCTSTR)key);
}

CWordSegment::CWordSegment()
{
	m_nInit=0;
	m_nOperateType=0;
	m_dSmoothingPara=0.1;//Smoothing parameter
}

CWordSegment::~CWordSegment()
{
	FreeWordSegment();
}

bool CWordSegment::InitWorgSegment(char *pPath, int nLanguage)
{
	if(!m_nInit)
	{
		char filename[MAX_PATH];
		if(nLanguage==0)
		{
			strcpy(filename,pPath);
			strcat(filename,"\\data\\coreDict.dct");
			if(!m_dictCore.Load(filename)) return false;

			strcpy(filename,pPath);
			strcat(filename,"\\data\\lexical.ctx");
			if(!m_POSTagger.LoadContext(filename))  return false;
			m_POSTagger.SetTagType();

			strcpy(filename,pPath);
			strcat(filename,"\\data\\nr");		
			if(!m_uPerson.Configure(filename,TT_PERSON)) return false;

			strcpy(filename,pPath);
			strcat(filename,"\\data\\ns");		
			if(!m_uPlace.Configure(filename,TT_PLACE)) return false;

			strcpy(filename,pPath);
			strcat(filename,"\\data\\tr");		
			if(!m_uTransPerson.Configure(filename,TT_TRANS_PERSON)) return false;

			strcpy(filename,pPath);
			strcat(filename,"\\data\\BigramDict.dct");		
			if(!m_dictBigram.Load(filename)) return false;
		}

		FILE *stream;
		char word[MAX_PATH];
		strcpy(filename,pPath);
		if(nLanguage==0)
		{
			strcat(filename,"\\data\\stopwords.txt");
			m_nInit=1;
		}
		else
		{
			strcat(filename,"\\data\\engstopwords.txt");
			m_nInit=2;
		}
		if((stream  = fopen( filename, "r" )) == NULL)
			return false;
		while(!feof(stream))
		{
			fscanf(stream,"%s\n",word); 
			if(word[0]!='\0') m_lstStopWords.SetAt(word,1);
		}
		m_lstStopWords.SetAt("\r\n",1);
		m_lstStopWords.SetAt(" \r\n",1);
		m_lstStopWords.SetAt("\n",1);
		m_lstStopWords.SetAt(" \n",1);
		fclose(stream);
	}
	return true;
}

void CWordSegment::FreeWordSegment(void)
{
	if(m_nInit==1)
	{
		m_lstStopWords.RemoveAll();
		m_dictCore.ReleaseDict();
		m_dictBigram.ReleaseDict();
		m_POSTagger.ReleaseSpan();
		m_uPerson.ReleaseUnknowWord();
		m_uTransPerson.ReleaseUnknowWord();
		m_uPlace.ReleaseUnknowWord();
	}
	if(m_nInit==2)
	{
		m_lstStopWords.RemoveAll();
	}
	m_nInit=0;
}

void CWordSegment::SetSegSetting(unsigned short Setting)
{
	m_nOperateType=Setting;
}

unsigned short CWordSegment::GetSegSetting()
{
	return m_nOperateType;
}

bool CWordSegment::Segment(char* sentence)
{
	int nIndex,nResultCount;
	strcpy(m_pNewSentence,SENTENCE_BEGIN);
	strcat(m_pNewSentence,sentence);
	strcat(m_pNewSentence,SENTENCE_END);
	m_Seg.BiSegment(m_pNewSentence, m_dSmoothingPara,m_dictCore,m_dictBigram,1);
	nResultCount=m_Seg.m_nSegmentCount;
	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
	{
		if(m_nOperateType&&uPerson)
			m_uPerson.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
		if(m_nOperateType&&uTransPerson)
			m_uTransPerson.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
		if(m_nOperateType&&uPlace)
		m_uPlace.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
	}		
	m_Seg.BiOptimumSegment(1,m_dSmoothingPara,m_dictBigram,m_dictCore);

	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
	{
		if(m_nOperateType&&uTag)
			m_POSTagger.POSTagging(m_Seg.m_pWordSeg[nIndex],m_dictCore,m_dictCore);
	}
	return true;
}

int CWordSegment::GetSegmentCount()
{
	return m_Seg.m_nSegmentCount;
}

PWORD_RESULT CWordSegment::GetWordSeg(int index)
{
	return m_Seg.m_pWordSeg[index];
}

bool CWordSegment::isInStopWords(char *pWord)
{
 	int t;
 	return(m_lstStopWords.Lookup(pWord,t)>0);
}

//bStopWord=true 去掉停用词
//bTag=true      结果文件中包含词性标记
int CWordSegment::SegmentFile(const char *source, const char *target, BOOL bStopWord, BOOL bTag)
{
	CFile fin;
	FILE *fout;
	if(!fin.Open(source,CFile::modeRead))
		return -1;
	if((fout=fopen(target,"w+"))==NULL)
	{
		fin.Close();
		return -1;
	}
	//读入文件的内容
	int num=0;
	unsigned int flen=fin.GetLength();
	char *buffer=new char[flen+1];
	flen=fin.ReadHuge(buffer,flen);
	buffer[flen]='\0';
	fin.Close();


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
				fprintf(fout,"%s\n",m_pSentence);
				realcnt++;
			}
		}
		else //如果是汉字串
		{
			if(nSentenceLen%2!=0) continue;
			if(nSentenceLen<=6) //如果是单个汉字
			{
				fprintf(fout,"%s\n",m_pSentence);;
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
						if(w[0]=='\0'||(bStopWord&&g_wordSeg.isInStopWords(w))) flag=false;		
						if(flag)
						{
							fprintf(fout,"%s\n",w);
							realcnt++;
						}
					}
				}
			}
		}
	}
	fclose(fout);
	delete[] buffer;
	return realcnt;
}

//对pBuffer中的文字进行处理，得到一个句子的结束位置nEnd
//并且将去掉空格回车等字母后的句子放入变量m_pSentence
int CWordSegment::ParseFile(char *pBuffer, int nStart, int &nEnd)
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
