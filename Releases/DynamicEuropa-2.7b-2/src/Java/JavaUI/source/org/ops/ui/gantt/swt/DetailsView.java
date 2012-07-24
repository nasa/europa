package org.ops.ui.gantt.swt;

import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.ScrolledComposite;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.ISelectionListener;
import org.eclipse.ui.IWorkbenchPart;
import org.ops.ui.gantt.model.EuropaGanttActivity;
import org.ops.ui.solver.swt.SolverModelViewImpl;

import psengine.PSToken;
import psengine.PSTokenList;



/**
 * Details View - SWT view to show details of a token clicked on in the gantt view.
 * 
 * @author Tristan Smith
 * 
 */
//public class DetailsView extends ViewPart implements ISelectionListener { 
public class DetailsView extends SolverModelViewImpl implements ISelectionListener { 
	public static final String VIEW_ID = "org.ops.ui.gantt.swt.DetailsView";

	private PSToken token = null;
	private Composite detailsComposite; // shown when there's a selected token
	private Composite parent = null;

	protected Composite yesComposite = null;
	protected Label noLabel = null;
	GridData noData = null; 
	GridData yesData = null;
	
	private Text idText;
	private Text typeText;
	private Text isFactText;
	private Text masterText;
	private Text slavesText;
	private Text violationText;
	
	private TableViewer parameterViewer;
	
	@Override
	public void createPartControl(final Composite parent) {
		// Sign up with GanttView so we can show details on click events
		getSite().getPage().addSelectionListener(this);

		this.parent = parent;

		parent.setLayout(new GridLayout());

		// Add one composite that will be visible
		// when no token is selected:
		noData = new GridData(SWT.CENTER, SWT.CENTER, true, false);
		noLabel = new Label(parent, SWT.NONE);
		noLabel.setText("Click on a token in Gantt Chart view to see details here.");
		noLabel.setLayoutData(noData);
		
		// Add a second composite that will be visible when
		// a token is selected:
		yesComposite = new Composite(parent,SWT.NONE);
		yesData = new GridData(SWT.FILL, SWT.FILL, true, true);
		yesComposite.setLayoutData(yesData);
		
		// We seem to need FillLayout for ScrolledComposite to work
		// (but embed a GridLayout inside that)
		yesComposite.setLayout(new FillLayout());

		createYesComposite();
	}

