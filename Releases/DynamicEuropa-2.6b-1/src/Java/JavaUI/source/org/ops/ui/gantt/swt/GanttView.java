package org.ops.ui.gantt.swt;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.FigureCanvas;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.Label;
import org.eclipse.draw2d.Panel;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Sash;
import org.ops.ui.gantt.model.GanttActivity;
import org.ops.ui.gantt.model.GanttModel;
import org.ops.ui.gantt.model.GanttResource;
import org.ops.ui.main.swt.EuropaPlugin;
import org.ops.ui.solver.swt.SolverModelViewImpl;

/**
 * Gantt chart view - SWT version for the Eclipse plugin
 * 
 * @author Tatiana Kichkaylo, Tristan Smith
 */
public class GanttView extends SolverModelViewImpl implements ISelectionProvider {
	public static final String VIEW_ID = "org.ops.ui.gantt.swt.GanttView";
	/** Step size in pixels */
	protected static int stepSizePx = 20;
	protected static int headerHeight = 20;

	private Canvas timeCanvas;
	/** Canvas for tokens. There is a <code>contents</code> panel inside */
	private FigureCanvas canvas;
	private Panel contents;
	/** Left side canvas for labels. There is a labelContents panel inside */
	private FigureCanvas labelCanvas;
	private Panel labelContents;

	/** Step count, computed in updateView and used for sizing right panels */
	private int stepCount = 1;
	private ArrayList<LinePanel> lines = new ArrayList<LinePanel>();
	/** Odd and even background colors */
	protected static final Color oddBg, evenBg, smallGrid,
			boldGrid = ColorConstants.gray;
	
	/** Make this thing configurable? Issue 117 */
	private boolean skipEmptyObjects = false;

	private Dimension largeSize = null;

	// The plugin should be initialized before this class is ever loaded
	static {
		EuropaPlugin pl = EuropaPlugin.getDefault();
		oddBg = pl.getColor(new RGB(250, 255, 250));
		evenBg = pl.getColor(new RGB(250, 250, 150));
		smallGrid = pl.getColor(new RGB(200, 200, 200));
	}
	
	/** Switch this view to the given model, possibly NULL */
	@Override
	public void setModel() {
		super.setModel();
		updateChart();
	}


	@Override
	public void createPartControl(final Composite parent) {
		final Sash sash = new Sash(parent, SWT.VERTICAL);
		labelCanvas = new FigureCanvas(parent, SWT.H_SCROLL);

		timeCanvas = new Canvas(parent, SWT.NONE);
		canvas = new FigureCanvas(parent, SWT.V_SCROLL | SWT.H_SCROLL);

		final FormLayout form = new FormLayout();
		parent.setLayout(form);

		FormData labelsData = new FormData();
		labelsData.left = new FormAttachment(0, 0);
		labelsData.right = new FormAttachment(sash, 0);
		labelsData.top = new FormAttachment(0, headerHeight);
		labelsData.bottom = new FormAttachment(100, 0);
		labelCanvas.setLayoutData(labelsData);

		final int limit = 100, percent = 10;
		final FormData sashData = new FormData();
		sashData.left = new FormAttachment(percent, 0);
		sashData.top = new FormAttachment(0, 0);
		sashData.bottom = new FormAttachment(100, 0);
		sash.setLayoutData(sashData);
		sash.addListener(SWT.Selection, new Listener() {
			@Override
			public void handleEvent(Event e) {
				Rectangle sashRect = sash.getBounds();
				Rectangle shellRect = parent.getClientArea();
				int right = shellRect.width - sashRect.width - limit;
				e.x = Math.max(Math.min(e.x, right), limit);
				if (e.x != sashRect.x) {
					sashData.left = new FormAttachment(0, e.x);
					parent.layout();
				}
			}
		});

		FormData rightData = new FormData();
		rightData.left = new FormAttachment(sash, 0);
		rightData.right = new FormAttachment(100, 0);
		rightData.top = new FormAttachment(0, 0);
		rightData.bottom = new FormAttachment(0, headerHeight);
		timeCanvas.setLayoutData(rightData);

		rightData = new FormData();
		rightData.left = new FormAttachment(sash, 0);
		rightData.right = new FormAttachment(100, 0);
		rightData.top = new FormAttachment(0, headerHeight);
		rightData.bottom = new FormAttachment(100, 0);
		canvas.setLayoutData(rightData);

		timeCanvas.addPaintListener(new PaintListener() {
			@Override
			public void paintControl(PaintEvent event) {
				paintTimeHeader(event);
			}
		});
		canvas.getHorizontalBar().addListener(SWT.Selection, new Listener() {
			@Override
			public void handleEvent(Event e) {
				timeCanvas.redraw();
			}
		});
		canvas.getVerticalBar().addListener(SWT.Selection, new Listener() {
			@Override
			public void handleEvent(Event e) {
				int x = labelCanvas.getViewport().getViewLocation().x;
				int y = canvas.getViewport().getViewLocation().y;
				labelCanvas.getViewport().setViewLocation(x, y);
				// labelCanvas.redraw();
			}
		});

		contents = new Panel() {
			@Override
			public void paint(Graphics g) {
				super.paint(g);
				paintGrid(g);
			}

			@Override
			public Dimension getPreferredSize(int wh, int hh) {
				if (largeSize == null)
					return new Dimension(10, 10);
				return largeSize;
			}
		};
		contents.setLayoutManager(null);
		canvas.setContents(contents);

		labelContents = new Panel() {
			@Override
			public Dimension getPreferredSize(int wh, int hh) {
				Dimension p = this.getParent().getSize();
				if (largeSize != null)
					p.height = largeSize.height;
				return p;
			}

			@Override
			public void setBounds(org.eclipse.draw2d.geometry.Rectangle rect) {
				super.setBounds(rect);
				for (LinePanel lp : lines) {
					Label lbl = lp.getLabel();
					org.eclipse.draw2d.geometry.Rectangle b = lbl.getBounds();
					b.width = rect.width;
					lbl.setBounds(b);
				}
				// The v scroll bar keeps reappearing, so we keep hiding it
				labelCanvas.getVerticalBar().setVisible(false);
			}
		};
		labelContents.setLayoutManager(null);
		labelCanvas.setContents(labelContents);

		// Kill vertical scroll bar for labels (need to reaffirm)
		labelCanvas.getVerticalBar().setEnabled(false);
		labelCanvas.getVerticalBar().setVisible(false);

		// Redraw the view
		setModel();
		updateChart();
		
		// Register so we can report when user selects tokens:
		getSite().setSelectionProvider(this);

	}

