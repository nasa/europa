package org.ops.ui.ash;

/*
 * InputHandler.java - Manages key bindings and executes actions
 * Copyright (C) 1999 Slava Pestov
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */

import javax.swing.text.*;
import javax.swing.JPopupMenu;
import java.awt.event.*;
import java.awt.Component;
import java.util.*;

/**
 * An input handler converts the user's key strokes into concrete actions.
 * It also takes care of macro recording and action repetition.<p>
 *
 * This class provides all the necessary support code for an input
 * handler, but doesn't actually do any key binding logic. It is up
 * to the implementations of this class to do so.
 *
 * @author Slava Pestov
 * @version $Id: InputHandler.java,v 1.1 2007-02-26 20:28:59 meboyce Exp $
 * @see org.gjt.sp.jedit.textarea.DefaultInputHandler
 */
public abstract class InputHandler extends KeyAdapter {
  /**
   * If this client property is set to Boolean.TRUE on the text area,
   * the home/end keys will support 'smart' BRIEF-like behaviour
   * (one press = start/end of line, two presses = start/end of
   * viewscreen, three presses = start/end of document). By default,
   * this property is not set.
   */
  public static final String SMART_HOME_END_PROPERTY = "InputHandler.homeEnd";

  public static final ActionListener BACKSPACE = new backspace();
  public static final ActionListener BACKSPACE_WORD = new backspace_word();
  public static final ActionListener DELETE = new delete();
  public static final ActionListener DELETE_WORD = new delete_word();
  public static final ActionListener END = new end(false);
  public static final ActionListener DOCUMENT_END = new document_end(false);
  public static final ActionListener SELECT_END = new end(true);
  public static final ActionListener SELECT_DOC_END = new document_end(true);
  public static final ActionListener INSERT_BREAK = new insert_break();
  public static final ActionListener INSERT_TAB = new insert_tab();
  public static final ActionListener HOME = new home(false);
  public static final ActionListener DOCUMENT_HOME = new document_home(false);
  public static final ActionListener SELECT_HOME = new home(true);
  public static final ActionListener SELECT_DOC_HOME = new document_home(true);
  public static final ActionListener NEXT_CHAR = new next_char(false);
  public static final ActionListener NEXT_LINE = new next_line(false);
  public static final ActionListener NEXT_PAGE = new next_page(false);
  public static final ActionListener NEXT_WORD = new next_word(false);
  public static final ActionListener SELECT_NEXT_CHAR = new next_char(true);
  public static final ActionListener SELECT_NEXT_LINE = new next_line(true);
  public static final ActionListener SELECT_NEXT_PAGE = new next_page(true);
  public static final ActionListener SELECT_NEXT_WORD = new next_word(true);
  public static final ActionListener OVERWRITE = new overwrite();
  public static final ActionListener PREV_CHAR = new prev_char(false);
  public static final ActionListener PREV_LINE = new prev_line(false);
  public static final ActionListener PREV_PAGE = new prev_page(false);
  public static final ActionListener PREV_WORD = new prev_word(false);
  public static final ActionListener SELECT_PREV_CHAR = new prev_char(true);
  public static final ActionListener SELECT_PREV_LINE = new prev_line(true);
  public static final ActionListener SELECT_PREV_PAGE = new prev_page(true);
  public static final ActionListener SELECT_PREV_WORD = new prev_word(true);
  public static final ActionListener REPEAT = new repeat();
  public static final ActionListener TOGGLE_RECT = new toggle_rect();
  public static final ActionListener CUT_SELECTION = new cut_selection();
  public static final ActionListener COPY_SELECTION = new copy_selection();
  public static final ActionListener PASTE_SELECTION = new paste_selection();

  public static final ActionListener NEW_LINE = new new_line();
  // Default action
  public static final ActionListener INSERT_CHAR = new insert_char();

  private static Hashtable actions;

