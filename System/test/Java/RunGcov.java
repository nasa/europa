import java.io.*;
import java.util.*;

public class RunGcov {

    private static String OUTPUT_FILE_NAME_DEFAULT = "europa-gcov-results.txt";
    private static String INPUT_DIRECTORY_DEFAULT = ".";
    private static String NO_RESULT = "NORESULT";

    public static void runGcov( String outputFileName, String inputDirectory ) {
   
        System.out.println("Identifying set of source code files");
	Vector sourceCodeFiles = getSourceCodeFiles ( new File(inputDirectory) );
        System.out.println("Identified " + sourceCodeFiles.size() + " files");
        System.out.println();
       
        System.out.println("Running gcov on each source code file");
        Vector results = applyGcov(inputDirectory, sourceCodeFiles);
        System.out.println();
       
        System.out.println("Writing results table to file");
        writeResultsToFile(results, outputFileName);
        System.out.println("Finished. Results were written to " + outputFileName);

    }

  
    private static Vector getSourceCodeFiles (File directory ) {
        System.out.println("Processing directory " + directory.toString() );
	Vector filesInDir = new Vector();
      
        FilenameFilter dotCCFilter = new FilenameFilter() {
          public boolean accept(File dir, String name) {
             return name.endsWith(".cc");
           }
        };

        FilenameFilter dotHHFilter = new FilenameFilter() {
          public boolean accept(File dir, String name) {
             return name.endsWith(".hh");
           }
	};

        String [] dotCCFiles = directory.list( dotCCFilter );
        String [] dotHHFiles = directory.list( dotHHFilter );

	// store files into vector. 

        for (int i=0; i < dotCCFiles.length; i++) {
            filesInDir.add ( directory.getPath() + "/" + dotCCFiles[i] );
            //System.out.println( directory.getPath() + "/" + dotCCFiles[i] );
        }

       for (int i=0; i < dotHHFiles.length; i++) {
            filesInDir.add ( directory.getPath() + "/" + dotHHFiles[i] );
            //System.out.println( directory.getPath() + "/" + dotHHFiles[i] );
       }

       // process all sub directories.
       String [] allFiles = directory.list();
      
       for (int i=0; i < allFiles.length; i++) {
	   File theFile = new File ( directory.getPath() + "/" + allFiles[i] );
           // only process if it is a directory
           if (theFile.isDirectory() && !allFiles[i].equalsIgnoreCase(".") && !allFiles[i].equalsIgnoreCase("..") &&  !allFiles[i].equalsIgnoreCase("cvs") &&
               !allFiles[i].equalsIgnoreCase("documentation")) {
	       Vector v =  getSourceCodeFiles( theFile );
               filesInDir.addAll(v);
           }
       }       


        return filesInDir;
 
    };


    private static Vector applyGcov(String inputDirectory, Vector files) {
        Vector results = new Vector();
 
        Iterator it = files.iterator();
        while (it.hasNext()) {
	    String fileName = (String) it.next();

            if ( blockFileExists(inputDirectory, fileName) ) {
		String cmd = "gcov -o " + inputDirectory + " " + fileName;
                String result = runCommand( cmd , fileName );
       
                if (! result.equalsIgnoreCase(NO_RESULT)) {  
                  results.add( result );
		}
            } else {
		results.add (fileName + " has no gcov data");
            }
        }
       
        return results;

    }
 
    private static boolean blockFileExists(String inputDirectory, String fileName) {
       
        String justFileName = fileName.substring( fileName.lastIndexOf("/") + 1, fileName.lastIndexOf(".") );       
	File f = new File (inputDirectory + "/" + justFileName + ".bb");
	return f.exists();

    }



  private static void writeResultsToFile( Vector results, String outputFileName) {
      try {
        BufferedWriter out = new BufferedWriter(new FileWriter( outputFileName));
       
        Iterator it = results.iterator();
        while (it.hasNext()) {
	  out.write( (String) it.next());
          out.newLine();
        }
        out.close();
      }	catch (IOException e) {
	  System.out.println("Error writing results to file " + outputFileName);
      }

  }



    private static String runCommand(String command, String fileName) {
	String result = NO_RESULT;
     
         try {
            System.out.println("Executing command " + command);
 
          
	    Process p = Runtime.getRuntime().exec(command);
       

            String ls_str;
            DataInputStream ls_in = new DataInputStream(
                                          p.getInputStream());

            try {
                while ((ls_str = ls_in.readLine()) != null) {
                
                  if (ls_str.toLowerCase().indexOf( fileName.toLowerCase() ) != -1) {
                      result = ls_str;
		  }

                }
            } catch (IOException e) {
                System.exit(0);
            }

        } catch (Exception e) {
            System.out.println(" Error executing " + command);
            System.out.println(e);
        }

	return result;
    }


        // Main program. Check arguments then call processing.
 
        public static void main(String argv[]) {
		
		 int numArgs = argv.length;
		 if ( numArgs <= 2 ) {
			 
			 System.out.println ("EUROPA GCOV script ");
                         System.out.println();
                        			 
                         String outputFileName = OUTPUT_FILE_NAME_DEFAULT;
                         String inputDirectory = INPUT_DIRECTORY_DEFAULT;

                         if ( numArgs == 1 ) {
			   outputFileName = argv[0];
                           System.out.println ( "Output filse set to: " + outputFileName );
                           System.out.println ( "Input directory set to default value: " + inputDirectory );
                         } else if ( numArgs == 2 ) {
			   outputFileName = argv[0];
                           inputDirectory = argv[1];
                           System.out.println ( "Output filse set to: " + outputFileName );
                           System.out.println ( "Input directory set to: " + inputDirectory );
                         } else {
                           System.out.println ( "Output filse set to default value: " + outputFileName );
                           System.out.println ( "Input directory set to default value: " + inputDirectory );
                         }
			 
                        // we can only proceed if the input directory exists
			 
			 File inputRootFile = new File( inputDirectory );
			 
			 if ( inputRootFile.exists() && inputRootFile.isDirectory() ) {
                  
			     runGcov( outputFileName, inputDirectory ); 
				 
			 } else {
				 
				 System.out.println ("Error: input directory " + inputDirectory + " does not exist.");
				 
			 } // end if inputRootFile exists and isDirectory.
				 
				 
		 } else {
			 
			 System.out.println ("Usage RunGcov [output file name] [path to gcov files]. Both arguments are optional");
			 
		 } // end if numArgs <= 2 
		 
	 }

}
