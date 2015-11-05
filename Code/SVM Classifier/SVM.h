// SVM.h: interface for the CSVM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SVM_H__169833C2_5291_4CDB_BE81_22DBD80908F0__INCLUDED_)
#define AFX_SVM_H__169833C2_5291_4CDB_BE81_22DBD80908F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Compute_Param.h"
#include "Compute_Prompt.h"
#include "Compute_Result.h"

# define CFLOAT  float       /* the type of float to use for caching */
/* kernel evaluations. Using float saves */
/* us some memory, but you can use double, too */
# define FNUM    long        /* the type used for storing feature ids */
# define FVAL    float       /* the type used for storing feature values */

# define LINEAR  0           /* linear kernel type */
# define POLY    1           /* polynoial kernel type */
# define RBF     2           /* rbf kernel type */
# define SIGMOID 3           /* sigmoid kernel type */
# define CUSTOM  4

typedef struct word
{
	FNUM    wnum;	
	FVAL    weight;
} SVM_WORD;

typedef struct doc
{
	long    docnum;
	double  twonorm_sq;
	SVM_WORD    *words;
} DOC;

typedef struct learn_parm 
{
	double svm_c;                /* upper bound C on alphas */
	double svm_costratio;        /* factor to multiply C for positive examples */
	double transduction_posratio;/* fraction of unlabeled examples to be */	/* classified as positives */
	long   biased_hyperplane;    /* if nonzero, use hyperplane w*x+b=0 	otherwise w*x=0 */
	long   svm_maxqpsize;        /* size q of working set */
	long   svm_newvarsinqp;      /* new variables to enter the working set in each iteration */
	double epsilon_crit;         /* tolerable error for distances used in stopping criterion */
	double epsilon_shrink;       /* how much a multiplier should be above  zero for shrinking */
	long   svm_iter_to_shrink;   /* iterations h after which an example can be removed by shrinking */
	long   remove_inconsistent;  /* exclude examples with alpha at C and  retrain */
	
								 
	long   skip_final_opt_check; 
								 /* do not check KT-Conditions at the end of optimization for examples removed by  
								 shrinking. WARNING: This might lead to sub-optimal solutions! */
	
	long   compute_loo;          /* if nonzero, computes leave-one-out	 estimates */
	double rho;                  /* parameter in xi/alpha-estimates and for pruning leave-one-out range [1..2] */
	long   xa_depth;             /* parameter in xi/alpha-estimates upper  bounding the number of SV the current alpha_t is distributed over */
	char predfile[200];          /* file for predicitions on unlabeled examples					 in transduction */
	char alphafile[200];         
								/* file to store optimal alphas in. use
									empty string if alphas should not be 	 output */
	
	
	/* you probably do not want to touch the following */
	double epsilon_const;        /* tolerable error on eq-constraint */
	double epsilon_a;            /* tolerable error on alphas at bounds */
	double opt_precision;        /* precision of solver, set to e.g. 1e-21 	 if you get convergence problems */
	
	
	/* the following are only for internal use */
	long   svm_c_steps;          /* do so many steps for finding optimal C */
	double svm_c_factor;         /* increase C by this factor every step */
	double svm_costratio_unlab;
	double svm_unlabbound;
	double *svm_cost;            /* individual upper bounds for each var */
	long   totwords;             /* number of features */
} LEARN_PARM;

typedef struct kernel_parm 
{
	long    kernel_type;   
	long    poly_degree;
	double  rbf_gamma;
	double  coef_lin;
	double  coef_const;
	char    custom[50];    /* for user supplied kernel */
} KERNEL_PARM;

typedef struct model 
{
	long    sv_num;	
	long    at_upper_bound;
	double  b;
	DOC     **supvec;
	double  *alpha;
	long    *index;       /* index from docnum to position in model */
	long    totwords;     /* number of features */
	long    totdoc;       /* number of training documents */
	KERNEL_PARM kernel_parm; /* kernel */
	
	/* the following values are not written to file */
	double  loo_error,loo_recall,loo_precision; /* leave-one-out estimates */
	double  xa_error,xa_recall,xa_precision;    /* xi/alpha estimates */
												double  *lin_weights;  	/* weights for linear case using folding */
} MODEL;