  static {
    actions = new Hashtable();
    actions.put("backspace",BACKSPACE);
    actions.put("backspace-word",BACKSPACE_WORD);
    actions.put("delete",DELETE);
    actions.put("delete-word",DELETE_WORD);
    actions.put("end",END);
    actions.put("select-end",SELECT_END);
    actions.put("document-end",DOCUMENT_END);
    actions.put("select-doc-end",SELECT_DOC_END);
    actions.put("insert-break",INSERT_BREAK);
    actions.put("insert-tab",INSERT_TAB);
    actions.put("home",HOME);
    actions.put("select-home",SELECT_HOME);
    actions.put("document-home",DOCUMENT_HOME);
    actions.put("select-doc-home",SELECT_DOC_HOME);
    actions.put("next-char",NEXT_CHAR);
    actions.put("next-line",NEXT_LINE);
    actions.put("next-page",NEXT_PAGE);
    actions.put("next-word",NEXT_WORD);
    actions.put("select-next-char",SELECT_NEXT_CHAR);
    actions.put("select-next-line",SELECT_NEXT_LINE);
    actions.put("select-next-page",SELECT_NEXT_PAGE);
    actions.put("select-next-word",SELECT_NEXT_WORD);
    actions.put("overwrite",OVERWRITE);
    actions.put("prev-char",PREV_CHAR);
    actions.put("prev-line",PREV_LINE);
    actions.put("prev-page",PREV_PAGE);
    actions.put("prev-word",PREV_WORD);
    actions.put("select-prev-char",SELECT_PREV_CHAR);
    actions.put("select-prev-line",SELECT_PREV_LINE);
    actions.put("select-prev-page",SELECT_PREV_PAGE);
    actions.put("select-prev-word",SELECT_PREV_WORD);
    actions.put("repeat",REPEAT);
    actions.put("toggle-rect",TOGGLE_RECT);
    actions.put("insert-char",INSERT_CHAR);
    actions.put("cut-selection", CUT_SELECTION);
    actions.put("copy-selection", COPY_SELECTION);
    actions.put("paste-selection", PASTE_SELECTION);
  }

  /**
   * Returns a named text area action.
   * @param name The action name
   */
  public static ActionListener getAction(String name) {
    return (ActionListener)actions.get(name);
  }

  /**
   * Returns the name of the specified text area action.
   * @param listener The action
   */
  public static String getActionName(ActionListener listener) {
    Enumeration enumer = getActions();
    while(enumer.hasMoreElements()) {
      String name = (String) enumer.nextElement();
      ActionListener _listener = getAction(name);
      if(_listener == listener)
        return name;
    }
    return null;
  }

  /**
   * Returns an enumeration of all available actions.
   */
  public static Enumeration getActions() {
    return actions.keys();
  }

  /**
   * Adds the default key bindings to this input handler.
   * This should not be called in the constructor of this
   * input handler, because applications might load the
   * key bindings from a file, etc.
   */
  public abstract void addDefaultKeyBindings();

  /**
   * Adds a key binding to this input handler.
   * @param keyBinding The key binding (the format of this is
   * input-handler specific)
   * @param action The action
   */
  public abstract void addKeyBinding(String keyBinding, ActionListener action);

  /**
   * Removes a key binding from this input handler.
   * @param keyBinding The key binding
   */
  public abstract void removeKeyBinding(String keyBinding);

  /**
   * Removes all key bindings from this input handler.
   */
  public abstract void removeAllKeyBindings();

  /**
   * Grabs the next key typed event and invokes the specified
   * action with the key as a the action command.
   * @param action The action
   */
  public void grabNextKeyStroke(ActionListener listener) {
    grabAction = listener;
  }

  /**
   * Returns if repeating is enabled. When repeating is enabled,
   * actions will be executed multiple times. This is usually
   * invoked with a special key stroke in the input handler.
   */
  public boolean isRepeatEnabled() {
    return repeat;
  }

  /**
   * Enables repeating. When repeating is enabled, actions will be
   * executed multiple times. Once repeating is enabled, the input
   * handler should read a number from the keyboard.
   */
  public void setRepeatEnabled(boolean repeat) {
    this.repeat = repeat;
  }

  /**
   * Returns the number of times the next action will be repeated.
   */
  public int getRepeatCount() {
    return (repeat ? Math.max(1,repeatCount) : 1);
  }

  /**
   * Sets the number of times the next action will be repeated.
   * @param repeatCount The repeat count
   */
  public void setRepeatCount(int repeatCount) {
    this.repeatCount = repeatCount;
  }

