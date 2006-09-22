package dsa;

import java.util.*;

public class ActionImpl extends TokenImpl implements Action {
    public ActionImpl(String type, int key){
	super(key);
	m_type = type;
    }

    public String getType(){return m_type;}

    public boolean hasViolations(){return false;}

    public List<Action> getChildActions(){return null;}

    public List<Proposition> getConditions(){return null;}

    public List<Proposition> getEffects(){return null;}

    public List<Violation> getViolations(){return null;}

    private String m_type;
}