typedef struct quadratic_program 
{
	long   opt_n;            /* number of variables */
	long   opt_m;            /* number of linear equality constraints */
	double *opt_ce,*opt_ce0; /* linear equality constraints */
	double *opt_g;           /* hessian of objective */
	double *opt_g0;          /* linear part of objective */
	double *opt_xinit;       /* initial value for variables */
	double *opt_low,*opt_up; /* box constraints */
} QP;

typedef struct kernel_cache 
{
	long   *index;  /* cache some kernel evalutations */
	CFLOAT *buffer; /* to improve speed */
	long   *invindex;
	long   *active2totdoc;
	long   *totdoc2active;
	long   *lru;
	long   *occu;
	long   elems;
	long   max_elems;
	long   time;
	long   activenum;
	long   buffsize;
} KERNEL_CACHE;

 
typedef struct timing_profile 
{
	long   time_kernel;
	long   time_opti;
	long   time_shrink;
	long   time_update;
	long   time_model;
	long   time_check;
	long   time_select;
} TIMING;
 
typedef struct shrink_state 
{
	long   *active;
	long   *inactive_since;
	long   deactnum;
	double **a_history;
} SHRINK_STATE;

typedef struct cache_parm_s {
  KERNEL_CACHE *kernel_cache;
  CFLOAT *cache;
  DOC *docs; 
  long m;
  KERNEL_PARM *kernel_parm;
  long offset,stepsize;
} cache_parm_t;

