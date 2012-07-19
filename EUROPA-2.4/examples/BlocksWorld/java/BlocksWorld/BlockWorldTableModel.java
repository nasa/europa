package BlocksWorld;

import java.util.List;
import javax.swing.JTable;
import javax.swing.table.TableModel;
import javax.swing.table.AbstractTableModel;

public class BlockWorldTableModel
    extends AbstractTableModel
{
	protected BlockWorld data_;
    protected int maxTowerSize_;
    
	public BlockWorldTableModel(BlockWorld bw)
	{
		data_ = bw;
		maxTowerSize_=0;
		for (int i=0;i<data_.getTowers().size();i++) {
			List tower = (List)data_.getTowers().get(i);
			if (tower.size() > maxTowerSize_)
				maxTowerSize_ = tower.size();
		}
	}

	public int getColumnCount() {  return data_.getTowers().size(); }
	public String getColumnName(int columnIndex) { return "Tower-"+columnIndex; }

	public int getRowCount() { return maxTowerSize_; }

	public Object getValueAt(int rowIndex, int columnIndex) 
	{
    	List tower = data_.getTowers().get(columnIndex);
	    if (tower.size() > rowIndex)
	    	return tower.get(rowIndex);
	    else
	    	return "";
	}
}

