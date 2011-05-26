package org.ops.ui.gantt.swing;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JViewport;

import org.ops.ui.gantt.model.EuropaGanttModel;
import org.ops.ui.gantt.model.IGanttActivity;
import org.ops.ui.gantt.model.IGanttModel;
import org.ops.ui.gantt.model.IGanttResource;
import org.ops.ui.main.swing.EuropaInternalFrame;
import org.ops.ui.solver.model.SolverAdapter;
import org.ops.ui.solver.model.SolverModel;

public class GanttView extends EuropaInternalFrame {

	/** Step size in pixels */
	protected static int stepSizePx = 20;

	/** Solver model to get data from */
	private SolverModel solverModel;
	/** Step count, computed in updateView and used for sizing right panels */
	private int stepCount = 1;

	/** Label to occupy space on the label side to match time header */
	protected JLabel headerSpaceHolder = new JLabel(" Times");
	/** Panel containing labels */
	protected JPanel labelPanel;
	/** Time header and label viewports */
	protected JViewport timeHeader, labelVPort;
	/** Panel that draws tokens - the main thing on the right side */
	protected TokenPanel tokenPanel;
	/** Scroll pane containing token panel */
	protected JScrollPane tokenScrollPane;
	/** Main split pane for the whole thing */
	protected JSplitPane splitPane;

	/** Make this thing configurable? Issue 117 */
	private boolean skipEmptyObjects = false;
	
	/** Odd and even background colors */
	protected static final Color oddBg = new Color(250, 255, 250),
			evenBg = new Color(250, 250, 150), boldGrid = Color.gray,
			smallGrid = new Color(200, 200, 200);

	private ArrayList<LinePanel> lines = new ArrayList<LinePanel>();

	public GanttView(SolverModel solverModel) {
		super("Gantt chart");
		this.solverModel = solverModel;

		splitPane = new JSplitPane();
		this.setContentPane(splitPane);

		JPanel left = new JPanel();
		left.setLayout(new BorderLayout());
		left.add(headerSpaceHolder, BorderLayout.NORTH);
		labelPanel = new LabelPanel();
		labelVPort = new JViewport();
		labelVPort.setView(labelPanel);
		left.add(labelVPort, BorderLayout.CENTER);
		splitPane.setLeftComponent(left);

		JPanel right = new JPanel();
		right.setLayout(new BorderLayout());
		timeHeader = new JViewport();
		timeHeader.setView(new TimeHeader());
		right.add(timeHeader, BorderLayout.NORTH);
		tokenPanel = new TokenPanel();
		tokenScrollPane = new JScrollPane(tokenPanel);
		right.add(tokenScrollPane, BorderLayout.CENTER);
		splitPane.setRightComponent(right);

		AdjustmentListener adjListener = new AdjustmentListener() {
			@Override
			public void adjustmentValueChanged(AdjustmentEvent event) {
				JViewport vp = tokenScrollPane.getViewport();
				Point p = vp.getViewPosition();
				timeHeader.setViewPosition(new Point(p.x, 0));
				labelVPort.setViewPosition(new Point(0, p.y - vp.getY()));
			}
		};
		tokenScrollPane.getHorizontalScrollBar().addAdjustmentListener(
				adjListener);
		tokenScrollPane.getVerticalScrollBar().addAdjustmentListener(
				adjListener);

		solverModel.addSolverListener(new SolverAdapter() {
			@Override
			public void afterStepping() {
				updateView();
				tokenPanel.repaint();
			}
		});

		updateView();
	}

	@Override
	public Dimension getFavoriteSize() {
		return new Dimension(500, 600);
	}

	protected void updateView() {
		// System.out.println("Updating Gantt");

		IGanttModel model = new EuropaGanttModel(solverModel);
		int start = model.getStart();
		int end = model.getEnd();

		stepCount = end - start + 1;

		lines.clear();
		tokenPanel.removeAll();
		labelPanel.removeAll();

		int index = 0;
		for (int i = 0; i < model.getResourceCount(); i++) {
			IGanttResource r = model.getResource(i);
			LinePanel line;
			if (r != null) {
				// Can resource timeline also have tokens?
				line = new ResourcePanel(r);				
//				. TODO
			} else {
				TimelinePanel tline = new TimelinePanel(model.getResourceName(i));
				// Skip timeline (non-resource) lines with no tokens
				List<IGanttActivity> all = model.getActivities(i);
				if (skipEmptyObjects && all.isEmpty())
					continue;
				for (IGanttActivity act : all) {
					tline.addToken(new TokenWidget(act, TokenWidget.DEFAULT_COLOR));
				}
				line = tline;
			}
			if (index++ % 2 == 0)
				line.setBackground(evenBg);
			else
				line.setBackground(oddBg);
			lines.add(line);
			tokenPanel.add(line);
			labelPanel.add(line.getLabel());
		}
		
		Point p = new Point(0,0);
		timeHeader.setViewPosition(p);
		labelVPort.setViewPosition(p);
		this.validate();
	}

	private class LabelPanel extends JPanel {
		// Noop the layout manager: the labels are placed by their timelines
		@Override
		public void doLayout() {
		}

		// Force width on all labels
		@Override
		public void setBounds(int x, int y, int width, int height) {
			super.setBounds(x, y, width, height);
			for (LinePanel tl : lines)
				if (tl.getLabel() != null) {
					Rectangle bnd = tl.getLabel().getBounds();
					bnd.width = width;
					tl.getLabel().setBounds(bnd);
				}
		}

		@Override
		public Dimension getPreferredSize() {
			Dimension res = super.getPreferredSize();
			int total = 0;
			for (LinePanel l : lines)
				total += l.getHeight();
			res.height = total;
			return res;
		}
	}

	private class TokenPanel extends JPanel {

		@Override
		public Dimension getPreferredSize() {
			int total = 0;
			for (LinePanel l : lines)
				total += l.getHeight();
			return new Dimension((stepCount - 1) * stepSizePx, total);
		}

		@Override
		public void doLayout() {
			int width = stepCount * stepSizePx;
			int[] hor = solverModel.getHorizon();
			int y = 0;
			for (LinePanel l : lines) {
				l.setBounds(0, y, width, l.getHeight());
				l.layout(stepSizePx, hor);
				y += l.getHeight();
			}
		}

		@Override
		public void paint(Graphics g) {
			super.paint(g);

			int h = getHeight();
			int x = stepSizePx;
			// Grid lines
			for (int i = 1; i < stepCount; i++, x += stepSizePx) {
				if (i % 10 == 0)
					g.setColor(boldGrid);
				else
					g.setColor(smallGrid);
				g.drawLine(x, 0, x, h);
			}
		}
	}

	private class TimeHeader extends JPanel {
		@Override
		public Dimension getPreferredSize() {
			int h = headerSpaceHolder.getPreferredSize().height;
			return new Dimension((stepCount - 1) * stepSizePx, h);
		}

		@Override
		public void paint(Graphics g) {
			super.paint(g);

			int x0 = tokenScrollPane.getViewport().getX();
			int h = headerSpaceHolder.getPreferredSize().height;
			g.setColor(Color.black);
			for (int i = 10; i < stepCount; i += 10) {
				int x = x0 + i * stepSizePx;
				g.drawLine(x, 0, x, h);
				g.drawString(String.valueOf(i), x + 2, h - 2);
			}
		}
	}
}
