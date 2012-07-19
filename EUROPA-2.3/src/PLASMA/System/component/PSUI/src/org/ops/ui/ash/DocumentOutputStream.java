package org.ops.ui.ash;

import java.io.OutputStream;
import java.io.IOException;
import javax.swing.text.Element;
import javax.swing.text.Document;
import javax.swing.text.BadLocationException;

public class DocumentOutputStream extends OutputStream {
  private Document doc;
  private String prefix;

  public DocumentOutputStream(Document doc, String prefix) {
    super();
    this.doc = doc;
    this.prefix = prefix;
  }

  private void ensureOpen() throws IOException {
    if(doc == null)
      throw new IOException("Stream closed");
  }

  public void close() throws IOException {
    flush();
    doc = null;
  }

  public void flush() throws IOException {
    ensureOpen();
    // flush is a nop here, we don't maintain any extra buffers.
  }

  public void write(int b) throws IOException {
    write(new byte[]{(byte)b}, 0, 1);
  }

  public void write(byte cbuf[], int off, int len) throws IOException {
    synchronized(this) {
      ensureOpen();
      try {
        int start = off;
        int end = start+1;
        while(end < off+len) {
          if(cbuf[end] == '\n') {
						if(prefixOff())
            	doc.insertString(doc.getLength(), prefix, null);
            doc.insertString(doc.getLength(), new String(cbuf, start, end-start), null);
            start = end++;
          }
          ++end;
        }
        if(cbuf[start] == '\n') {
          doc.insertString(doc.getLength(), "\n", null);
        }
        else if(start < off+len) {
					if(prefixOff())
          	doc.insertString(doc.getLength(), prefix, null);
          doc.insertString(doc.getLength(), new String(cbuf, start, len-start-off), null);
        }
      }
      catch(BadLocationException ex) {
        throw new IOException(ex.getMessage());
      }
    }
  }

	// return true if the current line in the doc doesn't contain the prefix
	private boolean prefixOff() {
		if(prefix != null) {
			Element lineEl = doc.getDefaultRootElement().getElement(doc.getDefaultRootElement().getElementCount()-1);
			try {
				String line = doc.getText(lineEl.getStartOffset(), lineEl.getEndOffset()-lineEl.getStartOffset()-1);
				return line.indexOf(prefix) == -1;
			}
			catch(BadLocationException ex) {}
		}
		return false;
	}
}
