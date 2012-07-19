package nddl;
import java.util.List;

public class NddlVariable
{
	private String name;
	private NddlType type;

	public NddlVariable(String name, NddlType type)
	{
		this.name = name;
		this.type = type;
	}
	public NddlVariable(String name)
	{
		this(name,null);
	}
	public void setType(NddlType type)
	{
		this.type = type;
	}
	public String getTypeName() {return getType().getName();}
	public NddlType getType() {return type;}
	public String getName() {return name;}
	public String toString() {return "["+name+": "+type+"]";}
	public boolean intersect(double lb, double ub) throws EmptyDomainException { return type.intersect(lb,ub); }
	public boolean intersect(Object singleton) throws EmptyDomainException { return type.intersect(singleton); }
	public boolean intersect(List enumDomain) throws EmptyDomainException { return type.intersect(enumDomain); }
	public boolean intersect(NddlType other) throws EmptyDomainException { return type.intersect(other); }
}
