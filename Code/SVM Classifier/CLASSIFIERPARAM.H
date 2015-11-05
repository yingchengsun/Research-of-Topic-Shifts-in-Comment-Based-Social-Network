// ClassifierParam.h: interface for the CClassifierParam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLASSIFIERPARAM_H__2EB59D01_DED9_11D3_A70C_B0BCFB5CF9A2__INCLUDED_)
#define AFX_CLASSIFIERPARAM_H__2EB59D01_DED9_11D3_A70C_B0BCFB5CF9A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CClassifierParam  
{
public:
	CClassifierParam();
	virtual ~CClassifierParam();
	void DumpToFile(CString strFileName);
	bool GetFromFile(CString strFileName);
private:
	void Serialize(CArchive &ar);
public:    //训练时需要使用的参数
	CString m_txtTrainDir;
	CString m_txtResultDir;
	int		m_nFSMode;        //特征评估函数
	int     m_nWordSize;      //特征数目
	double     m_nUKS_k;
	int     m_nSelMode;       //全局打分还是按类别打分计算特征的区分度
	int     m_nOpMode;        //计算概率的方式
	int     m_nLanguageType;  //文档的语种
	BOOL    m_bStem;          //是否进行词干抽取
	BOOL    m_nCS;
	BOOL    m_nRS;
	BOOL    m_Length;
	BOOL    m_nUKS;
	BOOL    m_nURL;
	BOOL    m_ClassifierW;
	int     m_nWeightMode;    //特征加权的方法
public:    //分类时需要使用的参数
	int m_nClassifyType;              //0  单类分类; 1 多类分类
	BOOL m_bEvaluation;               //是否需要对测试结果进行评价
	BOOL m_bCopyFiles;                //是否将分好类的文件拷贝到结果目录下
	CString m_strTestDir;             //测试文档或其所在的目录
	CString m_strResultDir;           //测试结果所在的目录
	CString m_strModelFile;
	int m_nKNN;                       //KNN算法的k值
	int m_nDocFormat;                 //测试文档的格式
	double m_dThreshold;              //多类分类时使用的阈值
	int m_nClassifierType;            //分类器的类型: -1 未知, 0 代表KNN, 1 代表SVM, 2 代表BAYES
public:
	void GetParamString(CString &strParam);
	// calculation model
	static const int nOpDocMode;      // based on document number model
	static const int nOpWordMode;     // based on word number model
	// feature evaluation fuction
	static const int nFS_IGMode;      // Information gain feature selection
	static const int nFS_MIMode;      // Mutual Informaiton feature selection
	static const int nFS_CEMode;      // Cross Entropy for text feature selection
	static const int nFS_X2Mode;      // X^2 Statistics feature selection
	static const int nFS_WEMode;      // Weight of Evielence for text feature selection
	static const int nFS_XXMode;      // Right half of IG
	// how to select features
	static const int nFSM_GolbalMode;  // 全局选
	static const int nFSM_IndividualModel; // 单独选
	// classifier type
	static const int nCT_Unknown;     // Unknown
	static const int nCT_KNN;         // KNN
	static const int nCT_SVM;         // SVM
	static const int nCT_BAYES;		  // BAYES
	// document language type
	static const int nLT_Chinese;     // Chinese
	static const int nLT_English;     // English
	// document format
	static const int nDF_Directory;   // Directory
	static const int nDF_Smart;       // Smart
	// classify type
	static const int nFT_Single;      // Single Classification
	static const int nFT_Multi;       // Multiple Classification
	// weight mode
	static const int nWM_TF_IDF;      // TF*IDF
	static const int nWM_TF_DIFF;     // TF*DIFF
	static const int nWM_TF_IDF_DIFF; // TF*IDF*DIFF
	static const int nWM_nCS_nRS;     //xby
};

#endif // !defined(AFX_CLASSIFIERPARAM_H__2EB59D01_DED9_11D3_A70C_B0BCFB5CF9A2__INCLUDED_)
