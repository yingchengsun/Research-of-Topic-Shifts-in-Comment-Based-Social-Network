// SVM.cpp: implementation of the CSVM class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SVM.h"
#include "Message.h"

# include <stdio.h>
# include <ctype.h>
# include <math.h>
# include <string.h>
# include <stdlib.h>
# include <time.h> 
# include <float.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CCompute_Prompt com_pro;
extern CCompute_Param com_param;
extern CCompute_Result com_result;

# define VERSION       "V3.50"
# define VERSION_DATE  "01.11.00"

# define PRIMAL_OPTIMAL      1
# define DUAL_OPTIMAL        2
# define MAXITER_EXCEEDED    3
# define NAN_SOLUTION        4
# define ONLY_ONE_VARIABLE   5

# define LARGEROUND          0
# define SMALLROUND          1

# define DEF_PRECISION          1E-5
# define DEF_MAX_ITERATIONS     200
# define DEF_LINDEP_SENSITIVITY 1E-8
# define EPSILON_HIDEO          1E-20
# define EPSILON_EQ             1E-5


CSVM theSVM;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSVM::CSVM()
{
	precision_violations=0;
	opt_precision=DEF_PRECISION;
	maxiter=DEF_MAX_ITERATIONS;
	lindep_sensitivity=DEF_LINDEP_SENSITIVITY;
	smallroundcount=0;
}

CSVM::~CSVM()
{

}

/***************************svm_learn_main****************************/
void CSVM::set_learn_parameters(LEARN_PARM* learn_parm,KERNEL_PARM* kernel_parm)
{
  learn_parm->biased_hyperplane=com_param.biased_Hyperplane;
  learn_parm->remove_inconsistent=com_param.remove_inconsitant;
  learn_parm->skip_final_opt_check=com_param.final_test;
  learn_parm->svm_maxqpsize=com_param.maximum_size;
  learn_parm->svm_newvarsinqp=com_param.new_variable;
  learn_parm->svm_iter_to_shrink=com_param.iteration_time;
  learn_parm->svm_c=com_param.C;
  learn_parm->transduction_posratio=com_param.fraction;
  learn_parm->svm_costratio=com_param.cost_factor;
  learn_parm->svm_costratio_unlab=1.0;
  learn_parm->svm_unlabbound=1E-5;
  learn_parm->epsilon_crit=0.001;
  learn_parm->epsilon_a=1E-15;
  learn_parm->compute_loo=com_param.loo;
  learn_parm->rho=com_param.rho;
  learn_parm->xa_depth=com_param.search_depth;

  kernel_parm->kernel_type=com_param.kernel_type;
  kernel_parm->poly_degree=com_param.poly_degree;
  kernel_parm->rbf_gamma=com_param.rbf_gamma;
  kernel_parm->coef_lin=com_param.poly_s;
  kernel_parm->coef_const=com_param.poly_c;
  //strcpy(kernel_parm->custom,com_param.);
}


int CSVM::svm_learn_main (int pos_label)
{  
  DOC *docs;  /* training examples */
  long *label,max_docs,max_words_doc;
  long totwords,totdoc,ll,i;
  KERNEL_CACHE kernel_cache;
  LEARN_PARM learn_parm;
  KERNEL_PARM kernel_parm;
  MODEL model;
  char docfile[200];
  char modelfile[200];
  
  if (com_pro.show_action)
	  printm("begin to compute");
  if (com_pro.show_action)
	printm("Scanning examples...");

  set_learn_parameters(&learn_parm,&kernel_parm);

  strcpy(docfile,com_param.trainfile);
  strcpy(modelfile,com_param.modelfile);
//  kernel_cache_size=com_param.cache_size;


  nol_ll(docfile,&max_docs,&max_words_doc,&ll); /* scan size of input file */
  max_words_doc+=2;
  ll+=2;
  max_docs+=2;


  docs = (DOC *)my_malloc(sizeof(DOC)*max_docs);
  label = (long *)my_malloc(sizeof(long)*max_docs);

  
  read_documents(docfile,docs,label,max_words_doc,ll,&totwords,&totdoc,pos_label);

  if(kernel_parm.kernel_type == LINEAR) /* don't need the cache */
  { 
    svm_learn(docs,label,totdoc,totwords,&learn_parm,&kernel_parm,NULL,&model);
  }
  else 
  {
    kernel_cache_init(&kernel_cache,totdoc,com_param.cache_size);
    svm_learn(docs,label,totdoc,totwords,&learn_parm,&kernel_parm,&kernel_cache,&model);
    kernel_cache_cleanup(&kernel_cache);
  }

  write_model(modelfile,&model);

  free(model.supvec);
  free(model.alpha);
  free(model.index);
  for(i=0;i<totdoc;i++) 
    free(docs[i].words);
  free(docs);
  free(label);
  if (com_pro.show_action)
	printm("Cease to compute");	
  return(0);
}
/********************************svm_learn_main****************************/

/********************************svm_classify****************************/
double CSVM::svm_classify(DOC &doc)
{
	long llsv,max_sv,max_words_sv,pred_format,j;
	double sim=0;
	MODEL model; 
	char modelfile[MAX_PATH];
		
	strcpy(modelfile,com_param.modelfile);
	pred_format=0;
	// scan size of model file
	nol_ll(modelfile,&max_sv,&max_words_sv,&llsv);
	max_words_sv+=2;
	llsv+=2;
	model.supvec = (DOC **)my_malloc(sizeof(DOC *)*max_sv);
	model.alpha = (double *)my_malloc(sizeof(double)*max_sv);
	read_model(modelfile,&model,max_words_sv,llsv);

	doc.twonorm_sq=sprod_ss(doc.words,doc.words);
	if(model.kernel_parm.kernel_type == 0) 
	{   // linear kernel, compute weight vector
		add_weight_vector_to_linear_model(&model);
		for(j=0;(doc.words[j]).wnum != 0;j++) 
		{  
			// Check if feature numbers are not larger than in    
			// model. Remove feature if necessary.
			if((doc.words[j]).wnum>model.totwords) 
				(doc.words[j]).wnum=0;              
		}                                        
		sim=classify_example_linear(&model,&doc);
	}
	// non-linear kernel
	else sim=classify_example(&model,&doc);

	free(model.supvec);
	free(model.alpha);
	//linear kernel
	if(model.kernel_parm.kernel_type == 0) free(model.lin_weights);
	return sim;
}

int CSVM::svm_classify(int post_label, double* weight)
{
	DOC doc;   /* test example */
	long max_docs,max_words_doc,lld,llsv;
	long max_sv,max_words_sv,totdoc=0,doc_label;
	long wnum,pred_format;
	long j;
	char *line; 
	FILE *docfl;
	MODEL model; 
	char docfile[MAX_PATH];
	char modelfile[MAX_PATH];
	
	strcpy(docfile,com_param.classifyfile);
	strcpy(modelfile,com_param.modelfile);
	pred_format=0;
	
	nol_ll(docfile,&max_docs,&max_words_doc,&lld); /* scan size of input file */
	max_words_doc+=2;
	lld+=2;
	nol_ll(modelfile,&max_sv,&max_words_sv,&llsv); /* scan size of model file */
	max_words_sv+=2;
	llsv+=2;
	model.supvec = (DOC **)my_malloc(sizeof(DOC *)*max_sv);
	model.alpha = (double *)my_malloc(sizeof(double)*max_sv);
	line = (char *)my_malloc(sizeof(char)*lld);
	doc.words = (SVM_WORD *)my_malloc(sizeof(SVM_WORD)*(max_words_doc+10));
	read_model(modelfile,&model,max_words_sv,llsv);
	/* linear kernel */
		/* compute weight vector */
	if(model.kernel_parm.kernel_type == 0)
	{ 
		add_weight_vector_to_linear_model(&model);
	}
	if (com_pro.show_action)
		printm("Classifying test examples..");

	if ((docfl = fopen (docfile, "r")) == NULL)
	{
		printe(docfile);
		return -1;
	}
//chen 10.9.2001

	while((!feof(docfl)) && fgets(line,(int)lld,docfl)) 
	{
		if(line[0] == '#') continue;  /* line contains comments */
		parse_document(line,&doc,&doc_label,&wnum,max_words_doc);
		if(model.kernel_parm.kernel_type == 0) 
		{   /* linear kernel */
			for(j=0;(doc.words[j]).wnum != 0;j++) 
			{  /* Check if feature numbers are not larger than in    
				 model. Remove feature if necessary.  */
				if((doc.words[j]).wnum>model.totwords) 
					(doc.words[j]).wnum=0;              
			}                                        
			weight[totdoc]=classify_example_linear(&model,&doc);
		}
		else /* non-linear kernel */                        
			weight[totdoc]=classify_example(&model,&doc);
		totdoc++;
	}  
	free(line);
	free(doc.words);
	free(model.supvec);
	free(model.alpha);
	if(model.kernel_parm.kernel_type == 0) 
	{ /* linear kernel */
		free(model.lin_weights);
	}	
	return(0);
}
/********************************svm_classify****************************/


/********************************svm_common****************************/
double CSVM::classify_example(MODEL *model,DOC *ex)
{
	register long i;
	register double dist;
	
	dist=0;
	for(i=1;i<model->sv_num;i++) {  
		dist+=kernel(&model->kernel_parm,model->supvec[i],ex)*model->alpha[i];
	}
	return(dist-model->b);
}

/*    classifies example for linear kernel 
important: the model must have the linear weight vector computed 
important: the feature numbers in the example to classify must 
not be larger than the weight vector!               */
double CSVM::classify_example_linear(MODEL *model,DOC *ex)
{
	return((double)(sprod_ns(model->lin_weights,ex->words)-model->b));
}

/* calculate the kernel function */
CFLOAT CSVM::kernel(KERNEL_PARM *kernel_parm,DOC *a,DOC*b)
{
	com_result.kernel_cache_statistic++;
	switch(kernel_parm->kernel_type)
	{
    case 0: /* linear */ 
		return((CFLOAT)sprod_ss(a->words,b->words)); 
    case 1: /* polynomial */
		return((CFLOAT)pow(kernel_parm->coef_lin*sprod_ss(a->words,b->words)+kernel_parm->coef_const,(double)kernel_parm->poly_degree)); 
    case 2: /* radial basis function */
		return((CFLOAT)exp(-kernel_parm->rbf_gamma*(a->twonorm_sq-2*sprod_ss(a->words,b->words)+b->twonorm_sq)));
    case 3: /* sigmoid neural net */
		return((CFLOAT)tanh(kernel_parm->coef_lin*sprod_ss(a->words,b->words)+kernel_parm->coef_const)); 
    case 4: /* custom-kernel supplied in file kernel.h*/
		return((CFLOAT)custom_kernel(kernel_parm,a,b)); 
		//chen .test sum of 
		//return((CFLOAT)pow(kernel_parm->coef_lin*sprod_ss(a->words,b->words)+kernel_parm->coef_const,(double)kernel_parm->poly_degree)+exp(-kernel_parm->rbf_gamma*(a->twonorm_sq-2*sprod_ss(a->words,b->words)+b->twonorm_sq))); 
    default: sprintf(temstr,"Error: Unknown kernel function");
		printm(temstr);
		return (-1);
	}
}

/* compute the inner product of two sparse vectors */
double CSVM::sprod_ss(SVM_WORD *a,SVM_WORD*b)
{
    register FVAL sum=0;
    register SVM_WORD *ai,*bj;
    ai=a;
    bj=b;
    while (ai->wnum && bj->wnum) {
		if(ai->wnum > bj->wnum) {
			bj++;
		}
		else if (ai->wnum < bj->wnum) {
			ai++;
		}
		else {
			sum+=ai->weight * bj->weight;
			ai++;
			bj++;
		}
    }
    return((double)sum);
}



/* compute the inner product of two sparse vectors,b right shit 1 bit */
double CSVM::sprod_ss1(SVM_WORD *a,SVM_WORD*b,int offset)
{
    register FVAL sum=0;
    register SVM_WORD *ai,*bj;
    ai=a;
    bj=b;
    while (ai->wnum && bj->wnum) {
		if(ai->wnum > bj->wnum+offset) {
			bj++;
		}
		else if (ai->wnum < bj->wnum+offset) {
			ai++;
		}
		else 
		{
			int np=(ai->wnum-1)%16+1+offset;
			if (np>0 && np<17) 
				sum+=ai->weight * bj->weight;
			ai++;
			bj++;
		}
    }
    return((double)sum);
}

double CSVM::sprod_ss2(SVM_WORD *a,SVM_WORD*b,int offset)
{
    register FVAL sum=0;
    register SVM_WORD *ai,*bj;
    ai=a;
    bj=b;
    while (ai->wnum && bj->wnum) {
		if(ai->wnum > bj->wnum+offset) {
			bj++;
		}
		else if (ai->wnum < bj->wnum+offset) {
			ai++;
		}
		else 
		{
			int np=ai->wnum+offset;
			if (np>0 && np<257) 
				sum+=ai->weight * bj->weight;
			ai++;
			bj++;
		}
    }
    return((double)sum);
}

/* compute length of weight vector */
double CSVM::model_length_s(MODEL *model,KERNEL_PARM *kernel_parm)
{
	register long i,j;
	register double sum=0,alphai;
	register DOC *supveci;
	
	for(i=1;i<model->sv_num;i++) {  
		alphai=model->alpha[i];
		supveci=model->supvec[i];
		for(j=1;j<model->sv_num;j++) {
			sum+=alphai*model->alpha[j]
				*kernel(kernel_parm,supveci,model->supvec[j]);
		}
	}
	return(sqrt(sum));
}

void CSVM::clear_vector_n(double *vec,long n)
{
	register long i;
	for(i=0;i<=n;i++) vec[i]=0;
}

void CSVM::add_vector_ns(double *vec_n,SVM_WORD *vec_s,double faktor)
{
	register SVM_WORD *ai;
	ai=vec_s;
	while (ai->wnum) {
		vec_n[ai->wnum]+=(faktor*ai->weight);
		ai++;
	}
}

double CSVM::sprod_ns(double *vec_n,SVM_WORD *vec_s)
{
	register double sum=0;
	register SVM_WORD *ai;
	ai=vec_s;
	while (ai->wnum) {
		sum+=(vec_n[ai->wnum]*ai->weight);
		ai++;
	}
	return(sum);
}

/* compute weight vector in linear case and add to model*/
void CSVM::add_weight_vector_to_linear_model(MODEL *model)
{
	long i;
	
	model->lin_weights=(double *)my_malloc(sizeof(double)*(model->totwords+1));
	clear_vector_n(model->lin_weights,model->totwords);
	for(i=1;i<model->sv_num;i++) {
		add_vector_ns(model->lin_weights,(model->supvec[i])->words,
			model->alpha[i]);
	}
}

int CSVM::read_model(char *modelfile,MODEL *model,long max_words,long ll)
{
	FILE *modelfl;
	long j,i;
	char *line;
	SVM_WORD *words;
	register long wpos;
	long wnum,pos;
	double weight;
	char version_buffer[100];

	if (com_pro.show_action)
	{
	
		sprintf(temstr,"Reading model..."); 
		printm(temstr); 
	}
	
	words = (SVM_WORD *)my_malloc(sizeof(SVM_WORD)*(max_words+10));
	line = (char *)my_malloc(sizeof(char)*ll);
	
	if ((modelfl = fopen (modelfile, "r")) == NULL)
	{
		printe (modelfile);  
		return -1;
	}
	
	fscanf(modelfl,"SVM-light Version %s\n",version_buffer);
	
	if(strcmp(version_buffer,VERSION)) 
	{
		printe ("Version of model-file does not match version of svm_classify!"); 
		return -1;
	}

	
	fscanf(modelfl,"%ld # kernel type\n",&(model->kernel_parm.kernel_type));
	fscanf(modelfl,"%ld # kernel parameter -d \n",&(model->kernel_parm.poly_degree));
	fscanf(modelfl,"%lf # kernel parameter -g \n",&(model->kernel_parm.rbf_gamma));
	fscanf(modelfl,"%lf # kernel parameter -s \n",&(model->kernel_parm.coef_lin));
	fscanf(modelfl,"%lf # kernel parameter -r \n",&(model->kernel_parm.coef_const));
	fscanf(modelfl,"%s # kernel parameter -u \n",&(model->kernel_parm.custom));
	fscanf(modelfl,"%ld # highest feature index \n",&(model->totwords));
	fscanf(modelfl,"%ld # number of training documents \n",&(model->totdoc));
	fscanf(modelfl,"%ld # number of support vectors plus 1 \n",&(model->sv_num));
	fscanf(modelfl,"%lf # threshold b \n",&(model->b));
	
	for(i=1;i<model->sv_num;i++) 
	{
		fgets(line,(int)ll,modelfl);
		pos=0;
		wpos=0;
		sscanf(line,"%lf",&model->alpha[i]);
		while(line[++pos]>' ');
		while((sscanf(line+pos,"%ld:%lf",&wnum,&weight) != EOF) && (wpos<max_words)) 
		{
			while(line[++pos]>' ');
			words[wpos].wnum=wnum;
			words[wpos].weight=weight; 
			wpos++;
		} 
		model->supvec[i] = (DOC *)my_malloc(sizeof(DOC));
		(model->supvec[i])->words = (SVM_WORD *)my_malloc(sizeof(SVM_WORD)*(wpos+1));
		for(j=0;j<wpos;j++)
		{
			(model->supvec[i])->words[j]=words[j]; 
		}
		((model->supvec[i])->words[wpos]).wnum=0;
		(model->supvec[i])->twonorm_sq = sprod_ss((model->supvec[i])->words,(model->supvec[i])->words);
		(model->supvec[i])->docnum = -1;
	}
	fclose(modelfl);
	free(line);
	free(words);
	if (com_pro.show_readfile)
	{
		sprintf(temstr, "OK. (%d support vectors read)",(int)(model->sv_num-1));
		printm(temstr);
	}
}
/*read the data from text documents*/
int CSVM::read_documents(char *docfile,
					DOC  *docs,
					long *label,
					long max_words_doc,
					long ll,
					long *totwords,
					long *totdoc,
					int post_label)
{
	char *line;
	DOC doc;
	long dnum=0,wpos,i,dpos=0,dneg=0,dunlab=0;
	long doc_label;
	FILE *docfl;

	line = (char *)my_malloc(sizeof(char)*ll);
	if ((docfl = fopen (docfile, "r")) == NULL)
	{ 
		printe (docfile);  
		return -1;
	}
	
	doc.words = (SVM_WORD *)my_malloc(sizeof(SVM_WORD)*(max_words_doc+10));
	if (com_pro.show_readfile)
	{
		sprintf(temstr,"Reading examples into memory..."); 
		printm(temstr);
	}
	
	dnum=0;
	(*totwords)=0;
	while((!feof(docfl)) && fgets(line,(int)ll,docfl)) {
		if(line[0] == '#') continue;  /* line contains comments */
		if(!parse_document(line,&doc,&doc_label,&wpos,max_words_doc)) 
		{
			sprintf(temstr,"Parsing error in line %ld!",dnum);
			printm(temstr);
		}

		if(doc_label==0)
		{
			label[dnum]=0;
			dunlab++;
		}
		else if(doc_label==post_label)
		{
			label[dnum]=1;
			dpos++;
		}
		else
		{
			label[dnum]=-1;
			dneg++;
		}

		if((wpos>1) && ((doc.words[wpos-2]).wnum>(*totwords))) 
			(*totwords)=(doc.words[wpos-2]).wnum;
		docs[dnum].words = (SVM_WORD *)my_malloc(sizeof(SVM_WORD)*wpos);
		docs[dnum].docnum=dnum;
		for(i=0;i<wpos;i++) 
			docs[dnum].words[i]=doc.words[i];
		docs[dnum].twonorm_sq=doc.twonorm_sq;
		dnum++;  
		if((dnum % 100) == 0&&com_pro.show_readfile) 
		{
			sprintf(temstr,"read %ld..",dnum); 
			printm(temstr);
		}
	} 
	
	fclose(docfl);
	free(line);
	free(doc.words);
	if (com_pro.show_action)
	{
		sprintf(temstr, "OK. (%ld examples read)", dnum);
		printm(temstr);
		sprintf(temstr,"%ld positive, %ld negative, and %ld unlabeled examples.",dpos,dneg,dunlab); 
		printm(temstr);
	}
	(*totdoc)=dnum;
}
/*Parse one  line of data file */
int CSVM::parse_document(char *line,DOC *doc,long *label,long*numwords,long max_words_doc)
{
	register long wpos,pos;
	long wnum;
	double weight;
	
	pos=0;
	while(line[pos]) 
	{      /* cut off comments */
		if(line[pos] == '#') 
		{
			line[pos]=0;
		}
		else 
		{
			pos++;
		}
	}
	wpos=0;
	if((sscanf(line,"%ld",label)) == EOF) return(0);
	pos=0;
	while(line[pos]==' ') pos++;
	while(line[pos]>' ') pos++;
	while((sscanf(line+pos,"%ld:%lf",&wnum,&weight) != EOF) && 	(wpos<max_words_doc))
	{
		while(line[pos++]==' ');
		while(line[++pos]>' ');
		if(wnum<=0) 
		{ 
			printe ("Feature numbers must be larger or equal to 1!!!"); 
			sprintf(temstr,"LINE: %s",line);
			printm(temstr);
			return (0);
		 
		}
		if((wpos>0) && ((doc->words[wpos-1]).wnum >= wnum))
		{ 
			printe ("Features must be in increasing order!!!"); 
			sprintf(temstr,"LINE: %s",line);
			printm(temstr);
			return (0);
		 
		}
		(doc->words[wpos]).wnum=wnum;
		(doc->words[wpos]).weight=weight; 
		wpos++;
	}
	(doc->words[wpos]).wnum=0;
	(*numwords)=wpos+1;
	doc->docnum=-1;
	doc->twonorm_sq=sprod_ss(doc->words,doc->words);
	return(1);
}
/* grep through file and count number of lines, 
maximum number of spaces per line, and 
longest line. */
int CSVM::nol_ll(char *file,long *nol,long *wol,long *ll) 
{
	FILE *fl;
	int ic;
	char c;
	long current_length,current_wol;
	
	if ((fl = fopen (file, "r")) == NULL)
	{ 
		printe (file);   
		return -1;
	}
	current_length=0;
	current_wol=0;
	(*ll)=0;
	(*nol)=1;
	(*wol)=0;
	while((ic=getc(fl)) != EOF) 
	{
		c=(char)ic;
		current_length++;
		if(c == ' ') 
		{
			current_wol++;
		}
		if(c == '\n') 
		{
			(*nol)++;
			if(current_length>(*ll)) 
			{
				(*ll)=current_length;
			}
			if(current_wol>(*wol)) 
			{
				(*wol)=current_wol;
			}
			current_length=0;
			current_wol=0;
		}
	}
	fclose(fl);
}

