package dsa;

import java.util.*;

public class TokenImpl extends Entity implements Slot, Proposition {

    public TokenImpl(int key){super(key);}

    public boolean isTrue(){return true;}

    public int getEarliestStart() {return m_earliestStart;}

    public int getLatestStart() {return m_latestStart;}

    public int getEarliestEnd() {return m_earliestEnd;}

    public int getLatestEnd() {return m_latestEnd;}

    public int getDurationMin() {return m_durationMin;}

    public int getDurationMax() {return m_durationMax;}

    public List<Parameter> getParameters(){
	return new Vector<Parameter>();
    }

    private int m_earliestStart;
    private int m_latestStart;
    private int m_earliestEnd;
    private int m_latestEnd;
    private int m_durationMin;
    private int m_durationMax;
    private List<Parameter> m_parameters;
}
