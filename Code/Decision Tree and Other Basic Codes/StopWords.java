package TopicShift;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.InputStreamReader;

public class StopWords {

	private String sw[] = { "a", "about", "above", "after", "again", "against",
			"all", "am", "an", "and", "any", "are", "aren't", "as", "at", "be",
			"because", "been", "before", "being", "below", "between", "both",
			"but", "by", "can't", "cannot", "could", "couldn't", "did",
			"didn't", "do", "does", "doesn't", "doing", "don't", "down",
			"during", "each", "few", "for", "from", "further", "had", "hadn't",
			"has", "hasn't", "have", "haven't", "having", "he", "he'd",
			"he'll", "he's", "her", "here", "here's", "hers", "herself", "him",
			"himself", "his", "how", "how's", "i", "i'd", "i'll", "i'm",
			"i've", "if", "in", "into", "is", "isn't", "it", "it's", "its",
			"itself", "let's", "me", "more", "most", "mustn't", "my", "myself",
			"no", "nor", "not", "of", "off", "on", "once", "only", "or",
			"other", "ought", "our", "ours", "ourselves", "out", "over", "own",
			"same", "shan't", "she", "she'd", "she'll", "she's", "should",
			"shouldn't", "so", "some", "such", "than", "that", "that's", "the",
			"their", "theirs", "them", "themselves", "then", "there",
			"there's", "these", "they", "they'd", "they'll", "they're",
			"they've", "this", "those", "through", "to", "too", "under",
			"until", "up", "very", "was", "wasn't", "we", "we'd", "we'll",
			"we're", "we've", "were", "weren't", "what", "what's", "when",
			"when's", "where", "where's", "which", "while", "who", "who's",
			"whom", "why", "why's", "with", "won't", "would", "wouldn't",
			"you", "you'd", "you'll", "you're", "you've", "your", "yours",
			"yourself", "yourselves" };

	public boolean inStopWords(String val) {

		String valWatch = val.toLowerCase();

		for (int i = 0; i < sw.length; i++) {
			if (sw[i].equals(valWatch)) {
				return true;
			}
		}

		return false;

	}

	public static void stopWords(String input, String output) {
		StopWords s = new StopWords();
		try {

			File FileIn = new File(input);
			File FileOut = new File(output);
			if (!FileOut.exists()) {
				FileOut.createNewFile();
			}
			FileWriter fw = new FileWriter(FileOut);
			BufferedWriter bw = new BufferedWriter(fw);
			String[] WordTXT;
			if (FileIn.isFile() && FileIn.exists()) {
				InputStreamReader read = new InputStreamReader(
						new FileInputStream(FileIn));
				BufferedReader br = new BufferedReader(read);
				String lineTXT = null;
				while ((lineTXT = br.readLine()) != null) {
					WordTXT = lineTXT.split(" ");
					for (int i = 0; i < WordTXT.length; i++) {
						// Eleminate the stopwords and number
						if ((s.inStopWords(WordTXT[i]) == false)
								&& (WordTXT[i]
										.matches("^[-+]?(([0-9]+)([.]([0-9]+))?|([.]([0-9]+))?)$") == false)) {
							bw.write(WordTXT[i]);
							bw.write(" ");
							// System.out.print(WordTXT[i]);
						}
					}
					bw.write("\r\n");
				}
				read.close();
				bw.flush();
				bw.close();
				fw.close();
			}

		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	public static void main(String argv[]) {
		String inputPath = "D:\\WDMProject\\data\\newsData.txt";
		String outputPath = "D:\\WDMProject\\data\\newsAfterSW.txt";
		stopWords(inputPath, outputPath);
		inputPath = "D:\\WDMProject\\data\\commentsData.txt";
		outputPath = "D:\\WDMProject\\data\\commentsAfterSW.txt";
		stopWords(inputPath, outputPath);
		System.out.println("OK!");
	}
}