long CSVM::minl(long a,long b)
{
	if(a<b)
		return(a);
	else
		return(b);
}

long CSVM::maxl(long a,long b)
{
	if(a>b)
		return(a);
	else
		return(b);
}

long CSVM::get_runtime() 
{
	clock_t start;
	start = clock();
	return((long)((double)start*100.0/(double)CLOCKS_PER_SEC));
}


int CSVM::isnan(double a)
{
	return(_isnan(a));
}

void * CSVM::my_malloc(long size) 
{
	void *ptr;
	ptr=(void *)malloc(size);
	if(!ptr)
	{ 
		printe ("Out of memory!"); 
		return (NULL);
	}
	return(ptr);
}
//print error on screen

void CSVM::printe(char* pInfo)
{
	CMessage::PrintError(pInfo);
}

//print message on screen
void CSVM::printm(char* pInfo)
{
	CMessage::PrintInfo(pInfo);
}
//custom kernel function
/////////////////////////////////////////////////////////////////
//chen 2001.09.14
/////////////////////////////////////////////////////////////////

double CSVM::ktt(int ta,int tb,double pa[],double pb[])
{
	int ya,yb;
	ya=((ta-1)%13)+1;
	yb=((tb-1)%13)+1;
	
	if (ya<13&&yb<13) 
		return pa[ta]*pa[ta+1]*pb[tb]*pb[tb+1];
	else return 0.0;
	
}

double CSVM::kt(int t,double ta[],double tb[])
{
	
	int x,y;
	double sum=0.0;
	x=((t-1)/16)+1;
	y=((t-1)%16)+1;
	if (x>1) 
		sum+=ta[t]*tb[t]*ta[t-16]*tb[t-16];
	if (x>2) 
		sum+=ta[t]*tb[t]*ta[t-32]*tb[t-32];
	if (x>3) 
		sum+=ta[t]*tb[t]*ta[t-48]*tb[t-48];
	if (x<14) 
		sum+=ta[t]*tb[t]*ta[t-48]*tb[t-48];
	if (x<15) 
		sum+=ta[t]*tb[t]*ta[t-32]*tb[t-32];
	if (x<16)
		sum+=ta[t]*tb[t]*ta[t+16]*tb[t+16];
	if (y>3) 
		sum+=ta[t]*tb[t]*ta[t-3]*tb[t-3];
	if (y>2) 
		sum+=ta[t]*tb[t]*ta[t-2]*tb[t-2];
	if (y>1) 
		sum+=ta[t]*tb[t]*ta[t-1]*tb[t-1];
	if (y<14) 
		sum+=ta[t]*tb[t]*ta[t+3]*tb[t+3];
	if (y<15) 
		sum+=ta[t]*tb[t]*ta[t+2]*tb[t+2];
	if (y<16) 
		sum+=ta[t]*tb[t]*ta[t+1]*tb[t+1];
	return sum;
}

double CSVM::fi(double* tt)
{
	int x,y;
	double sum=0.0;
	for (int t=1;t<=52;t++)
	{
		x=((t-1)/13)+1;
		y=((t-1)%13)+1;
		if (y<13)
			sum+=tt[t]*tt[t+1];
	}
	return sum;
}
double CSVM::fs(double ta[])
{
	double sum=0.0;
	int x,y;
	for (int i=1;i<256;i++)
	{
		x=((i-1)/16)+1;
		y=((i-1)%16)+1;
		
		if (x<16)
			sum+=ta[i]*ta[i+16];
		if (y<16) 
			sum+=ta[i]*ta[i+1];
	}
	return sum;
}
double CSVM::sumofword(DOC* a)
{
	double sum=0.0;
	SVM_WORD* pwa=a->words;
	while (pwa->wnum)
	{
		sum+=pwa->weight;
		pwa++;
	}
	return sum;



}
double CSVM::custom_kernel(KERNEL_PARM *kernel_parm,DOC *a,DOC*b)
{
	double sum=0;
    SVM_WORD *ai,*bj;
    ai=a->words;
    bj=b->words;
    while (ai->wnum || bj->wnum) 
	{
		if(ai->wnum == bj->wnum) 
		{
			sum+=(fabs(ai->weight-bj->weight))*(fabs(ai->weight-bj->weight));
			ai++;bj++;
		}
		else if ((ai!=0) &&(ai->wnum<bj->wnum || bj->wnum==0))
		{
			sum+=fabs(ai->weight)*fabs(ai->weight);
			ai++;
		}
		else if ((bj!=0) &&(bj->wnum<ai->wnum|| ai->wnum==0))
		{
			sum+=fabs(bj->weight)*fabs(bj->weight);
			bj++;
		}
    }
//   case 1: /* polynomial *///
		//return((CFLOAT)pow(kernel_parm->coef_lin*sprod_ss(a->words,b->words)+kernel_parm->coef_const,(double)kernel_parm->poly_degree)); 
    //case 2: /* radial basis function */
	//	return((CFLOAT)exp(-kernel_parm->rbf_gamma*(a->twonorm_sq-2*sprod_ss(a->words,b->words)+b->twonorm_sq)));
    //case 3: /* sigmoid neural net */
	//	return((CFLOAT)tanh(kernel_parm->coef_lin*sprod_ss(a->words,b->words)+kernel_parm->coef_const)); 
    //case 4: /* custom-kernel supplied in file kernel.h*/
	//	return((CFLOAT)custom_kernel(kernel_parm,a,b)); 

/*	
    SVM_WORD *ai,*bj;
    ai=a->words;
    bj=b->words;
	double suma=0.0,sumb=0.0;

    while (ai->wnum ) 
	{
		suma+=ai->weight;
		ai++;
	}
    while (bj->wnum ) 
	{
		sumb+=bj->weight;
		bj++;
	}
	*/
//	double K_rbf=exp(-0.001*(a->twonorm_sq-2*sprod_ss(a->words,b->words)+b->twonorm_sq));
	double K_Laplace=exp(-0.0001*sum);
	double K_poly=pow(sprod_ss(a->words,b->words)+20,3);
//	double K_linear=sprod_ss(a->words,b->words);
	//double sum;
	//double sum=suma*sumb;
	//sum=K_rbf*K_poly;
	//double sum=fabs(pro*pro+pro-tan(pro));
	return K_Laplace*K_poly;

}
/********************************svm_common****************************/


/********************************svm_hideo****************************/
double *CSVM::optimize_qp(QP *qp,
double *epsilon_crit,
long nx, /* Maximum number of variables in QP */
double *threshold, 
LEARN_PARM *learn_parm)
/* start the optimizer and return the optimal values */
/* The HIDEO optimizer does not necessarily fully solve the problem. */
/* Since it requires a strictly positive definite hessian, the solution */
/* is restricted to a linear independent subset in case the matrix is */
/* only semi-definite. */
{
  long i;
  int result;
  double eq;

  if(!primal) { /* allocate memory at first call */
    primal=(double *)my_malloc(sizeof(double)*nx);
    dual=(double *)my_malloc(sizeof(double)*((nx+1)*2));
    nonoptimal=(long *)my_malloc(sizeof(long)*(nx));
    buffer=(double *)my_malloc(sizeof(double)*((nx+1)*2*(nx+1)*2+
					       nx*nx+2*(nx+1)*2+2*nx+1+2*nx+
					       nx+nx+nx*nx));
    (*threshold)=0;
  }

    eq=qp->opt_ce0[0];
    for(i=0;i<qp->opt_n;i++) {
      eq+=qp->opt_xinit[i]*qp->opt_ce[i];
    }
    
  

  result=optimize_hildreth_despo(qp->opt_n,qp->opt_m,
				 opt_precision,(*epsilon_crit),
				 learn_parm->epsilon_a,maxiter,
				 /* (long)PRIMAL_OPTIMAL, */
				 (long)0, (long)0,
				 lindep_sensitivity,
				 qp->opt_g,qp->opt_g0,qp->opt_ce,qp->opt_ce0,
				 qp->opt_low,qp->opt_up,primal,qp->opt_xinit,
				 dual,nonoptimal,buffer);

  if(learn_parm->totwords < learn_parm->svm_maxqpsize) { 
    /* larger working sets will be linear dependent anyway */
    learn_parm->svm_maxqpsize=maxl(learn_parm->totwords,(long)2);
  }

  if(result == NAN_SOLUTION) {
    lindep_sensitivity*=2;  /* throw out linear dependent examples more */
                            /* generously */
    if(learn_parm->svm_maxqpsize>2) {
      learn_parm->svm_maxqpsize--;  /* decrease size of qp-subproblems */
    }
    precision_violations++;
  }

  /* take one round of only two variable to get unstuck */
  if(result != PRIMAL_OPTIMAL) {

    smallroundcount++;

    result=optimize_hildreth_despo(qp->opt_n,qp->opt_m,
				   opt_precision,(*epsilon_crit),
				   learn_parm->epsilon_a,(long)maxiter,
				   (long)PRIMAL_OPTIMAL,(long)SMALLROUND,
				   lindep_sensitivity,
				   qp->opt_g,qp->opt_g0,qp->opt_ce,qp->opt_ce0,
				   qp->opt_low,qp->opt_up,primal,qp->opt_xinit,
				   dual,nonoptimal,buffer);
    }

    if(result != PRIMAL_OPTIMAL) {
      if(result != ONLY_ONE_VARIABLE) 
	precision_violations++;
      if(result == MAXITER_EXCEEDED) 
	maxiter+=100;
      if(result == NAN_SOLUTION) {
	lindep_sensitivity*=2;  /* throw out linear dependent examples more */
	                        /* generously */
	/* results not valid, so return inital values */
	for(i=0;i<qp->opt_n;i++) {
	  primal[i]=qp->opt_xinit[i];
	}
      }
    }


  if(precision_violations > 50) {
    precision_violations=0;
    (*epsilon_crit)*=10.0; 
    }


  if((qp->opt_m>0) && (result != NAN_SOLUTION))
    (*threshold)=dual[1]-dual[0];
  else
    (*threshold)=0;

  return(primal);
}



int CSVM::optimize_hildreth_despo(
     long   n,            /* number of variables */
     long   m,            /* number of linear equality constraints [0,1] */
     double precision,    /* solve at least to this dual precision */
     double epsilon_crit, /* stop, if KT-Conditions approx fulfilled */
     double epsilon_a,    /* precision of alphas at bounds */
     long   maxiter,      /* stop after this many iterations */
     long   goal,         /* keep going until goal fulfilled */
     long   smallround,   /* use only two variables of steepest descent */
     double lindep_sensitivity, /* epsilon for detecting linear dependent ex */
     double *g,           /* hessian of objective */
     double *g0,          /* linear part of objective */
     double *ce,double *ce0,     /* linear equality constraints */
     double *low,double *up,     /* box constraints */
     double *primal,      /* primal variables */
     double *init,        /* initial values of primal */
     double *dual,        /* dual variables */
     long   *lin_dependent,
     double *buffer)
{
  long i,j,k,from,to,n_indep,changed;
  double sum,bmin=0,bmax=0;
  double *d,*d0,*ig,*dual_old,*temp,*start;       
  double *g0_new,*g_new,*ce_new,*ce0_new,*low_new,*up_new;
  double add,t;
  int result;
  //double obj_before,obj_after; 
  long b1,b2;

  g0_new=&(buffer[0]);    /* claim regions of buffer */
  d=&(buffer[n]);
  d0=&(buffer[n+(n+m)*2*(n+m)*2]);
  ce_new=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2]);
  ce0_new=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n]);
  ig=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m]);
  dual_old=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m+n*n]);
  low_new=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m+n*n+(n+m)*2]);
  up_new=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m+n*n+(n+m)*2+n]);
  start=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m+n*n+(n+m)*2+n+n]);
  g_new=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m+n*n+(n+m)*2+n+n+n]);
  temp=&(buffer[n+(n+m)*2*(n+m)*2+(n+m)*2+n+m+n*n+(n+m)*2+n+n+n+n*n]);

  b1=-1;
  b2=-1;
  for(i=0;i<n;i++) {   /* get variables with steepest feasible descent */
    sum=g0[i];         
    for(j=0;j<n;j++) 
      sum+=init[j]*g[i*n+j];
    sum=sum*ce[i];
    if(((b1==-1) || (sum<bmin)) 
       && (!((init[i]<=(low[i]+epsilon_a)) && (ce[i]<0.0)))
       && (!((init[i]>=( up[i]-epsilon_a)) && (ce[i]>0.0)))
       ) {
      bmin=sum;
      b1=i;
    }
    if(((b2==-1) || (sum>bmax)) 
       && (!((init[i]<=(low[i]+epsilon_a)) && (ce[i]>0.0)))
       && (!((init[i]>=( up[i]-epsilon_a)) && (ce[i]<0.0)))
       ) {
      bmax=sum;
      b2=i;
    }
  }
  
  for(i=0;i<n;i++) {
    start[i]=init[i];
  }

  /* in case both examples are equal */
  add=0;
  changed=0;
  if((-g[b1*n+b2] == g[b1*n+b1]) && (-g[b1*n+b2] == g[b2*n+b2])) {
    changed=1;
    if(ce[b1] == ce[b2]) { /* distribute evenly */
      start[b1]=(init[b1]+init[b2])/2.0;
      start[b2]=(init[b1]+init[b2])/2.0;
      if(start[b2] > up[b2]) {
	t=start[b2]-up[b2];
	start[b2]=up[b2];
	start[b1]+=t;
      }
      if(start[b1] > up[b1]) {
	t=start[b1]-up[b1];
	start[b1]=up[b1];
	start[b2]+=t;
      }
    }
    else { /* set to upper bound */
      t=up[b1]-init[b1];
      if((up[b2]-init[b2]) < t) {
	t=up[b2]-init[b2];
      }
      start[b1]=init[b1]+t;
      start[b2]=init[b2]+t;
    }
  }
  /* if we have a biased hyperplane, then adding a constant to the */
  /* hessian does not change the solution. So that is done for examples */
  /* with zero diagonal entry, since HIDEO cannot handle them. */
  else if((fabs(g[b1*n+b1]) < lindep_sensitivity) 
	  || (fabs(g[b2*n+b2]) < lindep_sensitivity)) {
    add+=0.093274;
  }    
  /* in case both examples are linear dependent */
  else if(fabs(g[b1*n+b1]/g[b1*n+b2] - g[b1*n+b2]/g[b2*n+b2])
	  < lindep_sensitivity) { 
    add+=0.078274;
  }


