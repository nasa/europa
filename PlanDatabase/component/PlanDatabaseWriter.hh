#ifndef _H_PlanDatabaseWriter
#define _H_PlanDatabaseWriter

#include "Timeline.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "ConstraintEngine.hh"
#include "IntervalIntDomain.hh"
#include "PlanDatabaseDefs.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"
#include "TestSupport.hh"

namespace EUROPA {

  class PlanDatabaseWriter {

  public:
    static void write(PlanDatabaseId db, std::ostream& os) {
      check_error(!db->getConstraintEngine()->provenInconsistent());
      ObjectSet objs = db->getObjects();
      TokenSet alltokens = db->getTokens();
      for (ObjectSet::const_iterator oit = objs.begin(); oit != objs.end() ; ++oit) {
	if (TimelineId::convertable((*oit))) {
	  TimelineId timeline = (*oit);
	  os << timeline->getType().toString() << ":" << timeline->getName().toString() << "*************************" << std::endl;
	  std::list<TokenId> toks = timeline->getTokenSequence();
	  for(std::list<TokenId>::const_iterator tokit = toks.begin(); tokit != toks.end(); ++tokit) {
	    TokenId t = (*tokit);
	    alltokens.erase(t);
	    writeToken(t, os);
	  }
	  os << "End Timeline: " << timeline->getName().toString() << "*************************" << std::endl;
	}
	else { // Treat as any object
	  ObjectId object = *oit;
	  os << object->getType().toString() << ":" << object->getName().toString() << "*************************" << std::endl;
	  const TokenSet& tokens = object->getTokens(); 
	  for(TokenSet::const_iterator tokit = tokens.begin(); tokit != tokens.end(); ++tokit){
	    TokenId t = *tokit;
	    alltokens.erase(t);
	    writeToken(t, os);
	  }

          // print variables associated with this object.
	  const std::vector<ConstrainedVariableId> variables = object->getVariables();
	  std::vector<ConstrainedVariableId>::const_iterator varit;
          for(varit = variables.begin(); varit != variables.end(); ++varit) {
 	    ConstrainedVariableId var = *varit;
	    writeVariable(var, os);
	  }
	}
      }
     
      // print global variables
      const ConstrainedVariableSet globalVariablesSet = db->getGlobalVariables();
      if (! globalVariablesSet.empty()) {
        os << "Global Variables" << "*************************" << std::endl;
        for(ConstrainedVariableSet::const_iterator it = globalVariablesSet.begin(); it != globalVariablesSet.end(); ++it) {
	  ConstrainedVariableId var = *it;
          check_error(var.isValid());
	  writeVariable(var,os);
        } 
      }
      if (alltokens.empty())
        return;
      TokenSet activeTokens;
      TokenSet inactiveTokens;
      TokenSet incompleteTokens;
      TokenSet mergedTokens;
      TokenSet rejectedTokens;
      TokenSet::const_iterator tokit = alltokens.begin();
      for ( ; tokit != alltokens.end(); tokit++) {
        TokenId t = *tokit;
        if (t->isMerged())
          mergedTokens.insert(t);
        else if (t->isActive())
          activeTokens.insert(t);
        else if (t->isRejected())
          rejectedTokens.insert(t);
        else if (t->isInactive())
          inactiveTokens.insert(t);
        else if (t->isIncomplete())
          incompleteTokens.insert(t);
        else
          check_error(false, "Token with unknown status");
      }
      printTokensHelper(os, "Active", activeTokens);
      printTokensHelper(os, "Rejected", rejectedTokens);
      printTokensHelper(os, "Inactive", inactiveTokens);
      printTokensHelper(os, "Incomplete", incompleteTokens);
      // Skip printing these for two reasons:
      // 1. Expensive due to constraints not being propagated to their variable domains.
      // 2. Their id's are printed when their active token is printed with writeToken().
      // printTokensHelper(os, "Merged", mergedTokens);
    }

  private:

    static void printTokensHelper(std::ostream& os,
                                  const std::string& name,
                                  const TokenSet& tokens) {
      if (tokens.empty())
        return;
      os << name << " Tokens: *************************" << std::endl;
      TokenSet::const_iterator it = tokens.begin();
      for ( ; it != tokens.end(); it++)
        writeToken(*it, os);
    }

    static void writeToken(const TokenId& t, ostream& os) {
      check_error(t.isValid());
      TempVarId st = t->getStart();
      os << "[ " << st->derivedDomain() << " ]"<< std::endl;
      os << "\t" << t->getPredicateName().toString() << "(" ;
      std::vector<ConstrainedVariableId> vars = t->getParameters();
      for (std::vector<ConstrainedVariableId>::const_iterator varit = vars.begin(); varit != vars.end(); ++varit) {
	ConstrainedVariableId v = (*varit);
	os << v->getName().toString() << "=" << v->derivedDomain();
      }
      os << ")" <<std::endl;
      os << "\tKey=" << t->getKey();
      if (t->getMaster().isNoId())
	os << "  Master=NONE" << std::endl;
      else
	os << "  Master=" << t->getMaster()->getKey() << std::endl;


      TokenSet mergedtoks = t->getMergedTokens();

      for (TokenSet::const_iterator mit = mergedtoks.begin(); mit != mergedtoks.end(); ++mit) 
	os << "\t\tMerged Key=" << (*mit)->getKey() << std::endl;

      os << "[ " << t->getEnd()->derivedDomain() << " ]"<< std::endl;
    }

    static void writeVariable(const ConstrainedVariableId& var, ostream& os) {
      check_error(var.isValid());
      os << var->getName().toString() << "=" << var->derivedDomain() << std::endl;
    }

  };

}

#endif /* #ifndef _H_PlanDatabaseWriter */
