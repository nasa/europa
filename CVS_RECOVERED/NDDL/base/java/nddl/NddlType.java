package nddl;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.HashSet;
import antlr.collections.AST;

public class NddlType implements NddlTokenTypes, Cloneable
{
	protected int type;
	protected String typeName;
	protected NddlType superType;
	protected double niDomainLower;
	protected double niDomainUpper;
	protected Set eDomain;
	protected Set constructors;
	protected Set predicates;

	public NddlType()
	{
		//typeless type... it must be a placeholder
		this(-1);
	}
	public NddlType(String typeName, NddlType superType, int type, Set eDomain)
	{
		this.type = type;
		this.typeName = typeName;
		this.superType = superType;
		this.niDomainLower = Double.NEGATIVE_INFINITY;
		this.niDomainUpper = Double.POSITIVE_INFINITY;
		this.eDomain = eDomain;
		this.constructors = null;
	}
	public NddlType(int type)
	{
		this(null,null,type,null);
	}
	public NddlType(int type, NddlType superType)
	{
		this(null,superType,type,null);
	}
	public NddlType(String typeName, NddlType superType)
	{
		this(typeName,superType,IDENT,null);
	}

	public void addPredicate(String predicate)
	{
		if(predicates == null) predicates = new HashSet();
		assert(DebugMsg.debugMsg("NddlType:addPredicate",NddlUtil.append(typeName,predicate)));
		predicates.add(predicate);
	}
	public Set getPredicates() {return predicates;}
	public void addConstructor(List params)
	{
		if(constructors == null) constructors = new HashSet();
		constructors.add(params);
	}
	// returns name of class (or parent class) which supports this constructor
	public String hasConstructor(List params)
	{
		boolean conflict = false;
		float bestDistance = Float.POSITIVE_INFINITY;
		if(constructors != null)
		{
			for(Iterator i = constructors.iterator(); i.hasNext();)
			{
				List curr = (List)i.next();
				float distance = 0;
				if(curr.size() == params.size())
					for(int j=0;j<params.size() && distance<=bestDistance;j++)
						distance += ((NddlType)params.get(j)).getTypeDistance(((NddlVariable)curr.get(j)).getType());
				else
					distance = Float.POSITIVE_INFINITY;
				if(distance<bestDistance) {conflict = false; bestDistance = distance;}
				else if(distance==bestDistance && distance < Float.POSITIVE_INFINITY) conflict = true;
			}
		}
		if(conflict) throw new RuntimeException("Ambiguous constructor call in "+getName()+", best cases all have Manhattan distance "+bestDistance);
		if(bestDistance < Float.POSITIVE_INFINITY) return getName();
		if(getSuperType() != null) return getSuperType().hasConstructor(params);
		if(params.size() == 0) return getName();
		return null;
	}
	public void setType(String typeName, NddlType superType, int type)
	{
		if(this.type == -1)
		{
			this.typeName = typeName;
			this.superType = superType;
			this.type = type;
		}
	}
	public float getTypeDistance(NddlType t)
	{
		if(t == null) return Float.POSITIVE_INFINITY;
		if(!isObject() && type == t.type || t.mangled().equals(mangled())) return 0;
		if(getSuperType()==null) return Float.POSITIVE_INFINITY;
		return getSuperType().getTypeDistance(t)+1;
	}
	public int getType() {return type;}
	public Object clone()
	{
		NddlType toRet = new NddlType();
		toRet.typeName = typeName;
		toRet.type = type;
		toRet.superType = superType;
		toRet.niDomainLower = niDomainLower;
		toRet.niDomainUpper = niDomainUpper;
		if(eDomain!=null)
		{
			Set copyDomain = new HashSet();
			copyDomain.addAll(eDomain);
			toRet.eDomain = copyDomain;
		}
		else
			toRet.eDomain = null;
		toRet.constructors = constructors;
		return toRet;
	}
	public void setSuperType(NddlType superType)
	{
		this.superType = superType;
	}
	public NddlType getSuperType() {return superType;}