/* sprintf(temstr,"b1=%ld,b2=%ld\n",b1,b2); */

  lcopy_matrix(g,n,d);
  if((m==1) && (add>0.0)) {
    for(j=0;j<n;j++) {
      for(k=0;k<n;k++) {
	d[j*n+k]+=add*ce[j]*ce[k];
      }
    }
  }
  else {
    add=0.0;
  }

  if(n>2) {                    /* switch, so that variables are better mixed */
    lswitchrk_matrix(d,n,b1,(long)0); 
    if(b2 == 0) 
      lswitchrk_matrix(d,n,b1,(long)1); 
    else
      lswitchrk_matrix(d,n,b2,(long)1); 
  }
  if(smallround == SMALLROUND) {
    for(i=2;i<n;i++) {
      lin_dependent[i]=1;
    }
    lin_dependent[0]=0;
    lin_dependent[1]=0;
  }
  else {
    for(i=0;i<n;i++) {
      lin_dependent[i]=0;
    }
  }
  linvert_matrix(d,n,ig,lindep_sensitivity,lin_dependent);
  if(n>2) {                    /* now switch back */
    if(b2 == 0) {
      lswitchrk_matrix(ig,n,b1,(long)1); 
      i=lin_dependent[1];  
      lin_dependent[1]=lin_dependent[b1];
      lin_dependent[b1]=i;
    }
    else {
      lswitchrk_matrix(ig,n,b2,(long)1); 
      i=lin_dependent[1];  
      lin_dependent[1]=lin_dependent[b2];
      lin_dependent[b2]=i;
    }
    lswitchrk_matrix(ig,n,b1,(long)0); 
    i=lin_dependent[0];  
    lin_dependent[0]=lin_dependent[b1];
    lin_dependent[b1]=i;
  }
  /* lprint_matrix(d,n); */
  /* lprint_matrix(ig,n); */

  lcopy_matrix(g,n,g_new);   /* restore g_new matrix */
  if(add>0)
    for(j=0;j<n;j++) {
      for(k=0;k<n;k++) {
	g_new[j*n+k]+=add*ce[j]*ce[k];
      }
    }

  for(i=0;i<n;i++) {  /* fix linear dependent vectors */
    g0_new[i]=g0[i]+add*ce0[0]*ce[i];
  }
  if(m>0) ce0_new[0]=-ce0[0];
  for(i=0;i<n;i++) {  /* fix linear dependent vectors */
    if(lin_dependent[i]) {
      for(j=0;j<n;j++) {
	if(!lin_dependent[j]) {
	  g0_new[j]+=start[i]*g_new[i*n+j];
	}
      }
      if(m>0) ce0_new[0]-=(start[i]*ce[i]);
    }
  }
  from=0;   /* remove linear dependent vectors */
  to=0;
  n_indep=0;
  for(i=0;i<n;i++) {
    if(!lin_dependent[i]) {
      g0_new[n_indep]=g0_new[i];
      ce_new[n_indep]=ce[i]; 
      low_new[n_indep]=low[i];
      up_new[n_indep]=up[i];
      primal[n_indep]=start[i];
      n_indep++;
    }
    for(j=0;j<n;j++) {
      if((!lin_dependent[i]) && (!lin_dependent[j])) {
        ig[to]=ig[from];
        g_new[to]=g_new[from];
	to++;
      }
      from++;
    }
  }

  
  /* cannot optimize with only one variable */
  if((n_indep<=1) && (m>0) && (!changed)) { 
    for(i=n-1;i>=0;i--) {
      primal[i]=init[i];
    }
    return((int)ONLY_ONE_VARIABLE);
  }

  result=solve_dual(n_indep,m,precision,epsilon_crit,maxiter,g_new,g0_new,
		    ce_new,ce0_new,low_new,up_new,primal,d,d0,ig,
		    dual,dual_old,temp,goal);
  
  j=n_indep;
  for(i=n-1;i>=0;i--) {
    if(!lin_dependent[i]) {
      j--;
      primal[i]=primal[j];
    }
    else if((m==0) && (g[i*n+i]==0)) {
      /* if we use a biased hyperplane, each example with a zero diagonal */
      /* entry must have an alpha at the upper bound. Doing this */
      /* is essential for the HIDEO optimizer, since it cannot handle zero */
      /* diagonal entries in the hessian for the unbiased hyperplane case. */
      primal[i]=up[i];  
    }
    else {
      primal[i]=start[i];  /* leave as is */
    }
    temp[i]=primal[i];
  }
   
 
  return((int)result);
}


int CSVM::solve_dual(
     /* Solves the dual using the method of Hildreth and D'Espo. */
     /* Can only handle problems with zero or exactly one */
     /* equality constraints. */

     long   n,            /* number of variables */
     long   m,            /* number of linear equality constraints */
     double precision,    /* solve at least to this dual precision */
     double epsilon_crit, /* stop, if KT-Conditions approx fulfilled */
     long   maxiter,      /* stop after that many iterations */
     double *g,
     double *g0,          /* linear part of objective */
     double *ce,double *ce0,     /* linear equality constraints */
     double *low,double *up,     /* box constraints */
     double *primal,      /* variables (with initial values) */
     double *d,double *d0,double *ig,double *dual,double *dual_old,double *temp,       /* buffer  */
     long goal)
{
  long i,j,k,iter;
  double sum,w,maxviol,viol,temp1,temp2,isnantest;
  double model_b,dist;
  long retrain,maxfaktor,primal_optimal=0,at_bound,scalemaxiter;
  double epsilon_a=1E-15,epsilon_hideo;
  double eq; 

  if((m<0) || (m>1)) 
    printe("SOLVE DUAL: inappropriate number of eq-constrains!");


  for(i=0;i<2*(n+m);i++) {
    dual[i]=0;
    dual_old[i]=0;
  }
  for(i=0;i<n;i++) {   
    for(j=0;j<n;j++) {   /* dual hessian for box constraints */
      d[i*2*(n+m)+j]=ig[i*n+j];
      d[(i+n)*2*(n+m)+j]=-ig[i*n+j];
      d[i*2*(n+m)+j+n]=-ig[i*n+j];
      d[(i+n)*2*(n+m)+j+n]=ig[i*n+j];
    }
    if(m>0) {
      sum=0;              /* dual hessian for eq constraints */
      for(j=0;j<n;j++) {
	sum+=(ce[j]*ig[i*n+j]);
      }
      d[i*2*(n+m)+2*n]=sum;
      d[i*2*(n+m)+2*n+1]=-sum;
      d[(n+i)*2*(n+m)+2*n]=-sum;
      d[(n+i)*2*(n+m)+2*n+1]=sum;
      d[(n+n)*2*(n+m)+i]=sum;
      d[(n+n+1)*2*(n+m)+i]=-sum;
      d[(n+n)*2*(n+m)+(n+i)]=-sum;
      d[(n+n+1)*2*(n+m)+(n+i)]=sum;
      
      sum=0;
      for(j=0;j<n;j++) {
	for(k=0;k<n;k++) {
	  sum+=(ce[k]*ce[j]*ig[j*n+k]);
	}
      }
      d[(n+n)*2*(n+m)+2*n]=sum;
      d[(n+n)*2*(n+m)+2*n+1]=-sum;
      d[(n+n+1)*2*(n+m)+2*n]=-sum;
      d[(n+n+1)*2*(n+m)+2*n+1]=sum;
    } 
  }

  for(i=0;i<n;i++) {   /* dual linear component for the box constraints */
    w=0;
    for(j=0;j<n;j++) {
      w+=(ig[i*n+j]*g0[j]); 
    }
    d0[i]=up[i]+w;
    d0[i+n]=-low[i]-w;
  }

  if(m>0) {  
    sum=0;             /* dual linear component for eq constraints */
    for(j=0;j<n;j++) {
      for(k=0;k<n;k++) {
	sum+=(ce[k]*ig[k*n+j]*g0[j]); 
      }
    }
    d0[2*n]=ce0[0]+sum;
    d0[2*n+1]=-ce0[0]-sum;
  }

  maxviol=999999;
  iter=0;
  retrain=1;
  maxfaktor=1;
  scalemaxiter=maxiter/5;
  while((retrain) && (maxviol > 0) && (iter < (scalemaxiter*maxfaktor))) {
    iter++;
    
    while((maxviol > precision) && (iter < (scalemaxiter*maxfaktor))) {
      iter++;
      maxviol=0;
      for(i=0;i<2*(n+m);i++) {
	sum=d0[i];
	for(j=0;j<2*(n+m);j++) {
	  sum+=d[i*2*(n+m)+j]*dual_old[j];
	}
	sum-=d[i*2*(n+m)+i]*dual_old[i];
	dual[i]=-sum/d[i*2*(n+m)+i];
	if(dual[i]<0) dual[i]=0;
	
	viol=fabs(dual[i]-dual_old[i]);
	if(viol>maxviol) 
	  maxviol=viol;
	dual_old[i]=dual[i];
      }
      /*
      sprintf(temstr,"%d) maxviol=%20f precision=%f\n",iter,maxviol,precision); 
      */
    }
  
    if(m>0) {
      for(i=0;i<n;i++) {
	temp[i]=dual[i]-dual[i+n]+ce[i]*(dual[n+n]-dual[n+n+1])+g0[i];
      }
    } 
    else {
      for(i=0;i<n;i++) {
	temp[i]=dual[i]-dual[i+n]+g0[i];
      }
    }
    for(i=0;i<n;i++) {
      primal[i]=0;             /* calc value of primal variables */
      for(j=0;j<n;j++) {
	primal[i]+=ig[i*n+j]*temp[j];
      }
      primal[i]*=-1.0;
      if(primal[i]<=(low[i])) {  /* clip conservatively */
	primal[i]=low[i];
      }
      else if(primal[i]>=(up[i])) {
	primal[i]=up[i];
      }
    }

    if(m>0) 
      model_b=dual[n+n+1]-dual[n+n];
    else
      model_b=0;

    epsilon_hideo=EPSILON_HIDEO;
    for(i=0;i<n;i++) {           /* check precision of alphas */
      isnantest+=primal[j];
      dist=-model_b*ce[i]; 
      dist+=(g0[i]+1.0);
      for(j=0;j<i;j++) {
	dist+=(primal[j]*g[j*n+i]);
      }
      for(j=i;j<n;j++) {
	dist+=(primal[j]*g[i*n+j]);
      }
      if((primal[i]<(up[i]-epsilon_hideo)) && (dist < (1.0-epsilon_crit))) {
	epsilon_hideo=(up[i]-primal[i])*2.0;
      }
      else if((primal[i]>(low[i]+epsilon_hideo)) &&(dist>(1.0+epsilon_crit))) {
	epsilon_hideo=(primal[i]-low[i])*2.0;
      }
    }
     /*sprintf(temstr,"\nEPSILON_HIDEO=%.30f\n",epsilon_hideo); */


    for(i=0;i<n;i++) {           /* clip alphas to bounds */
      if(primal[i]<=(low[i]+epsilon_hideo)) {
	primal[i]=low[i];
      }
      else if(primal[i]>=(up[i]-epsilon_hideo)) {
	primal[i]=up[i];
      }
    }

    retrain=0;
    primal_optimal=1;
    at_bound=0;
    for(i=0;(i<n);i++) {  /* check primal KT-Conditions */
      dist=-model_b*ce[i]; 
      dist+=(g0[i]+1.0);
      for(j=0;j<i;j++) {
	dist+=(primal[j]*g[j*n+i]);
      }
      for(j=i;j<n;j++) {
	dist+=(primal[j]*g[i*n+j]);
      }
      if((primal[i]<(up[i]-epsilon_a)) && (dist < (1.0-epsilon_crit))) {
	retrain=1;
	primal_optimal=0;
      }
      else if((primal[i]>(low[i]+epsilon_a)) && (dist > (1.0+epsilon_crit))) {
	retrain=1;
	primal_optimal=0;
      }
      if((primal[i]<=(low[i]+epsilon_a)) || (primal[i]>=(up[i]-epsilon_a))) {
	at_bound++;
      }
      /*    sprintf(temstr,"HIDEOtemp: a[%ld]=%.30f, dist=%.6f, b=%f, at_bound=%ld\n",i,primal[i],dist,model_b,at_bound);  */
    }
    if(m>0) {
      eq=-ce0[0];               /* check precision of eq-constraint */
      for(i=0;i<n;i++) { 
	eq+=(ce[i]*primal[i]);
      }
      if((EPSILON_EQ < fabs(eq)) 
	 /*
	 && !((goal==PRIMAL_OPTIMAL) 
	       && (at_bound==n)) */
	 ) {
	retrain=1;
	primal_optimal=0;
      }
      /* sprintf(temstr,"\n eq=%.30f ce0=%f at-bound=%ld\n",eq,ce0[0],at_bound);  */
    }

    if(retrain) {
      precision/=10;
      if(((goal == PRIMAL_OPTIMAL) && (maxfaktor < 50000))
	 || (maxfaktor < 5)) {
	maxfaktor++;
      }
    }
  }

  if(!primal_optimal) {
    for(i=0;i<n;i++) {
      primal[i]=0;             /* calc value of primal variables */
      for(j=0;j<n;j++) {
	primal[i]+=ig[i*n+j]*temp[j];
      }
      primal[i]*=-1.0;
      if(primal[i]<=(low[i]+epsilon_a)) {  /* clip conservatively */
	primal[i]=low[i];
      }
      else if(primal[i]>=(up[i]-epsilon_a)) {
	primal[i]=up[i];
      }
    }
  }

  isnantest=0;
  for(i=0;i<n;i++) {           /* check for isnan */
    isnantest+=primal[i];
  }

  if(m>0) {
    temp1=dual[n+n+1];   /* copy the dual variables for the eq */
    temp2=dual[n+n];     /* constraints to a handier location */
    for(i=n+n+1;i>=2;i--) {
      dual[i]=dual[i-2];
    }
    dual[0]=temp2;
    dual[1]=temp1;
    isnantest+=temp1+temp2;
  }

  if(isnan(isnantest)) {
    return((int)NAN_SOLUTION);
  }
  else if(primal_optimal) {
    return((int)PRIMAL_OPTIMAL);
  }
  else if(maxviol == 0.0) {
    return((int)DUAL_OPTIMAL);
  }
  else {
    return((int)MAXITER_EXCEEDED);
  }
}


void CSVM::linvert_matrix(
double *matrix,
long depth,
double *inverse,double lindep_sensitivity,
long *lin_dependent)  /* indicates the active parts of matrix on 
			 input and output*/
{
  long i,j,k;
  double factor;

  for(i=0;i<depth;i++) {
    /*    lin_dependent[i]=0; */
    for(j=0;j<depth;j++) {
      inverse[i*depth+j]=0.0;
    }
    inverse[i*depth+i]=1.0;
  }
  for(i=0;i<depth;i++) {
    if(lin_dependent[i] || (fabs(matrix[i*depth+i])<lindep_sensitivity)) {
      lin_dependent[i]=1;
    }
    else {
      for(j=i+1;j<depth;j++) {
	factor=matrix[j*depth+i]/matrix[i*depth+i];
	for(k=i;k<depth;k++) {
	  matrix[j*depth+k]-=(factor*matrix[i*depth+k]);
	}
	for(k=0;k<depth;k++) {
	  inverse[j*depth+k]-=(factor*inverse[i*depth+k]);
	}
      }
    }
  }
  for(i=depth-1;i>=0;i--) {
    if(!lin_dependent[i]) {
      factor=1/matrix[i*depth+i];
      for(k=0;k<depth;k++) {
	inverse[i*depth+k]*=factor;
      }
      matrix[i*depth+i]=1;
      for(j=i-1;j>=0;j--) {
	factor=matrix[j*depth+i];
	matrix[j*depth+i]=0;
	for(k=0;k<depth;k++) {
	  inverse[j*depth+k]-=(factor*inverse[i*depth+k]);
	}
      }
    }
  }
}

void CSVM::ladd_matrix(
double *matrix,
long depth,
double scalar)
{
  long i,j;
  for(i=0;i<depth;i++) {
    for(j=0;j<depth;j++) {
      matrix[i*depth+j]+=scalar;
    }
  }
}

void CSVM::lcopy_matrix(
double *matrix,
long depth,
double *matrix2)
{
  long i;
  
  for(i=0;i<(depth)*(depth);i++) {
    matrix2[i]=matrix[i];
  }
}

void CSVM::lswitch_rows_matrix(
double *matrix,
long depth,long r1,long r2)
{
  long i;
  double temp;

  for(i=0;i<depth;i++) {
    temp=matrix[r1*depth+i];
    matrix[r1*depth+i]=matrix[r2*depth+i];
    matrix[r2*depth+i]=temp;
  }
}

void CSVM::lswitchrk_matrix(
double *matrix,
long depth,long rk1,long rk2)
{
  long i;
  double temp;

  for(i=0;i<depth;i++) {
    temp=matrix[rk1*depth+i];
    matrix[rk1*depth+i]=matrix[rk2*depth+i];
    matrix[rk2*depth+i]=temp;
  }
  for(i=0;i<depth;i++) {
    temp=matrix[i*depth+rk1];
    matrix[i*depth+rk1]=matrix[i*depth+rk2];
    matrix[i*depth+rk2]=temp;
  }
}

double CSVM::calculate_qp_objective(
long opt_n,
double *opt_g,double *opt_g0,double *alpha)
{
  double obj;
  long i,j;
  obj=0;  /* calculate objective  */
  for(i=0;i<opt_n;i++) {
    obj+=(opt_g0[i]*alpha[i]);
    obj+=(0.5*alpha[i]*alpha[i]*opt_g[i*opt_n+i]);
    for(j=0;j<i;j++) {
      obj+=(alpha[j]*alpha[i]*opt_g[j*opt_n+i]);
    }
  }
  return(obj);
}
/********************************svm_hideo****************************/


/********************************svm_learn****************************/
/* Learns an SVM model based on the training data in docs/label. The resulting
model is returned in the structure model. */

