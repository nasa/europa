package org.ops.ui.editor.swing.ash;

/*
 * ConsolePainter.java - Paints the console.
 * Copyright (C) 1999 Slava Pestov
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */

import javax.swing.ToolTipManager;
import javax.swing.text.*;
import javax.swing.JComponent;
import java.awt.event.MouseEvent;
import java.awt.*;

/**
 * The console repaint manager. It performs double buffering and paints
 * lines of text.
 * @author Slava Pestov
 * @version $Id: ConsolePainter.java,v 1.4 2007-03-01 22:42:02 meboyce Exp $
 */
public class ConsolePainter extends JComponent implements TabExpander {
  // package-private members
  int lineGutterSize;
  int currentLineIndex;
  Token currentLineTokens;
  Segment currentLine;

  // protected members
  protected AshConsole console;

  protected SyntaxStyle[] styles;
  protected Color caretColor;
  protected Color selectionColor;
  protected Color bracketHighlightColor;
  protected Color eolMarkerColor;

  protected boolean blockCaret;
  protected boolean lineHighlight;
  protected boolean bracketHighlight;
  protected boolean paintInvalid;
  protected boolean eolMarkers;
  protected int cols;
  protected int rows;

  protected int tabSize;
  protected FontMetrics fm;

  protected Highlight highlights;

  /**
   * Creates a new repaint manager. This should be not be called
   * directly.
   */
  public ConsolePainter(AshConsole console, ConsoleDefaults defaults) {
    this.console = console;

    setAutoscrolls(true);
    setDoubleBuffered(true);
    setOpaque(true);

    ToolTipManager.sharedInstance().registerComponent(this);

    currentLine = new Segment();
    currentLineIndex = -1;

    setCursor(Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));

    setFont(new Font("Monospaced",Font.PLAIN,14));