	public Set getEnumDomain()  {return eDomain;}
	public boolean isTypeless()  {return type == -1;}
	public boolean isObject()    {return type == 0 || type == IDENT;}
	public boolean isInteger()   {return type == INT;}
	public boolean isFloat()     {return type == FLOAT;}
	public boolean isNumeric()   {return type == NUMERIC || (superType!=null && superType.isNumeric());}
	//public boolean isNumeric()   {return isInteger() || isFloat();}
	public boolean isString()    {return type == STRING;}
	public boolean isBool()      {return type == BOOL;}
	public boolean isSymbol()    {return type == SYMBOL;}
	public boolean isPredicate() {return type == PREDICATE;}
	public boolean isAssignableFrom(NddlType other)
	{
		if(other == null || other.isTypeless()) return false;
		if(isObject() && other.isObject())
		{
			return typeName.equals(other.typeName)
			    || isAssignableFrom(other.getSuperType());
		}
		if(type == other.type) return true;
		if(type == NUMERIC && other.isNumeric()) return true;
		return isTypeless();
	}
	public boolean isDomainEmpty()
	{
		return eDomain != null && eDomain.size() == 0;
	}
	public boolean intersect(double lower, double upper) throws EmptyDomainException
	{
		if(isNumeric())
		{
			if(eDomain == null) // both are intervals
			{
				niDomainLower = Math.max(niDomainLower,lower);
				niDomainUpper = Math.min(niDomainUpper,upper);
			}
			else // only the other domain is an interval
			{
				for(Iterator i = eDomain.iterator(); i.hasNext();)
				{
					Double el = (Double)i.next();
					if(el.doubleValue() < lower && el.doubleValue() > upper)
						eDomain.remove(el);
				}
			}
			if(isDomainEmpty()) throw new EmptyDomainException("Interval intersection with ["+lower+" "+upper+"] caused domain to be emptied.");
			return true;
		}
		return false;
	}
	public boolean intersect(Object singleton) throws EmptyDomainException
	{
		if(singleton == null) return false;
		if(singleton instanceof Double)
		{
			double d = ((Double)singleton).doubleValue();
			return intersect(d,d);
		}
		else if(eDomain == null)
		{
			eDomain = new HashSet();
			eDomain.add(singleton);
		}
		else
		{
			boolean addSingleton = eDomain.contains(singleton);
			eDomain.clear();
			if(addSingleton)
				eDomain.add(singleton);
		}
		if(isDomainEmpty()) throw new EmptyDomainException("Singleton intersection with {"+singleton+"} caused domain to be emptied.");
		return true;
	}
	public boolean intersect(Set otherDomain) throws EmptyDomainException
	{
		if(otherDomain == null) return false;
		if(eDomain != null)
		{
			eDomain.retainAll(otherDomain);
		}
		else if(isNumeric())
		{
			eDomain = new HashSet();
			for(Iterator i = otherDomain.iterator(); i.hasNext();)
			{
				Double el = (Double)i.next();
				if(el.doubleValue() >= niDomainLower && el.doubleValue() <= niDomainUpper)
					eDomain.add(el);
			}
			if(isDomainEmpty()) throw new EmptyDomainException("Enumeration intersection with "+otherDomain+" caused domain to be emptied.");
			return true;
		}
		else
		{
			eDomain = new HashSet();
			eDomain.addAll(otherDomain);
			if(isDomainEmpty()) throw new EmptyDomainException("Enumeration intersection with "+otherDomain+" caused domain to be emptied.");
			return true;
		}
		return false;
	}
	public boolean intersect(NddlType other) throws EmptyDomainException
	{
		if(other.eDomain == null && isNumeric())
			return intersect(other.niDomainLower,other.niDomainUpper);
		return intersect(other.eDomain);
	}
	public void add(Object o)
	{
		if(eDomain == null) eDomain = new HashSet();
		eDomain.add(o);
	}
	// this should only ever be used to muck typedefs into old enums, this fix is only temporary
	public void setName(String name)
	{
		typeName = name;
	}
	public String getName()
	{
		return typeName;
	}
	public String mangled()
	{
		if(typeName != null) return typeName;
		switch(type)
		{
			case INT:    return "int";
			case FLOAT:  return "float";
			case BOOL:   return "bool";
			case NUMERIC: return "numeric";
			case STRING: return "string";
			case IDENT: case SYMBOL:
			default:     throw new NullPointerException(debugString());
		}
	}
	public String toString() {return mangled();}
	public String debugString()
	{
		String par = super.toString();
		switch(type)
		{
			case IDENT:
				return "["+par+" (IDENT): "+typeName+", "+superType+", "+eDomain+"]";
			case SYMBOL:
				return "["+par+" (SYMBOL): "+typeName+", "+superType+", "+eDomain+"]";
			case INT:
				return "["+par+" (INT): "+typeName+", "+superType+", ["+niDomainLower+" "+niDomainUpper+"] or "+eDomain+"]";
			case FLOAT:
				return "["+par+" (FLOAT): "+typeName+", "+superType+", ["+niDomainLower+" "+niDomainUpper+"] or "+eDomain+"]";
			case BOOL:
				return "["+par+" (BOOL): "+typeName+", "+superType+", "+eDomain+"]";
			case NUMERIC:
				return "["+par+" (NUMERIC): "+typeName+", "+superType+", "+eDomain+"]";
			case STRING:
				return "["+par+" (STRING): "+typeName+", "+superType+", "+eDomain+"]";
			default:
				return "["+par+" ("+type+"): "+typeName+", "+superType+", "+eDomain+"]";
		}
	}
}
