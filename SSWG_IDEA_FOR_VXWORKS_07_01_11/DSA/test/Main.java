import dsa.*;
import junit.framework.*;
import java.util.*;

public class Main extends TestCase {
    static String s_path = null;
    static String libVersion="_g";
    
    public Main(String str){ super(str);}

    public static void main(String [] args){
	s_path = args[0];
	
	if (args.length > 1)
		libVersion = args[1];
	
	junit.textui.TestRunner.run(new TestSuite(Main.class));
    }

    static String makeLibName(String lib)
    {
        return LibraryLoader.mapLibraryName(lib+libVersion);
    }
    
    static String makeLibPath(String lib)
    {
    	System.out.println("\n"+s_path + "/" + makeLibName(lib));
    	return s_path + "/" + makeLibName(lib);
    }
    
    public void setUp(){
	DSAManager.getInstance();
    }

    public void testModelLoading(){
	try{
	    DSAManager.getInstance().loadModel(makeLibPath("model.1"));
	    DSAManager.getInstance().addPlan(s_path + "/model.1.xml",false);
	    DSAManager.getInstance().loadModel(makeLibPath("model.2"));
	    DSAManager.getInstance().addPlan(s_path + "/model.2.xml",false);
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }
   
    public void testComponentQuery(){
	try{
	    DSAManager.getInstance().loadModel(makeLibPath("model.1"));
	    DSAManager.getInstance().addPlan(s_path + "/model.1.xml",false);
	    List<Component> components = DSAManager.getInstance().getComponents();
	    Assert.assertTrue(components.size() == 1);
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }

    public void testActionQuery(){
	try{
	    DSAManager.getInstance().loadModel(makeLibPath("model.1"));
	    DSAManager.getInstance().addPlan(s_path + "/model.1.xml",false);
	    List<Component> components = DSAManager.getInstance().getComponents();
	    ListIterator<Component> it = components.listIterator();
	    while(it.hasNext()){
		Component component = it.next();
		List<Action> actions = component.getActions();
		Assert.assertTrue(actions != null);
	    }
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }

    public void testSolverExecution(){
	try{
	    DSAManager.getInstance().loadModel(makeLibPath("model.1"));
	    DSAManager.getInstance().addPlan(s_path + "/model.1.xml",false);

	    Solver solver = SolverManager.createInstance(s_path + "/solver.1.cfg", 0, 1000, 10, 10);
	    Assert.assertTrue(solver.solve());
	    Assert.assertTrue(solver.getStepCount() == 2);
	    Assert.assertTrue(solver.getDepth() == 2);

	    // Now reset the decisions and go again
	    solver.reset();
	    Assert.assertTrue(solver.solve());
	    Assert.assertTrue(solver.getStepCount() == 2);
	    Assert.assertTrue(solver.getDepth() == 2);

	    // Now allocate again and confirm the base data. This time no work to do
	    solver = SolverManager.createInstance(s_path + "/solver.1.cfg", 0, 1000, 10, 10);
	    Assert.assertTrue(solver.solve());
	    Assert.assertTrue(solver.getStepCount() == 0);
	    Assert.assertTrue(solver.getDepth() == 0);
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }
}
