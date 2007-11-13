package UBO;

import java.util.Vector;
import psengine.*;
import org.ops.ui.PSDesktop;
import org.ops.ui.solver.*;
import java.util.regex.*;

public class UBOSolverListener
    implements PSSolverDialogListener
{
    int lastDepth_;
    Vector<String> plan_;
    Vector decisionHistory_;
    Vector planHistory_;
    Vector<Integer> treeSize_;
    Vector currentTree_;
    
    public UBOSolverListener()
    {
        lastDepth_ = 0;
        plan_ = new Vector<String>();
        decisionHistory_ = new Vector();
        planHistory_ = new Vector();
        treeSize_ = new Vector();
        currentTree_ = new Vector();
    }
    
    PSEngine getPSEngine()
    {
    	return PSDesktop.desktop.getPSEngine();
    }
    
    public Vector getDecisionHistory() { return decisionHistory_; }
    public Vector getPlanHistory() { return planHistory_; }
    
    protected PlanHistoryEntry makePlanHistoryEntry(PSSolver s)
    {
        // Compute remaining nodes to explore
        long size = 1;
        for (Integer i : treeSize_) {
            if (i != null)
                size *= i;
        }        
        
        PlanHistoryEntry entry = new PlanHistoryEntry(s.getStepCount(),size,currentTree_.toString(),plan_.toString());

        return entry;
    }
    
    protected void decreasePlanSize(int delta)
    {
    	int newSize = Math.max(0,plan_.size()-delta);
    	
        plan_.setSize(newSize);
        treeSize_.setSize(newSize);
        currentTree_.setSize(newSize);
    }
    
    public void stepCompleted(PSSolver s)
    {
        String lastDecision = s.getLastExecutedDecision();        
        if (lastDecision.startsWith("INSTANT") && (lastDepth_ <= s.getDepth())) {
            Pattern p = Pattern.compile("token=[0-9]+");
            Matcher m = p.matcher(lastDecision);
            m.find();
            Integer predId = new Integer(lastDecision.substring(m.start()+6,m.end()));
            m.find();
            Integer succId = new Integer(lastDecision.substring(m.start()+6,m.end()));
            String tokenDecision = " tokens:{" + predId + "} < {" + succId +"}";
            
            PSObject resource = getPSEngine().getTokenByKey(predId).getOwner();
            PSToken pred = getPSEngine().getTokenByKey(predId).getMaster();
            PSToken succ = getPSEngine().getTokenByKey(succId).getMaster();
            String predAct = varValueToString(pred.getParameter("m_identifier"));
            String succAct = varValueToString(succ.getParameter("m_identifier"));

            // Get info to compute max remaining size of search tree
            Pattern p1 = Pattern.compile("CHOICE=[0-9]+");
            Matcher m1 = p1.matcher(lastDecision);
            m1.find();
            Integer choiceIdx = new Integer(lastDecision.substring(m1.start()+7,m1.end()));
            m1.find();
            Integer choiceCnt = new Integer(lastDecision.substring(m1.start()+7,m1.end()));
            Integer remaining = choiceCnt - choiceIdx;

            StringBuffer decisionBuf = new StringBuffer();
            decisionBuf.append("{").append(predAct).append("} < {").append(succAct).append("}");
            String decision = decisionBuf.toString();
            String decisionMsg = s.getStepCount() + ": "+ decision + " because of " + resource.getName() + tokenDecision;
            decisionHistory_.add(decisionMsg);
            //System.out.println(decisionMsg);            

            plan_.add(decision);
            treeSize_.add((remaining > 0) ? remaining : new Integer(1));
            currentTree_.add("("+choiceIdx+","+choiceCnt+")");
            PlanHistoryEntry entry = makePlanHistoryEntry(s);
            planHistory_.add(entry);        
            //System.out.println(entry);           
        }        
        
        if (lastDepth_ >= s.getDepth()) {
            String btMsg = s.getStepCount() + ": Backtracked! from "+plan_.size()+" to "+s.getDepth();
            decisionHistory_.add(btMsg);
            //System.out.println(btMsg); 

            decreasePlanSize(lastDepth_ - s.getDepth());
            PlanHistoryEntry entry = makePlanHistoryEntry(s);
            planHistory_.add(entry);        
            //System.out.println(entry);           
        }

        lastDepth_ = s.getDepth(); 
        //System.out.println(s.getStepCount()+" set last depth to:"+lastDepth_);
    }
    
    public static String valueToString(PSVarValue v)
    {
    	String type = v.getType().toString();
    	
    	if ("STRING".equals(type))
    		return v.asString();
    	if ("INTEGER".equals(type))
    		return new Integer(v.asInt()).toString();	
    	if ("DOUBLE".equals(type))
    		return new Double(v.asDouble()).toString();
    	if ("BOOLEAN".equals(type))
    		return new Boolean(v.asBoolean()).toString();
    	if ("OBJECT".equals(type))
    		return v.asObject().getName();
    	
    	return "ERROR!!! UNKNOWN TYPE :" + type;
    }

    public static String varValueToString(PSVariable var)
    {	
    	if (var.isSingleton()) 
    		return valueToString(var.getSingletonValue());	
    	else if (var.isInterval()) {
    	    StringBuffer buf = new StringBuffer();
    		buf.append("[").append(var.getLowerBound()).append(",")
    		               .append(var.getUpperBound()).append("]");
    		return buf.toString();
    	}
    	else if (var.isEnumerated()) {
    		PSValueList l = var.getValues();
    	    StringBuffer buf = new StringBuffer();
    	    buf.append("[");
    	    for (int i=0;i<l.size();i++) {
    	    	if (i>0)
    	    		buf.append(",");
    	    	buf.append(valueToString(l.get(i)));
    	    }
    	    buf.append("]");
    	    return buf.toString();
    	}
    	
    	throw new RuntimeException("Unexpected ERROR: variable "+var.getName()+" is not one of {Singleton, Interval, Enumeration}");
    }    
} 