	// Create what you see when a token is selected:
	protected void createYesComposite() {
		
		ScrolledComposite scroll = new ScrolledComposite(yesComposite, SWT.H_SCROLL | SWT.V_SCROLL | SWT.BORDER);

		detailsComposite = new Composite(scroll, SWT.NONE);

		scroll.setContent(detailsComposite);
		scroll.setExpandHorizontal(true);
		scroll.setExpandVertical(true);
		scroll.setLayout(new FillLayout());
		
		GridLayout layout = new GridLayout(3, false);
		detailsComposite.setLayout(layout);

		detailsComposite.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));

		idText = createLabelAndText("Id", 2);
		typeText = createLabelAndText("Type", 2);
		isFactText = createLabelAndText("Fact", 2);
		masterText = createLabelAndText("Master", 2);
		slavesText = createLabelAndText("Slaves", 2);
		violationText = createLabelAndText("Violation", 2);

		// Add table of parameters:
		parameterViewer = new TableViewer(detailsComposite, SWT.MULTI | SWT.H_SCROLL
				| SWT.V_SCROLL | SWT.FULL_SELECTION);
		
		// Add columns:
		parameterViewer.getTable().setHeaderVisible(true);
		parameterViewer.getTable().setLinesVisible(true);

		final TableColumn keyColumn = new TableViewerColumn(parameterViewer, SWT.NONE).getColumn();
		keyColumn.setText("Variable");
		keyColumn.setWidth(100);

		final TableColumn valueColumn = new TableViewerColumn(parameterViewer, SWT.NONE).getColumn();
		valueColumn.setText("Value");
		valueColumn.setWidth(100);
		
		parameterViewer.setContentProvider(new ParameterTableContentProvider());
		parameterViewer.setLabelProvider(new ParameterTableLabelProvider());

		parameterViewer.setInput(token);
		
		GridData gridData = new GridData();
		gridData.verticalAlignment = GridData.FILL;
		gridData.horizontalSpan = 2;
		gridData.grabExcessHorizontalSpace = true;
		gridData.grabExcessVerticalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		parameterViewer.getControl().setLayoutData(gridData);
		
		refresh();
	}


	
	protected Text createLabelAndText(String label, int colsForText) {
		Label l = new Label(detailsComposite, SWT.NONE);
		l.setText(label + ":");

		// NOTE:  I tried adding SWT.H_SCROLL, but that appears only to work when SWT.SINGLE is *not* selected
		int style = SWT.SINGLE | SWT.BORDER | SWT.READ_ONLY;

		Text retval = new Text(detailsComposite, style);

		Color c = parent.getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);
		retval.setBackground(c);

		GridData data = new GridData(SWT.FILL, SWT.FILL, true, false);
		data.horizontalSpan = colsForText;
		retval.setLayoutData(data);
		return retval;
	}

	public void refresh() {

		if(token == null) {
			hideViewContents();
			detailsComposite.setVisible(false);
		}
		else {
			showViewContents();
			detailsComposite.setVisible(true);
			idText.setText(new Integer(token.getEntityKey()).toString());
			typeText.setText(token.getTokenType());
			isFactText.setText(new Boolean(token.isFact()).toString());
			
			PSToken master = token.getMaster();
			masterText.setText(master == null ? "--" : master.toString());

			
			String slaveText = "";
			PSTokenList slaves = token.getSlaves();
			for(int i = 0; i < slaves.size(); ++ i) {
				if(i > 0) {
					slaveText += ", ";
				}
				slaveText += slaves.get(i).toString();
			}
			slavesText.setText(slaveText);
			
			violationText.setText(token.getViolation() > 0 ? token.getViolationExpl() : "--");
			parameterViewer.setInput(token);
		}
	}

	
	private void clear() {
		token = null;
		refresh();
	}
	
	// Ie no token selected
	protected void hideViewContents() {
		noData.heightHint = -1;
		yesData.heightHint = 0;
		
		noData.exclude = false;
		yesData.exclude = true;
		
		noLabel.setVisible(true);
		yesComposite.setVisible(false);
		redraw();
	}
	
	// Ie a token is selected
	protected void showViewContents() {
		noData.heightHint = 0;
		yesData.heightHint = -1;
		
		noData.exclude = true;
		yesData.exclude = false;

		noLabel.setVisible(false);
		yesComposite.setVisible(true);
		redraw();
	}
	
	private void redraw() {
		parent.layout(true, true);
		yesComposite.layout(true, true);
		
		noLabel.redraw();
		yesComposite.redraw();
	}

	
	//*********************************************************************
	// SolverListener Interface
	//
	// For now when something gets updated, we just clear our information.
	// We could consider updating the information for the given token
	// as well.
	//*********************************************************************

	@Override
	public void solverStarted() { clear(); }

	@Override
	public void solverStopped() { clear(); }

	@Override
	public void beforeStepping() { clear(); }

	@Override
	public void afterOneStep(long time) { clear(); }

	@Override
	public void afterStepping() { clear(); }

	
	//*********************************************************************
	// SolverModelView Interface
	//
	// For now when something gets updated, we just clear our information.
	// We could consider updating the information for the given token
	// as well.
	//*********************************************************************
	@Override
	public void setModel() {
		super.setModel();
		clear();
	}
	
	//*********************************************************************
	// ISelectionListener Interface
	//*********************************************************************

	@Override
	public void selectionChanged(IWorkbenchPart part, ISelection selection) {
		this.token = null;
		if ((selection instanceof TokenSelection)
				&& (((TokenSelection) selection).getToken() instanceof EuropaGanttActivity)) {
			this.token = ((EuropaGanttActivity) ((TokenSelection) selection)
					.getToken()).getData();
		}
		refresh();
	}



}
