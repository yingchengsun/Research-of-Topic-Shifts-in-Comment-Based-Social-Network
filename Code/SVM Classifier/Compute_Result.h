// Compute_Result.h: interface for the CCompute_Result class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPUTE_RESULT_H__45E1A883_9CBE_475E_AAB4_2A394A3C2142__INCLUDED_)
#define AFX_COMPUTE_RESULT_H__45E1A883_9CBE_475E_AAB4_2A394A3C2142__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompute_Result  
{
public:
	CCompute_Result();
public:
	double	train_error;
	double	max_difference;
	double	precision;
	double	recall;
	int		train_number;
	int		support_number;
	int		test_number;
	int		train_misclassify;
	int		test_misclassify;
	long	kernel_cache_statistic;
	virtual ~CCompute_Result();
};
extern CCompute_Result com_result;
#endif // !defined(AFX_COMPUTE_RESULT_H__45E1A883_9CBE_475E_AAB4_2A394A3C2142__INCLUDED_)
