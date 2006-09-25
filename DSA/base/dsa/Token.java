package dsa;

import java.util.List;

public interface Token {
    int getKey();
    String getType();
    int getEarliestStart();
    int getLatestStart();
    int getEarliestEnd();
    int getLatestEnd();
    int getDurationMin();
    int getDurationMax();
    List<Parameter> getParameters();
}
