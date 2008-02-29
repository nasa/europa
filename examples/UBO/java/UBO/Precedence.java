package UBO;

import psengine.PSResource;
import psengine.PSToken;

public class Precedence
    implements Comparable
{
    public PSResource res;
    public PSToken pred;
    public PSToken succ;
    public boolean isCritical;
    public int buffer;

    public Precedence(PSResource r,PSToken p,PSToken s)
    {
        this(r,p,s,0);
        int succStart = RCPSPUtil.getUb(succ.getStart());
        int predFinish = RCPSPUtil.getLb(pred.getEnd());
        buffer = succStart-predFinish;                      
    }

    public Precedence(PSResource r,PSToken p,PSToken s, int b)
    {
        res = r;
        pred = p;
        succ = s;       
        isCritical = true;
        buffer = b;
    }   

    public int compareTo(Object o)
    {
        Precedence rhs = (Precedence)o;
        return rhs.buffer - buffer;
    }
    
    public String toString()
    {
        StringBuffer buf = new StringBuffer();
        buf.append(RCPSPUtil.getActivity(pred)).append("<").append(RCPSPUtil.getActivity(succ));
        return buf.toString();
    }
}       
