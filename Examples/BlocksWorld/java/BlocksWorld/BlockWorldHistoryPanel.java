package BlocksWorld;

import java.util.List;
import java.util.Vector;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.JScrollPane;


public class BlockWorldHistoryPanel
    extends JPanel
    implements BlockWorldHistory, ActionListener
{
	protected List<BlockHistoryEntry> history_;
	protected List<BlockWorld> blockWorldHistory_;
	protected JTable historyTable_;
	protected int currentIdx_;
    protected JTextField gotoIdx_;
    protected JLabel status_;
    
	public BlockWorldHistoryPanel()
	{
		history_ = new Vector<BlockHistoryEntry>();
		blockWorldHistory_ = new Vector<BlockWorld>();
		
	    historyTable_ = new JTable();
	    currentIdx_ = -1;
	    
    	JPanel buttonPanel = new JPanel(new FlowLayout());
    	JButton b;
		b = new JButton("<<"); b.addActionListener(this);b.setActionCommand("prev"); buttonPanel.add(b);
		b = new JButton(">>"); b.addActionListener(this);b.setActionCommand("next"); buttonPanel.add(b);
		b = new JButton("Go To Step"); b.addActionListener(this);b.setActionCommand("goto"); buttonPanel.add(b);
		gotoIdx_ = new JTextField(6);
		buttonPanel.add(gotoIdx_);
		
        status_ = new JLabel("");
		JPanel statusPanel = new JPanel(new FlowLayout());
        statusPanel.add(status_);
        
		setLayout(new BorderLayout());
    	add(BorderLayout.NORTH,buttonPanel);
    	add(BorderLayout.CENTER,new JScrollPane(historyTable_));		
    	add(BorderLayout.SOUTH,statusPanel);
    	
    	updateStatus();
	}

    public List getHistory() { return history_; }
    public List getBlockWorldHistory() { return blockWorldHistory_; }
    
    protected void updateStatus() 
    {	
	    status_.setText("Currently showing step "+(currentIdx_+1)+" out of "+blockWorldHistory_.size()+" available");    	
    }
    
    public void showStep(int step) 
    { 
    	if (step>=0 && step<blockWorldHistory_.size()) {
        	currentIdx_ = step;
    	    historyTable_.setModel(new BlockWorldTableModel(blockWorldHistory_.get(currentIdx_)));
    	    updateStatus();
    	}
    }

	public void add(int stepCount,BlockWorld bw,String operatorHistory)
	{
		BlockHistoryEntry step = new BlockHistoryEntry(history_.size()+1,stepCount,bw.toString(),operatorHistory);
		blockWorldHistory_.add(bw);
		history_.add(step);
	    updateStatus();
		//System.out.println(step);
	}
	
    public void actionPerformed(ActionEvent e) 
    {
        if ("next".equals(e.getActionCommand())) {
        	showStep(currentIdx_+1);
        	return;
        } 

        if ("prev".equals(e.getActionCommand())) {
        	showStep(currentIdx_-1);
        	return;
        }         

        if ("goto".equals(e.getActionCommand())) {
    		int step;    		
    		try {
    			step = new Integer(gotoIdx_.getText());
    		}
    		catch (Exception ex) {
    			// TODO: display error message
    			return;
    		}
    		
		    showStep(step-1);
    		
        	return;
        }         
    }    	        
}

