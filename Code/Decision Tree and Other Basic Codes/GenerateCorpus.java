package TopicShift;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.util.Map; 
import java.util.Map.Entry; 
import java.util.HashMap; 
import java.util.StringTokenizer; 
import java.util.ArrayList; 
import java.util.Comparator; 
import java.util.List;
import java.util.Collections; 

public class GenerateCorpus {
	public static int wordCount=0;
	public static void writeData(Map<String,Integer> map){ 
	    List<Map.Entry<String, Integer>> infoIds = new ArrayList<Map.Entry<String, Integer>>(map.entrySet());  
	    Collections.sort(infoIds, new Comparator<Map.Entry<String, Integer>>() {    
	        public int compare(Map.Entry<String, Integer> o1, Map.Entry<String, Integer> o2) {    
	            return (o2.getValue() - o1.getValue());    
	        }    
	    }); 
	    try{
	    	File writename = new File("D:\\WDMProject\\data\\Corpus.txt");
	    	writename.createNewFile();
	    	BufferedWriter out = new BufferedWriter(new FileWriter(writename));
		
	    	for (int i = 0; i < infoIds.size(); i++) {  
	    		Entry<String, Integer> id = infoIds.get(i); 
	    		System.out.println(id.getKey()+":"+id.getValue()); 
	    		out.write(id.getKey()+" "+id.getValue()+"\r\n");
	    		out.flush();
	    	} 
	    	out.close();
	    } catch (Exception e) {
			e.printStackTrace();
		}
	    System.out.println(wordCount);
	} 
	

	public static void readData(String filePath,Map<String,Integer> map){ 
		
		try{
			
			File filename = new File(filePath); 
			InputStreamReader reader = new InputStreamReader(new FileInputStream(filename)); 
			BufferedReader br = new BufferedReader(reader); 
			String line = "";
			StringTokenizer token=new StringTokenizer(line);
			
			line = br.readLine();
			while(line!=null){
				if(line.length()>1 && line.substring(0,1).equals("<")){
					line=line.substring(line.indexOf(">")+1);
				}
				token=new StringTokenizer(line);
				while(token.hasMoreTokens()){
					wordCount+=1;
					String word = token.nextToken();
					if(map.containsKey(word)){ 
			            int count=map.get(word); 
			            map.put(word, count+1);
			        } 
			        else 
			            map.put(word, 1); 
				}
				
				line = br.readLine();
			}
			br.close();
			
			
			writeData(map);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public static void main(String arg[]){
		Map<String,Integer> map=new HashMap<String,Integer>();
		String filePath1 = "D:\\WDMProject\\data\\newsAfterStem.txt"; 
		readData(filePath1,map);
		String filePath2 = "D:\\WDMProject\\data\\commentsAfterStem.txt";
		readData(filePath2,map);
	}
}
