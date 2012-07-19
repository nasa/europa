package BlocksWorld;

public class BlockHistoryEntry
{
    protected Integer idx_;
    protected Integer step_;
    protected String towers_;
    protected String opHistory_;
    
    public BlockHistoryEntry(Integer idx,Integer step,String towers,String opHistory)
    {
    	idx_ = idx;
    	step_ = step;
    	towers_ = towers;
    	opHistory_ = opHistory;
    }
    
    public Integer getIndex() { return idx_; }
    public Integer getStepNumber() { return step_; }
    public String getTowers() { return towers_; }
    public String getOperatorHistory() { return opHistory_; }
}

