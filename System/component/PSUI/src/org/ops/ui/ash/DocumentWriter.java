package org.ops.ui.ash;

import java.io.Writer;
import java.io.IOException;
import javax.swing.text.Document;
import javax.swing.text.BadLocationException;

public class DocumentWriter extends Writer {
  Document doc;

  public DocumentWriter(Document doc) {
    super();
    this.doc = doc;
  }

  public DocumentWriter(Document doc, Object lock) {
    super(lock);
    this.doc = doc;
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

  public void write(char cbuf[], int off, int len) throws IOException {
    synchronized(lock) {
      ensureOpen();
      try {
        doc.insertString(doc.getLength(), new String(cbuf, off, len), null);
      }
      catch(BadLocationException ex) {
        throw new IOException(ex.getMessage());
      }
    }
  }
}