void CSVM::svm_learn(
               DOC *docs,                
               long *label,               
               long totdoc,               
               long totwords,             
               LEARN_PARM *learn_parm,    
               KERNEL_PARM *kernel_parm, 
               KERNEL_CACHE *kernel_cache,
               MODEL *model             
               )
{
    long *inconsistent,i;
    long inconsistentnum;
    long misclassified,upsupvecnum;
    double loss,model_length,example_length;
    double maxdiff,*lin,*a;
    long runtime_start,runtime_end;
    long iterations;
    long *unlabeled,transduction;
    long heldout;
    long loo_count=0,loo_count_pos=0,loo_count_neg=0,trainpos=0,trainneg=0;
    long loocomputed=0,runtime_start_loo=0,runtime_start_xa=0;
    double heldout_c=0,r_delta_sq=0,r_delta,r_delta_avg;
    
    double *xi_fullset; /* buffer for storing xi on full sample in loo */
    double *a_fullset;  /* buffer for storing alpha on full sample in loo */
    TIMING timing_profile;
    SHRINK_STATE shrink_state;
    
    runtime_start=get_runtime();

    timing_profile.time_kernel=0;
    timing_profile.time_opti=0;
    timing_profile.time_shrink=0;
    timing_profile.time_update=0;
    timing_profile.time_model=0;
    timing_profile.time_check=0;
    timing_profile.time_select=0;
    
	com_result.kernel_cache_statistic=0;
    
    learn_parm->totwords=totwords;
    
    /* make sure -n value is reasonable */
    if((learn_parm->svm_newvarsinqp < 2) || (learn_parm->svm_newvarsinqp > learn_parm->svm_maxqpsize))
    {
        learn_parm->svm_newvarsinqp=learn_parm->svm_maxqpsize;
    }
    
    init_shrink_state(&shrink_state,totdoc,(long)10000);
    
    inconsistent = (long *)my_malloc(sizeof(long)*totdoc);
    unlabeled = (long *)my_malloc(sizeof(long)*totdoc);
    a = (double *)my_malloc(sizeof(double)*totdoc);
    a_fullset = (double *)my_malloc(sizeof(double)*totdoc);
    xi_fullset = (double *)my_malloc(sizeof(double)*totdoc);
    lin = (double *)my_malloc(sizeof(double)*totdoc);
    learn_parm->svm_cost = (double *)my_malloc(sizeof(double)*totdoc);
    model->supvec = (DOC **)my_malloc(sizeof(DOC *)*(totdoc+2));
    model->alpha = (double *)my_malloc(sizeof(double)*(totdoc+2));
    model->index = (long *)my_malloc(sizeof(long)*(totdoc+2));

    
    model->at_upper_bound=0;
    model->b=0;        
    model->supvec[0]=0;  /* element 0 reserved and empty for now */
    model->alpha[0]=0;
    model->lin_weights=NULL;
    model->totwords=totwords;
    model->totdoc=totdoc;
    model->kernel_parm=(*kernel_parm);
    model->sv_num=1;
    model->loo_error=-1;
    model->loo_recall=-1;
    model->loo_precision=-1;
    model->xa_error=-1;
    model->xa_recall=-1;
    model->xa_precision=-1;
    inconsistentnum=0;
    transduction=0;
    
    r_delta=estimate_r_delta(docs,totdoc,kernel_parm);
    r_delta_sq=r_delta*r_delta;
    
    r_delta_avg=estimate_r_delta_average(docs,totdoc,kernel_parm);
    if(learn_parm->svm_c == 0.0)
    {  /* default value for C */
        learn_parm->svm_c=1.0/(r_delta_avg*r_delta_avg);
		if (com_pro.show_compute_1)
		{
        sprintf(temstr,"Setting default regularization parameter C=%.4f\n",learn_parm->svm_c);
        printm(temstr);
		}
    }
    
    for(i=0;i<totdoc;i++) 
    {    /* various inits */
        inconsistent[i]=0;
        a[i]=0;
        lin[i]=0;
        unlabeled[i]=0;
        if(label[i] == 0) 
        {
            unlabeled[i]=1;
            transduction=1;
        }
        if(label[i] > 0)
        {
            learn_parm->svm_cost[i]=learn_parm->svm_c*learn_parm->svm_costratio*
                fabs((double)label[i]);
            label[i]=1;
            trainpos++;
        }
        else if(label[i] < 0) 
        {
            learn_parm->svm_cost[i]=learn_parm->svm_c*fabs((double)label[i]);
            label[i]=-1;
            trainneg++;
        }
        else
        {
            learn_parm->svm_cost[i]=0;
        }
    }
    
    /* caching makes no sense for linear kernel */
    if(kernel_parm->kernel_type == LINEAR)
    {
        kernel_cache = NULL;   
    } 
    
    if(transduction) 
    {
        learn_parm->svm_iter_to_shrink=99999999;
        sprintf(temstr,"\nDeactivating Shrinking due to an incompatibility with the transductive \nlearner in the current version.\n\n");
        printm(temstr);
    }
    
    if(transduction && learn_parm->compute_loo) 
    {
        learn_parm->compute_loo=0;
        sprintf(temstr,"\nCannot compute leave-one-out estimates for transductive learner.\n\n");
        printm(temstr);
    }
    
    if(learn_parm->remove_inconsistent && learn_parm->compute_loo) 
    {
        learn_parm->compute_loo=0;
        sprintf(temstr,"\nCannot compute leave-one-out estimates when removing inconsistent examples.\n\n");
        printm(temstr);
    }    
    
    if((trainpos == 1) || (trainneg == 1)) 
    {
        learn_parm->compute_loo=0;
        sprintf(temstr,"\nCannot compute leave-one-out with only one example in one class.\n\n");
        printm(temstr);
    }    
    
    if (com_pro.show_action)
	{
		sprintf(temstr,"Optimizing..."); 
		printm(temstr);
	}
    /* train the svm */
    iterations=optimize_to_convergence(docs,label,totdoc,totwords,learn_parm,
        kernel_parm,kernel_cache,&shrink_state,model,inconsistent,unlabeled,a,lin,&timing_profile,  &maxdiff,(long)-1,(long)1);

    //sprintf(temstr,"(oooooo)");
	printm(temstr);
    if (com_pro.show_action)
	{
		sprintf(temstr,"done. (%ld iterations) ",iterations);
		printm(temstr);
	}
    
    misclassified=0;
    for(i=0;(i<totdoc);i++)
    { /* get final statistic */
        if((lin[i]-model->b)*(double)label[i] <= 0.0) 
            misclassified++;
    }
	if (com_pro.show_action)
	{
		printm("optimization finished");
	}
	if (com_pro.show_trainresult)
	{
		sprintf(temstr," (%ld misclassified, maxdiff=%.5f).\n", misclassified,maxdiff); 
		printm(temstr);
	}
	com_result.train_misclassify=misclassified;
	com_result.max_difference=maxdiff;
             
    runtime_end=get_runtime();
             
    if (learn_parm->remove_inconsistent)
    {     
        inconsistentnum=0;
        for(i=0;i<totdoc;i++) 
            if(inconsistent[i]) 
               inconsistentnum++;
        sprintf(temstr,"Number of SV: %ld (plus %ld inconsistent examples)\n", model->sv_num-1,inconsistentnum);
        printm(temstr);
    }
    
    else
    {
     upsupvecnum=0;
     for(i=1;i<model->sv_num;i++) 
     {
         if(fabs(model->alpha[i]) >= (learn_parm->svm_cost[(model->supvec[i])->docnum]-learn_parm->epsilon_a)) 
             upsupvecnum++;
     }
	 if (com_pro.show_trainresult)
	 {
	 sprintf(temstr,"Number of SV: %ld (including %ld at upper bound)\n", model->sv_num-1,upsupvecnum);
     printm(temstr);
	 }
    }
	
	if( (!learn_parm->skip_final_opt_check)) 
	{
		loss=0;
		model_length=0; 
		for(i=0;i<totdoc;i++)
		{
			if((lin[i]-model->b)*(double)label[i] < 1.0-learn_parm->epsilon_crit)
				loss+=1.0-(lin[i]-model->b)*(double)label[i];
			model_length+=a[i]*label[i]*lin[i];
		}
		model_length=sqrt(model_length);
		sprintf(temstr,"L1 loss: loss=%.5f\n",loss);   printm(temstr);
		sprintf(temstr,"Norm of weight vector: |w|=%.5f\n",model_length);printm(temstr);
		example_length=estimate_sphere(model,kernel_parm); 
		sprintf(temstr,"Norm of longest example vector: |x|=%.5f\n",  length_of_longest_document_vector(docs,totdoc,kernel_parm));
		printm(temstr);
		sprintf(temstr,"Estimated VCdim of classifier: VCdim<=%.5f\n",       estimate_margin_vcdim(model,model_length,example_length,    kernel_parm));
		printm(temstr);
		if((!learn_parm->remove_inconsistent) && (!transduction)) 
		{
			runtime_start_xa=get_runtime();
                     sprintf(temstr,"Computing XiAlpha-estimates..."); 
                     printm(temstr);
                     compute_xa_estimates(model,label,unlabeled,totdoc,docs,lin,a,
                         kernel_parm,learn_parm,&(model->xa_error),
                         &(model->xa_recall),&(model->xa_precision));
                     
                     
                     sprintf(temstr,"Runtime for XiAlpha-estimates in cpu-seconds: %.2f\n",
                         (get_runtime()-runtime_start_xa)/100.0);
                     printm(temstr);
                     
                     fprintf(stdout,"XiAlpha-estimate of the error: error<=%.2f%% (rho=%.2f,depth=%ld)\n",
                         model->xa_error,learn_parm->rho,learn_parm->xa_depth);
                     fprintf(stdout,"XiAlpha-estimate of the recall: recall=>%.2f%% (rho=%.2f,depth=%ld)\n",
                         model->xa_recall,learn_parm->rho,learn_parm->xa_depth);
                     fprintf(stdout,"XiAlpha-estimate of the precision: precision=>%.2f%% (rho=%.2f,depth=%ld)\n",
                         model->xa_precision,learn_parm->rho,learn_parm->xa_depth);
                 }
                 else if(!learn_parm->remove_inconsistent)
                 {
                     estimate_transduction_quality(model,label,unlabeled,totdoc,docs,lin);
                 }
             }
	if (com_pro.show_trainresult)
	{
		sprintf(temstr,"Number of kernel evaluations: %ld\n",com_result.kernel_cache_statistic);
        printm(temstr);
	}
             /* leave-one-out testing starts now */
             if(learn_parm->compute_loo)
             {
                 /* save results of training on full dataset for leave-one-out */
                 runtime_start_loo=get_runtime();
                 for(i=0;i<totdoc;i++) 
                 {
                     xi_fullset[i]=1.0-((lin[i]-model->b)*(double)label[i]);
                     a_fullset[i]=a[i];
                 }
                 sprintf(temstr,"Computing leave-one-out");
                 printm(temstr);
                 
                 /* repeat this loop for every held-out example */
                 for(heldout=0;(heldout<totdoc);heldout++)
                 {
                     if(learn_parm->rho*a_fullset[heldout]*r_delta_sq+xi_fullset[heldout]
                         < 1.0) 
                     { 
                         /* guaranteed to not produce a leave-one-out error */
                         sprintf(temstr,"+"); 
                         printm(temstr);
                     }
                     else if(xi_fullset[heldout] > 1.0) 
                     {
                         /* guaranteed to produce a leave-one-out error */
                         loo_count++;
                         if(label[heldout] > 0) loo_count_pos++; else loo_count_neg++;
                         sprintf(temstr,"-");  printm(temstr);
                     }
                     else
                     {
                         loocomputed++;
                         heldout_c=learn_parm->svm_cost[heldout]; /* set upper bound to zero */
                         learn_parm->svm_cost[heldout]=0;
                         /* make sure heldout example is not currently  */
                         /* shrunk away. Assumes that lin is up to date! */
                         shrink_state.active[heldout]=1;  
                         
                         
                         optimize_to_convergence(docs,label,totdoc,totwords,learn_parm,
                             kernel_parm,
                             kernel_cache,&shrink_state,model,inconsistent,unlabeled,
                             a,lin,&timing_profile,
                             &maxdiff,heldout,(long)2);
                         
                         /* printf("%f\n",(lin[heldout]-model->b)*(double)label[heldout]); */
                         
                         if(((lin[heldout]-model->b)*(double)label[heldout]) < 0.0)
                         { 
                             loo_count++;                           /* there was a loo-error */
                             if(label[heldout] > 0) loo_count_pos++; else loo_count_neg++;
                         }
                         else
                         {
                             
                         }
                         /* now we need to restore the original data set*/
                         learn_parm->svm_cost[heldout]=heldout_c; /* restore upper bound */
                     }
                 } /* end of leave-one-out loop */
                 
                 
                 sprintf(temstr,"\nRetrain on full problem");  printm(temstr);
                 optimize_to_convergence(docs,label,totdoc,totwords,learn_parm,
                     kernel_parm,
                     kernel_cache,&shrink_state,model,inconsistent,unlabeled,
                     a,lin,&timing_profile,
                     &maxdiff,(long)-1,(long)1);
                 
                 
                 /* after all leave-one-out computed */
                 model->loo_error=100.0*loo_count/(double)totdoc;
                 model->loo_recall=(1.0-(double)loo_count_pos/(double)trainpos)*100.0;
                 model->loo_precision=(trainpos-loo_count_pos)/
                     (double)(trainpos-loo_count_pos+loo_count_neg)*100.0;
                 fprintf(stdout,"Leave-one-out estimate of the error: error=%.2f%%\n",
                     model->loo_error);
                 fprintf(stdout,"Leave-one-out estimate of the recall: recall=%.2f%%\n",
                     model->loo_recall);
                 fprintf(stdout,"Leave-one-out estimate of the precision: precision=%.2f%%\n",
                     model->loo_precision);
                 fprintf(stdout,"Actual leave-one-outs computed:  %ld (rho=%.2f)\n",
                     loocomputed,learn_parm->rho);
                 sprintf(temstr,"Runtime for leave-one-out in cpu-seconds: %.2f\n",
                     (double)(get_runtime()-runtime_start_loo)/100.0);
                 printm(temstr);
             }
             
             // if(learn_parm->alphafile[0])
             //     write_alphas(learn_parm->alphafile,a,label,totdoc);
             
             shrink_state_cleanup(&shrink_state);
             
             free(inconsistent);
             free(unlabeled);
             free(a);
             free(a_fullset);
             free(xi_fullset);
             free(lin);
             free(learn_parm->svm_cost);
}



