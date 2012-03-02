package org.europa.engine;

import java.io.Reader;

public interface LanguageInterpreter 
{
	public String interpret(Reader input, String inputName);
}
