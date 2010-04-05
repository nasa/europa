package org.ops.ui.solver.swing;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTextPane;
import javax.swing.ListModel;
import javax.swing.event.ListDataListener;

import org.ops.ui.main.swing.EuropaInternalFrame;
import org.ops.ui.solver.model.SolverAdapter;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.model.StepStatisticsRecord;

/**
 * Panel for open decisions. This panel listens to SolverModel and gets all data
 * from it.
 * 
 * @author Tatiana Kichkaylo
 */
public class OpenDecisionsView extends EuropaInternalFrame {
	/** Minimum value of step allowed */
	private final int minStep = 0;

	/** Currently displayed step */
	private int currentStep = 0;
	/** Maximum possible step. Used to bound currentStep */
	private int maxStep = 0;

	/** Input for currentStep / display for it */
	private JTextField stepFld = new JTextField();
	/** Display of maxStep */
	private JLabel availableSteps = new JLabel(" of 0");
	/** Buttons */
	private JButton left, right, goTo;

	private JTextPane decisionMade = new JTextPane();
	private JList openDecisions = new JList();

	private SolverModel solver;

	public OpenDecisionsView(final SolverModel solver) {
		super("Open decisions");
		this.solver = solver;

		createUI();

		solver.addSolverListener(new SolverAdapter() {
			@Override
			public void afterStepping() {
				assert (maxStep <= solver.getStepCount());
				maxStep = solver.getStepCount();
				displayStepData(maxStep);
				availableSteps.setText("of " + maxStep);
			}
		});
		displayStepData(0);
	}

	private void createUI() {
		GridBagLayout layout = new GridBagLayout();
		this.setLayout(layout);
		GridBagConstraints c = new GridBagConstraints();
		c.insets.set(3, 3, 3, 3);

		left = new JButton("<<");
		left.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				displayStepData(currentStep - 1);
			}
		});
		right = new JButton(">>");
		right.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				displayStepData(currentStep + 1);
			}
		});
		goTo = new JButton("Go to step ");
		goTo.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try {
					displayStepData(new Integer(stepFld.getText()));
				} catch (NumberFormatException e) {
					JOptionPane.showMessageDialog(OpenDecisionsView.this, e
							.getMessage());
					displayStepData(currentStep);
				}
			}
		});
		JPanel topPanel = new JPanel();
		topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.X_AXIS));
		topPanel.add(left);
		topPanel.add(right);
		topPanel.add(goTo);
		topPanel.add(stepFld);
		stepFld.setMinimumSize(new Dimension(50, 10));
		topPanel.add(availableSteps);
		c.gridx = 0;
		c.gridy = 0;
		c.weightx = 1;
		c.weighty = 0;
		this.add(topPanel, c);

		JLabel label1 = new JLabel("Last decision made");
		Font bold = label1.getFont().deriveFont(Font.BOLD);
		label1.setFont(bold);
		c.gridy++;
		this.add(label1, c);

		c.gridy++;
		c.weighty = 1;
		c.fill = GridBagConstraints.BOTH;
		decisionMade.setContentType("text/html");
		this.add(new JScrollPane(decisionMade), c);

		JLabel label2 = new JLabel("Open decisions");
		label2.setFont(bold);
		c.gridy++;
		c.weighty = 0;
		c.fill = GridBagConstraints.NONE;
		this.add(label2, c);

		c.gridy++;
		c.weighty = 1;
		c.fill = GridBagConstraints.BOTH;
		this.add(new JScrollPane(openDecisions), c);
	}

	/** Check the value of step given, and if possible advance GUI to it */
	private void displayStepData(int step) {
		if (step < minStep || step > maxStep)
			step = currentStep;
		currentStep = step;

		StepStatisticsRecord rec = solver.getStepStatistics(step);
		StringBuffer buffer = rec.getDecisionAsHtml(null);
		this.decisionMade.setText(buffer.toString());

		ArrayList<String> open = rec.getFlaws();
		this.openDecisions.setModel(new DecisionListModel(open));

		stepFld.setText(String.valueOf(step));

		left.setEnabled(step > minStep);
		right.setEnabled(step < maxStep);
	}

	private class DecisionListModel implements ListModel {

		private ArrayList<String> data;

		public DecisionListModel(ArrayList<String> data) {
			this.data = data;
		}

		public Object getElementAt(int index) {
			return data.get(index);
		}

		public int getSize() {
			if (data == null)
				return 0;
			return data.size();
		}

		public void addListDataListener(ListDataListener l) {
		}

		public void removeListDataListener(ListDataListener l) {
		}
	}

	@Override
	public Dimension getFavoriteSize() {
		return new Dimension(400, 300);
	}
}
