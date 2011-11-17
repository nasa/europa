package org.ops.ui.editor.swing.anml;

/*
 * AnmlTokenMarker.java - Anml Shell token marker
 */

import org.ops.ui.editor.swing.ash.KeywordMap;
import org.ops.ui.editor.swing.ash.AshTokenMarker;

/**
 * Anml Shell token marker.
 *
 * @author Matthew E. Boyce
 * @version $Id: AnmlTokenMarker.java,v 1.1 2007-03-02 01:37:29 meboyce Exp $
 */
public class AnmlTokenMarker extends AshTokenMarker {
  public AnmlTokenMarker() {
		super("Anml % ");
    this.keywords = getKeywords();
  }

  public static KeywordMap getKeywords() {
    if(anmlKeywords == null) {
      anmlKeywords = new KeywordMap(false);

    }
    return anmlKeywords;
  }

  // private members
  private static KeywordMap anmlKeywords;
}
