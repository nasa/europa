package nddl;

import java.io.Reader;
import java.io.Writer;

public class StreamMerge implements Runnable
{
	Reader in = null;
	Writer out = null;

	public StreamMerge(Reader in, Writer out)
	{
		this.in = in;
		this.out = out;
	}
	public void run()
	{
		if(in == null || out == null) return;
		
		int c;
		try{
			while((c = in.read()) != -1)
			{
				out.write(c);
				if(c == '\n') out.flush();
			}
		}
		catch(Exception ex) {System.err.println("\nStream Merging failed.");}
		try{out.flush();} catch(Exception ex) {/* Nothing useful to say */}
		try{in.close();}  catch(Exception ex) {/* Nothing useful to say */}
	}
	public static void merge(Reader in, Writer out)
	{
		StreamMerge m = new StreamMerge(in, out);
		try { new Thread(m).start(); } catch(Exception ex) {/*not reachable*/}
	}
}
