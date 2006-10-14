package dsa;

import java.util.Iterator;

public interface ResourceProfile
{
    public Iterator<Integer> getTimes();
    public double getLowerBound(int time);
    public double getUpperBound(int time);
}
