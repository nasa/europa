package dsa;

import java.util.*;
import net.n3.nanoxml.IXMLElement;

public interface Action extends Token {
    public String getType();
    public boolean hasViolations();
    public List<Action> getChildActions();
    public List<Proposition> getConditions();
    public List<Proposition> getEffects();
    public List<Violation> getViolations();
}
