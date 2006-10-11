package dsa;

import java.util.Iterator;

public interface ResourceProfile
{
    public Iterator<Integer> getTimes();
    public double getValue(int time);
}
