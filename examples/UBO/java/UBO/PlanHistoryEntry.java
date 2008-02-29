package UBO;

public class PlanHistoryEntry
{
    Integer step_;
    Long remainingTreeSize_;
    String currentTree_;
    String currentPlan_;
               
    public PlanHistoryEntry(
               Integer step,
               Long remainingTreeSize,
               String currentTree,
               String currentPlan)
    {
        step_ = step;
        remainingTreeSize_ = remainingTreeSize;
        currentTree_ = currentTree;
        currentPlan_ = currentPlan;
    }
    
    public Integer getStepNumber() { return step_; }
    public Long getRemainingTreeSize() { return remainingTreeSize_; }
    public String getCurrentTree() { return currentTree_; }
    public String getCurrentPlan() { return currentPlan_; }               
    
    public String toString()
    {
        StringBuffer buf = new StringBuffer();
        buf.append(step_).append(": ")
           .append("(").append(remainingTreeSize_).append(")")
           .append(currentTree_)
           .append(currentPlan_); 
        return buf.toString();
    }
}