long CSVM::optimize_to_convergence(
                             DOC *docs,
                             long *label,
                             long totdoc,
                             long totwords,           
                             LEARN_PARM *learn_parm,    
                             KERNEL_PARM *kernel_parm,  
                             KERNEL_CACHE *kernel_cache,
                             SHRINK_STATE *shrink_state,
                             MODEL *model,              
                             long *inconsistent,
                             long *unlabeled,
                             double *a,
                             double *lin,
                             TIMING *timing_profile,
                             double *maxdiff,
                             long heldout,
                             long retrain)
{
    long *chosen,*key,i,j,jj,*last_suboptimal_at,noshrink;
    long inconsistentnum,choosenum,already_chosen=0,iteration;
    long misclassified,supvecnum=0,*active2dnum,inactivenum;
    long *working2dnum,*selexam;
    long activenum;
    double criterion,eq;
    double *a_old;
    long t0=0,t1=0,t2=0,t3=0,t4=0,t5=0,t6=0; /* timing */
    long transductcycle;
    long transduction;
    double epsilon_crit_org; 
    
     double *selcrit;  /* buffer for sorting */        
    CFLOAT *aicache;  /* buffer to keep one row of hessian */
     double *weights;  /* buffer for weight vector in linear case */
    QP qp;            /* buffer for one quadratic program */
    
    epsilon_crit_org=learn_parm->epsilon_crit; /* save org */
    if(kernel_parm->kernel_type == LINEAR)
    {
        learn_parm->epsilon_crit=2.0;
        kernel_cache=NULL;   /* caching makes no sense for linear kernel */
    } 
    learn_parm->epsilon_shrink=2;
    (*maxdiff)=1;

    learn_parm->totwords=totwords;
    
    chosen = (long *)my_malloc(sizeof(long)*totdoc);
    last_suboptimal_at = (long *)my_malloc(sizeof(long)*totdoc);
    key = (long *)my_malloc(sizeof(long)*(totdoc+11)); 
    selcrit = (double *)my_malloc(sizeof(double)*totdoc);
    selexam = (long *)my_malloc(sizeof(long)*totdoc);
    a_old = (double *)my_malloc(sizeof(double)*totdoc);
    aicache = (CFLOAT *)my_malloc(sizeof(CFLOAT)*totdoc);
    working2dnum = (long *)my_malloc(sizeof(long)*(totdoc+11));
    active2dnum = (long *)my_malloc(sizeof(long)*(totdoc+11));
    qp.opt_ce = (double *)my_malloc(sizeof(double)*learn_parm->svm_maxqpsize);
    qp.opt_ce0 = (double *)my_malloc(sizeof(double));
    qp.opt_g = (double *)my_malloc(sizeof(double)*learn_parm->svm_maxqpsize
        *learn_parm->svm_maxqpsize);
    qp.opt_g0 = (double *)my_malloc(sizeof(double)*learn_parm->svm_maxqpsize);
    qp.opt_xinit = (double *)my_malloc(sizeof(double)*learn_parm->svm_maxqpsize);
    qp.opt_low=(double *)my_malloc(sizeof(double)*learn_parm->svm_maxqpsize);
    qp.opt_up=(double *)my_malloc(sizeof(double)*learn_parm->svm_maxqpsize);
    weights=(double *)my_malloc(sizeof(double)*(totwords+1));
    
    choosenum=0;
    inconsistentnum=0;
    transductcycle=0;
    transduction=0;
    if(!retrain) retrain=1;
    iteration=1;
    
    if(kernel_cache)
    {
        kernel_cache->time=iteration;  /* for lru cache */
        kernel_cache_reset_lru(kernel_cache);
    }
    

    for(i=0;i<totdoc;i++)
    {    /* various inits */
        chosen[i]=0;
        a_old[i]=a[i];
        last_suboptimal_at[i]=1;
        if(inconsistent[i]) 
            inconsistentnum++;
        if(unlabeled[i]) 
        {
            transduction=1;
        }
    }
    activenum=compute_index(shrink_state->active,totdoc,active2dnum);
    inactivenum=totdoc-activenum;
    clear_index(working2dnum);


    /* repeat this loop until we have convergence */
    for(;retrain;iteration++) 
    {
        
        if(kernel_cache)
            kernel_cache->time=iteration;  /* for lru cache */
        i=0;
        for(jj=0;(j=working2dnum[jj])>=0;jj++)
        { /* clear working set */
            if((chosen[j]>=(learn_parm->svm_maxqpsize/
                minl(learn_parm->svm_maxqpsize,
                learn_parm->svm_newvarsinqp))) 
                || (inconsistent[j])
                || (j == heldout)) 
            {
                chosen[j]=0; 
                choosenum--; 
            }
            else 
            {
                chosen[j]++;
                working2dnum[i++]=j;
            }
        }
        working2dnum[i]=-1;
        
        if(retrain == 2)
        {
            choosenum=0;
            for(jj=0;(j=working2dnum[jj])>=0;jj++) 
            { /* fully clear working set */
                chosen[j]=0; 
            }
            clear_index(working2dnum);
            for(i=0;i<totdoc;i++) 
            { /* set inconsistent examples to zero (-i 1) */
                if((inconsistent[i] || (heldout==i)) && (a[i] != 0.0)) 
                {
                    chosen[i]=99999;
                    choosenum++;
                    a[i]=0;
                }
            }
            if(learn_parm->biased_hyperplane)
            {
                eq=0;
                for(i=0;i<totdoc;i++) 
                { /* make sure we fulfill equality constraint */
                    eq+=a[i]*label[i];
                }
                for(i=0;(i<totdoc) && (fabs(eq) > learn_parm->epsilon_a);i++) 
                {
                    if((eq*label[i] > 0) && (a[i] > 0)) 
                    {
                        chosen[i]=88888;
                        choosenum++;
                        if((eq*label[i]) > a[i]) 
                        {
                            eq-=(a[i]*label[i]);
                            a[i]=0;
                        }
                        else 
                        {
                            a[i]-=(eq*label[i]);
                            eq=0;
                        }
                    }
                }
            }
            compute_index(chosen,totdoc,working2dnum);
        }
        else
        {      /* select working set according to steepest gradient */ 
            if((minl(learn_parm->svm_newvarsinqp,learn_parm->svm_maxqpsize)>=4) 
                && (kernel_parm->kernel_type != LINEAR)) 
            {
                /* select part of the working set from cache */
                already_chosen=select_next_qp_subproblem_grad_cache(
                    label,unlabeled,a,lin,totdoc,
                    minl((long)(learn_parm->svm_maxqpsize-choosenum),
                    (long)(learn_parm->svm_newvarsinqp/2)),
                    learn_parm,inconsistent,active2dnum,
                    working2dnum,selcrit,selexam,kernel_cache,
                    key,chosen);
                choosenum+=already_chosen;
            }
            choosenum+=select_next_qp_subproblem_grad(label,unlabeled,a,lin,totdoc,
                minl((long)(learn_parm->svm_maxqpsize-choosenum),
                (long)(learn_parm->svm_newvarsinqp-already_chosen)),
                learn_parm,inconsistent,active2dnum,
                working2dnum,selcrit,selexam,kernel_cache,key,
                chosen);
        }
        
        //      sprintf(temstr," %ld vectors chosen\n",choosenum);  printm(temstr);
  
 
        t1=get_runtime();
        
        if(kernel_cache) 
            cache_multiple_kernel_rows(kernel_cache,docs,working2dnum,choosenum,kernel_parm); 
        
        
        t2=get_runtime();
        if(retrain != 2)
        {
            optimize_svm(docs,label,unlabeled,chosen,active2dnum,model,totdoc,
                working2dnum,choosenum,a,lin,learn_parm,aicache,
                kernel_parm,&qp,&epsilon_crit_org);
        }
        
        
        t3=get_runtime();
        update_linear_component(docs,label,active2dnum,a,a_old,working2dnum,totdoc,
            totwords,kernel_parm,kernel_cache,lin,aicache,weights);
        
        
        
        t4=get_runtime();
        supvecnum=calculate_svm_model(docs,label,unlabeled,lin,a,a_old,learn_parm,
            working2dnum,model);
        
        
        t5=get_runtime();
        
        /* The following computation of the objective function works only */
        /* relative to the active variables */
        
        criterion=compute_objective_function(a,lin,label,active2dnum);
        //  sprintf(temstr,"Objective function (over active variables): %.16f\n",criterion);
        //  printm(temstr);
        
        for(jj=0;(i=working2dnum[jj])>=0;jj++)
        {
            a_old[i]=a[i];
        }
        
        if(retrain == 2) 
        {  /* reset inconsistent unlabeled examples */
            for(i=0;(i<totdoc);i++) 
            {
                if(inconsistent[i] && unlabeled[i]) 
                {
                    inconsistent[i]=0;
                    label[i]=0;
                }
            }
        }
        
        retrain=check_optimality(model,label,unlabeled,a,lin,totdoc,learn_parm,
            maxdiff,epsilon_crit_org,&misclassified,
            inconsistent,active2dnum,last_suboptimal_at,
            iteration,kernel_parm);
        
        
        t6=get_runtime();
        timing_profile->time_select+=t1-t0;
        timing_profile->time_kernel+=t2-t1;
        timing_profile->time_opti+=t3-t2;
        timing_profile->time_update+=t4-t3;
        timing_profile->time_model+=t5-t4;
        timing_profile->time_check+=t6-t5;
        
        noshrink=0;
        if((!retrain) && (inactivenum>0) 
            && ((!learn_parm->skip_final_opt_check) 
            || (kernel_parm->kernel_type == LINEAR))) 
        {  
			if (com_pro.show_other)
			{
	            sprintf(temstr," Checking optimality of inactive variables..."); 
				printm(temstr);
			}
            
            t1=get_runtime();
            reactivate_inactive_examples(label,unlabeled,a,shrink_state,lin,totdoc,
                totwords,iteration,learn_parm,inconsistent,
                docs,kernel_parm,kernel_cache,model,aicache,
                weights,maxdiff);
            /* Update to new active variables. */
            activenum=compute_index(shrink_state->active,totdoc,active2dnum);
            inactivenum=totdoc-activenum;
            /* termination criterion */
            noshrink=1;
            retrain=0;
            if((*maxdiff) > learn_parm->epsilon_crit) 
                retrain=1;
            timing_profile->time_shrink+=get_runtime()-t1;
        }
        
        if((!retrain) && (learn_parm->epsilon_crit>(*maxdiff))) 
            learn_parm->epsilon_crit=(*maxdiff);
        if((!retrain) && (learn_parm->epsilon_crit>epsilon_crit_org)) 
        {  
            learn_parm->epsilon_crit/=2.0;
            retrain=1;
            noshrink=1;
        }
        if(learn_parm->epsilon_crit<epsilon_crit_org) 
            learn_parm->epsilon_crit=epsilon_crit_org;
        {
         ///   sprintf(temstr," => (%ld SV (incl. %ld SV at u-bound), max violation=%.5f)\n",
         //       supvecnum,model->at_upper_bound,(*maxdiff)); 
           // printm(temstr);
        }
        
        if((!retrain) && (transduction)) 
        { 
            for(i=0;(i<totdoc);i++)
            {
                shrink_state->active[i]=1;
            }
            activenum=compute_index(shrink_state->active,totdoc,active2dnum);
            inactivenum=0;
            
            retrain=incorporate_unlabeled_examples(model,label,inconsistent,
                unlabeled,a,lin,totdoc,
                selcrit,selexam,key,
                transductcycle,kernel_parm,
                learn_parm);
            epsilon_crit_org=learn_parm->epsilon_crit;
            if(kernel_parm->kernel_type == LINEAR)
                learn_parm->epsilon_crit=1; 
            transductcycle++;
        } 
        else if(((iteration % 10) == 0) && (!noshrink)) 
        {
            activenum=shrink_problem(learn_parm,shrink_state,active2dnum,iteration,last_suboptimal_at,
                totdoc,maxl((long)(activenum/10),100),
                a,inconsistent);
            inactivenum=totdoc-activenum;
            if((kernel_cache)
                && (supvecnum>kernel_cache->max_elems)
                && ((kernel_cache->activenum-activenum)>maxl((long)(activenum/10),500))) 
            {
                kernel_cache_shrink(kernel_cache,totdoc,maxl((long)(activenum/10),500),
                    shrink_state->active); 
            }
        }
        
        if((!retrain) && learn_parm->remove_inconsistent) 
        {
            
            sprintf(temstr," Moving training errors to inconsistent examples...");
            printm(temstr);
            
            if(learn_parm->remove_inconsistent == 1) 
            {
                retrain=identify_inconsistent(a,label,unlabeled,totdoc,learn_parm,
                    &inconsistentnum,inconsistent); 
            }
            else if(learn_parm->remove_inconsistent == 2) 
            {
                retrain=identify_misclassified(lin,label,unlabeled,totdoc,
                    model,&inconsistentnum,inconsistent); 
            }
            else if(learn_parm->remove_inconsistent == 3) 
            {
                retrain=identify_one_misclassified(lin,label,unlabeled,totdoc,
                    model,&inconsistentnum,inconsistent);
            }
            if(retrain) 
            {
                if(kernel_parm->kernel_type == LINEAR)
                { /* reinit shrinking */
                    learn_parm->epsilon_crit=2.0;
                } 
            }
            if(retrain)
            {
                sprintf(temstr," Now %ld inconsistent examples.\n",inconsistentnum);
                printm(temstr);
            }
        }
      	

   //sprintf(temstr,"( %d %d)",iteration,retrain);
	//printm(temstr);

  } /* end of loop */
        
 
  free(chosen);
  free(last_suboptimal_at);
  free(key);
  free(selcrit);
  free(selexam);
  free(a_old);
  free(aicache);
  free(working2dnum);
  free(active2dnum);
  free(qp.opt_ce);
  free(qp.opt_ce0);
  free(qp.opt_g);
  free(qp.opt_g0);
  free(qp.opt_xinit);
  free(qp.opt_low);
  free(qp.opt_up);
  free(weights);
  
  learn_parm->epsilon_crit=epsilon_crit_org; /* restore org */
  
  return(iteration);
}


double CSVM::compute_objective_function( double *a,double *lin,   long *label,long*active2dnum   )
{
    long i,ii;
    double criterion;
    /* calculate value of objective function */
    criterion=0;
    for(ii=0;active2dnum[ii]>=0;ii++) 
    {
        i=active2dnum[ii];
        criterion=criterion-a[i]+0.5*a[i]*label[i]*lin[i];
    } 
    return(criterion);
}

void CSVM::clear_index(long *index)
/* initializes and empties index */
{
    index[0]=-1;
} 

void CSVM::add_to_index(long *index,long elem)
/* initializes and empties index */
{
    register long i;
    for(i=0;index[i] != -1;i++);
    index[i]=elem;
    index[i+1]=-1;
}

long CSVM::compute_index(long *binfeature,long range,long *index)
/* create an inverted index of binfeature */
{               
    register long i,ii;
    
    ii=0;
    for(i=0;i<range;i++) 
    {
        if(binfeature[i]) 
        {
            index[ii]=i;
            ii++;
        }
    }
    for(i=0;i<4;i++) 
    {
        index[ii+i]=-1;
    }
    return(ii);
}

void CSVM::optimize_svm(
                  DOC *docs,               /* Do optimization on the working set. */
                  long *label,long *unlabeled,long *chosen,long *active2dnum,
                  MODEL *model,
                  long totdoc,long *working2dnum,long varnum,
                  double *a,double *lin,
                  LEARN_PARM *learn_parm,
                  CFLOAT *aicache,
                  KERNEL_PARM *kernel_parm,
                  QP *qp,
                  double *epsilon_crit_target
                  )
{
    long i;
    double *a_v;
    
    compute_matrices_for_optimization(docs,label,unlabeled,chosen,active2dnum,
        working2dnum,model,a,lin,varnum,
        totdoc,learn_parm,aicache,kernel_parm,
        qp);
    
	if (com_pro.show_compute_3) 
	{
     sprintf(temstr,"Running optimizer..."); 
	 printm(temstr);
	}
    
    /* call the qp-subsolver */
    a_v=optimize_qp(qp,epsilon_crit_target, learn_parm->svm_maxqpsize,&(model->b),learn_parm);
    for(i=0;i<varnum;i++) 
    {
        a[working2dnum[i]]=a_v[i];
    }
}

void CSVM::compute_matrices_for_optimization(
                                       DOC *docs,
                                       long *label,long *unlabeled,long *chosen,long *active2dnum,long *key,
                                       MODEL *model,
                                       double *a,double *lin,
                                       long varnum,long totdoc,
                                       LEARN_PARM *learn_parm,
                                       CFLOAT *aicache,
                                       KERNEL_PARM *kernel_parm,
                                       QP *qp)
{
    register long ki,kj,i,j;
    register double kernel_temp;
	if (com_pro.show_compute_3) 
	{
     sprintf(temstr,"Computing qp-matrices (type %ld kernel [degree %ld, rbf_gamma %f, coef_lin %f, coef_const %f])...",kernel_parm->kernel_type,kernel_parm->poly_degree,kernel_parm->rbf_gamma,kernel_parm->coef_lin,kernel_parm->coef_const);
     printm(temstr);
	}
    
    
    qp->opt_n=varnum;
    qp->opt_ce0[0]=0; /* compute the constant for equality constraint */
    for(j=1;j<model->sv_num;j++) { /* start at 1 */
        if(!chosen[(model->supvec[j])->docnum])
        {
            qp->opt_ce0[0]+=model->alpha[j];
        }
    } 
    if(learn_parm->biased_hyperplane) 
        qp->opt_m=1;
    else 
        qp->opt_m=0;  /* eq-constraint will be ignored */
    
    /* init linear part of objective function */
    for(i=0;i<varnum;i++)
    {
        qp->opt_g0[i]=lin[key[i]];
    }
    
    for(i=0;i<varnum;i++)
    {
        ki=key[i];
        
        /* Compute the matrix for equality constraints */
        qp->opt_ce[i]=label[ki];
        qp->opt_low[i]=0;
        qp->opt_up[i]=learn_parm->svm_cost[ki];
        
        kernel_temp=(double)kernel(kernel_parm,&(docs[ki]),&(docs[ki])); 
        /* compute linear part of objective function */
        qp->opt_g0[i]-=(kernel_temp*a[ki]*(double)label[ki]); 
        /* compute quadratic part of objective function */
        qp->opt_g[varnum*i+i]=kernel_temp;
        for(j=i+1;j<varnum;j++) {
            kj=key[j];
            kernel_temp=(double)kernel(kernel_parm,&(docs[ki]),&(docs[kj]));
            /* compute linear part of objective function */
            qp->opt_g0[i]-=(kernel_temp*a[kj]*(double)label[kj]);
            qp->opt_g0[j]-=(kernel_temp*a[ki]*(double)label[ki]); 
            /* compute quadratic part of objective function */
            qp->opt_g[varnum*i+j]=(double)label[ki]*(double)label[kj]*kernel_temp;
            qp->opt_g[varnum*j+i]=(double)label[ki]*(double)label[kj]*kernel_temp;
        }
        
        
        if(i % 20 == 0&&com_pro.show_compute_2) 
        {
            sprintf(temstr,"%ld..",i); 
			printm(temstr);
            
        }
    }
    
    for(i=0;i<varnum;i++) 
    {
        /* assure starting at feasible point */
        qp->opt_xinit[i]=a[key[i]];
        /* set linear part of objective function */
        qp->opt_g0[i]=-1.0+qp->opt_g0[i]*(double)label[key[i]];    
    }
    
    
    
}

long CSVM::calculate_svm_model(
                         DOC *docs,              /* Compute decision function based on current values */
                         long *label,long *unlabeled, /* of alpha. */
                         double *lin,double *a,double *a_old,
                         LEARN_PARM *learn_parm,
                         long *working2dnum,
                         MODEL *model)
{
    long i,ii,pos,b_calculated=0;
    double ex_c;
    
    
    //    sprintf(temstr,"Calculating model...");  printm(temstr);
    
    
    if(!learn_parm->biased_hyperplane)
    {
        model->b=0;
        b_calculated=1;
    }
    
    for(ii=0;(i=working2dnum[ii])>=0;ii++) 
    {
        if((a_old[i]>0) && (a[i]==0)) { /* remove from model */
            pos=model->index[i]; 
            model->index[i]=-1;
            (model->sv_num)--;
            model->supvec[pos]=model->supvec[model->sv_num];
            model->alpha[pos]=model->alpha[model->sv_num];
            model->index[(model->supvec[pos])->docnum]=pos;
        }
        else if((a_old[i]==0) && (a[i]>0)) 
        { /* add to model */
            model->supvec[model->sv_num]=&(docs[i]);
            model->alpha[model->sv_num]=a[i]*(double)label[i];
            model->index[i]=model->sv_num;
            (model->sv_num)++;
        }
        else if(a_old[i]==a[i]) 
        { /* nothing to do */
        }
        else
        {  /* just update alpha */
            model->alpha[model->index[i]]=a[i]*(double)label[i];
        }
        
        ex_c=learn_parm->svm_cost[i]-learn_parm->epsilon_a;
        if((a_old[i]>=ex_c) && (a[i]<ex_c)) 
        { 
            (model->at_upper_bound)--;
        }
        else if((a_old[i]<ex_c) && (a[i]>=ex_c))
        { 
            (model->at_upper_bound)++;
        }
        
        if((!b_calculated) 
            && (a[i]>learn_parm->epsilon_a) && (a[i]<ex_c))
        {   /* calculate b */
            model->b=(-(double)label[i]+lin[i]);
            b_calculated=1;
        }
    }      
    
    
    
    
    /* If there is no alpha in the working set not at bounds, then
    just use the model->b from the last iteration or the one provided 
    by the core optimizer */
    
    return(model->sv_num-1); /* have to substract one, since element 0 is empty*/
}

long CSVM::check_optimality(
                      MODEL *model,            /* Check KT-conditions */
                      long *label,long *unlabeled,
                      double *a,double *lin,
                      long totdoc,
                      LEARN_PARM *learn_parm,
                      double *maxdiff,double epsilon_crit_org,
                      long *misclassified,
                      long *inconsistent,long *active2dnum,long *last_suboptimal_at,long iteration,
                      KERNEL_PARM *kernel_parm)
{
    long i,ii,retrain;
    double dist,ex_c;
    
    if(kernel_parm->kernel_type == LINEAR)
    {  /* be optimistic */
        learn_parm->epsilon_shrink=-learn_parm->epsilon_crit+epsilon_crit_org;  
    }
    else
    {  /* be conservative */
        learn_parm->epsilon_shrink=learn_parm->epsilon_shrink*0.7+(*maxdiff)*0.3; 
    }
    retrain=0;
    (*maxdiff)=0;
    (*misclassified)=0;
    for(ii=0;(i=active2dnum[ii])>=0;ii++) 
    {
        if((!inconsistent[i]) && label[i])
        {
            dist=(lin[i]-model->b)*(double)label[i];/* 'distance' from hyperplane*/
            ex_c=learn_parm->svm_cost[i]-learn_parm->epsilon_a;
            if(dist <= 0)
            {       
                (*misclassified)++;  /* does not work due to deactivation of var */
            }
            if((a[i]>learn_parm->epsilon_a) && (dist > 1)) 
            {
                if((dist-1.0)>(*maxdiff))  /* largest violation */
                    (*maxdiff)=dist-1.0;
            }
            else if((a[i]<ex_c) && (dist < 1))
            {
                if((1.0-dist)>(*maxdiff))  /* largest violation */
                    (*maxdiff)=1.0-dist;
            }
            /* Count how long a variable was at lower/upper bound (and optimal).*/
            /* Variables, which were at the bound and optimal for a long */
            /* time are unlikely to become support vectors. In case our */
            /* cache is filled up, those variables are excluded to save */
            /* kernel evaluations. (See chapter 'Shrinking').*/ 
            if((a[i]>(learn_parm->epsilon_a)) && (a[i]<ex_c)) 
            { 
                last_suboptimal_at[i]=iteration;         /* not at bound */
            }
            else if((a[i]<=(learn_parm->epsilon_a)) 
                && (dist < (1.0+learn_parm->epsilon_shrink)))
            {
                last_suboptimal_at[i]=iteration;         /* not likely optimal */
            }
            else if((a[i]>=ex_c)
                && (dist > (1.0-learn_parm->epsilon_shrink)))  
            { 
                last_suboptimal_at[i]=iteration;         /* not likely optimal */
            }
        }   
    }
    /* termination criterion */
    if((!retrain) && ((*maxdiff) > learn_parm->epsilon_crit)) 
    {  
        retrain=1;
    }
    return(retrain);
}

