package org.europa.ce;

import java.util.List;
import java.util.Set;

import org.europa.engine.EngineComponent;

public interface ConstraintEngine 
	extends EngineComponent
{
	public CVariable createVariable(
		String name,
		String dataType,
		Domain baseDomain,
		Object parent,
		int index,
		boolean isInternal,
		boolean canBeSpecified);
	
	public CVariable getVariableByKey(long key);
	public Set<CVariable> getAllVariables();
	
	public Constraint createConstraint(
		String constraintType,
		List<CVariable> scope,
		String violationExpl);

	public Constraint getConstraintByKey(long key);
	public void deleteConstraint(Constraint c);
	public Set<Constraint> getAllConstraints();
	
	public double getViolation();
	public List<String> getViolationExpl();
	public Set<Constraint> getViolatedConstraints();
	
	public void propagate();
	public boolean getAutopropagate();
	public void setAutopropagate(boolean b);

	public CESchema getSchema();
}
