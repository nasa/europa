package nddl;

import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;

public class NddlName
{
	String name;
	Set enumsContaining = new HashSet();
	Set types = new HashSet();
	Set variables = new HashSet();
	Set predicates = new HashSet();

	public NddlName(String name)
	{
		this.name = name;
	}

	public boolean isEmpty() {
		return enumsContaining.size() == 0 && types.size() == 0 && variables.size() == 0 && predicates.size() == 0;
	}

	public boolean containedByEnum(NddlType enumeration) throws ClassCastException
	{
		assert(DebugMsg.debugMsg("NddlName:containedByEnum",name + ".containedByEnum("+enumeration+")"));
		if(!enumeration.isSymbol()) throw new ClassCastException();
		if(!enumeration.getEnumDomain().contains(name)) throw new ClassCastException();
		enumsContaining.add(enumeration);
		return true;
	}

	public void redefineType(NddlType type)
	{
		assert(DebugMsg.debugMsg("NddlName:redefineType",name + ".redefineType("+type+")"));
		types.remove(getType(type.getName()));
		types.add(type);
	}
	public boolean addType(NddlType type)
	{
		assert(DebugMsg.debugMsg("NddlName:addType",name + ".addType("+type+")"));
		if(containsType(type.getName())) return false;
		types.add(type);
		return true;
	}

	public boolean addVariable(NddlVariable variable)
	{
		assert(DebugMsg.debugMsg("NddlName:addVariable",name + ".addVariable("+variable+")"));
		if(containsVariable(variable.getName())) return false;
		variables.add(variable);
		return true;
	}

	public boolean addPredicate(NddlType pred) throws ClassCastException
	{
		assert(DebugMsg.debugMsg("NddlName:addPredicate",name + ".addPredicate("+pred+")"));
		if(!pred.isPredicate()) throw new ClassCastException();
		if(containsPredicate(pred.getName())) return false;
		assert(DebugMsg.debugMsg("NddlName:addPredicate","Performing ACTUAL add"));
		predicates.add(pred);
		return true;
	}

	public NddlType getType(String qualified)
	{
		for(Iterator i = types.iterator(); i.hasNext();)
		{
			NddlType curr = (NddlType)i.next();
			if(qualified.equals(curr.getName()))
				return curr;
		}
		return null;
	}
	public NddlVariable getVariable(String qualified)
	{
		for(Iterator i = variables.iterator(); i.hasNext();)
		{
			NddlVariable curr = (NddlVariable)i.next();
			if(qualified.equals(curr.getName()))
				return curr;
		}
		return null;
	}
	public NddlType getPredicate(String qualified)
	{
		assert(DebugMsg.debugMsg("NddlName:getPredicate","looking for "+qualified));
		for(Iterator i = predicates.iterator(); i.hasNext();)
		{
			NddlType curr = (NddlType)i.next();
			if(qualified.equals(curr.getName()))
			{
				assert(DebugMsg.debugMsg("NddlName:getPredicate","  -> "+curr));
				return curr;
			}
		}
		assert(DebugMsg.debugMsg("NddlName:getPredicate","  -> not found"));
		return null;
	}
	// given a context, returns the enum which belongs to that context.
	public NddlType getEnum(String qualified)
	{
		for(Iterator i = enumsContaining.iterator(); i.hasNext();)
		{
			NddlType curr = (NddlType)i.next();
			if(qualified.equals(NddlUtil.nonull(NddlUtil.butLast(curr.getName()))));
				return curr;
		}
		return null;
	}

	public boolean containsType(String qualified) {return getType(qualified) != null;}
	public boolean containsVariable(String qualified) {return getVariable(qualified) != null;}
	public boolean containsPredicate(String qualified) {return getPredicate(qualified) != null;}
	public boolean isSymbol() {return enumsContaining.size() != 0;}
}
