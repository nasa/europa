#ifndef _H_PlanDatabaseWriter
#define _H_PlanDatabaseWriter

#include "PlanDatabaseDefs.hh"
#include <sstream>

namespace EUROPA {

  class PlanDatabaseWriter {

  public:

    static std::string toString(const PlanDatabaseId& db, bool _useStandardKeys = true);

    static void write(PlanDatabaseId db, std::ostream& os);

  private:

    static void printTokensHelper(std::ostream& os,
                                  const std::string& name,
                                  const TokenSet& tokens);

    static void writeToken(const TokenId& t, std::ostream& os);

    static void writeVariable(const ConstrainedVariableId& var, std::ostream& os);

    static std::string getKey(const TokenId& token);

    static std::string indentation();

    static unsigned int& indent();

    static bool& useStandardKeys();

  };

}

#endif /* #ifndef _H_PlanDatabaseWriter */