  /**
   * Returns the macro recorder. If this is non-null, all executed
   * actions should be forwarded to the recorder.
   */
  public InputHandler.MacroRecorder getMacroRecorder() {
    return recorder;
  }

  /**
   * Sets the macro recorder. If this is non-null, all executed
   * actions should be forwarded to the recorder.
   * @param recorder The macro recorder
   */
  public void setMacroRecorder(InputHandler.MacroRecorder recorder) {
    this.recorder = recorder;
  }

  /**
   * Returns a copy of this input handler that shares the same
   * key bindings. Setting key bindings in the copy will also
   * set them in the original.
   */
  public abstract InputHandler copy();

  /**
   * Executes the specified action, repeating and recording it as
   * necessary.
   * @param listener The action listener
   * @param source The event source
   * @param actionCommand The action command
   */
  public void executeAction(ActionListener listener, Object source, String actionCommand) {
    // create event
    ActionEvent evt = new ActionEvent(source,
        ActionEvent.ACTION_PERFORMED,
        actionCommand);

    // don't do anything if the action is a wrapper
    // (like EditAction.Wrapper)
    if(listener instanceof Wrapper) {
      listener.actionPerformed(evt);
      return;
    }

    // remember old values, in case action changes them
    boolean _repeat = repeat;
    int _repeatCount = getRepeatCount();

    // execute the action
    if(listener instanceof InputHandler.NonRepeatable)
      listener.actionPerformed(evt);
    else {
      for(int i = 0; i < Math.max(1,repeatCount); i++)
        listener.actionPerformed(evt);
    }

    // do recording. Notice that we do no recording whatsoever
    // for actions that grab keys
    if(grabAction == null) {
      if(recorder != null) {
        if(!(listener instanceof InputHandler.NonRecordable)) {
          if(_repeatCount != 1)
            recorder.actionPerformed(REPEAT,String.valueOf(_repeatCount));

          recorder.actionPerformed(listener,actionCommand);
        }
      }

      // If repeat was true originally, clear it
      // Otherwise it might have been set by the action, etc
      if(_repeat) {
        repeat = false;
        repeatCount = 0;
      }
    }
  }

  /**
   * Returns the text area that fired the specified event.
   * @param evt The event
   */
  public static AshConsole getConsole(EventObject evt) {
    if(evt != null) {
      Object o = evt.getSource();
      if(o instanceof Component) {
        // find the parent text area
        Component c = (Component)o;
        for(;;) {
          if(c instanceof AshConsole)
            return (AshConsole)c;
          else if(c == null)
            break;
          if(c instanceof JPopupMenu)
            c = ((JPopupMenu)c)
              .getInvoker();
          else
            c = c.getParent();
        }
      }
    }

    // this shouldn't happen
    System.err.println("BUG: getConsole() returning null");
    System.err.println("Report this to Slava Pestov <sp@gjt.org>");
    return null;
  }

  // protected members

  /**
   * If a key is being grabbed, this method should be called with
   * the appropriate key event. It executes the grab action with
   * the typed character as the parameter.
   */
  protected void handleGrabAction(KeyEvent evt) {
    // Clear it *before* it is executed so that executeAction()
    // resets the repeat count
    ActionListener _grabAction = grabAction;
    grabAction = null;
    executeAction(_grabAction,evt.getSource(),
        String.valueOf(evt.getKeyChar()));
  }

  // protected members
  protected ActionListener grabAction;
  protected boolean repeat;
  protected int repeatCount;
  protected InputHandler.MacroRecorder recorder;

  /**
   * If an action implements this interface, it should not be repeated.
   * Instead, it will handle the repetition itself.
   */
  public interface NonRepeatable {}

  /**
   * If an action implements this interface, it should not be recorded
   * by the macro recorder. Instead, it will do its own recording.
   */
  public interface NonRecordable {}

  /**
   * For use by EditAction.Wrapper only.
   * @since jEdit 2.2final
   */
  public interface Wrapper {}

  /**
   * Macro recorder.
   */
  public interface MacroRecorder {
    void actionPerformed(ActionListener listener,
        String actionCommand);
  }

