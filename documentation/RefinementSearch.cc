// Function to solve a partial plan by successive refinement
// until all flaws are resolved.
bool solve(PartialPlan p){
  // Propagate the constraints to test for inconsistency. If found to be
  // inconsistent, then we can return false since this is a dead-end i.e. no refinements
  // to p can yield a consistent plan.
  if(isInconsistent(p))
    return false;

  // Non-deterministically choose a flaw from the set of available flaws.
  Flaw f = chooseFlaw(p);

  // If there are no flaws, then p is complete and we can terminate with success.
  if(f == NULL)
    return true;

  // Otherwise we formulate a decision point which is a branch in the search space. Each
  // choice is a particular refinement operation and the DecisionPoint collects all possible
  // refinement operations for the given flaw.
  DecisionPoint d = makeDecisionPoint(f, p);

  // Continue as long as we have something to try
  while(d.hasNext()){
    // A new partial plan is obtained by application of a refinement operator. Note that the ordering
    // over refinement operators to select is a non-deterministic step.
    PartialPlan pp = d.executeNext();

    // Recursive call to solve the new planning problem. If successful, then we are done.
    if(solve(pp))
      return true;
    else // Otherwise, retract the last refinement operation
      d.undo();
  }

  // If we arrive here, then we have exhausted all options to resolve the flaw, including the case where
  // no options were available initially. Thus the problem cannot be solved.
  return false;
}
