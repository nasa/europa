package aver;

import antlr.CommonAST;
import antlr.RecognitionException;
import antlr.TokenStreamException;
import antlr.collections.AST;

public abstract class AverHelper implements AverTokenTypes {

  protected static void checkValidType(int type, String message, AST ast) throws AverRuntimeException {
    switch(type) {
    case OBRACE:
    case OBRACKET:
    case NUMBER:
    case IN:
    case OUT:
    case EQ:
    case NE:
    case TOKENS:
    case STRING:
    case OBJECTS:
    case TRANSACTIONS:
    case ENTITY:
    case PROPERTY:
    case COUNT:
    case LT:
    case GT:
    case GE:
    case LE:
    case TEST:
    case INTERSECTS:
    case AT:
    case STEP:
      break;
    default:
      checkValidType(type, message + ast.toStringList());
    }
  }

  protected static void checkValidType(int type, String message) throws AverRuntimeException {
    switch(type) {
    case OBRACE:
    case OBRACKET:
    case NUMBER:
    case IN:
    case OUT:
    case EQ:
    case NE:
    case TOKENS:
    case STRING:
    case OBJECTS:
    case TRANSACTIONS:
    case ENTITY:
    case PROPERTY:
    case COUNT:
    case LT:
    case GT:
    case GE:
    case LE:
    case TEST:
    case INTERSECTS:
    case AT:
    case STEP:
      break;
    default:
      throw new AverRuntimeException(message);
    }
  }

  protected static void checkValidType(int type, int check, String message, AST ast) throws AverRuntimeException {
    if(type != check)
      checkValidType(type, check, message + ast.toStringList());
  }

  protected static void checkValidType(int type, int check, String message) throws AverRuntimeException {
    if(type != check)
      throw new AverRuntimeException(message);
  }

  protected static void checkValidType(String type, String message) throws AverRuntimeException {
    if(!(type.equals("IntervalDomain") || type.equals("EnumeratedDomain") ||
         type.equals("Value") || type.equals("in") || type.equals("out") ||
         type.equals("eq") || type.equals("ne") || type.equals("Tokens") ||
         type.equals("Objects") || type.equals("Transactions") ||
         type.equals("Entity") || type.equals("Property") ||
         type.equals("Count") || type.equals("lt") || type.equals("gt") ||
         type.equals("ge") || type.equals("le") || type.equals("Test") ||
         type.equals("At")))
      throw new AverRuntimeException(message);
  }

  protected static void checkValidType(String type, String check, String message) throws AverRuntimeException {
    if(!type.equals(check))
      throw new AverRuntimeException(message);
  }
}
