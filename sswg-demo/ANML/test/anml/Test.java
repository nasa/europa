package anml;

import java.io.FileInputStream;

public class Test 
{
    public static void main(String args[])
    {
    	try {
    		String filename = args[0];

    		FileInputStream input = new FileInputStream(filename);
    		ANMLLexer lexer = new ANMLLexer(input); // attach lexer to the input stream        
    		ANMLParser parser = new ANMLParser(lexer); // Create parser attached to lexer
    		// start up the parser by calling the rule at which you want to begin parsing.
    		parser.anml_program();
    	}
    	catch (Exception e) {
    		throw new RuntimeException(e);
    	}
    }	
}
