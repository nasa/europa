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
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;

import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.ops.ui.main.swing.EuropaInternalFrame;
import org.ops.ui.solver.model.SolverListener;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.model.StepStatisticsRecord;
import org.ops.ui.solver.model.TimeFormatHelper;

/**
 * Solver dialog. Contents of the original dialog folded into one panel.
 * 
 * @author Tatiana Kichkaylo
 */
public class PSSolverDialog extends EuropaInternalFrame implements
		SolverListener {

	/* Labels for chart */
	private final String lblTimePerStep = "Time (secs) per Step ",
			lblAvgPerStep = "Avg time (sec) per Step",
			lblStepNumber = "Step Number", lblTimeSec = "Time (secs)",
			lblOpenDecCount = "Open Decision Count",
			lblDecsInPlan = "Decisions in Plan";

	private JTextField stepCountFld;
	private JButton goButton;
	private JLabel stepLabel, timeLabel;

	/** Total running time, seconds */
	private double totalTimeSec = 0;

	protected XYSeries stepTimeSeries = new XYSeries(lblTimePerStep);
	protected XYSeries stepAvgTimeSeries = new XYSeries(lblAvgPerStep);
	protected XYSeries decisionCntSeries = new XYSeries(lblOpenDecCount);
	protected XYSeries solverDepthSeries = new XYSeries(lblDecsInPlan);

	/** Start of multi-step run */
	private long startOfRun;

	private SolverModel solver;

	private ConsoleView console;

	public PSSolverDialog(final SolverModel solver, ConsoleView console) {
		super("Solver");
		this.solver = solver;
		this.console = console;
		createUI();
		solver.addSolverListener(this);
	}

	private void createUI() {
		this.setLayout(new GridBagLayout());
		GridBagConstraints c = new GridBagConstraints();
		c.gridx = 0;
		c.gridy = 0;
		c.weightx = 0;
		c.weighty = 0;
		c.gridwidth = 3;
		c.insets.set(1, 1, 1, 1);

		this.goButton = new JButton("Go");
		this.goButton.setFont(this.goButton.getFont().deriveFont(Font.BOLD));
		this.goButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				try {
					int steps = new Integer(stepCountFld.getText());
					startOfRun = System.currentTimeMillis();
					solver.stepN(steps, true);
				} catch (Exception e) {
					e.printStackTrace();
					JOptionPane.showMessageDialog(PSSolverDialog.this, e
							.getMessage());
				} finally {
					// Enable buttons
					afterStepping();
				}
			}
		});
		this.stepCountFld = new JTextField("10", 4);

		JPanel line = new JPanel();
		line.setLayout(new BoxLayout(line, BoxLayout.X_AXIS));
		line.add(new JLabel("Run for "));
		this.stepCountFld.setMinimumSize(new Dimension(50, 10));
		line.add(this.stepCountFld);
		line.add(new JLabel(" steps   "));
		line.add(this.goButton);
		this.goButton.setPreferredSize(new Dimension(this.stepCountFld
				.getPreferredSize().width,
				this.goButton.getPreferredSize().height));
		this.add(line, c);

		line = new JPanel();
		line.setLayout(new BoxLayout(line, BoxLayout.X_AXIS));
		int[] horizon = solver.getHorizon();
		final JTextField horStart = new JTextField(String.valueOf(horizon[0]),
				5);
		horStart.setMinimumSize(new Dimension(50, 10));
		final JTextField horEnd = new JTextField(String.valueOf(horizon[1]), 5);
		horEnd.setMinimumSize(new Dimension(50, 10));
		line.add(new JLabel("Horizon "));
		line.add(horStart);
		line.add(new JLabel("to"));
		line.add(horEnd);
		JButton setHor = new JButton("Set horizon");
		setHor.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				try {
					int start = new Integer(horStart.getText());
					int end = new Integer(horEnd.getText());
					solver.setHorizon(start, end);
				} catch (Exception e) {
					JOptionPane.showMessageDialog(PSSolverDialog.this, e
							.toString());
				}
			}
		});
		line.add(setHor);
		c.gridy = 1;
		this.add(line, c);

		XYSeriesCollection data = new XYSeriesCollection();
		data.addSeries(this.stepTimeSeries);
		data.addSeries(this.stepAvgTimeSeries);
		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;
		c.weightx = 1;
		c.weighty = 1;
		c.fill = GridBagConstraints.BOTH;
		this.add(new SolverChartPanel(lblTimePerStep, lblStepNumber,
				lblTimeSec, data, false), c);
		data = new XYSeriesCollection();
		data.addSeries(this.decisionCntSeries);
		c.gridx++;
		this.add(new SolverChartPanel(lblOpenDecCount, lblStepNumber,
				lblOpenDecCount, data, false), c);
		data = new XYSeriesCollection();
		data.addSeries(this.solverDepthSeries);
		c.gridx++;
		this.add(new SolverChartPanel(lblDecsInPlan, lblStepNumber,
				lblDecsInPlan, data, false), c);

		this.stepLabel = new JLabel("no info");
		this.stepLabel.setFont(this.stepLabel.getFont().deriveFont(Font.BOLD));
		this.timeLabel = new JLabel("no info");
		this.timeLabel.setFont(this.timeLabel.getFont().deriveFont(Font.BOLD));
		JPanel bottom = new JPanel();
		bottom.add(new JLabel("Step count: "));
		bottom.add(this.stepLabel);
		bottom.add(new JLabel("  Run time: "));
		bottom.add(this.timeLabel);
		c.gridx = 0;
		c.gridy = 3;
		c.weightx = 0;
		c.weighty = 0;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		this.add(bottom, c);
	}

	@Override
	public void beforeStepping() {
		goButton.setEnabled(false);
	}

	@Override
	public void afterStepping() {
		goButton.setEnabled(true);
		checkEngineOutput();
	}

	@Override
	public void afterOneStep(long time) {
		timeLabel.setText(TimeFormatHelper.formatTime(System
				.currentTimeMillis()
				- startOfRun));
		int stepCnt = solver.getStepCount();
		stepLabel.setText(Integer.toString(stepCnt));
		double secs = time / 1000.0;
		totalTimeSec += secs;
		StepStatisticsRecord rec = solver.getStepStatistics(stepCnt);
		ArrayList<String> flaws = rec.getFlaws();
		int decs = flaws == null ? 0 : flaws.size();
		solverDepthSeries.add(stepCnt, rec.getDepth());
		stepTimeSeries.add(stepCnt, secs);
		stepAvgTimeSeries.add(stepCnt, totalTimeSec / stepCnt);
		decisionCntSeries.add(stepCnt, decs);
		checkEngineOutput();
	}

	@Override
	public Dimension getFavoriteSize() {
		return new Dimension(400, 300);
	}

	@Override
	public void solverStarted() {
		checkEngineOutput();
	}

	@Override
	public void solverStopped() {
		// Not important for Swing application
	}
	/**
	 * Check if engine produced any error output. If so, print that output on a
	 * console. If necessary, create the console and/or make it visible.
	 */
	protected void checkEngineOutput() {
		String msg = solver.retrieveEngineOutput();
		if (msg == null || msg.isEmpty())
			return; // nothing to do
		
		console.addText(msg);
		console.setVisible(true);
	}
}