    blockCaret = defaults.blockCaret;
    styles = defaults.styles;
    cols = defaults.cols;
    rows = defaults.rows;
    caretColor = defaults.caretColor;
    selectionColor = defaults.selectionColor;
    lineHighlight = defaults.lineHighlight;
    bracketHighlightColor = defaults.bracketHighlightColor;
    bracketHighlight = defaults.bracketHighlight;
    paintInvalid = defaults.paintInvalid;
    eolMarkerColor = defaults.eolMarkerColor;
    eolMarkers = defaults.eolMarkers;
		setForeground(Color.black);
		setBackground(Color.white);
  }

  /**
   * Returns if this component can be traversed by pressing the
   * Tab key. This returns false.
   */
  public final boolean isManagingFocus() {
    return false;
  }

  /**
   * Returns the syntax styles used to paint colorized text. Entry <i>n</i>
   * will be used to paint tokens with id = <i>n</i>.
   * @see org.gjt.sp.jedit.syntax.Token
   */
  public final SyntaxStyle[] getStyles() {
    return styles;
  }

  /**
   * Sets the syntax styles used to paint colorized text. Entry <i>n</i>
   * will be used to paint tokens with id = <i>n</i>.
   * @param styles The syntax styles
   * @see org.gjt.sp.jedit.syntax.Token
   */
  public final void setStyles(SyntaxStyle[] styles) {
    this.styles = styles;
    repaint();
  }

  /**
   * Returns the caret color.
   */
  public final Color getCaretColor() {
    return caretColor;
  }

  /**
   * Sets the caret color.
   * @param caretColor The caret color
   */
  public final void setCaretColor(Color caretColor) {
    this.caretColor = caretColor;
    invalidateSelectedLines();
  }

  /**
   * Returns the selection color.
   */
  public final Color getSelectionColor() {
    return selectionColor;
  }

  /**
   * Sets the selection color.
   * @param selectionColor The selection color
   */
  public final void setSelectionColor(Color selectionColor) {
    this.selectionColor = selectionColor;
    invalidateSelectedLines();
  }

  /**
   * Returns true if line highlight is enabled, false otherwise.
   */
  public final boolean isLineHighlightEnabled() {
    return lineHighlight;
  }

  /**
   * Enables or disables current line highlighting.
   * @param lineHighlight True if current line highlight should be enabled,
   * false otherwise
   */
  public final void setLineHighlightEnabled(boolean lineHighlight) {
    this.lineHighlight = lineHighlight;
    invalidateSelectedLines();
  }

  /**
   * Returns the bracket highlight color.
   */
  public final Color getBracketHighlightColor() {
    return bracketHighlightColor;
  }

  /**
   * Sets the bracket highlight color.
   * @param bracketHighlightColor The bracket highlight color
   */
  public final void setBracketHighlightColor(Color bracketHighlightColor) {
    this.bracketHighlightColor = bracketHighlightColor;
    invalidateLine(console.getBracketLine());
  }

  /**
   * Returns true if bracket highlighting is enabled, false otherwise.
   * When bracket highlighting is enabled, the bracket matching the
   * one before the caret (if any) is highlighted.
   */
  public final boolean isBracketHighlightEnabled() {
    return bracketHighlight;
  }

  /**
   * Enables or disables bracket highlighting.
   * When bracket highlighting is enabled, the bracket matching the
   * one before the caret (if any) is highlighted.
   * @param bracketHighlight True if bracket highlighting should be
   * enabled, false otherwise
   */
  public final void setBracketHighlightEnabled(boolean bracketHighlight) {
    this.bracketHighlight = bracketHighlight;
    invalidateLine(console.getBracketLine());
  }

  /**
   * Returns true if the caret should be drawn as a block, false otherwise.
   */
  public final boolean isBlockCaretEnabled() {
    return blockCaret;
  }

  /**
   * Sets if the caret should be drawn as a block, false otherwise.
   * @param blockCaret True if the caret should be drawn as a block,
   * false otherwise.
   */
  public final void setBlockCaretEnabled(boolean blockCaret) {
    this.blockCaret = blockCaret;
    invalidateSelectedLines();
  }

  /**
   * Returns the EOL marker color.
   */
  public final Color getEOLMarkerColor() {
    return eolMarkerColor;
  }

  /**
   * Sets the EOL marker color.
   * @param eolMarkerColor The EOL marker color
   */
  public final void setEOLMarkerColor(Color eolMarkerColor) {
    this.eolMarkerColor = eolMarkerColor;
    repaint();
  }

  /**
   * Returns true if EOL markers are drawn, false otherwise.
   */
  public final boolean getEOLMarkersPainted() {
    return eolMarkers;
  }

  /**
   * Sets if EOL markers are to be drawn.
   * @param eolMarkers True if EOL markers should be drawn, false otherwise
   */
  public final void setEOLMarkersPainted(boolean eolMarkers) {
    this.eolMarkers = eolMarkers;
    repaint();
  }

  /**
   * Returns true if invalid lines are painted as red tildes (~),
   * false otherwise.
   */
  public boolean getInvalidLinesPainted() {
    return paintInvalid;
  }

  /**
   * Sets if invalid lines are to be painted as red tildes.
   * @param paintInvalid True if invalid lines should be drawn, false otherwise
   */
  public void setInvalidLinesPainted(boolean paintInvalid) {
    this.paintInvalid = paintInvalid;
  }

  /**
   * Adds a custom highlight painter.
   * @param highlight The highlight
   */
  public void addCustomHighlight(Highlight highlight) {
    highlight.init(console,highlights);
    highlights = highlight;
  }

  /**
   * Highlight interface.
   */
  public interface Highlight {
    /**
     * Called after the highlight painter has been added.
     * @param console The console
     * @param next The painter this one should delegate to
     */
    void init(AshConsole console, Highlight next);

    /**
     * This should paint the highlight and delgate to the
     * next highlight painter.
     * @param gfx The graphics context
     * @param line The line number
     * @param y The y co-ordinate of the line
     */
    void paintHighlight(Graphics gfx, int line, int y);

    /**
     * Returns the tool tip to display at the specified
     * location. If this highlighter doesn't know what to
     * display, it should delegate to the next highlight
     * painter.
     * @param evt The mouse event
     */
    String getToolTipText(MouseEvent evt);
  }

  /**
   * Returns the tool tip to display at the specified location.
   * @param evt The mouse event
   */
  public String getToolTipText(MouseEvent evt) {
    if(highlights != null)
      return highlights.getToolTipText(evt);
    else
      return null;
  }

  /**
   * Returns the font metrics used by this component.
   */
  public FontMetrics getFontMetrics() {
    return fm;
  }

  /**
   * Sets the font for this component. This is overridden to update the
   * cached font metrics and to recalculate which lines are visible.
   * @param font The font
   */
  public void setFont(Font font) {
    super.setFont(font);
    fm = Toolkit.getDefaultToolkit().getFontMetrics(font);
    console.recalculateVisibleLines();
  }

  /**
   * Repaints the text.
   * @param g The graphics context
   */
  public void paint(Graphics gfx) {
    tabSize = fm.charWidth(' ') * ((Integer) console.getDocument().getProperty(PlainDocument.tabSizeAttribute)).intValue();

    Rectangle clipRect = gfx.getClipBounds();

    gfx.setColor(getBackground());
    gfx.fillRect(clipRect.x,clipRect.y,clipRect.width,clipRect.height);

    // We don't use yToLine() here because that method doesn't
    // return lines past the end of the document
    int height = (int) (fm.getHeight() * AshConsole.lineSpacing);
    int firstLine = console.getFirstLine();
    int firstInvalid = firstLine + clipRect.y / height;
    // Because the clipRect's height is usually an even multiple
    // of the font height, we subtract 1 from it, otherwise one
    // too many lines will always be painted.
    int lastInvalid = firstLine + (clipRect.y + clipRect.height - 1) / height;

    try {
      TokenMarker tokenMarker = console.getDocument().getTokenMarker();
      int x = console.getHorizontalOffset();

      for(int line = firstInvalid; line <= lastInvalid; line++)
        paintLine(gfx,tokenMarker,line,x);

      if(tokenMarker != null && tokenMarker.isNextLineRequested()) {
        int h = clipRect.y + clipRect.height;
        repaint(0,h,getWidth(),getHeight() - h);
      }
    }
    catch(Exception e) {
      System.err.println("Error repainting line range {" + firstInvalid + ", " + lastInvalid + "}:");
      e.printStackTrace();
    }
  }

  /**
   * Marks a line as needing a repaint.
   * @param line The line to invalidate
   */
  public final void invalidateLine(int line) {
    repaint(0,console.lineToY(line) + fm.getMaxDescent() + fm.getLeading(), getWidth(),(int) (fm.getHeight() * AshConsole.lineSpacing));
  }

  /**
   * Marks a range of lines as needing a repaint.
   * @param firstLine The first line to invalidate
   * @param lastLine The last line to invalidate
   */
  public final void invalidateLineRange(int firstLine, int lastLine) {
    repaint(0,console.lineToY(firstLine) + fm.getMaxDescent() + fm.getLeading(), getWidth(),(lastLine - firstLine + 1) * (int) (fm.getHeight() * AshConsole.lineSpacing));
  }

  /**
   * Repaints the lines containing the selection.
   */
  public final void invalidateSelectedLines() {
    invalidateLineRange(console.getSelectionStartLine(), console.getSelectionEndLine());
  }

  /**
   * Implementation of TabExpander interface. Returns next tab stop after
   * a specified point.
   * @param x The x co-ordinate
   * @param tabOffset Ignored
   * @return The next tab stop after <i>x</i>
   */
  public float nextTabStop(float x, int tabOffset) {
    int offset = console.getHorizontalOffset();
    int ntabs = ((int)x - offset) / tabSize;
    return (ntabs + 1) * tabSize + offset;
  }

  /**
   * Returns the painter's preferred size.
   */
  public Dimension getPreferredSize() {
    Dimension dim = new Dimension();
    dim.width = fm.charWidth('w') * cols;
    dim.height = (int) (fm.getHeight() * AshConsole.lineSpacing) * rows;
    return dim;
  }


  /**
   * Returns the painter's minimum size.
   */
  public Dimension getMinimumSize() {
    return getPreferredSize();
  }

  protected void paintLine(Graphics gfx, TokenMarker tokenMarker, int line, int x) {
    Font defaultFont = getFont();
    Color defaultColor = getForeground();

    currentLineIndex = line;
    int y = console.lineToY(line);

    if(line < 0 || line >= console.getLineCount()) {
      if(paintInvalid) {
        paintHighlight(gfx,line,y);
        styles[Token.INVALID].setGraphicsFlags(gfx,defaultFont);
        gfx.drawString("~",0,y + (int) (fm.getHeight() * AshConsole.lineDescent));
      }
    }
    else if(tokenMarker == null) {
      paintPlainLine(gfx,line,defaultFont,defaultColor,x,y);
    }
    else {
      paintSyntaxLine(gfx,tokenMarker,line,defaultFont, defaultColor,x,y);
    }
  }

  protected void paintPlainLine(Graphics gfx, int line, Font defaultFont, Color defaultColor, int x, int y) {
    paintHighlight(gfx,line,y);
    console.getLineText(line,currentLine);

    gfx.setFont(defaultFont);
    gfx.setColor(defaultColor);

    y += (int) (fm.getHeight() * AshConsole.lineDescent);
    x = Utilities.drawTabbedText(currentLine,x,y,gfx,this,0);

    if(eolMarkers) {
      gfx.setColor(eolMarkerColor);
      gfx.drawString(".",x,y);
    }
  }

  protected void paintSyntaxLine(Graphics gfx, TokenMarker tokenMarker, int line, Font defaultFont, Color defaultColor, int x, int y) {
    console.getLineText(currentLineIndex,currentLine);
    currentLineTokens = tokenMarker.markTokens(currentLine, currentLineIndex);

    paintHighlight(gfx,line,y);

    gfx.setFont(defaultFont);
    gfx.setColor(defaultColor);
    y += (int) (fm.getHeight() * AshConsole.lineDescent);
    x = SyntaxUtilities.paintSyntaxLine(currentLine, currentLineTokens,styles,this,gfx,x,y);

    if(eolMarkers) {
      gfx.setColor(eolMarkerColor);
      gfx.drawString(".",x,y);
    }
  }

  protected void paintHighlight(Graphics gfx, int line, int y) {
    if(line >= console.getSelectionStartLine() && line <= console.getSelectionEndLine())
      paintLineHighlight(gfx,line,y);

    if(highlights != null)
      highlights.paintHighlight(gfx,line,y);

    if(bracketHighlight && line == console.getBracketLine())
      paintBracketHighlight(gfx,line,y);

    if(line == console.getCaretLine())
      paintCaret(gfx,line,y);
  }

  protected void paintLineHighlight(Graphics gfx, int line, int y) {
    int height = (int) (fm.getHeight() * AshConsole.lineSpacing);
    y += fm.getLeading() + fm.getMaxDescent();

    int selectionStart = console.getSelectionStart();
    int selectionEnd = console.getSelectionEnd();

    if(selectionStart == selectionEnd) {
      if(lineHighlight) {
				Color bg = getBackground();
				float[] hsb = Color.RGBtoHSB(bg.getRed(), bg.getGreen(), bg.getBlue(), null);
				if(hsb[2] < .4)
        	gfx.setColor(Color.getHSBColor(hsb[0], hsb[1], hsb[2]+.05f));
				else
        	gfx.setColor(Color.getHSBColor(hsb[0], hsb[1], hsb[2]-.05f));
        gfx.fillRect(0,y,getWidth(),height);
      }
    }
    else {
      gfx.setColor(selectionColor);

      int selectionStartLine = console.getSelectionStartLine();
      int selectionEndLine = console.getSelectionEndLine();
      int lineStart = console.getLineStartOffset(line);

      int x1, x2;
      if(console.isSelectionRectangular()) {
        int lineLen = console.getLineLength(line);
        x1 = console._offsetToX(line,Math.min(lineLen, selectionStart - console.getLineStartOffset(selectionStartLine)));
        x2 = console._offsetToX(line,Math.min(lineLen, selectionEnd - console.getLineStartOffset(selectionEndLine)));
        if(x1 == x2)
          x2++;
      }
      else if(selectionStartLine == selectionEndLine) {
        x1 = console._offsetToX(line, selectionStart - lineStart);
        x2 = console._offsetToX(line, selectionEnd - lineStart);
      }
      else if(line == selectionStartLine) {
        x1 = console._offsetToX(line, selectionStart - lineStart);
        x2 = getWidth();
      }
      else if(line == selectionEndLine) {
        x1 = 0;
        x2 = console._offsetToX(line, selectionEnd - lineStart);
      }
      else {
        x1 = 0;
        x2 = getWidth();
      }

      // "inlined" min/max()
      gfx.fillRect(x1 > x2 ? x2 : x1,y,x1 > x2 ? (x1 - x2) : (x2 - x1),height);
    }

  }

  protected void paintBracketHighlight(Graphics gfx, int line, int y) {
    int position = console.getBracketPosition();
    if(position == -1)
      return;
    y += fm.getLeading() + fm.getMaxDescent();
    int x = console._offsetToX(line,position);
    gfx.setColor(bracketHighlightColor);
    // Hack!!! Since there is no fast way to get the character
    // from the bracket matching routine, we use ( since all
    // brackets probably have the same width anyway
    gfx.drawRect(x,y,fm.charWidth('(') - 1, (int) (fm.getHeight() * AshConsole.lineSpacing) - 1);
  }

  protected void paintCaret(Graphics gfx, int line, int y) {
    if(console.isCaretVisible()) {
      int offset = console.getCaretPosition() 
        - console.getLineStartOffset(line);
      int caretX = console._offsetToX(line,offset);
      int caretWidth = ((blockCaret || console.isOverwriteEnabled()) ? fm.charWidth('w') : 1);
      y += fm.getLeading() + fm.getMaxDescent();
      int height = (int) (fm.getHeight() * AshConsole.lineDescent);

      gfx.setColor(caretColor);

      if(console.isOverwriteEnabled())
        gfx.fillRect(caretX,y + height - 1, caretWidth,1);
      else
        gfx.drawRect(caretX,y,caretWidth - 1,height - 1);
    }
  }
}