long CSVM::identify_inconsistent(
                           double *a,
                           long *label,long *unlabeled,long totdoc,
                           LEARN_PARM *learn_parm,
                           long *inconsistentnum,long *inconsistent)
{
    long i,retrain;
    
    /* Throw out examples with multipliers at upper bound. This */
    /* corresponds to the -i 1 option. */
    /* ATTENTION: this is just a heuristic for finding a close */
    /*            to minimum number of examples to exclude to */
    /*            make the problem separable with desired margin */
    retrain=0;
    for(i=0;i<totdoc;i++)
    {
        if((!inconsistent[i]) && (!unlabeled[i]) 
            && (a[i]>=(learn_parm->svm_cost[i]-learn_parm->epsilon_a))) 
        { 
            (*inconsistentnum)++;
            inconsistent[i]=1;  /* never choose again */
            retrain=2;          /* start over */
            
            sprintf(temstr,"inconsistent(%ld)..",i);  printm(temstr);
        }
    }
    return(retrain);
}

long CSVM::identify_misclassified(
                            double *lin,
                            long *label,long *unlabeled,long totdoc,
                            MODEL *model,
                            long *inconsistentnum,long *inconsistent)
{
    long i,retrain;
    double dist;
    
    /* Throw out misclassified examples. This */
    /* corresponds to the -i 2 option. */
    /* ATTENTION: this is just a heuristic for finding a close */
    /*            to minimum number of examples to exclude to */
    /*            make the problem separable with desired margin */
    retrain=0;
    for(i=0;i<totdoc;i++) 
    {
        dist=(lin[i]-model->b)*(double)label[i]; /* 'distance' from hyperplane*/  
        if((!inconsistent[i]) && (!unlabeled[i]) && (dist <= 0)) 
        { 
            (*inconsistentnum)++;
            inconsistent[i]=1;  /* never choose again */
            retrain=2;          /* start over */
            
            sprintf(temstr,"inconsistent(%ld)..",i);  printm(temstr);
        }
    }
    return(retrain);
}

long CSVM::identify_one_misclassified(
                                double *lin,
                                long *label,long *unlabeled,long totdoc,
                                MODEL *model,
                                long *inconsistentnum,long *inconsistent)
{
    long i,retrain,maxex=-1;
    double dist,maxdist=0;
    
    /* Throw out the 'most misclassified' example. This */
    /* corresponds to the -i 3 option. */
    /* ATTENTION: this is just a heuristic for finding a close */
    /*            to minimum number of examples to exclude to */
    /*            make the problem separable with desired margin */
    retrain=0;
    for(i=0;i<totdoc;i++) 
    {
        if((!inconsistent[i]) && (!unlabeled[i]))
        {
            dist=(lin[i]-model->b)*(double)label[i];/* 'distance' from hyperplane*/  
            if(dist<maxdist)
            {
                maxdist=dist;
                maxex=i;
            }
        }
    }
    if(maxex>=0) 
    {
        (*inconsistentnum)++;
        inconsistent[maxex]=1;  /* never choose again */
        retrain=2;          /* start over */
        
        sprintf(temstr,"inconsistent(%ld)..",i);  printm(temstr);
        
    }
    return(retrain);
}

void CSVM::update_linear_component(
                             DOC *docs,
                             long *label,
                             long *active2dnum,                  /* keep track of the linear component */
                             double *a,double *a_old,                   /* lin of the gradient etc. by updating */
                             long *working2dnum,long totdoc,long totwords, /* based on the change of the variables */
                             KERNEL_PARM *kernel_parm,           /* in the current working set */
                             KERNEL_CACHE *kernel_cache,
                             double *lin,
                             CFLOAT *aicache,
                             double *weights)
{
    register long i,ii,j,jj;
    register double tec;
    
    if(kernel_parm->kernel_type==0) { /* special linear case */
        clear_vector_n(weights,totwords);
        for(ii=0;(i=working2dnum[ii])>=0;ii++) 
        {
            if(a[i] != a_old[i])
            {
                add_vector_ns(weights,docs[i].words,((a[i]-a_old[i])*(double)label[i]));
            }
        }
        for(jj=0;(j=active2dnum[jj])>=0;jj++)
        {
            lin[j]+=sprod_ns(weights,docs[j].words);
        }
    }
    else 
    {                            /* general case */
        for(jj=0;(i=working2dnum[jj])>=0;jj++) 
        {
            if(a[i] != a_old[i])
            {
                get_kernel_row(kernel_cache,docs,i,totdoc,active2dnum,aicache,
                    kernel_parm);
                for(ii=0;(j=active2dnum[ii])>=0;ii++)
                {
                    tec=aicache[j];
                    lin[j]+=(((a[i]*tec)-(a_old[i]*tec))*(double)label[i]);
                }
            }
        }
    }
}


long CSVM::incorporate_unlabeled_examples(
                                    MODEL *model,
                                    long *label,
                                    long *inconsistent,long *unlabeled,
                                    double *a,double *lin,
                                    long totdoc,
                                    double *selcrit,
                                    long *select,long *key,long transductcycle,
                                    KERNEL_PARM *kernel_parm,
                                    LEARN_PARM *learn_parm)
{
    long i,j,k,j1,j2,j3,j4,unsupaddnum1=0,unsupaddnum2=0;
    long pos,neg,upos,uneg,orgpos,orgneg,nolabel,newpos,newneg,allunlab;
    double dist,model_length,posratio,negratio;
    long check_every=2;
    double loss;
    static double switchsens=0.0,switchsensorg=0.0;
    double umin,umax,sumalpha;
    long imin=0,imax=0;
    static long switchnum=0;
    
    switchsens/=1.2;
    
    /* assumes that lin[] is up to date -> no inactive vars */
    
    orgpos=0;
    orgneg=0;
    newpos=0;
    newneg=0;
    nolabel=0;
    allunlab=0;
    for(i=0;i<totdoc;i++)
    {
        if(!unlabeled[i]) 
        {
            if(label[i] > 0) 
            {
                orgpos++;
            }
            else 
            {
                orgneg++;
            }
        }
        else 
        {
            allunlab++;
            if(unlabeled[i]) 
            {
                if(label[i] > 0)
                {
                    newpos++;
                }
                else if(label[i] < 0) 
                {
                    newneg++;
                }
            }
        }
        if(label[i]==0)
        {
            nolabel++;
        }
    }
    
    if(learn_parm->transduction_posratio >= 0) 
    {
        posratio=learn_parm->transduction_posratio;
    }
    else 
    {
        posratio=(double)orgpos/(double)(orgpos+orgneg); /* use ratio of pos/neg */
    }                                                  /* in training data */
    negratio=1.0-posratio;
    
    learn_parm->svm_costratio=1.0;                     /* global */
    if(posratio>0)
    {
        learn_parm->svm_costratio_unlab=negratio/posratio;
    }
    else 
    {
        learn_parm->svm_costratio_unlab=1.0;
    }
    
    pos=0;
    neg=0;
    upos=0;
    uneg=0;
    for(i=0;i<totdoc;i++)
    {
        dist=(lin[i]-model->b);  /* 'distance' from hyperplane*/
        if(dist>0)
        {
            pos++;
        }
        else
        {
            neg++;
        }
        if(unlabeled[i]) 
        {
            if(dist>0)
            {
                upos++;
            }
            else
            {
                uneg++;
            }
        }
        if((!unlabeled[i]) && (a[i]>(learn_parm->svm_cost[i]-learn_parm->epsilon_a))) 
        {
            /*      printf("Ubounded %ld (class %ld, unlabeled %ld)\n",i,label[i],unlabeled[i]); */
        }
    }
    
    sprintf(temstr,"POS=%ld, ORGPOS=%ld, ORGNEG=%ld\n",pos,orgpos,orgneg);
    printm(temstr);
    sprintf(temstr,"POS=%ld, NEWPOS=%ld, NEWNEG=%ld\n",pos,newpos,newneg);
    printm(temstr);
    sprintf(temstr,"pos ratio = %f (%f).\n",(double)(upos)/(double)(allunlab),posratio);
    printm(temstr);
    
    
    if(transductcycle == 0) 
    {
        j1=0; 
        j2=0;
        j4=0;
        for(i=0;i<totdoc;i++) 
        {
            dist=(lin[i]-model->b);  /* 'distance' from hyperplane*/
            if((label[i]==0) && (unlabeled[i]))
            {
                selcrit[j4]=dist;
                key[j4]=i;
                j4++;
            }
        }
        unsupaddnum1=0; 
        unsupaddnum2=0; 
        select_top_n(selcrit,j4,select,(long)(allunlab*posratio+0.5));
        for(k=0;(k<(long)(allunlab*posratio+0.5));k++) 
        {
            i=key[select[k]];
            label[i]=1;
            unsupaddnum1++; 
            j1++;
        }
        for(i=0;i<totdoc;i++)
        {
            if((label[i]==0) && (unlabeled[i])) 
            {
                label[i]=-1;
                j2++;
                unsupaddnum2++;
            }
        }
        for(i=0;i<totdoc;i++) 
        {  /* set upper bounds on vars */
            if(unlabeled[i])
            {
                if(label[i] == 1) 
                {
                    learn_parm->svm_cost[i]=learn_parm->svm_c*learn_parm->svm_costratio_unlab*learn_parm->svm_unlabbound;
                }
                else if(label[i] == -1) 
                {
                    learn_parm->svm_cost[i]=learn_parm->svm_c*learn_parm->svm_unlabbound;
                }
            }
        }
        
        sprintf(temstr,"costratio %lf, costratio_unlab %lf, unlabbound %lf\n",
            learn_parm->svm_costratio,learn_parm->svm_costratio_unlab,
            learn_parm->svm_unlabbound); 
        printm(temstr);
        sprintf(temstr,"Classifying unlabeled data as %ld POS / %ld NEG.\n",
            unsupaddnum1,unsupaddnum2); 
        printm(temstr);
        
        sprintf(temstr,"Retraining.");
        printm(temstr);
        sprintf(temstr,"\n");
        printm(temstr);
        
        return((long)3);
    }
    if((transductcycle % check_every) == 0) 
    {
        
        sprintf(temstr,"Retraining.");
        printm(temstr);
        sprintf(temstr,"\n");
        printm(temstr);
        j1=0;
        j2=0;
        unsupaddnum1=0;
        unsupaddnum2=0;
        for(i=0;i<totdoc;i++) 
        {
            if((unlabeled[i] == 2)) 
            {
                unlabeled[i]=1;
                label[i]=1;
                j1++;
                unsupaddnum1++;
            }
            else if((unlabeled[i] == 3))
            {
                unlabeled[i]=1;
                label[i]=-1;
                j2++;
                unsupaddnum2++;
            }
        }
        for(i=0;i<totdoc;i++)
        {  /* set upper bounds on vars */
            if(unlabeled[i]) 
            {
                if(label[i] == 1)
                {
                    learn_parm->svm_cost[i]=learn_parm->svm_c*
                        learn_parm->svm_costratio_unlab*learn_parm->svm_unlabbound;
                }
                else if(label[i] == -1) 
                {
                    learn_parm->svm_cost[i]=learn_parm->svm_c*
                        learn_parm->svm_unlabbound;
                }
            }
        }
        
        
        sprintf(temstr,"costratio %lf, costratio_unlab %lf, unlabbound %lf\n",
            learn_parm->svm_costratio,learn_parm->svm_costratio_unlab,
            learn_parm->svm_unlabbound); 
        printm(temstr);
        sprintf(temstr,"%ld positive -> Added %ld POS / %ld NEG unlabeled examples.\n",
            upos,unsupaddnum1,unsupaddnum2); 
        printm(temstr);
        
        if(learn_parm->svm_unlabbound == 1) 
        {
            learn_parm->epsilon_crit=0.001; /* do the last run right */
        }
        else 
        {
            learn_parm->epsilon_crit=0.01; /* otherwise, no need to be so picky */
        }
        
        return((long)3);
    }
    else if(((transductcycle % check_every) < check_every)) 
    { 
        model_length=0;
        sumalpha=0;
        loss=0;
        for(i=0;i<totdoc;i++)
        {
            model_length+=a[i]*label[i]*lin[i];
            sumalpha+=a[i];
            dist=(lin[i]-model->b);  /* 'distance' from hyperplane*/
            if((label[i]*dist)<(1.0-learn_parm->epsilon_crit))
            {
                loss+=(1.0-(label[i]*dist))*learn_parm->svm_cost[i]; 
            }
        }
        model_length=sqrt(model_length); 
        
        sprintf(temstr,"Model-length = %f (%f), loss = %f, objective = %f\n",
            model_length,sumalpha,loss,loss+0.5*model_length*model_length);
        printm(temstr);
        j1=0;
        j2=0;
        j3=0;
        j4=0;
        unsupaddnum1=0; 
        unsupaddnum2=0; 
        umin=99999;
        umax=-99999;
        j4=1;
        while(j4)
        {
            umin=99999;
            umax=-99999;
            for(i=0;(i<totdoc);i++)
            { 
                dist=(lin[i]-model->b);  
                if((label[i]>0) && (unlabeled[i]) && (!inconsistent[i]) 
                    && (dist<umin)) 
                {
                    umin=dist;
                    imin=i;
                }
                if((label[i]<0) && (unlabeled[i])  && (!inconsistent[i]) 
                    && (dist>umax))
                {
                    umax=dist;
                    imax=i;
                }
            }
            if((umin < (umax+switchsens-1E-4))) 
            {
                j1++;
                j2++;
                unsupaddnum1++; 
                unlabeled[imin]=3;
                inconsistent[imin]=1;
                unsupaddnum2++; 
                unlabeled[imax]=2;
                inconsistent[imax]=1;
            }
            else
                j4=0;
            j4=0;
        }
        for(j=0;(j<totdoc);j++) 
        {
            if(unlabeled[j] && (!inconsistent[j]))
            {
                if(label[j]>0) 
                {
                    unlabeled[j]=2;
                }
                else if(label[j]<0) 
                {
                    unlabeled[j]=3;
                }
                /* inconsistent[j]=1; */
                j3++;
            }
        }
        switchnum+=unsupaddnum1+unsupaddnum2;
        
        /* stop and print out current margin
        sprintf(temstr,"switchnum %ld %ld\n",switchnum,kernel_parm->poly_degree);
        if(switchnum == 2*kernel_parm->poly_degree) {
        learn_parm->svm_unlabbound=1;
        }
        */
        
        if((!unsupaddnum1) && (!unsupaddnum2)) 
        {
            if((learn_parm->svm_unlabbound>=1) && ((newpos+newneg) == allunlab)) 
            {
                for(j=0;(j<totdoc);j++) 
                {
                    inconsistent[j]=0;
                    if(unlabeled[j]) unlabeled[j]=1;
                }
                write_prediction(learn_parm->predfile,model,lin,a,unlabeled,label,
                    totdoc,learn_parm);  
                
                sprintf(temstr,"Number of switches: %ld\n",switchnum);
                return((long)0);
                switchsens=switchsensorg;
                learn_parm->svm_unlabbound*=1.5;
                if(learn_parm->svm_unlabbound>1) 
                {
                    learn_parm->svm_unlabbound=1;
                }
                model->at_upper_bound=0; /* since upper bound increased */
                
                sprintf(temstr,"Increasing influence of unlabeled examples to %f%% .",
                    learn_parm->svm_unlabbound*100.0);
                printm(temstr);
                
                learn_parm->epsilon_crit=0.5; /* don't need to be so picky */
                
                for(i=0;i<totdoc;i++) 
                {  /* set upper bounds on vars */
                    if(unlabeled[i]) 
                    {
                        if(label[i] == 1) 
                        {
                            learn_parm->svm_cost[i]=learn_parm->svm_c*
                                learn_parm->svm_costratio_unlab*learn_parm->svm_unlabbound;
                        }
                        else if(label[i] == -1) 
                        {
                            learn_parm->svm_cost[i]=learn_parm->svm_c*
                                learn_parm->svm_unlabbound;
                        }
                    }
                    return((long)2);
                }
        }     }
    }
    return((long)0); 
}

/*************************** Working set selection ***************************/

long CSVM::select_next_qp_subproblem_grad(
                                    long *label,long *unlabeled,
                                    double *a,double *lin,      /* Use the feasible direction approach to select the */
                                    long totdoc,long qp_size, /* next qp-subproblem  (see section 'Selecting a good */
                                    LEARN_PARM *learn_parm, /* working set') */
                                    long *inconsistent,long *active2dnum,long *working2dnum,
                                    double *selcrit,
                                    long *select,
                                    KERNEL_CACHE *kernel_cache,
                                    long *key,long *chosen)
{
    long choosenum,i,j,k,activedoc,inum;
    double s;
    
    for(inum=0;working2dnum[inum]>=0;inum++); /* find end of index */
    choosenum=0;
    activedoc=0;
    for(i=0;(j=active2dnum[i])>=0;i++)
    {
        s=-label[j];
        if((!((a[j]<=(0+learn_parm->epsilon_a)) && (s<0)))
            && (!((a[j]>=(learn_parm->svm_cost[j]-learn_parm->epsilon_a)) 
            && (s>0)))
            && (!inconsistent[j]) 
            && (label[j])
            && (!chosen[j]))
        {
            selcrit[activedoc]=lin[j]-(double)label[j];
            key[activedoc]=j;
            activedoc++;
        }
    }
    select_top_n(selcrit,activedoc,select,(long)(qp_size/2));
    for(k=0;(choosenum<(qp_size/2)) && (k<(qp_size/2)) && (k<activedoc);k++)
    {
        i=key[select[k]];
        chosen[i]=1;
        working2dnum[inum+choosenum]=i;
        choosenum+=1;
        kernel_cache_touch(kernel_cache,i); /* make sure it does not get kicked */
        /* out of cache */
    }
    
    activedoc=0;
    for(i=0;(j=active2dnum[i])>=0;i++)
    {
        s=label[j];
        if((!((a[j]<=(0+learn_parm->epsilon_a)) && (s<0)))
            && (!((a[j]>=(learn_parm->svm_cost[j]-learn_parm->epsilon_a)) 
            && (s>0))) 
            && (!inconsistent[j]) 
            && (label[j])
            && (!chosen[j]))
        {
            selcrit[activedoc]=(double)(label[j])-lin[j];
            key[activedoc]=j;
            activedoc++;
        }
    }
    select_top_n(selcrit,activedoc,select,(long)(qp_size/2));
    for(k=0;(choosenum<qp_size) && (k<(qp_size/2)) && (k<activedoc);k++)
    {
        i=key[select[k]];
        chosen[i]=1;
        working2dnum[inum+choosenum]=i;
        choosenum+=1;
        kernel_cache_touch(kernel_cache,i); /* make sure it does not get kicked */
        /* out of cache */
    } 
    working2dnum[inum+choosenum]=-1; /* complete index */
    return(choosenum);
}

