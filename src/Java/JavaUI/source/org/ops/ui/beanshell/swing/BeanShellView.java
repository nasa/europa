package org.ops.ui.beanshell.swing;

import java.awt.Dimension;
import java.io.File;

import javax.swing.JOptionPane;

import org.ops.ui.main.swing.EuropaInternalFrame;
import org.ops.ui.main.swing.PSDesktop;
import org.ops.ui.solver.model.SolverModel;

import bsh.EvalError;
import bsh.Interpreter;
import bsh.util.JConsole;

public class BeanShellView extends EuropaInternalFrame {

	private JConsole console;
	private Interpreter interpreter;

	public BeanShellView(SolverModel solverModel, PSDesktop psDesktop) {
		super("Bean shell");

		console = new JConsole();
		this.setContentPane(console);
		this.interpreter = new Interpreter(console);		
		
		// Register variables
		try {
			interpreter.set("desktop", psDesktop);
			interpreter.set("psengine", solverModel.getEngine());
			// interpreter.set("nddlInterp", nddlInterpreter_);
		} catch (EvalError e) {
			JOptionPane.showMessageDialog(this, "Cannot set BSH variables. "
					+ "The shell may not function properly.");
			e.printStackTrace();
		}

		new Thread(interpreter).start();
	}

	public void loadFile(File file) {
		final String CWD = "bsh.cwd";
		try {
			// Set current directory for the interpreter
			String dir = file.getParent();
			interpreter.set(CWD, dir);
			// Now interpret
			interpreter.eval("source(\"" + file + "\");");
		} catch (EvalError e) {
			e.printStackTrace();
			JOptionPane.showMessageDialog(this, "Cannot eval file\n" + file
					+ ":\n" + e.getMessage());
		}
	}

	@Override
	public Dimension getFavoriteSize() {
		return new Dimension(500, 600);
	}
}
