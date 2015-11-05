// Compute_Prompt.cpp: implementation of the CCompute_Prompt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextClassify.h"
#include "Compute_Prompt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompute_Prompt::CCompute_Prompt()
{

}

CCompute_Prompt::~CCompute_Prompt()
{

}
void CCompute_Prompt::Copy(const CCompute_Prompt& t)
{
	 
		show_action=t.show_action;
		show_compute_1=t.show_compute_1;
		show_compute_2=t.show_compute_2;
		show_compute_3=t.show_compute_3;
		show_other=t.show_other;
		show_readfile=t.show_readfile;
		show_testresult=t.show_testresult;
		show_title=t.show_title;
		show_trainresult=t.show_trainresult;
		show_writefile=t.show_writefile;
		show_openfile=t.show_openfile;
		return ;
 
	
}

void CCompute_Prompt::Clear()
{	 
		show_action=show_title=show_compute_1
			=show_other=show_trainresult=show_testresult
			=show_readfile=show_writefile=FALSE;
 
	
}
void CCompute_Prompt::Setup()
{
	show_action=show_title=show_compute_1
			=show_other=show_trainresult=show_testresult
			=show_readfile=show_writefile=TRUE;
}