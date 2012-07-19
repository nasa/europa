package nddl;

import java.io.*;
import java.util.Arrays;

public class IndentWriter extends Writer {
  private Writer writer;
  boolean indent = true;
  int spacesPerIndent = 2;
  int indentLevel = 0;
  public IndentWriter(Writer w) {
  	writer = w;
  }
  public void close() throws IOException {
  	writer.close();
  }
  public void flush() throws IOException {
  	writer.flush();
  }

    public void write(String msg) throws IOException {
      super.write(msg);
    }
  
  public void write(char[] cbuf, int off, int len) throws IOException {
  	for (int i = 0 ; i < len ; i++) {
  	  if (indent) {
  	  	writeindent();
  	  	indent = false;
  	  }
  	  writer.write(cbuf[off+i]);
  	  if (cbuf[off+i] == '\n') {
  	  	indent = true;
  	  }
  	}
  }

  //
  public void setSpacesPerIndent(int spaces) {
  	spacesPerIndent = spaces;
  }
  public void indent() {
  	indentLevel++;
  }
  public void unindent() {
    if(indentLevel <= 0)
      throw new RuntimeException("Cannot indent when indentation level is " + indentLevel);
    indentLevel--;
  }
  //
  private void writeindent() throws IOException {
    if(indentLevel < 0)
      throw new IOException("Invalid indentation level of " + indentLevel);
		char[]buffer = new char[indentLevel*spacesPerIndent];
		Arrays.fill(buffer,' ');
		writer.write(buffer);
  }
}
