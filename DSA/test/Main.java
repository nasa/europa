import dsa.*;
import org.junit.runner.*;
import junit.framework.*;
import java.util.*;

public class Main extends TestCase {
    static String s_path = null;

    public Main(String str){ super(str);}

    public static void main(String [] args){
	s_path = args[0];
	junit.textui.TestRunner.run(new TestSuite(Main.class));
    }

    public void setUp(){
	DSA.instance();
    }

    public void testModelLoading(){
	try{
	    DSA.instance().loadModel(s_path + "libmodel.1_g.so");
	    DSA.instance().addPlan(s_path + "model.1.xml");
	    DSA.instance().loadModel(s_path + "libmodel.2_g.so");
	    DSA.instance().addPlan(s_path + "model.2.xml");
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }

    public void testComponentQuery(){
	try{
	    DSA.instance().loadModel(s_path + "libmodel.1_g.so");
	    DSA.instance().addPlan(s_path + "model.1.xml");
	    List<Component> components = DSA.instance().getComponents();
	    Assert.assertTrue(components.size() == 1);
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }

    public void testSolverExecution(){
	try{
	    DSA.instance().loadModel(s_path + "libmodel.1_g.so");
	    DSA.instance().addPlan(s_path + "model.1.xml");

	    Solver solver = Solver.createInstance(s_path + "solver.1.cfg", 0, 1000, 10, 10);
	    Assert.assertTrue(solver.solve());
	    Assert.assertTrue(solver.getStepCount() == 2);
	    Assert.assertTrue(solver.getDepth() == 2);

	    // Now reset the decisions and go again
	    solver.reset();
	    Assert.assertTrue(solver.solve());
	    Assert.assertTrue(solver.getStepCount() == 2);
	    Assert.assertTrue(solver.getDepth() == 2);

	    // Now allocate againa and confirm the base data. This time no work to do
	    solver = Solver.createInstance(s_path + "solver.1.cfg", 0, 1000, 10, 10);
	    Assert.assertTrue(solver.solve());
	    Assert.assertTrue(solver.getStepCount() == 0);
	    Assert.assertTrue(solver.getDepth() == 0);
	}
	catch(Exception e){
	    Assert.assertTrue(false);
	}
    }

}
