package TopicShift;
import java.io.File;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileWriter;

public class Classify_Shift_NonShift {
	public static void main(String args[]) {
		//comment marks that imply comment or replay
		String commentIdMark = "s";
		String replyIdMark = "n";
	

		try {

			//read file
			String pathname = "D:\\WDMProject\\data\\commentsAfterStem.txt";
			String pathname2 = "D:\\WDMProject\\data\\commentsData.txt";
			String pathname3 = "D:\\WDMProject\\data\\simResult.txt";
			File filename = new File(pathname); 
			InputStreamReader reader = new InputStreamReader(new FileInputStream(filename)); 
			BufferedReader br = new BufferedReader(reader); 
			String line = "";
			
			File filename2 = new File(pathname2); 
			InputStreamReader reader2 = new InputStreamReader(new FileInputStream(filename2)); 
			BufferedReader br2 = new BufferedReader(reader2); 
			String line2 = "";
			
			File filename3 = new File(pathname3); 
			InputStreamReader reader3 = new InputStreamReader(new FileInputStream(filename3)); 
			BufferedReader br3 = new BufferedReader(reader3); 
			String line3 = "";
			
			//write to output file
			File writename = new File("D:\\WDMProject\\data\\Shift.txt");
			File writename2 = new File("D:\\WDMProject\\data\\notShift.txt");

			writename.createNewFile(); 
			writename2.createNewFile();
			BufferedWriter out = new BufferedWriter(new FileWriter(writename));
			BufferedWriter out2 = new BufferedWriter(new FileWriter(writename2));
			
			String mark, mark2;
			line = br.readLine();
			
			int count;
			double countDecimal;
			int layer;
			int commentNum=0;
			
			int textState=1; //1:comment 2:reply

			while (line != null) {
				mark=(String) line.subSequence(0, 1);
				line=(String) line.subSequence(2,line.length()-1);
				
				line2 = br2.readLine();
				mark2 = (String) line2.subSequence(1, 2);
				if(mark2.equals("*"))
					layer=1;
				else
					layer=0;
				
				count=line2.split(" ").length-1;
				line3 = br3.readLine();
				
				countDecimal=count/1000.0;
				System.out.println(line);
				System.out.println(countDecimal);
				System.out.println(line3);
				
				if(mark.indexOf(commentIdMark)!=-1){
					textState=1;
				}else if(mark.indexOf(replyIdMark)!=-1){
					textState=2;
				}

					if(textState==1){
						commentNum+=1;
						out.write("<"+commentNum+">"+" "+"Politic"+" "+layer+" "+countDecimal+" "+line3+" "+"["+line+"]"+"\r\n");						//out.write("health"+" "+commentNum+" "+"["+line.replaceAll("<br>", "").trim()+"]"+" "+"1"+" "+"1"+" "+"yahoo"+"\r\n");

						
					}else if(textState==2){
						commentNum+=1;
						out2.write("<"+commentNum+">"+" "+"Politic"+" "+layer+" "+countDecimal+" "+line3+" "+"["+line+"]"+"\r\n");
					}
				
				out.flush();
				out2.flush();
				line = br.readLine();

			}

			br.close();
			out.close();
			out2.close();
			System.out.println(commentNum);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}