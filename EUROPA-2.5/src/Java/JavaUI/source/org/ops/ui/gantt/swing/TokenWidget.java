package org.ops.ui.gantt.swing;

import java.awt.Color;
import java.awt.Graphics;

import javax.swing.JPanel;

import org.ops.ui.gantt.model.GanttActivity;

public class TokenWidget extends JPanel {
	public static final int halfHeight = 5;

	private static final Color startC = new Color(100, 255, 120);
	private static final Color endC = new Color(100, 240, 255);
	private static final Color durC = new Color(100, 140, 155);
	public static final TokenColor DEFAULT_COLOR = new TokenColor(startC, endC,
			durC, new Color(0, 100, 0));

	private GanttActivity activity;
	private TokenColor color;

	public TokenWidget(GanttActivity activity, TokenColor color) {
		this.activity = activity;
		this.color = color;

		StringBuffer b = new StringBuffer();
		b.append(activity.getText());
		b.append(" s=[").append(activity.getStartMin()).append(",").append(
				str(activity.getStartMax()));
		b.append("] d=[").append(activity.getDurMin()).append(",").append(
				str(activity.getDurMax()));
		b.append("] e=[").append(activity.getEndMin()).append(",").append(
				str(activity.getEndMax()));
		b.append("]");
		this.setToolTipText(b.toString());
		this.setBackground(color.body);
	}
	
	private static String str(int num) {
		if (num == Integer.MAX_VALUE)
			return "inf";
		return String.valueOf(num);
	}

	@Override
	public void paint(Graphics g) {
		super.paint(g);
		int x0 = activity.getStartMin();
		paintBox(g, 0, color.start, x0, activity.getStartMax());
		if (TimelinePanel.showDurationLine) {
			// Zero to min is background, min to max is colored, duration beyond
			// max is cleared to background color
			paintBox(g, halfHeight, color.duration, activity.getDurMin() + x0,
					activity.getDurMax() + x0);
			int dms = activity.getDurMax() + x0; // duration max shifted
			// If max duration is finite, clear the end of the token to
			// background. MAX_INT will overflow into negative numbers
			if (dms > 0 && dms < Integer.MAX_VALUE)
				paintBox(g, halfHeight, this.getParent().getBackground(), dms,
						activity.getEndMax());
			paintBox(g, halfHeight * 2, color.end, activity.getEndMin(),
					activity.getEndMax());
		} else
			paintBox(g, halfHeight, color.end, activity.getEndMin(), activity
					.getEndMax());
	}

	private void paintBox(Graphics g, int y, Color col, int min, int max) {
		int tickSize = GanttView.stepSizePx;

		int x1 = (min - activity.getStartMin()) * tickSize;
		int x2 = (max - activity.getStartMin()) * tickSize;

		// Truncate ends
		if (x1 < 0)
			x1 = 0;
		if (x2 < 0 || x2 > this.getWidth())
			x2 = this.getWidth();

		g.setColor(col);
		g.fillRect(x1, y, x2 - x1, halfHeight);
	}

	public void place(int y, int stepSize, int[] hor) {
		int x = Math.max(activity.getStartMin(), hor[0]);
		int w = Math.min(activity.getEndMax(), hor[1]) - x;
		int h = halfHeight * (2 + (TimelinePanel.showDurationLine ? 1 : 0));
		setBounds(x * stepSize, y + halfHeight * 2, w * stepSize, h);
	}

	public GanttActivity getActivity() {
		return this.activity;
	}

	@Override
	public String toString() {
		return activity.getText() + this.getBounds();
	}
}
