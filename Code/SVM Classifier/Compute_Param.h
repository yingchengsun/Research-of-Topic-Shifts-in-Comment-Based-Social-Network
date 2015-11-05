// Compute_Param.h: interface for the CCompute_Param class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPUTE_PARAM_H__0155D7A3_9A2A_4712_A5F1_CC2E36C30257__INCLUDED_)
#define AFX_COMPUTE_PARAM_H__0155D7A3_9A2A_4712_A5F1_CC2E36C30257__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompute_Param  
{
public:
	CCompute_Param();
	virtual ~CCompute_Param();
	void DumpToFile(CString strFileName);
	bool GetFromFile(CString strFileName);
private:
	void Serialize(CArchive &ar);
public:
	//Learning options
	double	C;
	double	cost_factor;
	long	biased_Hyperplane;
	long	remove_inconsitant;
	//performance estimations options
	long	loo;
	double	rho;
	long	search_depth;
	//transduction option
	double	fraction;
	//kernel option
	long	kernel_type;
	long	poly_degree;
	double	rbf_gamma;
	double	poly_s;
	double	poly_c;
	
	//optimization options 
	long	maximum_size;
	long 	new_variable;//2..maximun_size
	double	cache_size;//5..,the larger, the faster
	double	epsion;
	long	iteration_time;//default 100
	int 	final_test;//default 1, to do final test.

	int     classifier_num;     //类别总数
	int		classifier_type;    //分类器的类型
	//output options

	CString	trainfile;
	CString	modelfile;
	CString	resultfile;
	CString	classifyfile;
	CString matrixfile;
	CString resultpath;

	int	running;  //0--not running  1--generate document vectors  2--train svm
	bool paused;
	//global variables
	double	Coff[2];
};

#endif // !defined(AFX_COMPUTE_PARAM_H__0155D7A3_9A2A_4712_A5F1_CC2E36C30257__INCLUDED_)
