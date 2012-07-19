package org.ops.ui.gantt.swt;

import org.eclipse.jface.viewers.ColumnLabelProvider;
import org.eclipse.jface.viewers.ViewerCell;
import org.eclipse.swt.graphics.Image;

import psengine.PSVariable;

/**
 * Label provider for token parameters (ie variables), assumes you 
 * want the variable name in first column and user-friendly value
 * in second.
 * 
 * @author Tristan Smith
 * 
 */

public class ParameterTableLabelProvider extends ColumnLabelProvider {

	
	public void update(ViewerCell cell) {
		Object element = cell.getElement();
		cell.setText(getColumnText(element, cell.getColumnIndex()));
		Image image = getImage(element);
		cell.setImage(image);
		cell.setBackground(getBackground(element));
		cell.setForeground(getForeground(element));
		cell.setFont(getFont(element));
	}

	
//	@Override
	public String getColumnText(Object element, int index) {
		PSVariable variable = (PSVariable) element;
		
		if(variable == null) {
			return "--";
		}
		else {
			if(index == 0) {
				return variable.getEntityName();
			}
			else {
				if(variable.isSingleton()) {
					return variable.getSingletonValue().toString();
				}
				else {
					// TODO:  Combine this with gantt hover functionality for a common output
					// format for variables:
					return "[" + variable.getLowerBound() + ", " + variable.getUpperBound() + "]";
				}
			}
		}
	}
}
