package org.ops.ui.gantt.swing;

import org.ops.ui.gantt.model.GanttActivity;

public interface GanttContextProvider {
	public String getContext(GanttActivity activity);
}
