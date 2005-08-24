#ifndef _H_PlanDatabaseWriter
#define _H_PlanDatabaseWriter

#include "Timeline.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "ConstraintEngine.hh"
#include "IntervalIntDomain.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

  class PlanDatabaseWriter {

  public:
    static void write(PlanDatabaseId db, std::ostream& os) {
      check_error(!db->getConstraintEngine()->provenInconsistent());
      ObjectSet objs = db->getObjects();
      TokenSet alltokens = db->getTokens();
      os << "Objects *************************" << std::endl;
      indent()++;
      for (ObjectSet::const_iterator oit = objs.begin(); oit != objs.end() ; ++oit) {
	ObjectId object = *oit;
	os << indentation() << object->getType().toString() << ":" 
	   << object->getName().toString() << "*************************" << std::endl;


	std::list<TokenId> toks;
	if (TimelineId::convertable((*oit))) {
	  TimelineId timeline = (*oit);
	  toks = timeline->getTokenSequence();
	}
	else { // Treat as any object
	  const TokenSet& tokens = object->getTokens(); 
	  for(TokenSet::const_iterator tokit = tokens.begin(); tokit != tokens.end(); ++tokit){
	    TokenId t = *tokit;
	    toks.push_back(t);
	  }
	}

	if(!toks.empty()){
	  indent()++;
	  os << indentation() << "Tokens *************************" << std::endl;
	  for(std::list<TokenId>::const_iterator tokit = toks.begin(); tokit != toks.end(); ++tokit) {
	    TokenId t = (*tokit);
	    alltokens.erase(t);
	    writeToken(t, os);
	  }

	  os << indentation() << "End Tokens *********************" << std::endl;
	  indent()--;
	}

	// print variables associated with this object.
	const std::vector<ConstrainedVariableId> variables = object->getVariables();
	std::vector<ConstrainedVariableId>::const_iterator varit;
	if(!variables.empty()){
	  indent()++;
	  os << indentation() << "Variables *************************" << std::endl;
	  for(varit = variables.begin(); varit != variables.end(); ++varit) {
	    ConstrainedVariableId var = *varit;
	    writeVariable(var, os);
	  }
	  os << indentation() << "End Variables *********************" << std::endl;
	  indent()--;
	}

	os << indentation() << "End " << object->getType().toString() << ":" << object->getName().toString() 
	   << "*************************" << std::endl;
      }
      indent()--;

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
      printTokensHelper(os, "Merged", mergedTokens);
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
      indent()++;
      check_error(t.isValid());
      TempVarId st = t->getStart();
      os << indentation() << "[ " << st->derivedDomain() << " ]"<< std::endl;
      os << indentation() << "\t" << t->getPredicateName().toString() << "(" ;
      std::vector<ConstrainedVariableId> vars = t->getParameters();
      for (std::vector<ConstrainedVariableId>::const_iterator varit = vars.begin(); varit != vars.end(); ++varit) {
	ConstrainedVariableId v = (*varit);
	os << v->getName().toString() << "=" << v->derivedDomain();
      }
      os << ")" <<std::endl;
      os << indentation() << "\tKey=" << t->getKey();
      if (t->getMaster().isNoId())
	os << "  Master=NONE" << std::endl;
      else
	os << "  Master=" << t->getMaster()->getKey() << std::endl;


      TokenSet mergedtoks = t->getMergedTokens();

      for (TokenSet::const_iterator mit = mergedtoks.begin(); mit != mergedtoks.end(); ++mit) 
	os << indentation() << "\t\tMerged Key=" << (*mit)->getKey() << std::endl;

      os << indentation() << "[ " << t->getEnd()->derivedDomain() << " ]"<< std::endl;
      indent()--;
    }

    static void writeVariable(const ConstrainedVariableId& var, ostream& os) {
      check_error(var.isValid());
      indent()++;
      os << indentation() << var->getName().toString() << "=" << var->derivedDomain() << std::endl;
      indent()--;
    }

  private:
    static std::string indentation(){
      std::string s;
      for(unsigned int i=0; i<indent(); i++)
	s = s + "\t";
      return s;
    }

    static unsigned int& indent(){
      static unsigned int sl_indentLevel(0);
      return sl_indentLevel;
    }

  };

}

#endif /* #ifndef _H_PlanDatabaseWriter */
