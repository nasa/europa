package org.ops.ui.utils.swing;

import java.util.Arrays;
import java.util.List;
import java.util.Vector;
import java.lang.reflect.Method;

import javax.swing.JTable;
import javax.swing.table.TableModel;
import javax.swing.table.AbstractTableModel;

public class Util 
{
    public static JTable makeTable(List<? extends Object> l,String fields[])
    {
    	JTable table =  new JTable(Util.makeTableModel(l,fields));
    	return table;
    }
    
    public static TableModel makeTableModel(List<? extends Object> l, String fields[]) 
    {
        return new ListTableModel(l,fields);	
    }
    
    protected static class ListTableModel
        extends AbstractTableModel
    {
		private static final long serialVersionUID = -4844388715602653749L;

		protected List<? extends Object> data_;
		protected List<String> columnNames_;
		protected List<Method> columnMethods_;
		
		public ListTableModel(List<? extends Object> l,String fields[])
    	{
			data_ = new Vector<Object>();
			columnNames_ = new Vector<String>();
			columnMethods_ = new Vector<Method>();

			if (l == null) 
				return;
			
			data_ = l;
			if (fields != null) {
			    columnNames_ = Arrays.asList(fields);
			    mapFields(fields);
			}
			else {
				mapFields();
			}
    	}

		protected void mapFields(String fields[])
		{
			if (data_.size() == 0)
				return;
			
			Class<? extends Object> c = data_.get(0).getClass();
			
			int i=0;
			try {
			    for (String s : fields) {
			    	String name = (s.equals("toString") ? s : "get"+s);
			    	columnMethods_.add(c.getMethod(name, (Class[])null));
			    	i++;
			    }
			}
			catch (Exception e) {
				// TODO: use logger
				System.err.println("Error mapping field "+fields[i]+" : "+e.getMessage());
			}
		}

		protected void mapFields()
		{
			columnNames_ = new Vector<String>();
			
			if (data_.size() == 0)
				return;

			Class<? extends Object> c = data_.get(0).getClass();			
			
			try {
			    for (Method m : c.getMethods()) { 
			    	Class<? extends Object> paramTypes[] = m.getParameterTypes();
			    	if (m.getName().startsWith("get") && 
			    		(paramTypes==null || paramTypes.length==0)) {
			    		columnMethods_.add(m);
			    		columnNames_.add(m.getName().substring(3));
			    	}
			    }
			}
			catch (Exception e) {
				// TODO: use logger
				System.err.println("Error mapping field : "+e.getMessage());
			}
		}
		
		public int getColumnCount() {  return columnNames_.size(); }
		public String getColumnName(int columnIndex) { return columnNames_.get(columnIndex); }
		
		public int getRowCount() { return data_.size(); }

		public Object getValueAt(int rowIndex, int columnIndex) 
		{
			try {
		        Object o = data_.get(rowIndex);
		        Method m = columnMethods_.get(columnIndex);
		        
		        return m.invoke(o, (Object[])null);
			}
			catch (Exception e) {
				return e.getMessage();				
			}
		}
    }
    
    static public class MatrixTableModel
        extends AbstractTableModel
    {
		private static final long serialVersionUID = -8922640086581386437L;
		
		protected List<Object> data_;
		protected List<Object> columnNames_;
		
		public MatrixTableModel(List<Object> data,List<Object> columnNames)
		{
		    data_ = data;
		    columnNames_ = columnNames;
		}

		public int getColumnCount()  { return columnNames_.size(); }		
		public String getColumnName(int columnIndex) { return columnNames_.get(columnIndex).toString(); } 

		public int getRowCount() { return data_.size();	}

		@SuppressWarnings("unchecked")
		public Object getValueAt(int rowIndex, int columnIndex) 
		{
			return ((List<Object>)data_.get(rowIndex)).get(columnIndex);
		}    	
    }
}
