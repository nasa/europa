package testLang;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.Iterator;
import java.util.LinkedList;

public class CppFile {
  private static final String indent = "  ";
  private String indentation;
  private LinkedList source;
  public CppFile() {
    indentation = "";
    source = new LinkedList();
  }
  public void addLine(final String line) {
    String [] lines = line.split("\\n");
    for(int i = 0; i < lines.length; i++)
      source.addLast(indentation + lines[i] + "\n");
//     String text = indentation + line + "\n";
//     source.addLast(text);
  }
  public void add(CppFile file) {
    for(Iterator it = file.source.iterator(); it.hasNext();)
      source.addLast(indentation + ((String)it.next()));
  }
  public void indent() {
    indentation = indentation + indent;
  }
  public void unindent() {
    if(indentation.length() == 0)
      return;
    indentation = indentation.substring(2);
  }
  public String toString() {
    StringBuffer retval = new StringBuffer();
    for(Iterator it = source.iterator(); it.hasNext();)
      retval.append(it.next().toString());
    return retval.toString();
  }
  public void write(OutputStream out) throws IOException {
    OutputStreamWriter writer = new OutputStreamWriter(out);
    for(Iterator it = source.iterator(); it.hasNext();) {
      String str = (String) it.next();
      writer.write(str, 0, str.length());
    }
    writer.flush();
    source.clear();
  }
}
