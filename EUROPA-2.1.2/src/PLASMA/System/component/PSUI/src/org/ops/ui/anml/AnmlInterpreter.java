package org.ops.ui.anml;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.EOFException;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.io.StringReader;
import java.io.StringWriter;

import nddl.ModelAccessor;
import nddl.NddlParser;
import nddl.NddlParserState;
import nddl.NddlTreeParser;

import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.XMLElement;
import net.n3.nanoxml.XMLWriter;

import org.ops.ui.PSDesktop;
import org.ops.ui.ash.AshConsole;
import org.ops.ui.ash.AshInterpreter;
import org.ops.ui.ash.DocumentOutputStream;

import psengine.PSEngine;
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
