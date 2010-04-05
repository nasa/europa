package org.ops.ui.gantt.swing;

import java.util.ArrayList;

/**
 * Single resource time line, may include multiple rows of tokens. Rows are
 * ordered by the earliest token start, ties are broken by the order of arrival.
 * 
 * @author Tatiana Kichkaylo
 */
public class TimelinePanel extends LinePanel {
	/**
	 * Whether or not we have a three line token representation - with duration
	 * in the middle
	 */
	public static boolean showDurationLine = true;

	/**
	 * One full line below, one above, half for start, half for end, and maybe
	 * half for duration
	 */
	private static int lineHeight = TokenWidget.halfHeight
			* (6 + (showDurationLine ? 1 : 0));

	ArrayList<ArrayList<TokenWidget>> lines = new ArrayList<ArrayList<TokenWidget>>();

	public TimelinePanel(String resourceName) {
		super(resourceName);
		this.lines.add(new ArrayList<TokenWidget>());
	}

	@Override
	public int getHeight() {
		return lines.size() * lineHeight;
	}

	public void addToken(TokenWidget token) {
		// Earliest start of the new widget
		int es = token.getActivity().getStartMin();
		ArrayList<TokenWidget> line = null;

		// Find a line where the token fits
		int index = 0;
		while (index < lines.size()) {
			line = lines.get(index);
			if (line.isEmpty())
				break;
			TokenWidget last = line.get(line.size() - 1);
			if (last.getActivity().getEndMax() < es) // <= ?
				break;
			index++;
		}

		if (index >= lines.size()) {
			line = new ArrayList<TokenWidget>();
			
			// Insert so that the earliest token starts are sorted
			int mine = token.getActivity().getStartMin();
			for (index=0; index<lines.size(); index++) {
				int theirs = lines.get(index).get(0).getActivity().getStartMin();
				if (mine < theirs) {
					lines.add(index, line);
					break;
				}
			}
			if (index >= lines.size())
				lines.add(line); // add to the end
		} // else line is already what we are looking for
		assert (line != null);

		line.add(token);
		this.add(token);
	}

	@Override
	public void layout(int stepSize, int[] hor) {
		for (int l = 0; l < lines.size(); l++) {
			ArrayList<TokenWidget> line = lines.get(l);
			int y = l * lineHeight;
			for (TokenWidget tok : line)
				tok.place(y, GanttView.stepSizePx, hor);
		}
	}
}
