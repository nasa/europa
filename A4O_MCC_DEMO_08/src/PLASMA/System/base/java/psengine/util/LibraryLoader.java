package psengine.util;

import java.util.List;
import java.util.ArrayList;
import java.io.File;
import java.io.IOException;

public class LibraryLoader {
  private static List tested = null;

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
      String testedList = "";
      for(int i=0; i < tested.size(); ++i)
        testedList += "\n" + tested.get(i).toString();
      throw new UnsatisfiedLinkError("no " + libname + " in java.library.path tested for:" + testedList);
    }
    System.load(resolvedLibName);
    tested = null;
  }

  public static String getResolvedName(String libname) {
    String resolvedLibName = null;
    String localName = mapLibraryName(libname);
    String[] libpath = System.getProperty("java.library.path").split(File.pathSeparator);
    tested = new ArrayList(libpath.length + 2);
    for(int i=0; i < libpath.length && resolvedLibName == null; ++i) {
      resolvedLibName = testLibName(libpath[i], localName);
    }
    // We're all nice people here... you meant to define LD_LIBRARY_PATH and just forgot right?
    if(resolvedLibName == null && System.getenv("EUROPA_HOME") != null)
      resolvedLibName = testLibName(System.getenv("EUROPA_HOME") + File.separator + "lib", localName);
    if(resolvedLibName == null && System.getenv("PLASMA_HOME") != null)
      resolvedLibName = testLibName(System.getenv("PLASMA_HOME") + File.separator + "lib", localName);

    return resolvedLibName;
  }

  private static String testLibName(String path, String localName) {
    File lib = new File(path, localName);
    tested.add(lib);
    if(lib.exists()) {
      try {
        return lib.getCanonicalPath();
      }
      catch(IOException ex) {
        return lib.getAbsolutePath();
      }
    }
    return null;
  }
}
