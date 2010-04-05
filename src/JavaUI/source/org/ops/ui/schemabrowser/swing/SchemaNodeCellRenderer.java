package org.ops.ui.schemabrowser.swing;

import java.awt.Color;
import java.awt.Component;
import java.awt.Font;

import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import org.ops.ui.schemabrowser.model.SchemaNode;

public class SchemaNodeCellRenderer extends DefaultTreeCellRenderer {

	private Color categoryColor = new Color(0, 128, 128);

	@Override
	public Component getTreeCellRendererComponent(JTree tree, Object value,
			boolean sel, boolean expanded, boolean leaf, int row,
			boolean hasFocus) {

		DefaultTreeCellRenderer comp = (DefaultTreeCellRenderer) super
				.getTreeCellRendererComponent(tree, value, sel, expanded, leaf,
						row, hasFocus);

		if (!(value instanceof SchemaNode))
			return comp;
		SchemaNode node = (SchemaNode) value;
		switch (node.getNodeType()) {
		case CATEGORY:
			changeFont(comp, Font.BOLD);
			comp.setForeground(categoryColor);
			break;
		case OBJECT_TYPE:
			changeFont(comp, Font.BOLD);
			break;
		case TOKEN_TYPE_PARAMETER:
			changeFont(comp, Font.ITALIC);
			break;
		default:
			changeFont(comp, Font.PLAIN);
		}
		return comp;
	}

	private void changeFont(DefaultTreeCellRenderer comp, int style) {
		Font base = comp.getFont();
		comp.setFont(base.deriveFont(style));
	}

}
