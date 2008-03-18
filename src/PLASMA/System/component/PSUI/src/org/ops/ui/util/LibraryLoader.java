package org.ops.ui.util;

import java.io.File;
import java.io.IOException;

public class LibraryLoader {

  // This class only contains static functions.
  private LibraryLoader() {}

  public static String mapLibraryName(String libname) {
    String osname = System.getProperty("os.name");
    String localName = System.mapLibraryName(libname);
    if(osname.equals("Mac OS X"))
      localName = localName.replaceAll("jnilib$", "dylib");
    return localName;
  }

  public static void loadLibrary(String libname) {
	  String resolvedLibName = getResolvedName(libname);
	  if(resolvedLibName == null) {
		  throw new UnsatisfiedLinkError("no " + libname + " in java.library.path");
	  }
      System.load(resolvedLibName);
  }

  public static String getResolvedName(String libname) {
	  String resolvedLibName = null;
	  String localName = mapLibraryName(libname);
	  String[]libpath = System.getProperty("java.library.path").split(File.pathSeparator);
	  for(int i=0; i<libpath.length; ++i) {
		  File lib = new File(libpath[i], localName);
		  if(lib.exists()) {
			  try {
				  resolvedLibName = lib.getCanonicalPath();
			  }
			  catch(IOException ex) {
				  resolvedLibName = lib.getAbsolutePath();
			  }
			  break;
		  }
	  }
	  return resolvedLibName;
  }
}
