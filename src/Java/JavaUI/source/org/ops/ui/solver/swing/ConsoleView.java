package org.ops.ui.solver.swing;

import java.awt.Dimension;

import javax.swing.JTextArea;

import org.ops.ui.main.swing.EuropaInternalFrame;

/**
 * A read-only view to capture engine's error output. Used by PSSolverDialog.
 * 
 * @author Tatiana
 */
public class ConsoleView extends EuropaInternalFrame {
	private JTextArea text;

	public ConsoleView() {
		super("Engine output");
		text = new JTextArea();
		text.setEditable(false);
		this.add(text);
	}

	@Override
	public Dimension getFavoriteSize() {
		return new Dimension(400, 100);
	}

	public void addText(String msg) {
		text.setText(text.getText() + msg);
	}
}
