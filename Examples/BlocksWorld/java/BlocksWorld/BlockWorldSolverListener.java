package BlocksWorld;

import java.util.List;
import java.util.StringTokenizer;
import java.util.Comparator;
import java.util.Collections;
import psengine.*;
import org.ops.ui.solver.*;
import org.ops.ui.util.Util;

public class BlockWorldSolverListener
    implements PSSolverDialogListener
{
	protected PSEngine psengine_;
	protected BlockWorldHistory history_;

    public BlockWorldSolverListener(PSEngine pse,BlockWorldHistory h)
    {
    	psengine_ = pse;
    	history_ = h;
    }
    
    protected String getBlockName(String s)
    {
        StringTokenizer tok = new StringTokenizer(s,".");
        
        // name is second token 
        tok.nextToken();
        return tok.nextToken();
    }
    
    protected PSToken getLastAction(List actions)
    {
    	PSToken last = (PSToken)actions.get(actions.size()-1);
    	return last;
    }
    
    static class ActionComparator implements Comparator
    {
        public int compare(Object lhs, Object rhs) 
        { 
        	return (int) (((PSToken)lhs).getStart().getLowerBound() - ((PSToken)rhs).getStart().getLowerBound()); }
    }
    
    ActionComparator actionComparator_ = new ActionComparator();
    
    protected String getHistory(PSObject c)
    {
    	StringBuffer buf = new StringBuffer();
    	
        List actions = Util.SWIGList(c.getTokens()); 
        Collections.sort(actions,actionComparator_); 
    	for (Object o : actions) {
    		PSToken a = (PSToken)o;
    		buf.append("{").append(a.getEntityName()) //append(a.getEntityName().substring(9,a.getEntityName().length()))
    		   .append(BlockWorld.getBounds(a))
    		   .append("(");
    		PSVariableList parameters = a.getParameters();
    		for (int i=5; i<parameters.size(); i++) {
    		    PSVariable p = parameters.get(i);
    		    if (i>5) buf.append(",");
    		    String value = BlockWorld.varValueToString(p);
                buf//.append(p.getEntityName().substring(10,p.getEntityName().length())).append("=")
                   //.append(p.getSingletonValue().asString().substring(23,p.getSingletonValue().asString().length()-6));
                   .append(value.substring(10,value.length()));
    		}
    		buf.append("} ");
    	}    	
    	
    	return buf.toString();
    }
    
    public void stepCompleted(PSSolver s)
    {
    	BlockWorld bw = new BlockWorld();
    	PSObjectList components = psengine_.getObjectsByType("Timeline");
    	String operatorHistory = null;
    	
    	for (int i=0;i<components.size(); i++) {
    		PSObject c = components.get(i);
    		if (c.getEntityName().endsWith("Bottom")) {
    			String blockName = getBlockName(c.getEntityName());
    			blockName = blockName.substring(0,blockName.length()-5);
                List actions = Util.SWIGList(c.getTokens()); 
                Collections.sort(actions,actionComparator_); 
    			if (actions.size()==0)
    				continue;

    			PSToken a = getLastAction(actions);

    			String blockState=null;
    			String bottomBlock=null;

    			if (a.getEntityName().endsWith("OnTable")) {
    				blockState = "OnTable";
    			}
    			else if (a.getEntityName().endsWith("On")) {
    				blockState = "On";
    				bottomBlock = BlockWorld.varValueToString(a.getParameters().get(5));
    				int start = bottomBlock.lastIndexOf('.');
    				bottomBlock = bottomBlock.substring(start+1, bottomBlock.length()-5);
    			}
    			
    			if (blockState != null)	
    			    bw.addBlock(blockName,blockState,bottomBlock,a);
    		}	
    		
    		if (c.getEntityName().endsWith("operatorTL"))
    			operatorHistory = getHistory(c);
    	}
    	
    	history_.add(s.getStepCount(), bw, operatorHistory);    
    }    
}
