package org.ops.ui.gantt.swing;

import java.awt.Color;
import org.ops.ui.gantt.model.PSGanttActivity;

public interface PSGanttColorProvider 
{
    public Color getColor(PSGanttActivity activity);
}
