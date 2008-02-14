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
            String predAct = RCPSPUtil.varValueToString(pred.getParameter("m_identifier"));
            String succAct = RCPSPUtil.varValueToString(succ.getParameter("m_identifier"));

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
            String decisionMsg = s.getStepCount() + ": "+ decision + " because of " + resource.getEntityName() + tokenDecision;
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
} 

