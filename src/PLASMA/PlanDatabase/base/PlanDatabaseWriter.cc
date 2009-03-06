#include "PlanDatabaseWriter.hh"
#include "Timeline.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "ConstraintEngine.hh"
#include "IntervalIntDomain.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"
#include "Utils.hh"
#include "Debug.hh"

namespace EUROPA {

  // bool PlanDatabaseWriter::s_useStandardKeys = true;

    std::string PlanDatabaseWriter::toString(const PlanDatabaseId& db, bool _useStandardKeys){
      useStandardKeys() = _useStandardKeys;
      checkError(useStandardKeys() == _useStandardKeys, "Failed to set standardKeys to method's input values");
      std::stringstream sstr;
      write(db, sstr);
      return sstr.str();
    }

    void PlanDatabaseWriter::write(PlanDatabaseId db, std::ostream& os) {
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
	  const TokenSet& tokens = object->tokens(); 
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
	checkError(t.isValid(), t);
	checkError(!t->isTerminated(), t->getKey());

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

    void PlanDatabaseWriter::printTokensHelper(std::ostream& os,
                                  const std::string& name,
                                  const TokenSet& tokens) {
      if (tokens.empty())
        return;
      os << name << " Tokens: *************************" << std::endl;
      TokenSet::const_iterator it = tokens.begin();
      for ( ; it != tokens.end(); ++it){
	TokenId tok = *it;
	checkError(tok.isValid(), tok);
        writeToken(tok, os);
      }
    }

  std::string PlanDatabaseWriter::timeDomain(const AbstractDomain& dom){
    std::stringstream ss;

    if(dom.isSingleton())
      ss << "{";
    else
      ss << "[";

    if(dom.getLowerBound() == MINUS_INFINITY)
      ss << "-inf";
    else if(dom.getLowerBound() == PLUS_INFINITY)
      ss << "+inf";
    else 
      ss << dom.getLowerBound();

    if(!dom.isSingleton()){
      ss << ", ";
      if(dom.getUpperBound() == MINUS_INFINITY)
	ss << "-inf";
      else if(dom.getUpperBound() == PLUS_INFINITY)
	ss << "+inf";
      else
	ss << dom.getUpperBound();
    }

    if(dom.isSingleton())
      ss << "}";
    else
      ss << "]";

    return ss.str();
  }

  std::string PlanDatabaseWriter::simpleTokenSummary(const TokenId& token) {
    std::stringstream ss;
    ss << token->toString() << timeDomain(token->start()->lastDomain()) <<  " --> " << timeDomain(token->end()->lastDomain());
    return ss.str();

  }
    void PlanDatabaseWriter::writeToken(const TokenId& t, std::ostream& os) {
      indent()++;
      check_error(t.isValid());
      TempVarId st = t->start();
      os << indentation() << "\t" << timeDomain(st->lastDomain()) << std::endl;
      os << indentation() << "\t" << t->getPredicateName().toString() << "(" ;
      std::vector<ConstrainedVariableId> vars = t->parameters();
      for (std::vector<ConstrainedVariableId>::const_iterator varit = vars.begin(); varit != vars.end(); ++varit) {
	ConstrainedVariableId v = (*varit);
	checkError(v.isValid(), v);
	const AbstractDomain& dom = v->lastDomain();

	if(dom.isNumeric() && dom.minDelta() < 1)
	  os.setf(std::ios::fixed);
	else
	  os.unsetf(std::ios::fixed);

	os << v->getName().toString() << "=" << dom;
	os.unsetf(std::ios::fixed);
      }
      os << ")" <<std::endl;
      os << indentation() << "\tKey=" << getKey(t);
      if (t->master().isNoId())
	os << "  Master=NONE" << std::endl;
      else
	os << "  Master=" << getKey(t->master()) << " " << simpleTokenSummary(t->master()) << std::endl;


      TokenSet mergedtoks = t->getMergedTokens();

      for (TokenSet::const_iterator mit = mergedtoks.begin(); mit != mergedtoks.end(); ++mit) {
	TokenId mergedToken = *mit;
	os << indentation() << "\t\tMerged Key=" << getKey(mergedToken);
	if(mergedToken->master().isId()){
	  os << " from " << simpleTokenSummary(mergedToken->master());
	}
	else 
	  os << " ROOT";

	os  << std::endl;
      }

      os << indentation() << "\t" << timeDomain(t->end()->lastDomain()) << std::endl;
      indent()--;
    }

    void PlanDatabaseWriter::writeVariable(const ConstrainedVariableId& var, std::ostream& os) {
      check_error(var.isValid());
      indent()++;
      os << indentation() << var->getName().toString() << "=" << var->lastDomain() << std::endl;
      indent()--;
    }

    std::string PlanDatabaseWriter::getKey(const TokenId& token){
      if(useStandardKeys())
	return EUROPA::toString(token->getKey());
      else
	return token->getPlanDatabase()->getClient()->getPathAsString(token);
    }

    std::string PlanDatabaseWriter::indentation(){
      std::string s;
      for(unsigned int i=0; i<indent(); i++)
	s = s + "\t";
      return s;
    }

    unsigned int& PlanDatabaseWriter::indent(){
      static unsigned int sl_indentLevel(0);
      return sl_indentLevel;
    }

    bool& PlanDatabaseWriter::useStandardKeys() {
      static bool sl_useStandardKeys(true);
      return sl_useStandardKeys;
    }

}
