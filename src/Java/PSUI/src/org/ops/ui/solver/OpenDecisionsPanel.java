package org.ops.ui.solver;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

import org.ops.ui.util.Util;

public class OpenDecisionsPanel 
    extends JPanel 
    implements ActionListener
{
	private static final long serialVersionUID = 2128067653391774807L;

	protected JButton btnNext_;
	protected JButton btnPrev_;
	protected JLabel labelTotal_;
	protected JLabel labelCurrent_;
	protected JTable odTable_;
	protected JTextArea lastExecutedDecision_;
	protected JTable ledTable_;
	protected JTextField gotoIdx_;
	protected int currentIdx_;
	protected List<List<String>> odList_;  // OpenDecisionStacks

	// Not all stepNumbers have a corresponding OpenDecisionStack
	// so need to keep track explicitly
    protected List<Integer> snList_;       // StepNumbers
    protected List<String> ledList_;      // LastExecutedDecisions
    
	public OpenDecisionsPanel()
	{
		odList_ = new Vector<List<String>>();
		snList_ = new Vector<Integer>();
		ledList_ = new Vector<String>();
		
		currentIdx_ = -1;
		
		labelTotal_ = new JLabel(makeLabelTotal(),SwingConstants.CENTER);
		labelCurrent_ = new JLabel(makeLabelCurrent(),SwingConstants.CENTER);
		JPanel btnPanel = new JPanel(new FlowLayout());
		
		btnPrev_ = new JButton("<<"); btnPrev_.addActionListener(this);btnPrev_.setActionCommand("prev");
		btnNext_ = new JButton(">>"); btnNext_.addActionListener(this);btnNext_.setActionCommand("next");
        checkButtons();
        JButton btn;
		btn = new JButton("Go To Step"); btn.addActionListener(this);btn.setActionCommand("goto");
                
		gotoIdx_ = new JTextField(6);
		btnPanel.add(btnPrev_);
		btnPanel.add(btnNext_);
		btnPanel.add(btn);
		btnPanel.add(gotoIdx_);

		lastExecutedDecision_ = new JTextArea(5,10);
		lastExecutedDecision_.setLineWrap(true);
		
		JPanel topPanel = new JPanel(new BorderLayout());
		JPanel p = new JPanel(new GridLayout(3,1));
		p.add(labelTotal_);
		p.add(btnPanel);
		p.add(new JLabel("Last Executed Decision",SwingConstants.CENTER));
		
		ledTable_ = new JTable();
        ledTable_.setTableHeader(null);
        ledTable_.setPreferredScrollableViewportSize(new Dimension(500, 70));
		
		JTabbedPane tp = new JTabbedPane();
		tp.addTab("As Table", new JScrollPane(ledTable_));
		tp.addTab("As Text", new JScrollPane(lastExecutedDecision_));

		topPanel.add(BorderLayout.NORTH,p);
        topPanel.add(BorderLayout.CENTER,tp);	
		odTable_ = new JTable(Util.makeTableModel(odList_,new String[]{"toString"}));
        odTable_.setTableHeader(null);
        odTable_.setPreferredScrollableViewportSize(new Dimension(500, 70));
        
		setLayout(new BorderLayout());
		add(BorderLayout.NORTH, topPanel);
		
		JPanel odPanel = new JPanel(new BorderLayout());
		odPanel.add(BorderLayout.NORTH,labelCurrent_);
		odPanel.add(BorderLayout.CENTER, new JScrollPane(odTable_));				
		add(BorderLayout.CENTER, odPanel);				
	}
	
	protected String makeLabelTotal()
	{
		String retval = "Open Decisions for "+odList_.size()+" steps are available";
		
		return retval;
	}

	protected String makeLabelCurrent()
	{
		String retval="";
		
		if (currentIdx_ >= 0)
			retval = "Currently looking at "+ odList_.get(currentIdx_).size() +" Open Decisions for step "+snList_.get(currentIdx_);
		
		return retval;
	}
	
	protected void checkButtons()
	{
		btnPrev_.setEnabled(currentIdx_ > 0);
		btnNext_.setEnabled(currentIdx_ >=0 && currentIdx_<odList_.size()-1);
	}

	protected void updateTable()
	{
		// TODO: put in Swing thread?
        odTable_.setModel(Util.makeTableModel(odList_.get(currentIdx_),new String[]{"toString"}));		
	}
	
	protected void updateStatus()
	{
		checkButtons();
		updateTable();
		labelCurrent_.setText(makeLabelCurrent());
		String led = ledList_.get(currentIdx_);
		lastExecutedDecision_.setText(led);
		lastExecutedDecision_.setCaretPosition(0);
		
		List<String> details = new Vector<String>();
    	StringTokenizer tok = new StringTokenizer(led,":");
    	while (tok.hasMoreTokens()) 
    		details.add(tok.nextToken());

    	// TODO: put in Swing thread?
        ledTable_.setModel(Util.makeTableModel(details,new String[]{"toString"}));		
	}
	
	protected void showNext()
	{
		currentIdx_++;
		updateStatus();
	}
	
	protected void showPrev()
	{
		currentIdx_--;
		updateStatus();
	}
	
	protected void beep()
	{
		java.awt.Toolkit.getDefaultToolkit().beep();		
	}
	
	protected void gotoStep()
	{
		int step;
		
		try {
			step = new Integer(gotoIdx_.getText());
		}
		catch (Exception e) {
			// TODO: display error message
			beep();
			return;
		}
		
		for (int i=0;i < snList_.size();i++) {
			if (snList_.get(i)==step) {
			    currentIdx_=i;
				updateStatus();
			}
		}
		
		// TODO: display error message
		beep();
	}
	
	public void addEntry(int stepNumber,List<String> od,String lastExecutedDecision)
	{
		snList_.add(stepNumber);
		odList_.add(od);
		ledList_.add(lastExecutedDecision);
		
		if (snList_.size() == 1)
			showNext();
		
		checkButtons();
		labelTotal_.setText(makeLabelTotal());
	}
	
    public void actionPerformed(ActionEvent e) 
    {
        if ("next".equals(e.getActionCommand())) {
        	showNext();
        	return;
        } 

        if ("prev".equals(e.getActionCommand())) {
        	showPrev();
        	return;
        }         

        if ("goto".equals(e.getActionCommand())) {
        	gotoStep();
        	return;
        }         
    }    	
}