	@Override
	public void afterStepping() {
		updateChart();
	}

	@Override
	public void solverStarted() {
		updateChart();
	}

	@Override
	public void solverStopped() {
		updateChart();
	}

	private void doUpdate() {
		contents.removeAll();
		lines.clear();
		labelContents.removeAll();
		
		if (model == null || model.isTerminated()) {
			stepCount = 0;
			largeSize = null;
			return;
		}

		GanttModel gmodel = new GanttModel(model);
		int start = gmodel.getStart();
		int end = gmodel.getEnd();

		stepCount = end - start + 1;

		int index = 0;
		for (int i = 0; i < gmodel.getResourceCount(); i++) {
			GanttResource r = gmodel.getResource(i);
			LinePanel line = null;
			if (r != null) {
				// Can resource timeline also have tokens?
				line = new ResourcePanel(r);
			} else {
				TimelinePanel tline = new TimelinePanel(gmodel
						.getResourceName(i));
				// Skip timeline (non-resource) lines with no tokens
				List<GanttActivity> all = gmodel.getActivities(i);
				if (skipEmptyObjects && all.isEmpty())
					continue;
				for (GanttActivity act : all) {
					tline.addToken(new TokenWidget(act,
							TokenWidget.DEFAULT_COLOR, this));
				}
				line = tline;
			}
			if (index++ % 2 == 0)
				line.setBackgroundColor(evenBg);
			else
				line.setBackgroundColor(oddBg);
			lines.add(line);
			contents.add(line);
			labelContents.add(line.getLabel());
		}

		this.doLayout();
	}

	protected void updateChart() {
		Display.getDefault().asyncExec(new Runnable() {
			@Override
			public void run() {
				canvas.getViewport().setViewLocation(0, 0);
				labelCanvas.getViewport().setViewLocation(0, 0);
				doUpdate();
				canvas.redraw();
				timeCanvas.redraw();
			}
		});
	}

	public void doLayout() {
		int width = stepCount * stepSizePx;
		int[] hor = model.getHorizon();
		int y = 0;
		for (LinePanel l : lines) {
			l.setBounds(new org.eclipse.draw2d.geometry.Rectangle(0, y, width,
					l.getHeight()));
			l.layout(stepSizePx, hor);
			y += l.getHeight();
		}
		largeSize = new Dimension(stepCount * stepSizePx, y);
	}

	protected void paintTimeHeader(PaintEvent event) {
		int h = headerHeight;
		int x0 = -canvas.getViewport().getViewLocation().x;
		event.gc.setForeground(ColorConstants.black);
		for (int i = 10; i < stepCount; i += 10) {
			int x = x0 + i * stepSizePx;
			event.gc.drawLine(x, 0, x, h);
			event.gc.drawString(String.valueOf(i), x + 2, 2);
		}
	}

	protected void paintGrid(Graphics gc) {
		if (largeSize == null)
			return;
		int h = largeSize.height;
		int x = stepSizePx;
		// Grid lines
		for (int i = 1; i < stepCount; i++, x += stepSizePx) {
			if (i % 10 == 0)
				gc.setForegroundColor(boldGrid);
			else
				gc.setForegroundColor(smallGrid);
			gc.drawLine(x, 0, x, h);
		}
	}


	// --------------------------------------------------------------------------
	// REGISTERING AS A SELECTION PROVIDER
	// --------------------------------------------------------------------------
	private Set<ISelectionChangedListener> selectionChangedListeners = new HashSet<ISelectionChangedListener>();

	@Override
	public void addSelectionChangedListener(ISelectionChangedListener listener) {
		selectionChangedListeners.add(listener);
	}

	@Override
	public ISelection getSelection() {
		// For now only selection when clicked
		return null;
	}


	@Override
	public void removeSelectionChangedListener(
			ISelectionChangedListener listener) {
		selectionChangedListeners.remove(listener);
	}

	@Override
	public void setSelection(ISelection selection) {
		for (ISelectionChangedListener listener : selectionChangedListeners ) {
			listener.selectionChanged(new SelectionChangedEvent(
					this, selection/*getSelection()*/));				
		}
	}
	
}
