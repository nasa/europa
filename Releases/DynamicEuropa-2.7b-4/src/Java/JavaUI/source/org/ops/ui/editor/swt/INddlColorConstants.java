package org.ops.ui.editor.swt;

import org.eclipse.swt.graphics.RGB;

/** Default colors for syntax highlighting in NDDL */
public interface INddlColorConstants {
	/** Default text color */
	RGB DEFAULT = new RGB(0, 0, 0);
	RGB NDDL_COMMENT = new RGB(0, 128, 128);
	RGB STRING = new RGB(0, 0, 200);
	RGB KEYWORD = new RGB(150, 0, 150);
	RGB BRACKET = new RGB(0, 0, 0);
}
