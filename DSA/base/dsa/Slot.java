package dsa;

interface Slot extends ParameterCollection {
    int getKey();
    int getEarliestStart();
    int getLatestStart();
    int getEarliestEnd();
    int getLatestEnd();
}
