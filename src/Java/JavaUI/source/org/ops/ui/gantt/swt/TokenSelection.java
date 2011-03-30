package org.ops.ui.gantt.swt;

import org.eclipse.jface.viewers.ISelection;

import psengine.PSToken;

public class TokenSelection implements ISelection {

	private PSToken token;
	
	TokenSelection(PSToken token) {
		this.token = token;
	}
	
	public PSToken getToken() { return token; }
	
	@Override
	public boolean isEmpty() {
		return token == null;
	}
}
