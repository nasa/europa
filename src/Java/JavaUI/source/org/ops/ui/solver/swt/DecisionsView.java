package org.ops.ui.solver.swt;

import java.util.ArrayList;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.part.ViewPart;
import org.ops.ui.main.swt.EuropaPlugin;
import org.ops.ui.solver.model.SolverListener;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.model.StepStatisticsRecord;

/**
 * SWT version of Open Decision dialog
 * 
 * @author Tatiana Kichkaylo
 */
public class DecisionsView extends ViewPart implements SolverListener {
	public static final String VIEW_ID = "org.ops.ui.solver.swt.DecisionsView";
	/** Minimum width of text fields */
	private static final int TEXT_WIDTH = 50;

	/** Solver model, initialized in createPartControl */
	private SolverModel model;

	private Button leftButton, rightButton, nstepButton;

	private Text stepField;

	private Label availableSteps;

	private Text decisionMade;

	private Table restTable;
	private TableViewer restViewer;

	/** Minimum value of step allowed */
	private final int minStep = 0;
	/** Currently displayed step */
	private int currentStep = 0;
	/** Maximum possible step. Used to bound currentStep */
	private int maxStep = 0;

	@Override
	public void createPartControl(Composite parent) {
		model = EuropaPlugin.getDefault().getSolverModel();
		model.addSolverListener(this);
		parent.setLayout(new FormLayout());

		leftButton = new Button(parent, SWT.PUSH);
		leftButton.setText("<<");
		leftButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				displayStepData(currentStep - 1);
			}
		});
		FormData data = new FormData();
		data.left = new FormAttachment(0, 1);
		data.top = new FormAttachment(0, 1);
		leftButton.setLayoutData(data);

		rightButton = new Button(parent, SWT.PUSH);
		rightButton.setText(">>");
		rightButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				displayStepData(currentStep + 1);
			}
		});
		rightButton.setLayoutData(rightOf(leftButton));

		nstepButton = new Button(parent, SWT.PUSH);
		nstepButton.setText("Go to step ");
		nstepButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent event) {
				try {
					displayStepData(new Integer(stepField.getText()));
				} catch (NumberFormatException e) {
					MessageDialog.openError(null, "Not a number", e
							.getMessage());
					displayStepData(currentStep);
				}
			}
		});
		nstepButton.setLayoutData(rightOf(rightButton));

		stepField = new Text(parent, SWT.BORDER);
		data = rightOf(nstepButton);
		data.width = TEXT_WIDTH;
		stepField.setLayoutData(data);

		availableSteps = new Label(parent, SWT.NONE);
		data = rightOf(stepField);
		data.width = TEXT_WIDTH;
		data.top = new FormAttachment(stepField, 0, SWT.CENTER);
		availableSteps.setLayoutData(data);

		Label curTitle = new Label(parent, SWT.NONE);
		curTitle.setText("Last decision made");
		data = new FormData();
		data.left = new FormAttachment(0, 1);
		data.top = new FormAttachment(leftButton);
		data.right = new FormAttachment(50, -1);
		curTitle.setLayoutData(data);

		Label restTitle = new Label(parent, SWT.NONE);
		restTitle.setText("Open decisions");
		data = new FormData();
		data.left = new FormAttachment(50, 1);
		data.top = new FormAttachment(leftButton);
		data.right = new FormAttachment(100, -1);
		restTitle.setLayoutData(data);

		decisionMade = new Text(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL
				| SWT.BORDER);
		decisionMade.setEditable(false);
		data = new FormData();
		data.left = new FormAttachment(0, 1);
		data.bottom = new FormAttachment(100, -1);
		data.top = new FormAttachment(curTitle);
		data.right = new FormAttachment(50, -1);
		decisionMade.setLayoutData(data);

		restTable = new Table(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL
				| SWT.BORDER);
		restViewer = new TableViewer(restTable);
		restViewer.setContentProvider(new ArrayContentProvider());
		data = new FormData();
		data.left = new FormAttachment(50, 1);
		data.bottom = new FormAttachment(100, -1);
		data.top = new FormAttachment(restTitle);
		data.right = new FormAttachment(100, -1);
		restTable.setLayoutData(data);

		// Load data, if any
		if (model.isConfigured()) {
			maxStep = model.getStepCount();
			displayStepData(maxStep);
			availableSteps.setText("of " + maxStep);
		} else {
			availableSteps.setText("no data");
		}
		setAllEnabled(model.isConfigured());
	}

	private FormData rightOf(Control control) {
		FormData data = new FormData();
		data.left = new FormAttachment(control, 1, SWT.RIGHT);
		data.bottom = new FormAttachment(control, 0, SWT.BOTTOM);
		data.top = new FormAttachment(control, 0, SWT.TOP);
		return data;
	}

	@Override
	public void dispose() {
		model.removeSolverListener(this);
		super.dispose();
	}

	@Override
	public void setFocus() {
		rightButton.setFocus();
	}

	public void afterOneStep(long time) {
		// Nothing
	}

	public void afterStepping() {
		setAllEnabled(true);
		assert (maxStep <= model.getStepCount());
		maxStep = model.getStepCount();
		displayStepData(maxStep);
		availableSteps.setText("of " + maxStep);
	}

	public void beforeStepping() {
		setAllEnabled(false);
	}

	public void solverStarted() {
		setAllEnabled(true);
		maxStep = model.getStepCount();
		displayStepData(maxStep);
		availableSteps.setText("of " + maxStep);
	}

	public void solverStopped() {
		setAllEnabled(false);
	}

	private void setAllEnabled(boolean value) {
		leftButton.setEnabled(value);
		rightButton.setEnabled(value);
		nstepButton.setEnabled(value);
		stepField.setEnabled(value);
		decisionMade.setEnabled(value);
		restTable.setEnabled(value);
	}

	/** Check the value of step given, and if possible advance GUI to it */
	private void displayStepData(int step) {
		if (step < minStep || step > maxStep)
			step = currentStep;
		currentStep = step;

		StepStatisticsRecord rec = model.getStepStatistics(step);

		// Some widget to do HTML?
		String buffer = rec.getDecisionAsHtml(null).toString();
		buffer = buffer.replaceAll("<br/>", "");
		decisionMade.setText(buffer);

		ArrayList<String> open = rec.getFlaws();
		restViewer.setInput(open);

		stepField.setText(String.valueOf(step));

		leftButton.setEnabled(step > minStep);
		rightButton.setEnabled(step < maxStep);
	}
}
