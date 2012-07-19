package dsa;

import java.util.*;

import net.n3.nanoxml.IXMLElement;

public interface Action 
    extends Token 
{
    public String getType();
    public List<Action> getChildActions();
    public List<Proposition> getConditions();
    public List<Proposition> getEffects();

    public boolean hasViolations();
    // @return Sum of penalty values for all violated constraints by this activity
    double getViolation();
    public List<Violation> getViolations();
}
