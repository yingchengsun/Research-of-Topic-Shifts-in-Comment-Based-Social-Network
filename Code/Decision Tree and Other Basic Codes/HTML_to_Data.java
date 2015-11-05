package TopicShift;
import java.io.File;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileWriter;

public class HTML_to_Data {
	public static void main(String args[]) {
		//comment marks that imply comment or replay
		String commentIdMark = "<li class=\"js-item comment";
		String replyIdMark = "<li class=\"js-item reply\"";
		String commentContenMark ="<p class=\"comment-content\"";

		try {

			//read raw HTML file
			String pathname = "D:\\WDMProject\\data\\commentsHTML.txt";
			File filename = new File(pathname); 
			InputStreamReader reader = new InputStreamReader(new FileInputStream(filename)); 
			BufferedReader br = new BufferedReader(reader); 
			String line = "";
			
			//write to output file
			File writename = new File("D:\\WDMProject\\data\\commentsData.txt");
			writename.createNewFile(); // �������ļ�
			BufferedWriter out = new BufferedWriter(new FileWriter(writename));
			
			line = br.readLine();
			int commentNum=0;
			int replyNum=0;
			int replyCount=0;
			int textState=1; //1:comment 2:reply
			
			while (line != null) {
				if(line.indexOf(commentIdMark)!=-1){
					textState=1;
				}else if(line.indexOf(replyIdMark)!=-1){
					textState=2;
				}
				
				if(line.indexOf(commentContenMark)!=-1){
					line = br.readLine();
					if(textState==1){
						commentNum+=1;
						out.write("ebola"+" "+commentNum+" "+"["+line.replaceAll("<br>", "").trim()+"]"+" "+"1"+" "+"1"+" "+"first"+"\r\n");
						replyNum=0;
						
					}else if(textState==2){
						replyNum+=1;
						out.write("ebola"+" "+replyNum+" "+"["+line.replaceAll("<br>", "").trim()+"]"+" "+"1"+" "+"1"+" "+"second"+"\r\n");
						//out.write("ebola "+replyNum+" ["+line.replaceAll("<br>", "")+"] "+"1"+"1"+"second"+"\r\n");
						replyCount++;
					}
				}
				
				out.flush();
				line = br.readLine();				
			}

			br.close();
			out.close();
		
		
			System.out.println(commentNum);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}