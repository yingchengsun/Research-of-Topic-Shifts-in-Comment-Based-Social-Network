// Compute_Param.cpp: implementation of the CCompute_Param class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Compute_Param.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompute_Param::CCompute_Param()
{
	biased_Hyperplane=1;
	remove_inconsitant=0;
	C                =0.0;
	cost_factor      =1.0;
	loo              =0;
	search_depth     =0;
	rho              =1.0;
	fraction         =1.0;
	rbf_gamma        =1.0;
	poly_c           =0.0;
	poly_s           =1.0;
	poly_degree      =1;
	kernel_type      =0;

	epsion           =0.001;
	iteration_time   =100;
	cache_size       =40;
	new_variable     =10;
	maximum_size     =10;
	final_test       =1;

	classifier_num   =0;
	classifier_type  =0;

	running          =0;
	paused           =false;
}

CCompute_Param::~CCompute_Param()
{

}

void CCompute_Param::Serialize(CArchive &ar)
{
	if(ar.IsStoring())
	{
		ar<<C;
		ar<<cost_factor;
		ar<<biased_Hyperplane;
		ar<<remove_inconsitant;
		ar<<loo;
		ar<<rho;
		ar<<search_depth;
		ar<<fraction;
		ar<<kernel_type;
		ar<<poly_degree;
		ar<<rbf_gamma;
		ar<<poly_s;
		ar<<poly_c;
		ar<<maximum_size;
		ar<<new_variable;//2..maximun_size
		ar<<cache_size;//5..,the larger, the faster
		ar<<epsion;
		ar<<iteration_time;//default 100
		ar<<final_test;//default 1, to do final test.
		ar<<classifier_num;
		ar<<classifier_type;

		ar<<trainfile;
		ar<<modelfile;
		ar<<resultfile;
		ar<<classifyfile;
		ar<<matrixfile;
		ar<<resultpath;
		ar<<Coff[2];
	}
	else
	{
		ar>>C;
		ar>>cost_factor;
		ar>>biased_Hyperplane;
		ar>>remove_inconsitant;
		ar>>loo;
		ar>>rho;
		ar>>search_depth;
		ar>>fraction;
		ar>>kernel_type;
		ar>>poly_degree;
		ar>>rbf_gamma;
		ar>>poly_s;
		ar>>poly_c;
		ar>>maximum_size;
		ar>>new_variable;//2..maximun_size
		ar>>cache_size;//5..,the larger, the faster
		ar>>epsion;
		ar>>iteration_time;//default 100
		ar>>final_test;//default 1, to do final test.
		ar>>classifier_num;
		ar>>classifier_type;

		ar>>trainfile;
		ar>>modelfile;
		ar>>resultfile;
		ar>>classifyfile;
		ar>>matrixfile;
		ar>>resultpath;
		ar>>Coff[2];
	}
}

void CCompute_Param::DumpToFile(CString strFileName)
{
	CFile		fBinOut;
	if(!fBinOut.Open(strFileName,CFile::modeWrite | CFile::modeCreate) )
	{
		AfxMessageBox( "无法创建文件h"+strFileName+"!") ;
		return;
	}
	CArchive ar(&fBinOut,CArchive::store);
	Serialize(ar);
	ar.Close();
	fBinOut.Close();
}

bool CCompute_Param::GetFromFile(CString strFileName)
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