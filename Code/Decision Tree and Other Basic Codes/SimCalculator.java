package TopicShift;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

public class SimCalculator {
	public static int N = 3000;// N>=|corpus|
	public static int M = 1000;// M>=number of comments

	public static void buildCorpus(String filePath, Map<String, Integer> corpus) {
		try {
			int n = 1; // location of a word in the vector, start from 1
			File filename = new File(filePath);
			InputStreamReader reader = new InputStreamReader(
					new FileInputStream(filename));
			BufferedReader br = new BufferedReader(reader);
			String line = "";
			StringTokenizer token = new StringTokenizer(line);

			line = br.readLine();
			while (line != null) {
				token = new StringTokenizer(line);
				String word = token.nextToken();
				// Integer count = Integer.parseInt(token.nextToken());
				corpus.put(word, n);
				n = n + 1;
				line = br.readLine();
			}
			br.close();

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static void buildNewsVector(String filePath,
			Map<String, Integer> corpus, int news[]) {
		try {
			File filename = new File(filePath);
			InputStreamReader reader = new InputStreamReader(
					new FileInputStream(filename));
			BufferedReader br = new BufferedReader(reader);
			String line = "";
			StringTokenizer token = new StringTokenizer(line);

			line = br.readLine();
			while (line != null) {

				token = new StringTokenizer(line);
				while (token.hasMoreTokens()) {
					String word = token.nextToken();
					if (corpus.containsKey(word)) {
						int location = corpus.get(word);
						news[location] += 1;

					}
				}

				line = br.readLine();
			}
			br.close();

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static void buildCommentsVector(String filePath,
			Map<String, Integer> corpus, int comments[][]) {
		try {
			File filename = new File(filePath);
			InputStreamReader reader = new InputStreamReader(
					new FileInputStream(filename));
			BufferedReader br = new BufferedReader(reader);
			String line = "";
			StringTokenizer token = new StringTokenizer(line);

			int n = 1; // comment id, start from 1
			line = br.readLine();
			while (line != null) {
				if (line.length() > 1 && line.substring(0, 1).equals("<")) {
					line = line.substring(10);
				}

				token = new StringTokenizer(line);
				while (token.hasMoreTokens()) {
					String word = token.nextToken();
					if (corpus.containsKey(word)) {
						int location = corpus.get(word);
						comments[n][location] += 1;
					}
				}

				line = br.readLine();
				n = n + 1;
			}
			br.close();

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static double simCal(int news[], int comments[][], int cid) {
		double A = 0, B = 0, AB = 0;
		double sim;
		for (int i = 1; i < N; i++) {
			AB = AB + news[i] * comments[cid][i];
		}
		for (int i = 1; i < N; i++) {
			A = A + news[i] * news[i];
		}
		A = Math.sqrt(A);
		for (int i = 1; i < N; i++) {
			B = B + comments[cid][i] * comments[cid][i];
		}
		B = Math.sqrt(B);

		sim = AB / (A * B);

		return (sim);
	}

	public static void main(String arg[]) {
		Map<String, Integer> corpus = new HashMap<String, Integer>();
		int news[];
		news = new int[N];
		int comments[][];
		comments = new int[M][N];

		String filePath = "D:\\WDMProject\\data\\Corpus.txt";
		buildCorpus(filePath, corpus);

		filePath = "D:\\WDMProject\\data\\newsAfterStem.txt";
		buildNewsVector(filePath, corpus, news);

		filePath = "D:\\WDMProject\\data\\commentsAfterStem.txt";
		buildCommentsVector(filePath, corpus, comments);

		File filename = new File(filePath);
		InputStreamReader reader;
		try {
			reader = new InputStreamReader(new FileInputStream(filename));
			BufferedReader br = new BufferedReader(reader);

			try {
				File writename = new File("D:\\WDMProject\\data\\simResult.txt");
				writename.createNewFile();
				BufferedWriter out = new BufferedWriter(new FileWriter(
						writename));
				double r;
				int i = 1;
				while (br.readLine() != null) {
					r = simCal(news, comments, i);
					System.out.println(i + ":" + r);
					//out.write(i + ":" + r + "\r\n");
					out.write(r + "\r\n");
					out.flush();
					i++;
				}
				out.close();
			} catch (Exception e) {
				e.printStackTrace();
			}

		} catch (FileNotFoundException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

	}
}
