package dsa;

import java.net.*;
import java.io.*;
import java.awt.Toolkit;

public class SocketListener {
    public static void main(String [] args){
	Toolkit.getDefaultToolkit().beep();

	try {
	    System.getSecurityManager().checkAccept("localhost", 1200);
	}
	catch(SecurityException e){
	    Toolkit.getDefaultToolkit().beep();
	}

	try{
	    String modelSource = args[0];
	    System.load(modelSource);
	    System.out.println("Attempting to load " + modelSource);

	    ServerSocket server = new ServerSocket(1200);
	    Socket connection = server.accept();

	    System.out.println("Connection made");

	    BufferedReader in = new BufferedReader( new InputStreamReader(connection.getInputStream()));
	    String inputLine;

	    while((inputLine = in.readLine()) != null){
		System.out.println(inputLine);
	    }

	    connection.close();
	}
	catch(Exception e){
	    e.printStackTrace();
	}
    }
}
