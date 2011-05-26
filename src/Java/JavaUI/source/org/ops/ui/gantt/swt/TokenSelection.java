package org.ops.ui.gantt.swt;

import org.eclipse.jface.viewers.ISelection;
import org.ops.ui.gantt.model.IGanttActivity;

public class TokenSelection implements ISelection {

	private IGanttActivity token;
	
	TokenSelection(IGanttActivity activity) {
		this.token = activity;
	}
	
	public IGanttActivity getToken() { return token; }
	
	@Override
	public boolean isEmpty() {
		return token == null;
	}
}
