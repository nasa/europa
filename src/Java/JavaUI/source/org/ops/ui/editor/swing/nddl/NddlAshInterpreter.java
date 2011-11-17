package org.ops.ui.editor.swing.nddl;

import java.io.PrintStream;
import java.io.BufferedOutputStream;

import org.ops.ui.editor.swing.ash.AshConsole;
import org.ops.ui.editor.swing.ash.AshInterpreter;
import org.ops.ui.editor.swing.ash.DocumentOutputStream;

//import psengine.NddlInterpreter;
import psengine.PSEngine;

public class NddlAshInterpreter extends AshInterpreter 
{
    PrintStream consoleErr_ = System.err;
    //NddlInterpreter interpreter_ = null;

    public NddlAshInterpreter(PSEngine pse) 
    {
        super("Nddl");
        //interpreter_ = pse.getNddlInterpreter();
    }

    public void setConsole(AshConsole console) 
    {
        consoleErr_ = new PrintStream(new BufferedOutputStream(new DocumentOutputStream(console.getDocument(), "!!")), true);
        //interpreter_.setErrorStream(consoleErr_);
    }

    public void source(String filename) 
    {
        //interpreter_.source(filename);    
    }

    public boolean eval(String toEval) 
    {
    	return false;
        //return interpreter_.eval(toEval);    
    }
}
