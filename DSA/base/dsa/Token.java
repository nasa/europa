package dsa;

import java.util.List;

interface Token {
    int getKey();
    int getEarliestStart();
    int getLatestStart();
    int getEarliestEnd();
    int getLatestEnd();
    int getDurationMin();
    int getDurationMax();
    List<Parameter> getParameters();
}