long CSVM::select_next_qp_subproblem_grad_cache(
                                          long *label,long *unlabeled,
                                          double *a,double *lin,         /* Use the feasible direction approach to select the */
                                          long totdoc,long qp_size,    /* next qp-subproblem  (see chapter 'Selecting a  */
                                          LEARN_PARM *learn_parm, /* good working set') among the variable with */
                                          long *inconsistent,long *active2dnum,long *working2dnum, /* cached kernel */
                                          double *selcrit,
                                          long *select,
                                          KERNEL_CACHE *kernel_cache,
                                          long *key,long *chosen)
{
    long choosenum,i,j,k,activedoc,inum;
    double s;
    
    for(inum=0;working2dnum[inum]>=0;inum++); /* find end of index */
    choosenum=0;
    activedoc=0;
    for(i=0;(j=active2dnum[i])>=0;i++) 
    {
        s=-label[j];
        if((kernel_cache->index[j]>=0)
            && (!((a[j]<=(0+learn_parm->epsilon_a)) && (s<0)))
            && (!((a[j]>=(learn_parm->svm_cost[j]-learn_parm->epsilon_a)) 
            && (s>0)))
            && (!chosen[j]) 
            && (label[j])
            && (!inconsistent[j]))
        {
            selcrit[activedoc]=(double)label[j]*(-1.0+(double)label[j]*lin[j]);
            key[activedoc]=j;
            activedoc++;
        }
    }
    select_top_n(selcrit,activedoc,select,(long)(qp_size/2));
    for(k=0;(choosenum<(qp_size/2)) && (k<(qp_size/2)) && (k<activedoc);k++)
    {
        i=key[select[k]];
        chosen[i]=1;
        working2dnum[inum+choosenum]=i;
        choosenum+=1;
        kernel_cache_touch(kernel_cache,i); /* make sure it does not get kicked */
        /* out of cache */
    }
    
    activedoc=0;
    for(i=0;(j=active2dnum[i])>=0;i++) 
    {
        s=label[j];
        if((kernel_cache->index[j]>=0)
            && (!((a[j]<=(0+learn_parm->epsilon_a)) && (s<0)))
            && (!((a[j]>=(learn_parm->svm_cost[j]-learn_parm->epsilon_a)) 
            && (s>0))) 
            && (!chosen[j]) 
            && (label[j])
            && (!inconsistent[j])) 
        {
            selcrit[activedoc]=-(double)(label[j]*(-1.0+(double)label[j]*lin[j]));
            key[activedoc]=j;
            activedoc++;
        }
    }
    select_top_n(selcrit,activedoc,select,(long)(qp_size/2));
    for(k=0;(choosenum<qp_size) && (k<(qp_size/2)) && (k<activedoc);k++) 
    {
        i=key[select[k]];
        chosen[i]=1;
        working2dnum[inum+choosenum]=i;
        choosenum+=1;
        kernel_cache_touch(kernel_cache,i); /* make sure it does not get kicked */
        /* out of cache */
    } 
    working2dnum[inum+choosenum]=-1; /* complete index */
    return(choosenum);
}

void CSVM::select_top_n(
                  double *selcrit,
                  long range,long *select,long n)
{
    register long i,j;
    
    for(i=0;(i<n) && (i<range);i++)
    { /* Initialize with the first n elements */
        for(j=i;j>=0;j--) 
        {
            if((j>0) && (selcrit[select[j-1]]<selcrit[i]))
            {
                select[j]=select[j-1];
            }
            else 
            {
                select[j]=i;
                j=-1;
            }
        }
    }
    for(i=n;i<range;i++)
    {   
        if(selcrit[i]>selcrit[select[n-1]]) 
        {
            for(j=n-1;j>=0;j--)
            {
                if((j>0) && (selcrit[select[j-1]]<selcrit[i])) {
                    select[j]=select[j-1];
                }
                else
                {
                    select[j]=i;
                    j=-1;
                }
            }
        }
    }
}      


/******************************** Shrinking  *********************************/

void CSVM::init_shrink_state(SHRINK_STATE *shrink_state,
                       long totdoc,
                       long maxhistory)
{
    long i;
    
    shrink_state->deactnum=0;
    shrink_state->active = (long *)my_malloc(sizeof(long)*totdoc);
    shrink_state->inactive_since = (long *)my_malloc(sizeof(long)*totdoc);
    shrink_state->a_history = (double **)my_malloc(sizeof(double *)*10000);
    
    for(i=0;i<totdoc;i++)
    { 
        shrink_state->active[i]=1;
        shrink_state->inactive_since[i]=0;
    }
}

void CSVM::shrink_state_cleanup(SHRINK_STATE *shrink_state)
{
    free(shrink_state->active);
    free(shrink_state->inactive_since);
    if(shrink_state->deactnum > 0) 
        free(shrink_state->a_history[shrink_state->deactnum-1]);
    free(shrink_state->a_history);
}

long CSVM::shrink_problem(
                    /* shrink some variables away */
                    /* do the shrinking only if at least minshrink variables can be removed */
                    LEARN_PARM *learn_parm,
                    SHRINK_STATE *shrink_state,
                    long *active2dnum,long iteration,long* last_suboptimal_at,
                    long totdoc,long minshrink,
                    double *a,
                    long *inconsistent)
{
    long i,ii,change,activenum;
    double *a_old;
    
    activenum=0;
    change=0;
    for(ii=0;active2dnum[ii]>=0;ii++) 
    {
        i=active2dnum[ii];
        activenum++;
        if(((iteration-last_suboptimal_at[i])
            > learn_parm->svm_iter_to_shrink) 
            || (inconsistent[i]))
        {
            change++;
        }
    }
    if(change>=minshrink) { /* shrink only if sufficiently many candidates */
        /* Shrink problem by removing those variables which are */
        /* optimal at a bound for a minimum number of iterations */
        if (com_pro.show_other)
		{
        sprintf(temstr," Shrinking...");  printm(temstr);
		}
        a_old=(double *)my_malloc(sizeof(double)*totdoc);
        shrink_state->a_history[shrink_state->deactnum]=a_old;
        for(i=0;i<totdoc;i++)
        {
            a_old[i]=a[i];
        }
        change=0;
        for(ii=0;active2dnum[ii]>=0;ii++) 
        {
            i=active2dnum[ii];
            if((((iteration-last_suboptimal_at[i])
                >learn_parm->svm_iter_to_shrink) 
                || (inconsistent[i]))) 
            {
                shrink_state->active[i]=0;
                shrink_state->inactive_since[i]=shrink_state->deactnum;
                change++;
            }
        }
        activenum=compute_index(shrink_state->active,totdoc,active2dnum);
        shrink_state->deactnum++;
		if (com_pro.show_other)
		{
	        sprintf(temstr," Number of inactive variables = %ld\n",totdoc-activenum);
		    printm(temstr);
		}
        
    }
    return(activenum);
} 


void CSVM::reactivate_inactive_examples(
                                  long *label,long *unlabeled,              /* Make all variables active again */
                                  double *a,                      /* which had been removed by shrinking. */
                                  SHRINK_STATE *shrink_state,
                                  double *lin,
                                  long totdoc,long totwords,long iteration,      /* Computes lin for those */
                                  LEARN_PARM *learn_parm,              /* variables from scratch. */
                                  long *inconsistent,
                                  DOC *docs,
                                  KERNEL_PARM *kernel_parm,
                                  KERNEL_CACHE *kernel_cache,
                                  MODEL *model,
                                  CFLOAT *aicache,
                                  double *weights,
                                  double *maxdiff)
{
    register long i,j,ii,jj,t,*changed2dnum,*inactive2dnum;
    long *changed,*inactive;
    register double kernel_val,*a_old,dist;
    double ex_c;
    
    changed=(long *)my_malloc(sizeof(long)*totdoc);
    changed2dnum=(long *)my_malloc(sizeof(long)*(totdoc+11));
    inactive=(long *)my_malloc(sizeof(long)*totdoc);
    inactive2dnum=(long *)my_malloc(sizeof(long)*(totdoc+11));
    for(t=shrink_state->deactnum-1;(t>=0) && shrink_state->a_history[t];t--)
    {
		if (com_pro.show_other)
		{
        sprintf(temstr,"%ld..",t);  printm(temstr);
		}
        a_old=shrink_state->a_history[t];    
        for(i=0;i<totdoc;i++)
        {
            inactive[i]=((!shrink_state->active[i]) 
                && (shrink_state->inactive_since[i] == t));
            changed[i]= (a[i] != a_old[i]);
        }
        compute_index(inactive,totdoc,inactive2dnum);
        compute_index(changed,totdoc,changed2dnum);
        
        if(kernel_parm->kernel_type == LINEAR) 
        { /* special linear case */
            clear_vector_n(weights,totwords);
            for(ii=0;changed2dnum[ii]>=0;ii++) 
            {
                i=changed2dnum[ii];
                add_vector_ns(weights,docs[i].words,
                    ((a[i]-a_old[i])*(double)label[i]));
            }
            for(jj=0;(j=inactive2dnum[jj])>=0;jj++) {
                lin[j]+=sprod_ns(weights,docs[j].words);
            }
        }
        else 
        {
            for(ii=0;(i=changed2dnum[ii])>=0;ii++)
            {
                get_kernel_row(kernel_cache,docs,i,totdoc,inactive2dnum,aicache,
                    kernel_parm);
                for(jj=0;(j=inactive2dnum[jj])>=0;jj++)
                {
                    kernel_val=aicache[j];
                    lin[j]+=(((a[i]*kernel_val)-(a_old[i]*kernel_val))*(double)label[i]);
                }
            }
        }
    }
    (*maxdiff)=0;
    for(i=0;i<totdoc;i++) 
    {
        shrink_state->inactive_since[i]=shrink_state->deactnum-1;
        if(!inconsistent[i])
        {
            dist=(lin[i]-model->b)*(double)label[i];
            ex_c=learn_parm->svm_cost[i]-learn_parm->epsilon_a;
            if((a[i]>learn_parm->epsilon_a) && (dist > 1)) 
            {
                if((dist-1.0)>(*maxdiff))  /* largest violation */
                    (*maxdiff)=dist-1.0;
            }
            else if((a[i]<ex_c) && (dist < 1)) 
            {
                if((1.0-dist)>(*maxdiff))  /* largest violation */
                    (*maxdiff)=1.0-dist;
            }
            if((a[i]>(0+learn_parm->epsilon_a)) 
                && (a[i]<ex_c)) 
            { 
                shrink_state->active[i]=1;                         /* not at bound */
            }
            else if((a[i]<=(0+learn_parm->epsilon_a)) && (dist < (1+learn_parm->epsilon_shrink))) 
            {
                shrink_state->active[i]=1;
            }
            else if((a[i]>=ex_c)
                && (dist > (1-learn_parm->epsilon_shrink)))
            {
                shrink_state->active[i]=1;
            }
        }
    }
    for(i=0;i<totdoc;i++) 
    {
        (shrink_state->a_history[shrink_state->deactnum-1])[i]=a[i];
    }
    for(t=shrink_state->deactnum-2;(t>=0) && shrink_state->a_history[t];t--) 
    {
        free(shrink_state->a_history[t]);
        shrink_state->a_history[t]=0;
    }
    free(changed);
    free(changed2dnum);
    free(inactive);
    free(inactive2dnum);
								  
}
                                  


/****************************** Cache handling *******************************/

void CSVM::get_kernel_row(
					KERNEL_CACHE *kernel_cache,
					DOC *docs,            /* Get's a row of the matrix of kernel values */
					long docnum,long totdoc, /* This matrix has the same form as the Hessian, */ 
					long *active2dnum,    /* just that the elements are not multiplied by */
					CFLOAT *buffer,   /* y_i * y_j * a_i * a_j */
					KERNEL_PARM *kernel_parm) /* Takes the values from the cache if available. */
{
	register long i,j,start;
	DOC *ex;
	
	ex=&(docs[docnum]);
	if(kernel_cache->index[docnum] != -1)
	{ /* is cached? */
		kernel_cache->lru[kernel_cache->index[docnum]]=kernel_cache->time; /* lru */
		start=kernel_cache->activenum*kernel_cache->index[docnum];
		for(i=0;(j=active2dnum[i])>=0;i++)
		{
			if(kernel_cache->totdoc2active[j] >= 0) 
			{
				buffer[j]=kernel_cache->buffer[start+kernel_cache->totdoc2active[j]];
			}
			else 
			{
				buffer[j]=(CFLOAT)kernel(kernel_parm,ex,&(docs[j]));
			}
		}
	}
	else {
		for(i=0;(j=active2dnum[i])>=0;i++) 
		{
			buffer[j]=(CFLOAT)kernel(kernel_parm,ex,&(docs[j]));
		}
	}
}


void CSVM::cache_kernel_row(
					  KERNEL_CACHE *kernel_cache,
					  DOC *docs,  /* Fills cache for the row m */
					  long m,
					  KERNEL_PARM *kernel_parm)
{
	register DOC *ex;
	register long j,k,l;
	register CFLOAT *cache;
	
	if(!kernel_cache_check(kernel_cache,m))
	{  /* not cached yet*/
		cache = kernel_cache_clean_and_malloc(kernel_cache,m);
		if(cache)
		{
			l=kernel_cache->totdoc2active[m];
			ex=&(docs[m]);
			for(j=0;j<kernel_cache->activenum;j++)
			{  /* fill cache */
				k=kernel_cache->active2totdoc[j];
				if((kernel_cache->index[k] != -1) && (l != -1) && (k != m))
				{
					cache[j]=kernel_cache->buffer[kernel_cache->activenum
						*kernel_cache->index[k]+l];
				}
				else 
				{
					cache[j]=kernel(kernel_parm,ex,&(docs[k]));
				} 
			}
		}
		else 
		{
			printe("Error: Kernel cache full! => increase cache size");
		}
	}
}


void CSVM::cache_multiple_kernel_rows(
								KERNEL_CACHE *kernel_cache,
								DOC *docs,    /* Fills cache for the rows in key */
								long *key,
								long varnum,
								KERNEL_PARM *kernel_parm)
{
	register long i;
	
	for(i=0;i<varnum;i++) 
	{  /* fill up kernel cache */
		cache_kernel_row(kernel_cache,docs,key[i],kernel_parm);
	}
}


void CSVM::kernel_cache_shrink(
						 KERNEL_CACHE *kernel_cache,
						 long totdoc,long numshrink,long *after) /* remove numshrink columns in the cache */
{                         /* which correspond to examples marked  */
	register long i,j,jj,from=0,to=0,scount;   /* 0 in after. */
	long *keep;
	
	
	sprintf(temstr," Reorganizing cache...");  printm(temstr);
	
	
	keep=(long *)my_malloc(sizeof(long)*totdoc);
	for(j=0;j<totdoc;j++)
	{
		keep[j]=1;
	}
	scount=0;
	for(jj=0;(jj<kernel_cache->activenum) && (scount<numshrink);jj++) 
	{
		j=kernel_cache->active2totdoc[jj];
		if(!after[j]) 
		{
			scount++;
			keep[j]=0;
		}
	}
	
	for(i=0;i<kernel_cache->max_elems;i++)
	{
		for(jj=0;jj<kernel_cache->activenum;jj++)
		{
			j=kernel_cache->active2totdoc[jj];
			if(!keep[j]) 
			{
				from++;
			}
			else 
			{
				kernel_cache->buffer[to]=kernel_cache->buffer[from];
				to++;
				from++;
			}
		}
	}
	
	kernel_cache->activenum=0;
	for(j=0;j<totdoc;j++) 
	{
		if((keep[j]) && (kernel_cache->totdoc2active[j] != -1)) 
		{
			kernel_cache->active2totdoc[kernel_cache->activenum]=j;
			kernel_cache->totdoc2active[j]=kernel_cache->activenum;
			kernel_cache->activenum++;
		}
		else
		{
			kernel_cache->totdoc2active[j]=-1;
		}
	}
	
	kernel_cache->max_elems=(long)(kernel_cache->buffsize/kernel_cache->activenum);
	if(kernel_cache->max_elems>totdoc)
	{
		kernel_cache->max_elems=totdoc;
	}
	
	free(keep);
	
	if (com_pro.show_action)
	{
	sprintf(temstr,"done.\n");    printm(temstr);
	}
	if (com_pro.show_compute_2)
	{
		sprintf(temstr," Cache-size in rows = %ld",kernel_cache->max_elems);
		printm(temstr);
	}
	
}

void CSVM::kernel_cache_init(KERNEL_CACHE *kernel_cache,
					   long totdoc,long buffsize)
{
	long i;
	
	kernel_cache->index = (long *)my_malloc(sizeof(long)*totdoc);
	kernel_cache->occu = (long *)my_malloc(sizeof(long)*totdoc);
	kernel_cache->lru = (long *)my_malloc(sizeof(long)*totdoc);
	kernel_cache->invindex = (long *)my_malloc(sizeof(long)*totdoc);
	kernel_cache->active2totdoc = (long *)my_malloc(sizeof(long)*totdoc);
	kernel_cache->totdoc2active = (long *)my_malloc(sizeof(long)*totdoc);
	kernel_cache->buffer = (CFLOAT *)my_malloc(buffsize*1024*1024);
	
	kernel_cache->buffsize=(long)(buffsize*1024*1024/sizeof(CFLOAT));
	
	kernel_cache->max_elems=(long)(kernel_cache->buffsize/totdoc);
	if(kernel_cache->max_elems>totdoc) 
	{
		kernel_cache->max_elems=totdoc;
	}
	
	if (com_pro.show_compute_2)
	{
	sprintf(temstr," Cache-size in rows = %ld\n",kernel_cache->max_elems);
	printm(temstr);
	}
	
	
	kernel_cache->elems=0;     /* initialize cache */
	for(i=0;i<totdoc;i++) 
	{
		kernel_cache->index[i]=-1;
		kernel_cache->lru[i]=0;
	}
	for(i=0;i<kernel_cache->max_elems;i++) 
	{
		kernel_cache->occu[i]=0;
		kernel_cache->invindex[i]=-1;
	}
	
	kernel_cache->activenum=totdoc;;
	for(i=0;i<totdoc;i++)
	{
		kernel_cache->active2totdoc[i]=i;
		kernel_cache->totdoc2active[i]=i;
	}
	
	kernel_cache->time=0;  
} 

void CSVM::kernel_cache_reset_lru(KERNEL_CACHE *kernel_cache)
{
	long maxlru=0,k;
	
	for(k=0;k<kernel_cache->max_elems;k++) 
	{
		if(maxlru < kernel_cache->lru[k]) 
			maxlru=kernel_cache->lru[k];
	}
	for(k=0;k<kernel_cache->max_elems;k++) 
	{
		kernel_cache->lru[k]-=maxlru;
	}
}

void CSVM::kernel_cache_cleanup(KERNEL_CACHE *kernel_cache)
{
	free(kernel_cache->index);
	free(kernel_cache->occu);
	free(kernel_cache->lru);
	free(kernel_cache->invindex);
	free(kernel_cache->active2totdoc);
	free(kernel_cache->totdoc2active);
	free(kernel_cache->buffer);
}

long CSVM::kernel_cache_malloc(KERNEL_CACHE *kernel_cache)
{
	long i;
	
	if(kernel_cache->elems < kernel_cache->max_elems)
	{
		for(i=0;i<kernel_cache->max_elems;i++) 
		{
			if(!kernel_cache->occu[i])
			{
				kernel_cache->occu[i]=1;
				kernel_cache->elems++;
				return(i);
			}
		}
	}
	return(-1);
}