class CSVM  
{
public: 
	CSVM();
	virtual ~CSVM();
	int svm_learn_main (int);
	int svm_classify (int, double*);
	double svm_classify(DOC &doc);
private:
	void set_learn_parameters(LEARN_PARM* learn_parm,KERNEL_PARM* kernel_parm);
private:   //svm_common.h
	double classify_example(MODEL *, DOC *);
	double classify_example_linear(MODEL *, DOC *);
	CFLOAT kernel(KERNEL_PARM *, DOC *, DOC *); 
	double custom_kernel(KERNEL_PARM *, DOC *, DOC *); 
	double sprod_ss(SVM_WORD *, SVM_WORD *);
	double model_length_s(MODEL *, KERNEL_PARM *);
	void   clear_vector_n(double *, long);
	void   add_vector_ns(double *, SVM_WORD *, double);
	double sprod_ns(double *, SVM_WORD *);
	double sprod_ss1(SVM_WORD *a,SVM_WORD*b,int offset);
	double sprod_ss2(SVM_WORD *a,SVM_WORD*b,int offset);
	void   add_weight_vector_to_linear_model(MODEL *);
	int    read_model(char *, MODEL *, long, long);
	int    read_documents(char *, DOC *, long *, long, long, long *, long *, int);
	int    parse_document(char *, DOC *, long *, long *, long);
	int    nol_ll(char *, long *, long *, long *);
	double ktt(int ta,int tb,double pa[],double pb[]);
	double kt(int t,double ta[],double tb[]);
	double fi(double* tt);
	double fs(double ta[]);
	double sumofword(DOC* a);
	long   minl(long, long);
	long   maxl(long, long);
	long   get_runtime();
	void   *my_malloc(long); 
	void   copyright_notice();
	//void  SetInitParam();         ?????????
	int isnan(double);
private:    //svm_common.h
	void	printm(char*);
	void    printe(char*);
private:    //svm_learn.h
	void   svm_learn(DOC *, long *, long, long, LEARN_PARM *, KERNEL_PARM *, 
		 KERNEL_CACHE *, MODEL *);
	long   optimize_to_convergence(DOC *, long *, long, long, LEARN_PARM *,
			       KERNEL_PARM *, KERNEL_CACHE *, SHRINK_STATE *,
			       MODEL *, long *, long *, double *, double *, 
			       TIMING *, double *, long, long);
	double compute_objective_function(double *, double *, long *, long *);
	void   clear_index(long *);
	void   add_to_index(long *, long);
	long   compute_index(long *,long, long *);
	void   optimize_svm(DOC *, long *, long *, long *, long *, MODEL *, long, 
		    long *, long, double *, double *, LEARN_PARM *, CFLOAT *, 
		    KERNEL_PARM *, QP *, double *);
	void   compute_matrices_for_optimization(DOC *, long *, long *, long *, 
					 long *, long *, MODEL *, double *, 
					 double *, long, long, LEARN_PARM *, 
					 CFLOAT *, KERNEL_PARM *, QP *);
	long   calculate_svm_model(DOC *, long *, long *, double *, double *, 
			   double *, LEARN_PARM *, long *, MODEL *);
	long   check_optimality(MODEL *, long *, long *, double *, double *,long, 
			LEARN_PARM *,double *, double, long *, long *, long *,
			long *, long, KERNEL_PARM *);
	long   identify_inconsistent(double *, long *, long *, long, LEARN_PARM *, 
			     long *, long *);
	long   identify_misclassified(double *, long *, long *, long,
			      MODEL *, long *, long *);
	long   identify_one_misclassified(double *, long *, long *, long,
				  MODEL *, long *, long *);
	long   incorporate_unlabeled_examples(MODEL *, long *,long *, long *,
				      double *, double *, long, double *,
				      long *, long *, long, KERNEL_PARM *,
				      LEARN_PARM *);
	void   update_linear_component(DOC *, long *, long *, double *, double *, 
			       long *, long, long, KERNEL_PARM *, 
			       KERNEL_CACHE *, double *,
			       CFLOAT *, double *);
	long   select_next_qp_subproblem_grad(long *, long *, double *, double *, long,
				      long, LEARN_PARM *, long *, long *, 
				      long *, double *, long *, KERNEL_CACHE *,
				      long *, long *);
	long   select_next_qp_subproblem_grad_cache(long *, long *, double *, double *,
					    long, long, LEARN_PARM *, long *, 
					    long *, long *, double *, long *,
					    KERNEL_CACHE *, long *, long *);
	void   select_top_n(double *, long, long *, long);
	void   init_shrink_state(SHRINK_STATE *, long, long);
	void   shrink_state_cleanup(SHRINK_STATE *);
	long shrink_problem(
			/* shrink some variables away */
			/* do the shrinking only if at least minshrink variables can be removed */
			LEARN_PARM *learn_parm,
			SHRINK_STATE *shrink_state,
			long *active2dnum,long iteration,long* last_suboptimal_at,
			long totdoc,long minshrink,
			double *a,
			long *inconsistent);
	void   reactivate_inactive_examples(long *, long *, double *, SHRINK_STATE *,
				    double *, long, long, long, LEARN_PARM *, 
				    long *, DOC *, KERNEL_PARM *,
				    KERNEL_CACHE *, MODEL *, CFLOAT *, 
				    double *, double *);
	/* cache kernel evalutations to improve speed */
	void   get_kernel_row(KERNEL_CACHE *,DOC *, long, long, long *, CFLOAT *, 
		      KERNEL_PARM *);
	void   cache_kernel_row(KERNEL_CACHE *,DOC *, long, KERNEL_PARM *);
	void   cache_multiple_kernel_rows(KERNEL_CACHE *,DOC *, long *, long, 
				  KERNEL_PARM *);
	void   kernel_cache_shrink(KERNEL_CACHE *,long, long, long *);
	void   kernel_cache_init(KERNEL_CACHE *,long, long);
	void   kernel_cache_reset_lru(KERNEL_CACHE *);
	void   kernel_cache_cleanup(KERNEL_CACHE *);
	long   kernel_cache_malloc(KERNEL_CACHE *);
	void   kernel_cache_free(KERNEL_CACHE *,long);
	long   kernel_cache_free_lru(KERNEL_CACHE *);
	CFLOAT *kernel_cache_clean_and_malloc(KERNEL_CACHE *,long);
	long   kernel_cache_touch(KERNEL_CACHE *,long);
	long   kernel_cache_check(KERNEL_CACHE *,long);
	void compute_xa_estimates(
		MODEL *model,                           /* xa-estimate of error rate, */
		long *label,long *unlabeled,long totdoc,          /* recall, and precision      */
		DOC	*docs,       
		double *lin,double *a,                      
		KERNEL_PARM *kernel_parm,
		LEARN_PARM *learn_parm,
		double *error,double *recall,double *precision);
	double xa_estimate_error(MODEL *, long *, long *, long, DOC *, 
			 double *, double *, KERNEL_PARM *, 
			 LEARN_PARM *);
	double xa_estimate_recall(MODEL *, long *, long *, long, DOC *, 
			  double *, double *, KERNEL_PARM *, 
			  LEARN_PARM *);
	double xa_estimate_precision(MODEL *, long *, long *, long, DOC *, 
			     double *, double *, KERNEL_PARM *, 
			     LEARN_PARM *);
	void avg_similarity_of_sv_of_one_class(MODEL *, DOC *, double *, long *, KERNEL_PARM *, double *, double *);
	double most_similar_sv_of_same_class(MODEL *, DOC *, double *, long, long *, KERNEL_PARM *, LEARN_PARM *);
	double distribute_alpha_t_greedily(long *, long, DOC *, double *, long, long *, KERNEL_PARM *, LEARN_PARM *, double);
	double distribute_alpha_t_greedily_noindex(MODEL *, DOC *, double *, long, long *, KERNEL_PARM *, LEARN_PARM *, double); 
	void estimate_transduction_quality(MODEL *, long *, long *, long, DOC *, double *);
	double estimate_margin_vcdim(MODEL *, double, double, KERNEL_PARM *);
	double estimate_sphere(MODEL *, KERNEL_PARM *);
	double estimate_r_delta_average(DOC *, long, KERNEL_PARM *); 
	double estimate_r_delta(DOC *, long, KERNEL_PARM *); 
	double length_of_longest_document_vector(DOC *, long, KERNEL_PARM *); 
	void   write_model(char *, MODEL *);
	void   write_prediction(char *, MODEL *, double *, double *, long *, long *,
			long, LEARN_PARM *);
	void   write_alphas(char *, double *, long *, long);
private:    //svm_hideo.h
	double *optimize_qp(QP *, double *, long, double *, LEARN_PARM *);
	int optimize_hildreth_despo(long,long,double,double,double,long,long,long,double,double *,
			    double *,double *,double *,double *,double *,
			    double *,double *,double *,long *,double *);
	int solve_dual(long,long,double,double,long,double *,double *,double *,
	       double *,double *,double *,double *,double *,double *,
	       double *,double *,double *,double *,long);
	void linvert_matrix(double *, long, double *, double, long *);
	void lprint_matrix(double *, long);
	void ladd_matrix(double *, long, double);
	void lcopy_matrix(double *, long, double *);
	void lswitch_rows_matrix(double *, long, long, long);
	void lswitchrk_matrix(double *, long, long, long);
	double calculate_qp_objective(long, double *, double *, double *);
public:
	CCompute_Prompt com_pro;
	CCompute_Param com_param;
	CCompute_Result com_result;
private:    //svm_hideo.h
	double *primal,*dual;
	long   precision_violations;
	double opt_precision;
	long   maxiter;
	double lindep_sensitivity;
	double *buffer;
	long   *nonoptimal;
	long  smallroundcount;
private:
	char temstr[MAX_PATH*10];
};

#endif // !defined(AFX_SVM_H__169833C2_5291_4CDB_BE81_22DBD80908F0__INCLUDED_)
