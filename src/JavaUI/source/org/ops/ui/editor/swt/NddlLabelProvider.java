package org.ops.ui.editor.swt;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;
import org.ops.ui.editor.model.OutlineNode;
import org.ops.ui.main.swt.EuropaPlugin;

/**
 * Label/icon provider for the NDDL outline
 * 
 * @author Tatiana Kichkaylo
 */
public class NddlLabelProvider extends LabelProvider {
	private final String path = "icons/";

	private final String[] names = { "class.png", "class_forward.png",
			"enum.png", "var.png", "pred.png", "cons.png", "fact.png",
			"constr.png", "goal.png", "var_def.png" };

	private Image[] images = null;

	@Override
	public String getText(Object object) {
		return ((OutlineNode) object).getText();
	}

	private void loadImages() {
		images = new Image[names.length];
		for (int i = 0; i < names.length; i++) {
			try {
				ImageDescriptor descr = EuropaPlugin.getImageDescriptor(path
						+ names[i]);
				images[i] = descr.createImage();
			} catch (Exception e) {
				throw new RuntimeException("Cannot load icon " + names[i]);
			}
		}
	}

	@Override
	public Image getImage(Object object) {
		if (images == null)
			loadImages();

		switch (((OutlineNode) object).getType()) {
		case CLASS_DEF:
			return images[0];
		case ENUM:
			return images[2];
		case VARIABLE:
			return images[3];
		case PREDICATE_IN_CLASS:
			return images[4];
		case CONSTRUCTOR:
			return images[5];
		case FACT:
			return images[6];
		case CONSTRAINT_INST:
			return images[7];
		case GOAL:
			return images[8];
		case VARIABLE_INST:
			return images[9];
		}
		return null;
	}

	@Override
	public void dispose() {
		if (images != null)
			for (Image im : images)
				im.dispose();
		super.dispose();
	}
}
