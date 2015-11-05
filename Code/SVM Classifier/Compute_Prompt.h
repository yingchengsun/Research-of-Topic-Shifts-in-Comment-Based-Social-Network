// Compute_Prompt.h: interface for the CCompute_Prompt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPUTE_PROMPT_H__E5D0B2B6_36D7_443E_999B_665DE743191A__INCLUDED_)
#define AFX_COMPUTE_PROMPT_H__E5D0B2B6_36D7_443E_999B_665DE743191A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompute_Prompt  
{
public:
	CCompute_Prompt();


public:
	BOOL show_action;
	BOOL show_title;
	BOOL show_compute_1;
	BOOL show_compute_2;
	BOOL show_compute_3;
	BOOL show_other;
	BOOL show_trainresult;
	BOOL show_testresult;
	BOOL show_readfile;
	BOOL show_writefile; 
	BOOL show_openfile;
	void Copy(const CCompute_Prompt& t);
	void Clear();
	void Setup(); 
	virtual ~CCompute_Prompt();

};

#endif // !defined(AFX_COMPUTE_PROMPT_H__E5D0B2B6_36D7_443E_999B_665DE743191A__INCLUDED_)