void CSVM::kernel_cache_free(KERNEL_CACHE *kernel_cache,long i)
{
	kernel_cache->occu[i]=0;
	kernel_cache->elems--;
}

long CSVM::kernel_cache_free_lru(KERNEL_CACHE *kernel_cache) /* remove least recently used cache */
/* element */
{                                   
	register long k,least_elem=-1,least_time;
	
	least_time=kernel_cache->time+1;
	for(k=0;k<kernel_cache->max_elems;k++)
	{
		if(kernel_cache->invindex[k] != -1) 
		{
			if(kernel_cache->lru[k]<least_time)
			{
				least_time=kernel_cache->lru[k];
				least_elem=k;
			}
		}
	}
	if(least_elem != -1) 
	{
		kernel_cache_free(kernel_cache,least_elem);
		kernel_cache->index[kernel_cache->invindex[least_elem]]=-1;
		kernel_cache->invindex[least_elem]=-1;
		return(1);
	}
	return(0);
}


CFLOAT *CSVM::kernel_cache_clean_and_malloc(
                                      KERNEL_CACHE *kernel_cache, 
                                      long docnum)  /* Get a free cache entry. In case cache is full, the lru */
{           /* element is removed. */
	long result;
	if((result = kernel_cache_malloc(kernel_cache)) == -1) 
	{
		if(kernel_cache_free_lru(kernel_cache))
		{
			result = kernel_cache_malloc(kernel_cache);
		}
	}
	kernel_cache->index[docnum]=result;
	if(result == -1) 
	{
		return(0);
	}
	kernel_cache->invindex[result]=docnum;
	kernel_cache->lru[kernel_cache->index[docnum]]=kernel_cache->time; /* lru */
	return((CFLOAT *)((long)kernel_cache->buffer
		+(kernel_cache->activenum*sizeof(CFLOAT)*
		kernel_cache->index[docnum])));
}

long CSVM::kernel_cache_touch(
						KERNEL_CACHE *kernel_cache,
						long docnum) /* Update lru time to avoid removal from cache. */
{
	if(kernel_cache && kernel_cache->index[docnum] != -1) {
		kernel_cache->lru[kernel_cache->index[docnum]]=kernel_cache->time; /* lru */
		return(1);
	}
	return(0);
}

long CSVM::kernel_cache_check(KERNEL_CACHE *kernel_cache, 
						long docnum) /* Is that row cached? */
{
	return(kernel_cache->index[docnum] != -1);
}

/************************** Compute estimates ******************************/

void CSVM::compute_xa_estimates(
						  MODEL *model,                           /* xa-estimate of error rate, */
						  long *label,long *unlabeled,long totdoc,          /* recall, and precision      */
						  DOC *docs,       
						  double *lin,double *a,                      
						  KERNEL_PARM *kernel_parm,
						  LEARN_PARM *learn_parm,
						  double *error,double *recall,double *precision)
{
	long i,looerror,looposerror,loonegerror;
	long totex,totposex;
	double xi,r_delta,r_delta_sq,sim=0;
	long *sv2dnum=NULL,*sv=NULL,svnum;
	
	r_delta=estimate_r_delta(docs,totdoc,kernel_parm); 
	r_delta_sq=r_delta*r_delta;
	
	looerror=0;
	looposerror=0;
	loonegerror=0;
	totex=0;
	totposex=0;
	svnum=0;
	
	if(learn_parm->xa_depth > 0)
	{
		sv = (long *)my_malloc(sizeof(long)*(totdoc+11));
		for(i=0;i<totdoc;i++) 
			sv[i]=0;
		for(i=1;i<model->sv_num;i++) 
			if(a[model->supvec[i]->docnum] 
				< (learn_parm->svm_cost[model->supvec[i]->docnum]
				-learn_parm->epsilon_a)) 
			{
				sv[model->supvec[i]->docnum]=1;
				svnum++;
			}
			sv2dnum = (long *)my_malloc(sizeof(long)*(totdoc+11));
			clear_index(sv2dnum);
			compute_index(sv,totdoc,sv2dnum);
	}
	
	for(i=0;i<totdoc;i++) 
	{
		if(unlabeled[i]) 
		{
			/* ignore it */
		}
		else
		{
			xi=1.0-((lin[i]-model->b)*(double)label[i]);
			if(xi<0) xi=0;
			if(label[i]>0)
			{
				totposex++;
			}
			if((learn_parm->rho*a[i]*r_delta_sq+xi) >= 1.0)
			{
				if(learn_parm->xa_depth > 0)
				{ /* makes assumptions */
					sim=distribute_alpha_t_greedily(sv2dnum,svnum,docs,a,i,label,
						kernel_parm,learn_parm,
						(double)((1.0-xi-a[i]*r_delta_sq)/(2.0*a[i])));
				}
				if((learn_parm->xa_depth == 0) ||     ((a[i]*docs[i].twonorm_sq+a[i]*2.0*sim+xi) >= 1.0))
				{ 
					looerror++;
					if(label[i]>0)
					{
						looposerror++;
					}
					else 
					{
						loonegerror++;
					}
				}
			}
			totex++;
		}
	}
	
	(*error)=((double)looerror/(double)totex)*100.0;
	(*recall)=(1.0-(double)looposerror/(double)totposex)*100.0;
	(*precision)=(((double)totposex-(double)looposerror)
		/((double)totposex-(double)looposerror+(double)loonegerror))*100.0;
	
	free(sv);
	free(sv2dnum);
}


double CSVM::distribute_alpha_t_greedily(
								   long *sv2dnum,long svnum, 
								   DOC *docs,        
								   double *a,
								   long docnum,long *label,
								   KERNEL_PARM *kernel_parm,
								   LEARN_PARM *learn_parm,
								   double thresh)
{
	long best_depth=0;
	long i,j,k,d,skip,allskip;
	double best,best_val[101],val,init_val_sq,init_val_lin;
	long best_ex[101];
	CFLOAT *cache,*trow;
	
	cache=(CFLOAT *)my_malloc(sizeof(CFLOAT)*learn_parm->xa_depth*svnum);
	trow = (CFLOAT *)my_malloc(sizeof(CFLOAT)*svnum);
	
	for(k=0;k<svnum;k++)
	{
		trow[k]=kernel(kernel_parm,&(docs[docnum]),&(docs[sv2dnum[k]]));
	}
	
	init_val_sq=0;
	init_val_lin=0;
	best=0;
	
	for(d=0;d<learn_parm->xa_depth;d++) 
	{
		allskip=1;
		if(d>=1)
		{
			init_val_sq+=cache[best_ex[d-1]+svnum*(d-1)]; 
			for(k=0;k<d-1;k++)
			{
				init_val_sq+=2.0*cache[best_ex[k]+svnum*(d-1)]; 
			}
			init_val_lin+=trow[best_ex[d-1]]; 
		}
		for(i=0;i<svnum;i++)
		{
			skip=0;
			if(sv2dnum[i] == docnum) skip=1;
			for(j=0;j<d;j++) 
			{
				if(i == best_ex[j]) skip=1;
			}
			
			if(!skip)
			{
				val=init_val_sq;
				val+=docs[sv2dnum[i]].twonorm_sq;
				for(j=0;j<d;j++) 
				{
					val+=2.0*cache[i+j*svnum];
				}
				val*=(1.0/(2.0*(d+1.0)*(d+1.0)));
				val-=((init_val_lin+trow[i])/(d+1.0));
				
				if(allskip || (val < best_val[d]))
				{
					best_val[d]=val;
					best_ex[d]=i;
				}
				allskip=0;
				if(val < thresh)
				{
					i=svnum;
					/*      sprintf(temstr,"EARLY"); */
				}
			}
		}
		if(!allskip)
		{
			for(k=0;k<svnum;k++)
			{
				cache[d*svnum+k]=kernel(kernel_parm,
					&(docs[sv2dnum[best_ex[d]]]),
					&(docs[sv2dnum[k]]));
			}
		}
		if((!allskip) && ((best_val[d] < best) || (d == 0))) 
		{
			best=best_val[d];
			best_depth=d;
		}
		if(allskip || (best < thresh)) 
		{
			d=learn_parm->xa_depth;
		}
	}  
	
	free(cache);
	free(trow);
	
	/*    sprintf(temstr,"Distribute[%ld](%ld)=%f, ",docnum,best_depth,best); */
	return(best);
}


void CSVM::estimate_transduction_quality(
								   MODEL *model,                        /* loo-bound based on observation */
								   long *label,long *unlabeled,long totdoc,       /* that loo-errors must have an   */
								   DOC *docs,
								   double *lin)
{
	long i,j,l=0,ulab=0,lab=0,labpos=0,labneg=0,ulabpos=0,ulabneg=0,totulab=0;
	double totlab=0,totlabpos=0,totlabneg=0,labsum=0,ulabsum=0;
	double r_delta,r_delta_sq,xi,xisum=0,asum=0;
	
	r_delta=estimate_r_delta(docs,totdoc,&(model->kernel_parm)); 
	r_delta_sq=r_delta*r_delta;
	
	for(j=0;j<totdoc;j++)
	{
		if(unlabeled[j]) 
		{
			totulab++;
		}
		else 
		{
			totlab++;
			if(label[j] > 0) 
				totlabpos++;
			else 
				totlabneg++;
		}
	}
	for(j=1;j<model->sv_num;j++) 
	{
		i=model->supvec[j]->docnum;
		xi=1.0-((lin[i]-model->b)*(double)label[i]);
		if(xi<0) xi=0;
		
		xisum+=xi;
		asum+=fabs(model->alpha[j]);
		if(unlabeled[i]) 
		{
			ulabsum+=(fabs(model->alpha[j])*r_delta_sq+xi);
		}
		else 
		{
			labsum+=(fabs(model->alpha[j])*r_delta_sq+xi);
		}
		if((fabs(model->alpha[j])*r_delta_sq+xi) >= 1)
		{ 
			l++;
			if(unlabeled[model->supvec[j]->docnum]) 
			{
				ulab++;
				if(model->alpha[j] > 0) 
					ulabpos++;
				else 
					ulabneg++;
			}
			else
			{
				lab++;
				if(model->alpha[j] > 0) 
					labpos++;
				else 
					labneg++;
			}
		}
	}
	sprintf(temstr,"xacrit>=1: labeledpos=%.5f labeledneg=%.5f default=%.5f\n",(double)labpos/(double)totlab*100.0,(double)labneg/(double)totlab*100.0,(double)totlabpos/(double)(totlab)*100.0);
	printm(temstr);
	sprintf(temstr,"xacrit>=1: unlabelpos=%.5f unlabelneg=%.5f\n",(double)ulabpos/(double)totulab*100.0,(double)ulabneg/(double)totulab*100.0);
	printm(temstr);
	sprintf(temstr,"xacrit>=1: labeled=%.5f unlabled=%.5f all=%.5f\n",(double)lab/(double)totlab*100.0,(double)ulab/(double)totulab*100.0,(double)l/(double)(totdoc)*100.0);
	printm(temstr);
	sprintf(temstr,"xacritsum: labeled=%.5f unlabled=%.5f all=%.5f\n",(double)labsum/(double)totlab*100.0,(double)ulabsum/(double)totulab*100.0,(double)(labsum+ulabsum)/(double)(totdoc)*100.0);
	printm(temstr);
	sprintf(temstr,"r_delta_sq=%.5f xisum=%.5f asum=%.5f\n",r_delta_sq,xisum,asum);
	printm(temstr);
	
}


double CSVM::estimate_margin_vcdim(
							 MODEL *model,
							 double w, /* optional: length of model vector in feature space */
							 double R, /* optional: radius of ball containing the data */
							 KERNEL_PARM *kernel_parm)
{
	double h;
	
	/* follows chapter 5.6.4 in [Vapnik/95] */
	
	if(w<0) 
	{
		w=model_length_s(model,kernel_parm);
	}
	if(R<0) 
	{
		R=estimate_sphere(model,kernel_parm); 
	}
	h = w*w * R*R +1; 
	return(h);
}


double CSVM::estimate_sphere(
					   MODEL *model,          /* Approximates the radius of the ball containing */
					   KERNEL_PARM *kernel_parm) /* the support vectors by bounding it with the */
{                       /* length of the longest support vector. This is */
	register long j;      /* pretty good for text categorization, since all */
	double xlen,maxxlen=0;    /* documents have feature vectors of length 1. It */
	DOC nulldoc;          /* assumes that the center of the ball is at the */
	SVM_WORD nullword;            /* origin of the space. */
	
	nullword.wnum=0;
	nulldoc.words=&nullword;
	nulldoc.twonorm_sq=0;
	nulldoc.docnum=-1;
	
	for(j=1;j<model->sv_num;j++)
	{
		xlen=sqrt(kernel(kernel_parm,model->supvec[j],model->supvec[j])
			-2*kernel(kernel_parm,model->supvec[j],&nulldoc)
			+kernel(kernel_parm,&nulldoc,&nulldoc));
		if(xlen>maxxlen)
		{
			maxxlen=xlen;
		}
	}
	
	return(maxxlen);
}

double CSVM::estimate_r_delta(DOC *docs,    long totdoc,KERNEL_PARM *kernel_parm)
{
	long i;
	double maxxlen,xlen;
	DOC nulldoc;          /* assumes that the center of the ball is at the */
	SVM_WORD nullword;            /* origin of the space. */
	
	nullword.wnum=0;
	nulldoc.words=&nullword;
	nulldoc.twonorm_sq=0;
	nulldoc.docnum=-1;
	
	maxxlen=0;
	for(i=0;i<totdoc;i++) 
	{
		xlen=sqrt(kernel(kernel_parm,&(docs[i]),&(docs[i]))
			-2*kernel(kernel_parm,&(docs[i]),&nulldoc)
			+kernel(kernel_parm,&nulldoc,&nulldoc));
		if(xlen>maxxlen) 
		{
			maxxlen=xlen;
		}
	}
	
	return(maxxlen);
}

double CSVM::estimate_r_delta_average(DOC *docs,    long totdoc,KERNEL_PARM *kernel_parm)
{
	long i;
	double avgxlen;
	DOC nulldoc;          /* assumes that the center of the ball is at the */
	SVM_WORD nullword;            /* origin of the space. */
	
	nullword.wnum=0;
	nulldoc.words=&nullword;
	nulldoc.twonorm_sq=0;
	nulldoc.docnum=-1;
	
	avgxlen=0;
	for(i=0;i<totdoc;i++)
	{
		avgxlen+=sqrt(kernel(kernel_parm,&(docs[i]),&(docs[i]))
			-2*kernel(kernel_parm,&(docs[i]),&nulldoc)
			+kernel(kernel_parm,&nulldoc,&nulldoc));
	}
	
	return(avgxlen/totdoc);
}


double CSVM::length_of_longest_document_vector(DOC *docs, long totdoc,KERNEL_PARM *kernel_parm)
{
	long i;
	double maxxlen,xlen;
	maxxlen=0;
	for(i=0;i<totdoc;i++) 
	{
		xlen=sqrt(kernel(kernel_parm,&(docs[i]),&(docs[i])));
		if(xlen>maxxlen) 
		{
			maxxlen=xlen;
		}
	}
	
	return(maxxlen);
}

/****************************** IO-handling **********************************/
//write model to text file.

void CSVM::write_model(char *modelfile,MODEL *model)
{
	FILE *modelfl;
	long j,i;
	
	if (com_pro.show_action)
	{
		sprintf(temstr,"Writing model file...");  printm(temstr);
	}
	
	if ((modelfl = fopen (modelfile, "w")) == NULL)
	{
		printe (modelfile);   
	}
	else 
	{
		fprintf(modelfl,"SVM-light Version %s\n",VERSION);
		fprintf(modelfl,"%ld # kernel type\n",
			model->kernel_parm.kernel_type);
		fprintf(modelfl,"%ld # kernel parameter -d \n",
			model->kernel_parm.poly_degree);
		fprintf(modelfl,"%.8g # kernel parameter -g \n",
			model->kernel_parm.rbf_gamma);
		fprintf(modelfl,"%.8g # kernel parameter -s \n",
			model->kernel_parm.coef_lin);
		fprintf(modelfl,"%.8g # kernel parameter -r \n",
			model->kernel_parm.coef_const);
		//fprintf(modelfl,"%s # kernel parameter -u \n",model->kernel_parm.custom);
		fprintf(modelfl,"EmptyParam # kernel parameter -u \n");
		fprintf(modelfl,"%ld # highest feature index \n",model->totwords);
		fprintf(modelfl,"%ld # number of training documents \n",model->totdoc);
		
		fprintf(modelfl,"%ld # number of support vectors plus 1 \n",model->sv_num);
		fprintf(modelfl,"%.8g # threshold b \n",model->b);
		
		for(i=1;i<model->sv_num;i++) 
		{
			fprintf(modelfl,"%.32g ",model->alpha[i]);
			for (j=0; ((model->supvec[i])->words[j]).wnum; j++) 
			{
				fprintf(modelfl,"%ld:%.8g ",
					(long)((model->supvec[i])->words[j]).wnum,
					(double)(((model->supvec[i])->words[j]).weight));
			}
			fprintf(modelfl,"\n");
		}
		fclose(modelfl);
	}
	
	
}


void CSVM::write_prediction(
					  char *predfile,
					  MODEL *model,
					  double *lin,double *a,
					  long *unlabeled,
					  long *label,
					  long totdoc,
					  LEARN_PARM *learn_parm)
{
	FILE *predfl;
	long i;
	double dist,a_max;
	
	
	sprintf(temstr,"Writing prediction file...");  printm(temstr);
	
	if ((predfl = fopen (predfile, "w")) == NULL)
	{ printe (predfile);   }
	a_max=learn_parm->epsilon_a;
	for(i=0;i<totdoc;i++) {
		if((unlabeled[i]) && (a[i]>a_max)) {
			a_max=a[i];
		}
	}
	for(i=0;i<totdoc;i++) {
		if(unlabeled[i]) {
			if((a[i]>(learn_parm->epsilon_a))) {
				dist=(double)label[i]*(1.0-learn_parm->epsilon_crit-a[i]/(a_max*2.0));
			}
			else {
				dist=(lin[i]-model->b);
			}
			if(dist>0) {
				fprintf(predfl,"%.8g:+1 n",dist);
			}
			else {
				fprintf(predfl,"%.8g:-1 \n",-dist);
			}
		}
	}
	fclose(predfl);
	
	
}

void CSVM::write_alphas(
				  char *alphafile,
				  double *a,
				  long *label,
				  long totdoc)
{
	FILE *alphafl;
	long i;
	sprintf(temstr,"Writing alpha file...");  printm(temstr);
	if ((alphafl = fopen (alphafile, "w")) == NULL)
		printe (alphafile);   
	for(i=0;i<totdoc;i++) 
		fprintf(alphafl,"%.8g\n",a[i]*(double)label[i]);
	fclose(alphafl);
}