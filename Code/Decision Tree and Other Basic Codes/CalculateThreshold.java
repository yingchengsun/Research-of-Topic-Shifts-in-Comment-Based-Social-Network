package wdmProjectEvaluation;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.InputStreamReader;

public class CalculateThreshold {
	private static int N=4372;//comments number
	private static int M=200; //random select 200 commments to manually label
	private static int numInFolder=20;
	private static String simValuePath="D:\\workspace\\wdmProject\\data\\simResult.txt";
	private static String labelValuePath="D:\\workspace\\wdmProject\\data\\labelData.txt";
	private static float[] simValue = new float[N+1];
	private static float[] commentSimValue = new float[N+1];
	private static float[] replySimValue = new float[N+1];
	private static int[][] label = new int[M+1][2];
	private static int[][] sort = new int[M+1][2];
	private static float[][] threshold = new float[M+1][2];
	
	static int commentNum=0;
	static int replyNum=0;
	private static void readSimValue(){
		try{
			File filename = new File(simValuePath); 
			InputStreamReader reader = new InputStreamReader(new FileInputStream(filename)); 
			BufferedReader br = new BufferedReader(reader); 
			String line = "";
			int i=0;
			line = br.readLine();
			while(line!=null&&i<=N){
				i=i+1;
				
				if(line.substring(0,1).equals("c")){
					commentNum+=1;
					if(line.length()>1){
						line=line.substring(line.indexOf(":")+1);
					}
				commentSimValue[commentNum]=Float.parseFloat(line);
				}
				else if(line.substring(0,1).equals("r")){
					replyNum+=1;
					if(line.length()>1){
						line=line.substring(line.indexOf(":")+1);
					}
				replySimValue[replyNum]=Float.parseFloat(line);
				}
				simValue[i]=Float.parseFloat(line);
				
				line = br.readLine();
				
			}
			br.close();
			
		}catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private static void readLabelValue(){
		try{
			File filename = new File(labelValuePath); 
			InputStreamReader reader = new InputStreamReader(new FileInputStream(filename)); 
			BufferedReader br = new BufferedReader(reader); 
			String line = "";
			String labelValue="";
			int i=0;
			int commentNo=0;
			line = br.readLine();
			while(line!=null&&i<N){
				i=i+1;
				if(line.length()>1){
					labelValue=line.substring(0,1);
					commentNo=Integer.parseInt(line.substring(line.indexOf("<")+1,line.indexOf(">")));
				
					if(labelValue.equals("s")){
						label[i][0]=0;
					}
					else if(labelValue.equals("n")){
						label[i][0]=1;
					}
					else{
						label[i][0]=-1;
					}
					label[i][1]=commentNo;
					System.out.println(i+":"+label[i][0]+" "+label[i][1]);
				}
				line = br.readLine();
				
			}
			br.close();
			
		}catch (Exception e) {
			e.printStackTrace();
		}
		
		
		
		for(int i=1;i<=M;i++){
			System.out.println(i+"("+label[i][1]+"/"+label[i][0]+")"+":"+simValue[label[i][1]]);
		}
		
	}
	
	private static float result(){
		float tempThreshold=0;
		int m=0;
		double c1nos=0;
		double c2nos=0;
		double totalNum=M;
		double entropy,entropyc1,entropyc2=0;
		//sort
				int tempLabelValue=0;
				int tempCommentNo=0;
				int m2 = M;
				int sortNo =0;
				for(int i=1;i<=M;i++){
					
						sortNo+=1;
						sort[sortNo][0]=label[i][0];
						sort[sortNo][1]=label[i][1];
					
				}
				for(int i=1;i<=M;i++){
					
					tempLabelValue=sort[1][0];
					tempCommentNo=sort[1][1];
					for(int j=2;j<=m2;j++){
						if(simValue[sort[j-1][1]]>simValue[sort[j][1]]){
							tempLabelValue=sort[j-1][0];
							tempCommentNo=sort[j-1][1];
							sort[j-1][0]=sort[j][0];
							sort[j-1][1]=sort[j][1];
							sort[j][0]=tempLabelValue;
							sort[j][1]=tempCommentNo;
						}
						
					}
					m2=m2-1;
					
				}
		for(int i=1;i<M;i++){
	
			c1nos=c2nos=entropy=entropyc1=entropyc2=0;
			
			if(sort[i][0]!=sort[i+1][0]){
				m=m+1;
				tempThreshold=(simValue[sort[i][1]]+simValue[sort[i+1][1]])/2;
				threshold[m][0]=tempThreshold;
				for(int j=1;j<=i;j++){
					if(sort[j][0]==0){
						c1nos+=1;
					}
				}
				for(int j=i+1;j<=M;j++){
					if(sort[j][0]==0){
						c2nos+=1;
					}
				}
				if(c1nos==0||c1nos==i){
					entropyc1=0;
				}else{
					entropyc1=-(c1nos/i)*(Math.log(c1nos/i)/Math.log(2))-(1-c1nos/i)*(Math.log(1-c1nos/i)/Math.log(2));
				}
				if(c2nos==0||c2nos==M-i){
					entropyc2=0;
				}else{
					entropyc2=-(c2nos/(totalNum-i))*(Math.log(c2nos/(totalNum-i))/Math.log(2))-(1-c2nos/(totalNum-i))*(Math.log(1-c2nos/(totalNum-i))/Math.log(2));
				}
				entropy=(i/totalNum)*entropyc1+(1-i/totalNum)*entropyc2;
				System.out.println(i+" entropy:"+entropy);
				threshold[m][1]=(float)entropy;
				
			}
			}
		
		
		//select the smallest entropy
		float tempEntropy=threshold[1][1];
		for(int i=1;i<=m;i++){
			if(threshold[i][1]<tempEntropy){
				tempEntropy=threshold[i][1];
				tempThreshold=threshold[i][0];
			}
		}
		System.out.println("Threshold = "+tempThreshold+" entropy = "+tempEntropy);
		
		return tempThreshold;
	}
	
	private static float calculateThreshold(int folder){
		float tempThreshold=0;
		int m=0;
		double c1nos=0;
		double c2nos=0;
		double totalNum=M-numInFolder;
		double entropy,entropyc1,entropyc2=0;
		//sort
				int tempLabelValue=0;
				int tempCommentNo=0;
				int m2 = M-numInFolder;
				int sortNo =0;
				for(int i=1;i<=M;i++){
					if(i<=(folder-1)*numInFolder||i>(folder)*numInFolder){
						sortNo+=1;
						sort[sortNo][0]=label[i][0];
						sort[sortNo][1]=label[i][1];
					}
				}
				for(int i=1;i<=M;i++){
					
					tempLabelValue=sort[1][0];
					tempCommentNo=sort[1][1];
					for(int j=2;j<=m2;j++){
						if(simValue[sort[j-1][1]]>simValue[sort[j][1]]){
							tempLabelValue=sort[j-1][0];
							tempCommentNo=sort[j-1][1];
							sort[j-1][0]=sort[j][0];
							sort[j-1][1]=sort[j][1];
							sort[j][0]=tempLabelValue;
							sort[j][1]=tempCommentNo;
						}
						
					}
					m2=m2-1;
					
				}
		for(int i=1;i<M-numInFolder;i++){
	
			c1nos=c2nos=entropy=entropyc1=entropyc2=0;
			
			if(sort[i][0]!=sort[i+1][0]){
				m=m+1;
				tempThreshold=(simValue[sort[i][1]]+simValue[sort[i+1][1]])/2;
				threshold[m][0]=tempThreshold;
				for(int j=1;j<=i;j++){
					if(sort[j][0]==0){
						c1nos+=1;
					}
				}
				for(int j=i+1;j<=M-numInFolder;j++){
					if(sort[j][0]==0){
						c2nos+=1;
					}
				}
				if(c1nos==0||c1nos==i){
					entropyc1=0;
				}else{
					entropyc1=-(c1nos/i)*(Math.log(c1nos/i)/Math.log(2))-(1-c1nos/i)*(Math.log(1-c1nos/i)/Math.log(2));
				}
				if(c2nos==0||c2nos==M-i){
					entropyc2=0;
				}else{
					entropyc2=-(c2nos/(totalNum-i))*(Math.log(c2nos/(totalNum-i))/Math.log(2))-(1-c2nos/(totalNum-i))*(Math.log(1-c2nos/(totalNum-i))/Math.log(2));
				}
				entropy=(i/totalNum)*entropyc1+(1-i/totalNum)*entropyc2;
				System.out.println(i+" entropy:"+entropy);
				threshold[m][1]=(float)entropy;
				
			}
			}
		
		
		//select the smallest entropy
		float tempEntropy=threshold[1][1];
		for(int i=1;i<=m;i++){
			if(threshold[i][1]<tempEntropy){
				tempEntropy=threshold[i][1];
				tempThreshold=threshold[i][0];
			}
		}
		System.out.println("Threshold = "+tempThreshold+" entropy = "+tempEntropy);
		
		return tempThreshold;
	}
	
	
	public static void main(String arg[]){
			readSimValue();
			readLabelValue();
			
			float threshold;
			int c1no,a,b;
			float p,pAll=0;
			for(int j=1;j<=M/numInFolder;j++){
			threshold = calculateThreshold(j);
			
			c1no=0;
			a=0;
			b=0;
			for(int i=(j-1)*numInFolder+1;i<=j*numInFolder;i++){
				if(simValue[label[i][1]]<threshold){
					c1no+=1;
					if(label[i][0]==0){
						a+=1;
					}
				}else{
					if(label[i][0]==0){
						b+=1;
					}
				}
			}
			p=(float)(a+(numInFolder-c1no-b))/(float)(numInFolder);
			pAll+=p;
			System.out.println("There are "+c1no+" comments under threshold, and "+a+" of them are shifted. "+b+"p="+p);
			}
			pAll=pAll*(float)numInFolder/(float)M;
			System.out.println("Accuracy is "+pAll);
			threshold=result();
			int shiftNo=0;
			for(int i=1;i<=N;i++){
				if(simValue[i]<threshold){
					shiftNo+=1;
				}
			}
			float shiftPercentage = (float)shiftNo/(float)N;
			System.out.println(shiftPercentage+" of comments shift");
			
			shiftNo=0;
			for(int i=1;i<=commentNum;i++){
				if(commentSimValue[i]<threshold){
					shiftNo+=1;
				}
			}
			shiftPercentage = (float)shiftNo/(float)commentNum;
			System.out.println(shiftPercentage+" of comments shift"+commentNum);
			
			shiftNo=0;
			for(int i=1;i<=replyNum;i++){
				if(replySimValue[i]<threshold){
					shiftNo+=1;
				}
			}
			shiftPercentage = (float)shiftNo/(float)replyNum;
			System.out.println(shiftPercentage+" of reply shift"+replyNum);
			
	}
}