  public static class backspace implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);

      if(!console.isEditable()) {
        console.getToolkit().beep();
        return;
      }

      if(console.getSelectionStart()
          != console.getSelectionEnd()) {
        console.setSelectedText("");
      }
      else {
        if(console.getCaretPosition() < console.getCommandStart())
          console.setCaretPosition(console.getDocumentLength());
        else if(console.getCaretPosition() == console.getCommandStart()) {
          console.getToolkit().beep();
          return;
        }

        int caret = console.getCaretPosition();
        if(caret == 0 || caret == console.getCommandStart()) {
          console.getToolkit().beep();
          return;
        }
        try {
          console.getDocument().remove(caret - 1,1);
        }
        catch(BadLocationException bl) {
          bl.printStackTrace();
        }
      }
    }
  }

  public static class backspace_word implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int start = console.getSelectionStart();
      if(start != console.getSelectionEnd()) {
        console.setSelectedText("");
      }

      if(console.getCaretPosition() < console.getCommandStart())
        console.setCaretPosition(console.getDocumentLength());
      else if(console.getCaretPosition() < console.getCommandStart()) {
        console.getToolkit().beep();
        return;
      }
      int line = console.getCaretLine();
      int lineStart = console.getLineStartOffset(line);
      int caret = start - lineStart;

      String lineText = console.getLineText(console
          .getCaretLine());

      if(caret == 0) {
        if(lineStart == 0) {
          console.getToolkit().beep();
          return;
        }
        caret--;
      }
      else {
        String noWordSep = (String)console.getDocument().getProperty("noWordSep");
        caret = TextUtilities.findWordStart(lineText,caret,noWordSep);
      }

      try {
        console.getDocument().remove(
            caret + lineStart,
            start - (caret + lineStart));
      }
      catch(BadLocationException bl) {
        bl.printStackTrace();
      }
    }
  }

  public static class delete implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);

      if(!console.isEditable()) {
        console.getToolkit().beep();
        return;
      }

      if(console.getSelectionStart()
          != console.getSelectionEnd()) {
        console.setSelectedText("");
      }
      else {
        if(console.getCaretPosition() < console.getCommandStart())
          console.setCaretPosition(console.getCommandStart());

        int caret = console.getCaretPosition();
        if(caret == console.getDocumentLength()) {
          console.getToolkit().beep();
          return;
        }
        try {
          console.getDocument().remove(caret,1);
        }
        catch(BadLocationException bl) {
          bl.printStackTrace();
        }
      }
    }
  }

  public static class delete_word implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int start = console.getSelectionStart();
      if(start != console.getSelectionEnd()) {
        console.setSelectedText("");
      }

      if(console.getCaretPosition() < console.getCommandStart())
        console.setCaretPosition(console.getCommandStart());
      int line = console.getCaretLine();
      int lineStart = console.getLineStartOffset(line);
      int caret = start - lineStart;

      String lineText = console.getLineText(console
          .getCaretLine());

      if(caret == lineText.length()) {
        if(lineStart + caret == console.getDocumentLength()) {
          console.getToolkit().beep();
          return;
        }
        caret++;
      }
      else {
        String noWordSep = (String)console.getDocument().getProperty("noWordSep");
        caret = TextUtilities.findWordEnd(lineText,caret,noWordSep);
      }

      try {
        console.getDocument().remove(start,
            (caret + lineStart) - start);
      }
      catch(BadLocationException bl) {
        bl.printStackTrace();
      }
    }
  }

  public static class end implements ActionListener {
    private boolean select;

    public end(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);

      int caret = console.getCaretPosition();

      int lastOfLine = console.getLineEndOffset(
          console.getCaretLine()) - 1;
      int lastVisibleLine = console.getFirstLine()
        + console.getVisibleLines();
      if(lastVisibleLine >= console.getLineCount()) {
        lastVisibleLine = Math.min(console.getLineCount() - 1,
            lastVisibleLine);
      }
      else
        lastVisibleLine -= (console.getElectricScroll() + 1);

      int lastVisible = console.getLineEndOffset(lastVisibleLine) - 1;
      int lastDocument = console.getDocumentLength();

      if(caret == lastDocument) {
        console.getToolkit().beep();
        return;
      }
      else if(!Boolean.TRUE.equals(console.getClientProperty(
              SMART_HOME_END_PROPERTY)))
        caret = lastOfLine;
      else if(caret == lastVisible)
        caret = lastDocument;
      else if(caret == lastOfLine)
        caret = lastVisible;
      else
        caret = lastOfLine;

      if(select)
        console.select(console.getMarkPosition(),caret);
      else
        console.setCaretPosition(caret);
    }
  }

  public static class document_end implements ActionListener {
    private boolean select;

    public document_end(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      if(select)
        console.select(console.getMarkPosition(),
            console.getDocumentLength());
      else
        console.setCaretPosition(console
            .getDocumentLength());
    }
  }

  public static class home implements ActionListener {
    private boolean select;

    public home(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);

      int caret = console.getCaretPosition();

      int firstLine = console.getFirstLine();

      int firstOfLine = console.getLineStartOffset(
          console.getCaretLine());
      int firstVisibleLine = (firstLine == 0 ? 0 :
          firstLine + console.getElectricScroll());
      int firstVisible = console.getLineStartOffset(
          firstVisibleLine);

      if(caret == 0) {
        console.getToolkit().beep();
        return;
      }
      else if(!Boolean.TRUE.equals(console.getClientProperty(SMART_HOME_END_PROPERTY)))
        caret = firstOfLine;
      else if(caret == firstVisible)
        caret = 0;
      else if(caret == firstOfLine)
        caret = firstVisible;
      else
        caret = firstOfLine;

      if(select)
        console.select(console.getMarkPosition(),caret);
      else
        console.setCaretPosition(caret);
    }
  }

  public static class document_home implements ActionListener {
    private boolean select;

    public document_home(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      if(select)
        console.select(console.getMarkPosition(),0);
      else
        console.setCaretPosition(0);
    }
  }

  public static class insert_break implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);

      if(!console.isEditable()) {
        console.getToolkit().beep();
        return;
      }

      console.setSelectedText("\n  ");
    }
  }

  public static class insert_tab implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);

      if(!console.isEditable()) {
        console.getToolkit().beep();
        return;
      }

      console.overwriteSetSelectedText("\t");
    }
  }

  public static class next_char implements ActionListener {
    private boolean select;

    public next_char(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int caret = console.getCaretPosition();
      if(caret == console.getDocumentLength()) {
        console.getToolkit().beep();
        return;
      }

      if(select)
        console.select(console.getMarkPosition(),
            caret + 1);
      else
        console.setCaretPosition(caret + 1);
    }
  }

  public static class next_line implements ActionListener {
    private boolean select;

    public next_line(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      if(select) {
        int caret = console.getCaretPosition();
        int line = console.getCaretLine();

        if(line == console.getLineCount() - 1) {
          console.getToolkit().beep();
          return;
        }

        int magic = console.getMagicCaretPosition();
        if(magic == -1) {
          magic = console.offsetToX(line,
              caret - console.getLineStartOffset(line));
        }

        caret = console.getLineStartOffset(line + 1)
          + console.xToOffset(line + 1,magic);
        console.select(console.getMarkPosition(),caret);
        console.setMagicCaretPosition(magic);
      }
      else {
        console.downHistory();
      }
    }
  }

  public static class next_page implements ActionListener {
    private boolean select;

    public next_page(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int lineCount = console.getLineCount();
      int firstLine = console.getFirstLine();
      int visibleLines = console.getVisibleLines();
      int line = console.getCaretLine();

      firstLine += visibleLines;

      if(firstLine + visibleLines >= lineCount - 1)
        firstLine = lineCount - visibleLines;

      console.setFirstLine(firstLine);

      int caret = console.getLineStartOffset(
          Math.min(console.getLineCount() - 1,
            line + visibleLines));
      if(select)
        console.select(console.getMarkPosition(),caret);
      else
        console.setCaretPosition(caret);
    }
  }

  public static class next_word implements ActionListener {
    private boolean select;

    public next_word(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int caret = console.getCaretPosition();
      int line = console.getCaretLine();
      int lineStart = console.getLineStartOffset(line);
      caret -= lineStart;

      String lineText = console.getLineText(console
          .getCaretLine());

      if(caret == lineText.length()) {
        if(lineStart + caret == console.getDocumentLength()) {
          console.getToolkit().beep();
          return;
        }
        caret++;
      }
      else {
        String noWordSep = (String)console.getDocument().getProperty("noWordSep");
        caret = TextUtilities.findWordEnd(lineText,caret,noWordSep);
      }

      if(select)
        console.select(console.getMarkPosition(),
            lineStart + caret);
      else
        console.setCaretPosition(lineStart + caret);
    }
  }

  public static class overwrite implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      console.setOverwriteEnabled(
          !console.isOverwriteEnabled());
    }
  }

  public static class prev_char implements ActionListener {
    private boolean select;

    public prev_char(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int caret = console.getCaretPosition();
      if(caret == 0) {
        console.getToolkit().beep();
        return;
      }

      if(select)
        console.select(console.getMarkPosition(),
            caret - 1);
      else
        console.setCaretPosition(caret - 1);
    }
  }

  public static class prev_line implements ActionListener {
    private boolean select;

    public prev_line(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      if(select) {
        int caret = console.getCaretPosition();
        int line = console.getCaretLine();

        if(line == 0) {
          console.getToolkit().beep();
          return;
        }

        int magic = console.getMagicCaretPosition();
        if(magic == -1) {
          magic = console.offsetToX(line,
              caret - console.getLineStartOffset(line));
        }

        caret = console.getLineStartOffset(line - 1)
          + console.xToOffset(line - 1,magic);
        console.select(console.getMarkPosition(),caret);
        console.setMagicCaretPosition(magic);
      }
      else {
        console.upHistory();
      }
    }
  }

  public static class prev_page implements ActionListener {
    private boolean select;

    public prev_page(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int firstLine = console.getFirstLine();
      int visibleLines = console.getVisibleLines();
      int line = console.getCaretLine();

      if(firstLine < visibleLines)
        firstLine = visibleLines;

      console.setFirstLine(firstLine - visibleLines);

      int caret = console.getLineStartOffset(
          Math.max(0,line - visibleLines));
      if(select)
        console.select(console.getMarkPosition(),caret);
      else
        console.setCaretPosition(caret);
    }
  }

  public static class prev_word implements ActionListener {
    private boolean select;

    public prev_word(boolean select) {
      this.select = select;
    }

    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      int caret = console.getCaretPosition();
      int line = console.getCaretLine();
      int lineStart = console.getLineStartOffset(line);
      caret -= lineStart;

      String lineText = console.getLineText(console
          .getCaretLine());

      if(caret == 0) {
        if(lineStart == 0) {
          console.getToolkit().beep();
          return;
        }
        caret--;
      }
      else {
        String noWordSep = (String)console.getDocument().getProperty("noWordSep");
        caret = TextUtilities.findWordStart(lineText,caret,noWordSep);
      }

      if(select)
        console.select(console.getMarkPosition(),
            lineStart + caret);
      else
        console.setCaretPosition(lineStart + caret);
    }
  }

  public static class repeat implements ActionListener, InputHandler.NonRecordable {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      console.getInputHandler().setRepeatEnabled(true);
      String actionCommand = evt.getActionCommand();
      if(actionCommand != null) {
        console.getInputHandler().setRepeatCount(
            Integer.parseInt(actionCommand));
      }
    }
  }

  public static class toggle_rect implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      console.setSelectionRectangular(
          !console.isSelectionRectangular());
    }
  }

  public static class new_line implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      String str = evt.getActionCommand();

      if(console.isEditable()) {
        console.acceptCommand();
      }
      else {
        console.getToolkit().beep();
      }
    }
  }

  public static class cut_selection implements ActionListener, InputHandler.NonRepeatable {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      console.cut();
    }
  }

  public static class copy_selection implements ActionListener, InputHandler.NonRepeatable {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      console.copy();
    }
  }

  public static class paste_selection implements ActionListener {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      console.paste();
    }
  }

  public static class insert_char implements ActionListener, InputHandler.NonRepeatable {
    public void actionPerformed(ActionEvent evt) {
      AshConsole console = getConsole(evt);
      String str = evt.getActionCommand();
      int repeatCount = console.getInputHandler().getRepeatCount();

      if(console.isEditable()) {
        StringBuffer buf = new StringBuffer();
        for(int i = 0; i < repeatCount; i++)
          buf.append(str);
        console.overwriteSetSelectedText(buf.toString());
      }
      else {
        console.getToolkit().beep();
      }
    }
  }
}
