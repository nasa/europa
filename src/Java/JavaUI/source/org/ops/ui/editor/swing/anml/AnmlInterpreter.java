package org.ops.ui.editor.swing.anml;

import java.io.BufferedOutputStream;
import java.io.PrintStream;

import org.ops.ui.main.swing.PSDesktop;
import org.ops.ui.editor.swing.ash.AshConsole;
import org.ops.ui.editor.swing.ash.AshInterpreter;
import org.ops.ui.editor.swing.ash.DocumentOutputStream;

import psengine.PSException;

public class AnmlInterpreter extends AshInterpreter {
	PrintStream consoleErr = null;

  public AnmlInterpreter() {
    super("Anml");
  }

  public void setConsole(AshConsole console) {
    consoleErr= new PrintStream(new BufferedOutputStream(new DocumentOutputStream(console.getDocument(), "!!")), true);
  }

  public boolean eval(String toEval) {
		if(toEval.equals(""))
			return true;
		try {
    	    PSDesktop.getInstance().getPSEngine().executeScript("anml", toEval, false /*isFile*/);
		}
		catch(PSException ex) {
			consoleErr.print(ex.getFile());
			consoleErr.print(":");
			consoleErr.print(ex.getLine());
			consoleErr.print(": error: ");
			consoleErr.println(ex.getMessage());
			return true;
		}
		/*
		catch(EOFException ex) {
			return false;
		}
		*/
    return true;
  }

  public void source(String filename) {
		try {
    	    PSDesktop.getInstance().getPSEngine().executeScript("anml", filename, true /*isFile*/);
		}
		catch(PSException ex) {
			consoleErr.print(ex.getFile());
			consoleErr.print(":");
			consoleErr.print(ex.getLine());
			consoleErr.print(": error: ");
			consoleErr.println(ex.getMessage());
		}
		/*
		catch(EOFException ex) {
			consoleErr.println("error: premature EOF");
		}
		*/
  }
}
