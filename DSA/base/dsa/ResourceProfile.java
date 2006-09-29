package dsa;

import java.util.List;

public interface ResourceProfile
{
    public List<Instant> getInstants();
    public double getValue(Instant i);
}
