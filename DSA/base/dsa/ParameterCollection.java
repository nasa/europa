package dsa;

import java.util.List;

interface ParameterCollection {
    int size();
    List<Parameter> elements();
    Parameter element(int index);
}
