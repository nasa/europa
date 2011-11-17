package org.ops.ui.editor.swing.ash;

/*
 * ConsoleDefaults.java - Encapsulates default values for various settings
 * Copyright (C) 1999 Slava Pestov
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */

import javax.swing.JPopupMenu;
import java.awt.Color;

/**
 * Encapsulates default settings for a text area. This can be passed
 * to the constructor once the necessary fields have been filled out.
 * The advantage of doing this over calling lots of set() methods after
 * creating the text area is that this method is faster.
 */
public class ConsoleDefaults
{
	private static ConsoleDefaults DEFAULTS;

	public InputHandler inputHandler;
	public SyntaxDocument document;
	public boolean editable;

	public boolean caretVisible;
	public boolean caretBlinks;
	public boolean blockCaret;
	public int electricScroll;

	public int cols;
	public int rows;
	public SyntaxStyle[] styles;
	public Color caretColor;
	public Color selectionColor;
	public Color lineHighlightColor;
	public boolean lineHighlight;
	public Color bracketHighlightColor;
	public boolean bracketHighlight;
	public Color eolMarkerColor;
	public boolean eolMarkers;
	public boolean paintInvalid;

	public JPopupMenu popup;

	/**
	 * Returns a new ConsoleDefaults object with the default values filled
	 * in.
	 */
	public static ConsoleDefaults getDefaults()
	{
		if(DEFAULTS == null)
		{
			DEFAULTS = new ConsoleDefaults();

			DEFAULTS.inputHandler = new DefaultInputHandler();
			DEFAULTS.inputHandler.addDefaultKeyBindings();
			DEFAULTS.editable = true;

			DEFAULTS.caretVisible = true;
			DEFAULTS.caretBlinks = true;
			DEFAULTS.electricScroll = 3;

			DEFAULTS.cols = 80;
			DEFAULTS.rows = 25;
			DEFAULTS.styles = SyntaxUtilities.getDefaultSyntaxStyles();
			DEFAULTS.caretColor = Color.red;
			DEFAULTS.selectionColor = new Color(0x000088);
			DEFAULTS.lineHighlight = true;
			DEFAULTS.bracketHighlightColor = Color.white;
			DEFAULTS.bracketHighlight = false;
			DEFAULTS.eolMarkerColor = new Color(0x009999);
			DEFAULTS.eolMarkers = false;
			DEFAULTS.paintInvalid = false;
		}
		DEFAULTS.document = new SyntaxDocument();

		return DEFAULTS;
	}
}
