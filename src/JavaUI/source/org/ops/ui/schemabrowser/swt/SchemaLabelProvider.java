package org.ops.ui.schemabrowser.swt;

import org.eclipse.jface.viewers.IColorProvider;
import org.eclipse.jface.viewers.IFontProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.FontData;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Display;
import org.ops.ui.schemabrowser.model.SchemaNode;

/**
 * Label and icon provider for the SWT version of schema browser
 * 
 * @author Tatiana Kichkaylo
 */
public class SchemaLabelProvider extends LabelProvider implements
		IColorProvider, IFontProvider {
	private Color categoryColor;
	private Font boldFont, italicFont;

	public SchemaLabelProvider(Font font) {
		categoryColor = new Color(Display.getCurrent(), new RGB(0, 128, 128));
		FontData[] data = font.getFontData();
		data[0].setStyle(SWT.BOLD);
		boldFont = new Font(font.getDevice(), data);
		data[0].setStyle(SWT.ITALIC);
		italicFont = new Font(font.getDevice(), data);
	}

	@Override
	public void dispose() {
		categoryColor.dispose();
		boldFont.dispose();
		italicFont.dispose();
		super.dispose();
	}

	@Override
	public String getText(Object obj) {
		return obj.toString();
	}

	public Color getBackground(Object element) {
		return null;
	}

	public Color getForeground(Object element) {
		if (!(element instanceof SchemaNode))
			return null;
		SchemaNode node = (SchemaNode) element;
		switch (node.getNodeType()) {
		case CATEGORY:
			return categoryColor;
		}
		return null;
	}

	public Font getFont(Object element) {
		if (!(element instanceof SchemaNode))
			return null;
		SchemaNode node = (SchemaNode) element;
		switch (node.getNodeType()) {
		case CATEGORY:
		case OBJECT_TYPE:
			return boldFont;
		case TOKEN_TYPE_PARAMETER:
			return italicFont;
		}
		return null;
	}

	// public Image getImage(Object obj) {
	// String imageKey = ISharedImages.IMG_OBJ_ELEMENT;
	// return PlatformUI.getWorkbench().getSharedImages().getImage(imageKey);
	// }
}
