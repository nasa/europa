package org.ops.ui.gantt.swt;

import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.Viewer;

import psengine.PSToken;
import psengine.PSVariable;
import psengine.PSVariableList;

/**
 * Content provider for token parameters (ie variables), 
 * simply returns all variables as an array.
 * 
 * @author Tristan Smith
 * 
 */

public class ParameterTableContentProvider implements
		IStructuredContentProvider {

	@Override
	public void dispose() {
	}

	@Override
	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
	}

	@Override
	public Object[] getElements(Object inputElement) {
		PSToken token = (PSToken) inputElement;
		
		if(token == null) {
			return null;
		}
		else {
			PSVariableList list = token.getParameters();
			PSVariable[] array = new PSVariable[list.size()];
			for(int i = 0; i < list.size(); ++i) {
				array[i] = list.get(i);
			}
			return array;
		}
	}
}
