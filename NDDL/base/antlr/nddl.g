header {
package nddl;

import antlr.TokenStreamRecognitionException;
import antlr.MismatchedCharException;
import java.util.Arrays;
import java.util.List;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.io.EOFException;
import java.io.FileNotFoundException;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.Reader;
import java.io.Writer;
}

class NddlParser extends Parser;
options {
  k = 3;
  exportVocab = Nddl;
  buildAST = true;
  defaultErrorHandler = true; // Generate parser error handler
}

tokens {

  GOAL_KEYWORD="goal";
  FACT_KEYWORD="fact";
  REJECTABLE_KEYWORD="rejectable";
  // access modifiers
  PRIVATE_KEYWORD="private";
  PROTECTED_KEYWORD="protected";
  PUBLIC_KEYWORD="public";

  SUPER_KEYWORD="super";
  THIS_KEYWORD="this";
  NEW_KEYWORD="new";

  EXTENDS_KEYWORD="extends";
  USES_KEYWORD="using";
  CLASS_KEYWORD="class";
  CONSTRAINT_KEYWORD="constraint";
  PREDICATE_KEYWORD="predicate";
  ENUM_KEYWORD="enum";
  TYPEDEF_KEYWORD="typedef";
  INITIAL_KEYWORD="initial";
  IF_KEYWORD="if";
  ELSE_KEYWORD="else";
  FOREACH_KEYWORD="foreach";
  IN_KEYWORD="in";
  SPECIFY_KEYWORD="specify";
  CLOSE_KEYWORD="close";
  FREE_KEYWORD="free";
  CONSTRAIN_KEYWORD="constrain";
  MERGE_KEYWORD="merge";
  REJECT_KEYWORD="reject";
  ACTIVATE_KEYWORD="activate";
  CANCEL_KEYWORD="cancel";
  RESET_KEYWORD="reset";

  INT_KEYWORD="int"; FLOAT_KEYWORD="float";
  STRING_KEYWORD="string"; BOOL_KEYWORD="bool";
  NUMERIC_KEYWORD="numeric";
  // bool keywords
  FALSE_KEYWORD="false"; TRUE_KEYWORD="true";

  // Temporal relations
  TR_ANY_KEYWORD="any";
  TR_ENDS_KEYWORD="ends";
  TR_STARTS_KEYWORD="starts";
  TR_EQUALS_KEYWORD="equals";                       TR_EQUAL_KEYWORD="equal";
  // symmetric pairs
  TR_BEFORE_KEYWORD="before";                       TR_AFTER_KEYWORD="after";
  TR_CONTAINS_KEYWORD="contains";                   TR_CONTAINED_BY_KEYWORD="contained_by";
  TR_ENDS_BEFORE_KEYWORD="ends_before";             TR_ENDS_AFTER_KEYWORD="ends_after";
  TR_STARTS_BEFORE_END_KEYWORD="starts_before_end"; TR_ENDS_AFTER_START_KEYWORD="ends_after_start";
  TR_CONTAINS_START_KEYWORD="contains_start";       TR_STARTS_DURING_KEYWORD="starts_during";
  TR_CONTAINS_END_KEYWORD="contains_end";           TR_ENDS_DURING_KEYWORD="ends_during";
  TR_MEETS_KEYWORD="meets";                         TR_MET_BY_KEYWORD="met_by";
  TR_PARALLELS_KEYWORD="parallels";                 TR_PARALLELED_BY_KEYWORD="paralleled_by";
  TR_STARTS_BEFORE_KEYWORD="starts_before";         TR_STARTS_AFTER_KEYWORD="starts_after";

  FILTER_KEYWORD="filter";

  NDDL; // a root node, so that there is ALWAYS an AST
  SYMBOL; // for changing the token type of IDENTs
  TYPE; // for appending type information to VARIABLEs
  PINFF; NINFF;
  FLOAT; NUMERIC; BOOL; VARIABLE;
  SUBGOAL;
  PREDICATE;
  CONSTRUCTOR;
  CONSTRUCTOR_INVOCATION;
  CONSTRAINT_INSTANTIATION;
  MODIFIERS;
}

{
  /// for some strange reason there is no access to the LLkParser's k variable.
  static final int K = 3;

  /// a set of enabled warning messages
  public static final Set warnings = new HashSet();
  private boolean interp = false;

  NddlParserState state;
  NddlLexer lexer;

  // users may wish to call nddl.NddlParser instead of nddl.Parse.
  public static void main(String [] args) throws Exception {
    Parse.main(args);
  }

  public NddlParser(NddlLexer lexer, NddlParserState state) {
    this(lexer);
    this.lexer = lexer;
    setASTFactory(new NddlASTFactory(lexer));
    this.state = new NddlParserState(state);
  }

  public NddlParserState getState() {
    return state;
  }

  private static String getCanonicalPath(File file) {
    try {
      return file.getCanonicalPath();
    }
    catch(java.io.IOException ex) {
      return file.getName();
    }
  }

  private String infixString(AST ast) {
    if(ast==null)
      return "";
    String op = ast.getText();
    AST child = ast.getFirstChild();
    if(child == null)
      return op;
    String toRet = infixString(child);
    while(child.getNextSibling()!=null) {
      child = child.getNextSibling();
      toRet += op + infixString(child);
    }
    return toRet;
  }

  public String getFilename() {
    return lexer.getFilename();
  }

  public NddlLexer getLexer() {
    return lexer;
  }

  public static NddlParser parse(File file) throws RecognitionException, TokenStreamException, FileNotFoundException {
    return parse(null,file,null);
  }

  public static NddlParser parse(File file, Writer debugWriter) throws RecognitionException, TokenStreamException, FileNotFoundException {
    return parse(null,file,debugWriter);
  }

  public static NddlParser parse(NddlParserState init, File file, Writer debugWriter) throws RecognitionException, TokenStreamException, FileNotFoundException {
    if(!file.exists())
      throw new FileNotFoundException("Could not parse missing file: "+file);
    if(init != null && init.containsFile(file)) {
      init.warn("includes","Previously parsed: "+getCanonicalPath(file));
      return null;
    }

    BufferedReader reader = new BufferedReader(new FileReader(file));
    NddlLexer lexer = new NddlLexer(reader,file, false);
    NddlParser parser = new NddlParser(lexer, init);

    parser.getState().pushParser(parser);
    parser.getState().addFile(file);
    parser.nddl();
    parser.getState().popParser();

    if(debugWriter!=null) {
      try {
        if(parser.getAST()!=null)
          ((antlr.BaseAST)parser.getAST()).xmlSerialize(debugWriter);
        debugWriter.flush();
      }
      catch(java.io.IOException ex) {
        System.err.println("An I/O error occurred while attempting to write to debug file:");
        ex.printStackTrace();
      }
    }
    return parser;
  }

  public static NddlParser eval(NddlParserState init, Reader reader) throws RecognitionException, TokenStreamException, EOFException {
    NddlLexer lexer = new NddlLexer(reader, null, true);
    NddlParser parser = new NddlParser(lexer, init);

    parser.getState().pushParser(parser);
    parser.interp = true;
    try {
      parser.nddlInterp();
    }
    catch(TokenStreamRecognitionException ex) {
      // Magic number is EOF
      if((ex.recog instanceof MismatchedCharException) && ((MismatchedCharException) ex.recog).foundChar == 65535)
        throw new EOFException();
      throw ex;
    }
    if(parser.getState().getErrorCount() == 0 && lexer.hasPrematureEOF())
      throw new EOFException();
    parser.getState().popParser();

    return parser;
  }

  private AST addFileToState(String fn) throws RecognitionException, TokenStreamException, FileNotFoundException {
    String directory = (lexer.getFile() != null)? lexer.getFile().getParent() : null;
    File include = ModelAccessor.generateIncludeFileName(directory, fn);
    if(include == null)
      throw new FileNotFoundException("Cannot find \""+fn+"\"");

    if(state.containsFile(include)) {
      state.warn("includes","Previously parsed: "+getCanonicalPath(include));
      return null;
    }

    NddlParser parser = NddlParser.parse(state,include,null);
    state = parser.getState();

    if(parser!=null)
      return parser.getAST();

    return null;
  }

  private void addClass(Token name, Token superType) throws SemanticException {
    if(superType!=null)
      state.addClass(name.getText(),superType.getText());
    else
      state.addClass(name.getText(),"Object");
  }

  private boolean isPredVar(String name) throws SemanticException {
    NddlVariable var = state.getVariable(name);
    if(var == null)
      return false;
    return var.getType().isPredicate();
  }

  private void checkPred(String name) throws SemanticException, TokenStreamException {
    if(!state.isPredicateType(name))
      throw new SemanticException("\""+name+"\" is not a recognized predicate type");
  }

  private void intersect(int check, NddlType type, Object obj) throws SemanticException, TokenStreamException {
    if(type==null)
      return;
    if(type.getType() != check)
      throw new SemanticException("Cannot intersect "+type.mangled()+" with "+getTokenName(check));
    try {
      if(obj instanceof Set)
        type.intersect((Set)obj);
      else
        type.intersect(obj);
    }
    catch(EmptyDomainException ex) {
      throw new SemanticException(ex.getMessage());
    }
    catch(ClassCastException ex) {
      throw new SemanticException(ex.getMessage());
    }
  }

  private void intersect(int check, NddlType type, double lb, double ub) throws SemanticException, TokenStreamException {
    if(type==null)
      return;
    if(type.getType() != check)
      throw new SemanticException("Cannot intersect "+type.mangled()+" with "+getTokenName(check));
    try {
      type.intersect(lb,ub);
    }
    catch(EmptyDomainException ex) {
      throw new SemanticException(ex.getMessage());
    }
    catch(ClassCastException ex) {
      throw new SemanticException(ex.getMessage());
    }
  }

  private void copyLocation(AST a, AST b) throws ClassCastException {
    if(!(a instanceof NddlASTNode) || !(b instanceof NddlASTNode))
      throw new ClassCastException();
    else {
      ((NddlASTNode)a).setColumn(((NddlASTNode)b).getColumn());
      ((NddlASTNode)a).setLine(((NddlASTNode)b).getLine());
    }
  }

  public String getTokenName(int i) {
    switch(i) {
      case -1: return "[NO TYPE]";
      case 0: return "OBJECT";
      default: return super.getTokenName(i);
    }
  }

  public void reportError(RecognitionException ex) {
    // let's the interpreter know about incomplete code (missing EOF) rather than erroring out.
    if(interp &&
       (((ex instanceof MismatchedTokenException) && ((MismatchedTokenException) ex).token.getType() == Token.EOF_TYPE) ||
         ((ex instanceof NoViableAltException) && ((NoViableAltException) ex).token.getType() == Token.EOF_TYPE)))
      lexer.setPrematureEOF();
    else {
      state.error(ex);
    }
  }

  public void reportWarning(RecognitionException ex) {
    state.warn("all",ex);
  }

  /**
   * Generate a string suitable for printing current LA tokens for tracing.
   */
  private String lookaheadString() throws TokenStreamException {
    StringBuffer toRet = new StringBuffer(10*K);
    toRet.append("LA(1)==").append(LT(1).getText());
    for (int i = 2; i <= K; i++) {
      if ( LT(i)!=null )
        toRet.append(" LA(").append(i).append(")==").append(LT(i).getText());
      else
        toRet.append(" LA(").append(i).append(")==null");
    }
    return toRet.toString();
  }

  /**
   * Custom traceIn for Antlr which uses debugMsg.
   */
  public void traceIn(String rname) throws TokenStreamException {
    char[]indent = new char[traceDepth];
    Arrays.fill(indent,' ');
    traceDepth += 1;

    StringBuffer marker = new StringBuffer(rname.length()+19);
    marker.append("NddlParser:traceIn:").append(rname);

    StringBuffer data = new StringBuffer(traceDepth + rname.length()+20+(10*K)); // attempt to hit the correct length right off
    data.append(indent).append("> ").append(rname).append((inputState.guessing > 0)? "; [guessing]" : "; ").append(lookaheadString());
    assert(DebugMsg.debugMsg(marker.toString(), data.toString(), false));
  }

  /**
   * Custom traceOut for Antlr which uses debugMsg.
   */
  public void traceOut(String rname) throws TokenStreamException {
    traceDepth -= 1;
    char[]indent = new char[traceDepth];
    Arrays.fill(indent,' ');

    StringBuffer marker = new StringBuffer(rname.length()+20);
    marker.append("NddlParser:traceOut:").append(rname);

    StringBuffer data = new StringBuffer(traceDepth + rname.length()+20+(10*K)); // attempt to hit the correct length right off
    data.append(indent).append("< ").append(rname).append((inputState.guessing > 0)? "; [guessing]" : "; ").append(lookaheadString());
    assert(DebugMsg.debugMsg(marker.toString(), data.toString(), false));
  }
}

nddlInterp:
    (nddlStatement)* EOF!
    {#nddlInterp = #(#[NDDL,"NDDL"],#nddlInterp);}
  ;

nddl:
    (nddlStatement)* EOF!
    {#nddl = #(#[NDDL,"NDDL"],#nddl);}
  ;

nddlStatement:
    inclusion
  | enumeration
  | typeDefinition SEMICOLON!
  | constraintSignature
  | allocation[null] SEMICOLON!
  | (type IDENT)=> variableDeclaration SEMICOLON!
  | classDeclaration
  | rule
  | goal SEMICOLON!
  | fact SEMICOLON!
  | ((IDENT)? temporalRelation)=> relation SEMICOLON!
  | (qualified[null,true,true,true,false] DOT)=> function SEMICOLON!
  | (qualified[null,true,false,false,false])=> assignment SEMICOLON!
  | CLOSE_KEYWORD^ LPAREN! RPAREN! SEMICOLON!
  | constraintInstantiation SEMICOLON!
  | SEMICOLON!
  ;
  exception 
  catch [RecognitionException ex] {
    reportError(ex);
    match(LA(1));
  } 
  catch [TokenStreamException tx] {
    state.error(tx.getMessage()); match(LA(1));
  }

inclusion!:
  INCLUDE_DECL^ s:STRING
  { try {
      AST include = addFileToState(s.getText());
      if(include!=null)
        #inclusion = include.getFirstChild();
    }
    catch (FileNotFoundException ex) {
      state.error(ex.getMessage());
    }
  }
  ;

enumeration {Set s;}:
    k:ENUM_KEYWORD^ e:IDENT s=symbolSet {state.addEnumeration(e.getText(),s);}
  ; 

symbolSet returns [Set s = new HashSet()]:
    LBRACE^
    (symbolDefinitions[s])?
    RBRACE!
  ;

symbolDefinitions[Set s] {String d;}:
    d=symbolDefinition {s.add(d);}(COMMA! symbolDefinitions[s])?
  ;

symbolDefinition returns [String r = null]:
    s:IDENT {r = s.getText(); #s.setType(SYMBOL);}
  ;

typeDefinition! {NddlType t;}:
    k:TYPEDEF_KEYWORD^ t=b:typeWithBase n:IDENT
    { b_AST.setType(TYPE);
      t.setName(n.getText());
      state.defineType(n.getText(),t);
      #typeDefinition = #(#k, #n, #b);}
  ;

typeWithBase returns [NddlType t = null]:
    INT_KEYWORD^    {t = (NddlType)state.getPrimative("int").clone();}    (intervalIntDomain[t]|enumeratedIntDomain[t])?
  | FLOAT_KEYWORD^  {t = (NddlType)state.getPrimative("float").clone();}  (intervalFloatDomain[t]|enumeratedFloatDomain[t])?
  | BOOL_KEYWORD^   {t = (NddlType)state.getPrimative("bool").clone();}   (enumeratedBoolDomain[t])?
  | STRING_KEYWORD^ {t = (NddlType)state.getPrimative("string").clone();} (enumeratedStringDomain[t])?
  | {state.isEnumerationType(LT(1).getText())}? i:IDENT^ {t = (NddlType)state.getType(#i.getText()).clone();}
    (enumeratedSymbolDomain[t])?
  | {state.isObjectType(LT(1).getText())}? i2:IDENT^     {t = (NddlType)state.getType(#i2.getText()).clone();}
    (enumeratedObjectDomain[t])?
  ;

constraintSignature! {List na = null, ua=null; ConstraintSignature cs = null, sup = null;}:
    ck:CONSTRAINT_KEYWORD n:IDENT na=typeArgumentList
    ( uk:EXTENDS_KEYWORD u:IDENT ua=typeArgumentList {sup = state.getConstraint(u.getText(),ua);})? 
      {cs = new ConstraintSignature(n.getText(),na,sup);}
      (sb:signatureBlock[na] {cs.setBlock(#sb,state);}
    | SEMICOLON!) {state.defineConstraint(cs);}
  ;

signatureBlock[List m]:
    LBRACE^ (signatureExpression[m])? RBRACE!
  ;

signatureExpression[List m]:
    signatureAtom[m] ((DAMP^ | DPIPE^) signatureExpression[m])?
  ;

signatureAtom[List m] {NddlType temp;}:
    LPAREN^ signatureExpression[m] RPAREN!
  | left:IDENT {if(!m.contains(#left.getText())) throw new SemanticException("Type variable '"+left.getText()+"' not defined in signature.");}
    IS_A^ ( {state.isType(LT(1).getText())}? temp=t:type {t_AST.setType(TYPE);}
  | n:NUMERIC_KEYWORD {n_AST.setType(TYPE);}
  | right:IDENT {if(!m.contains(#right.getText())) throw new SemanticException("Type variable '"+right.getText()+"' not defined in signature.");})
  ;

classDeclaration!:
    ck:CLASS_KEYWORD c:IDENT (((xk:EXTENDS_KEYWORD x:IDENT)? 
    {addClass(c,x);}
    {state.openContext(c.getText());} cb:classBlock {state.closeContext();}
    {#classDeclaration = #(#ck, #c, #(#xk, #x), #cb);})
  | s:SEMICOLON
    {state.addPredeclaredClass(c.getText());}
    {#classDeclaration = #(#ck, #c, #s);})
  ;

// add GNATS and suspend for supporting modifiers
classBlock:
    LBRACE^ ((accessModifier)? classStatement)* RBRACE!
  ;

classStatement:
    (PREDICATE_KEYWORD | IDENT LPAREN)=> (constructor | predicate)
  | variableDeclaration SEMICOLON!
  | SEMICOLON!
  ; 

constructor! {List l;}:
    c:IDENT l=p:constructorParameterList {state.openContext(state.addConstructor(c.getText(),l));}
    b:constructorBlock {state.closeContext();}
    {#constructor = #(#[CONSTRUCTOR,"constructor"], #c, #p, #b);}
  ;

constructorBlock:
    LBRACE^ (constructorStatement)* RBRACE!
  ;

constructorStatement:
    (assignment | superInvocation) SEMICOLON!
  | SEMICOLON!
  ;

constructorParameterList returns [List l = new ArrayList()]:
    LPAREN^ (constructorParameters[l])? RPAREN!
  ;

constructorParameters[List l] {Object o;}:
    o=constructorParameter {l.add(o);} (COMMA! constructorParameters[l])?
  ;

constructorParameter! returns [NddlVariable p = null] {NddlType t = null;}:
    t=type i:IDENT
    {p = new NddlVariable(i.getText(),t);}
    {#constructorParameter = #(#[VARIABLE,i.getText()],i,#[TYPE,t.mangled()]);}
  ;

predicate:
    p:PREDICATE_KEYWORD^
    n:IDENT {state.addPredicate(n.getText());state.openContext(n.getText());} predicateBlock {state.closeContext();}
  ;

predicateBlock:
    LBRACE^ (predicateStatement)* RBRACE!
  ;

predicateStatement:
    ( variableDeclarationNoAlloc
    | constraintInstantiation
    | assignmentNoAlloc)? SEMICOLON!
  ;

rule:
    c:IDENT {state.openContext(c.getText());} DCOLON^ p:IDENT
    { checkPred(NddlUtil.append(c.getText(),p.getText())); 
      state.openContext(p.getText());
      state.openAnonymousContext();}
    ruleBlock {state.closeContext();state.closeContext();state.closeContext();}
  ;

/*
  The following allows for either a block of rule statements {stmt; stmt; stmt;} or a single statement stmt;
  This is primarily for a braceless if statement, but allows for all ruleblocks to have this behavior.
  The NDDL user should exercise common sense when using this feature and avoid creating unreadable code.
*/
ruleBlock:
    LBRACE^ (ruleStatement)* RBRACE!
  | ruleStatement {#ruleBlock = #(#[LBRACE,"{"],#ruleBlock);}
  ;

ruleStatement:
    (((IDENT|thisIdent)? temporalRelation)=> relation
  | variableDeclaration
  | constraintInstantiation) SEMICOLON!
  | flowControl
  | SEMICOLON!
  ;


type returns [NddlType t = null]:
    INT_KEYWORD    {t = (NddlType)state.getPrimative("int").clone();}
  | FLOAT_KEYWORD  {t = (NddlType)state.getPrimative("float").clone();}
  | BOOL_KEYWORD   {t = (NddlType)state.getPrimative("bool").clone();}
  | STRING_KEYWORD {t = (NddlType)state.getPrimative("string").clone();}
  | {state.isType(LT(1).getText())}? i:IDENT
    {t = (NddlType)state.getType(#i.getText()).clone();}
  ;

relation!:
    ((p:IDENT {state.isPredicateVariable(#p.getText());})|thisIdent)?
    r:temporalRelation
    args:predicateArgumentList
    {#relation = #(#[SUBGOAL,"subgoal"],#p,#r,#args);}
  ;

goal:
    (REJECTABLE_KEYWORD^ | GOAL_KEYWORD^) predicateArgumentList
  ;

fact:
    FACT_KEYWORD^ predicateArgumentList
  ;
//not arguments to a predicate, arguments which are predicates (possibly named)
predicateArgumentList:
    {LT(1).getType()!=DOT && isPredVar(LT(1).getText())}? IDENT
  | LPAREN^ (predicateArguments)? RPAREN!
  ;

predicateArguments:
    predicateArgument (COMMA! predicateArgument)*
  ;

predicateArgument!:
    p:qualified[null,false,false,true,false] {checkPred(#p.getText());}
    (n:IDENT {state.addVariable(n.getText(),state.getPredicate(#p.getText()));})?
    { p_AST.setType(TYPE);
      #predicateArgument= #(#p,#n);}
  ;

constraintInstantiation {List a;}:
    c:IDENT a=variableArgumentList
    { try{state.validateConstraint(#c.getText(),a);} catch(SemanticException ex) {reportError(ex);}
      #constraintInstantiation = #(#[CONSTRAINT_INSTANTIATION,"constraint"],#constraintInstantiation);}
  ;

constructorInvocation[NddlType superType] {List a;}:
    c:IDENT a=variableArgumentList
    { if(!state.isConstructorType(#c.getText(),a))
        throw new SemanticException(#c.getText()+NddlUtil.listAsString(a)+" is not a recognized constructor type.");
      #constructorInvocation = #(#[CONSTRUCTOR_INVOCATION,"call"], #constructorInvocation);}
  ;

superInvocation {List a;}:
    s:SUPER_KEYWORD^ a=l:variableArgumentList
    //this error message could use a little work, but it won't matter until isConstructorType is correctly implemented.
    //also s.getText() will be incorrect once isConstructorType is actually working.
    { if(!state.isConstructorType(#s.getText(),a))
        throw new SemanticException(#s.getText()+a+" cannot be found for this context.");}
  ;

variableArgumentList returns [List l = new LinkedList()]:
    LPAREN^ (variableArguments[l])? RPAREN!
  ;

variableArguments[List l] {NddlType t=null;}:
    t=variableArgument {l.add(t);} (COMMA! variableArguments[l])?
  ;

variableArgument returns [NddlType t = new NddlType()]:
    literalOrName[t]
  ;

typeArgumentList returns [List l = new LinkedList()]:
    LPAREN^ (typeArguments[l])? RPAREN!
  ;

typeArguments[List l] {String s=null;}:
    s=typeArgument
    { if(!l.contains(s))
        l.add(s);
      else
        throw new SemanticException("Duplicate type variable name '"+s+"'");}
    (COMMA! typeArguments[l])?
  ;

typeArgument returns [String s = ""]:
    i:IDENT {s = i.getText();}
  ;


// only literal domains
domain[NddlType t] {Double l;}:
    {if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("int"),INT);} l=intLiteral {intersect(INT,t,l);}
  | intervalIntDomain[t]
  | enumeratedIntDomain[t]
  | {if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("float"),FLOAT);} l=floatLiteral {intersect(FLOAT,t,l);}
  | intervalFloatDomain[t]
  | enumeratedFloatDomain[t]
  | enumeratedStringDomain[t]
  | enumeratedBoolDomain[t]
  | enumeratedSymbolDomain[t]
  ;

intervalIntDomain[NddlType t] {Double lb, ub;}:
    LBRACKET^
    lb=intLiteral (COMMA!)?
    ub=intLiteral
    RBRACKET!
    { if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("int"),INT);
      intersect(INT,t,lb.doubleValue(),ub.doubleValue());}
  ;

intervalFloatDomain[NddlType t] {Double lb, ub;}:
    LBRACKET^
    lb=floatLiteral (COMMA!)?
    ub=floatLiteral
    RBRACKET!
    { if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("float"),FLOAT);
      intersect(FLOAT,t,lb.doubleValue(),ub.doubleValue());}
  ;

enumeratedIntDomain[NddlType t] {Set s;}:
    LBRACE^
    s=intSet
    { if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("int"),INT); intersect(INT,t,s);}
    RBRACE!
  ;

intSet returns [Set s = new HashSet(2);] {Double n;}:
    n=intLiteral {s.add(n);} (COMMA! n=intLiteral {s.add(n);})*
  ;

enumeratedFloatDomain[NddlType t] {Set s;}:
    LBRACE^
    s=floatSet
    { if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("float"),FLOAT); intersect(FLOAT,t,s);}
    RBRACE!
  ;

floatSet returns [Set s = new HashSet(2);] {Double n;}:
    n=floatLiteral {s.add(n);} (COMMA! n=floatLiteral {s.add(n);})*
  ;

enumeratedObjectDomain[NddlType t] {Set s;}:
    LBRACE^ s=objectSet[t] {intersect(0,t,s);} RBRACE!
  ;

objectSet[NddlType t] returns [Set s = new HashSet(2);]:
    (constructorInvocation[t]|qualifiedName[t]) {/*s.add(o);*/}
    (COMMA! (constructorInvocation[t]|qualifiedName[t]) {/*s.add(o);*/})*
  ;

enumeratedSymbolDomain[NddlType t] {Set s;}:
    LBRACE^
    s=qsymbolSet[t]
    { if(t != null && t.isTypeless()) {
        // I think this should work.
        NddlType domain = (NddlType)state.getSymbol(s.iterator().next().toString());
        t.setType(domain.getName(),domain,SYMBOL);
      }
      intersect(SYMBOL,t,s);}
    RBRACE!
  ;

qsymbolSet[NddlType t] returns [Set s = new HashSet(2);]:
    q:qualified[t,false,false,false,true] {s.add(#q.getText());}
    (COMMA! q2:qualified[t,false,false,false,true] {s.add(#q2.getText());})*
  ;

enumeratedStringDomain[NddlType t] {Set s;}:
    LBRACE^
    s=stringSet
    { if(t != null && t.isTypeless())
        t.setType(null,state.getPrimative("string"),STRING);
      intersect(STRING,t,s);}
    RBRACE!
  ;

stringSet returns [Set s = new HashSet(2);]:
    st:STRING {s.add(st.getText());}
    (COMMA! st2:STRING {s.add(st2.getText());})*
  ;

enumeratedBoolDomain[NddlType t] {Set s;}:
    LBRACE^ s=boolSet {if(t != null && t.isTypeless()) t.setType(null,state.getPrimative("bool"),BOOL);intersect(BOOL,t,s);} RBRACE!
  ;

boolSet returns [Set s = new HashSet(2);] {Boolean b;}:
    b=boolLiteral {s.add(b);} (COMMA! b=boolLiteral {s.add(b);})*
  ;

flowControl:
    (IF_KEYWORD^ x:expression {state.openAnonymousContext();} ruleBlock {state.closeContext();}
    /* It's okay to turn off the ambiguity warning, this is the textbook case for that feature */
    ( options {warnWhenFollowAmbig=false;} :
      { if(#x.getType() != DEQUALS && #x.getType() != NEQUALS)
          throw new SemanticException("If/else construct must guard using a comparison operator.");}
      ELSE_KEYWORD! {state.openAnonymousContext();} ruleBlock {state.closeContext();})?)
    | ( FOREACH_KEYWORD^ LPAREN! v:IDENT IN_KEYWORD! q:qualifiedName[null] RPAREN!
        { state.openAnonymousContext();
          NddlType type = state.getVariable(#q.getText()).getType();
          state.addVariable(v.getText(),type);
          astFactory.addASTChild(currentAST, #[TYPE,type.mangled()]);}
        ruleBlock {state.closeContext();})
  ;

expression:
    LPAREN! literalOrName[null] ((DEQUALS^|NEQUALS^) literalOrName[null])? RPAREN!
  ;

allocation[NddlType t]:
    NEW_KEYWORD! constructorInvocation[t]
  ;

variableDeclaration {NddlType t;}:
    m:modifiers[true]!
    t=type! nameWithBase[#m,t] (COMMA! nameWithBase[#m,t])* 
  ;

nameWithBase![AST modifiers, NddlType type]:
    n:IDENT^ (LPAREN! b:anyValue[type] RPAREN!)?
    { #nameWithBase = #([VARIABLE,n.getText()],n,#[TYPE,type.mangled()],modifiers,b);
      state.addVariable(n.getText(),type);}
  | n2:IDENT^ EQUALS! b2:anyValue[type]
    { #nameWithBase = #([VARIABLE,n2.getText()],n2,#[TYPE,type.mangled()],modifiers,b2);
      state.addVariable(n2.getText(),type);}
  ;

assignment {NddlVariable variable = null; NddlType lhtype = null;}:
    lhs:qualifiedName[null]
    (IN_KEYWORD^|EQUALS^)
    { variable = state.getVariable(#lhs.getText());
      if(variable != null && variable.getType() != null)
        lhtype = (NddlType)variable.getType().clone();
    }
    rhs:anyValue[lhtype]
    {if(variable!=null && state.isInherited(variable)) #assignment.addChild(#([EXTENDS_KEYWORD]));}
  ;

variableDeclarationNoAlloc {NddlType t;}:
    m:modifiers[false]!
    t=type! nameWithBaseNoAlloc[#m,t] (COMMA! nameWithBaseNoAlloc[#m,t])*
  ;

nameWithBaseNoAlloc![AST modifiers, NddlType type]:
    n:IDENT^ (LPAREN! (b:literalOrName[type]) RPAREN!)?
    { #nameWithBaseNoAlloc = #([VARIABLE,"variable"],n,#[TYPE,type.mangled()],modifiers,b);
      state.addVariable(n.getText(),type);}
  | n2:IDENT^ EQUALS! (b2:literalOrName[type])
    { #nameWithBaseNoAlloc = #([VARIABLE,"variable"],n2,#[TYPE,type.mangled()],modifiers,b2);
      state.addVariable(n2.getText(),type);}
  ;

assignmentNoAlloc {NddlVariable variable = null;}:
    lhs:qualifiedName[null]
    (IN_KEYWORD^|EQUALS^)
    {variable = state.getVariable(#lhs.getText());}
    rhs:literalOrName[(NddlType)variable.getType().clone()]
    {if(state.isInherited(variable)) #assignmentNoAlloc.addChild(#([EXTENDS_KEYWORD]));}
  ;

modifiers![boolean filter_permitted]:
    ((
      { if(!filter_permitted&&#f!=null)
          state.warn("modifiers","filter modifier not permitted here");
        else if(#f!=null)
          state.warn("modifiers","filter modifier was already declared");}
      f:FILTER_KEYWORD
    ))*
    {#modifiers = #([MODIFIERS,"modifiers"],f);}
  ;

anyValue[NddlType t]:
    literalOrName[t]
  | allocation[t]
  ;

// this rule CAN'T call literal, it wouldn't be able to distinguish
// between qualified and qualifiedName
literalOrName[NddlType t] {Boolean b;}:
    a:STRING
    { if(t!=null) {
        if(t.isTypeless())
          t.setType(null,state.getPrimative("string"),STRING);
        intersect(STRING,t,a.getText());
      }
    }
  | b=boolLiteral {if(t!=null&&t.isTypeless()) t.setType(null,state.getPrimative("bool"),BOOL); intersect(BOOL,t,b);}
  | qualified[t,true,false,false,true]
  | domain[t]
  ;

qualifiedName[NddlType t]:
    q:qualified[t,true,false,false,false]
    { if(#q.getType() == SYMBOL)
        throw new SemanticException("Expecting a name, found \""+#q.getText()+"\".");}
  ;

qualified[NddlType t, boolean searchVars, boolean searchTypes, boolean searchPredicates, boolean searchSymbols]:
    (THIS_KEYWORD ~DOT)=> thisIdent
    { if(t != null && t.isTypeless()) {
        String name = state.getThisTypeName();
        NddlType nameType = state.getNameType(name,t,false,true,true,false);
        t.setType(name,nameType,nameType.getType());
      }}
  | (THIS_KEYWORD DOT^)? q:qualifiedPart
    { String name = infixString(#qualified);
      Object nameTypeOrVariable = state.getName(name,t,searchVars,searchTypes,searchPredicates,searchSymbols);
      if(nameTypeOrVariable == null)
        throw new SemanticException("\""+name+"\" undeclared");

      #qualified = #(#[IDENT,name]);

      NddlType nameType = null;
      if(nameTypeOrVariable instanceof NddlType)
        nameType = (NddlType)nameTypeOrVariable;
      else if(nameTypeOrVariable instanceof NddlVariable) {
        nameType = ((NddlVariable)nameTypeOrVariable).getType();
        #qualified.addChild(#[TYPE,nameType.mangled()]);
      }

      if(t != null && t.isTypeless())
        t.setType(nameType.getName(),nameType,nameType.getType());
      if(t != null && !t.isAssignableFrom(nameType))
        throw new SemanticException(nameType.getName()+" not assignable to "+t.getName());

      if(nameTypeOrVariable instanceof NddlType && nameType.isSymbol())
        #qualified = #(#[SYMBOL,NddlUtil.last(name)],#[TYPE,nameType.mangled()]);
      copyLocation(#qualified,#q);}
      // cause parser to ignore bad names for tree construction.
  ;
  exception
  catch [RecognitionException ex] {
    state.error(ex);
  }

thisIdent!:
    t:THIS_KEYWORD
    { #thisIdent = #(#[IDENT,"this"],#[TYPE,state.getThisTypeName()]);
      copyLocation(#thisIdent,#t);}
  ;

qualifiedPart:
    (IDENT DOT IDENT)=> IDENT DOT^ qualifiedPart
  | IDENT
  ;

accessModifier:
    PRIVATE_KEYWORD
  | PROTECTED_KEYWORD
  | PUBLIC_KEYWORD;

temporalRelation:
    TR_ANY_KEYWORD
  | TR_ENDS_KEYWORD | TR_STARTS_KEYWORD
  | TR_EQUALS_KEYWORD | TR_EQUAL_KEYWORD
  | TR_BEFORE_KEYWORD | TR_AFTER_KEYWORD
  | TR_CONTAINS_KEYWORD | TR_CONTAINED_BY_KEYWORD
  | TR_ENDS_BEFORE_KEYWORD | TR_ENDS_AFTER_KEYWORD
  | TR_STARTS_BEFORE_END_KEYWORD | TR_ENDS_AFTER_START_KEYWORD
  | TR_CONTAINS_START_KEYWORD | TR_STARTS_DURING_KEYWORD
  | TR_CONTAINS_END_KEYWORD | TR_ENDS_DURING_KEYWORD
  | TR_MEETS_KEYWORD | TR_MET_BY_KEYWORD
  | TR_PARALLELS_KEYWORD | TR_PARALLELED_BY_KEYWORD
  | TR_STARTS_BEFORE_KEYWORD | TR_STARTS_AFTER_KEYWORD
  ;


intLiteral returns [Double d = null]:
    i:INT
    {
      String number = i.getText();
      int start = 0;
      if(number.charAt(0) == '-' || number.charAt(0) == '+')
        start = 1;

      if(number.length() > start+2 && number.charAt(start) == '0') {
        if(number.charAt(start+1) == 'x' || number.charAt(start+1) == 'X') {
          if(start == 1 && number.charAt(0) == '-')
            d = new Double(-Integer.parseInt(number.substring(3), 16));
          else
            d = new Double(Integer.parseInt(number.substring(2), 16));
        }
        else
          d = new Double(Integer.parseInt(number, 8));
      }
      else {
        d = new Double(number);
      }
    }
  | PINF {d = new Double(Double.POSITIVE_INFINITY);}
  | NINF {d = new Double(Double.NEGATIVE_INFINITY);}
  ;

floatLiteral returns [Double d = null]:
    f:FLOAT {d = new Double(f.getText());}
  | PINFF {d = new Double(Double.POSITIVE_INFINITY);}
  | NINFF {d = new Double(Double.NEGATIVE_INFINITY);}
  ;

boolLiteral returns [Boolean b = null]:
    TRUE_KEYWORD  {b = new Boolean(true);}
  | FALSE_KEYWORD {b = new Boolean(false);}
  ;

literal[NddlType t] {Boolean b;}:
    STRING
  | b=boolLiteral {intersect(BOOL,t,b);}
  | domain[t]
  | q:qualified[t,false,false,false,true]
    { if(#q.getType() != SYMBOL)
        throw new SemanticException("\""+#q.getText()+"\" is not a symbol literal.");}
  ;

function {List a;}:
    qualifiedName[null] DOT! 
    ( SPECIFY_KEYWORD^ a=variableArgumentList
    | FREE_KEYWORD^ a=variableArgumentList
    | CONSTRAIN_KEYWORD^ a=variableArgumentList
    | MERGE_KEYWORD^ a=variableArgumentList
    | ACTIVATE_KEYWORD^ LPAREN! RPAREN!
    | RESET_KEYWORD^ LPAREN! RPAREN!
    | REJECT_KEYWORD^ LPAREN! RPAREN!
    | CANCEL_KEYWORD^ LPAREN! RPAREN!)
  | IDENT DOT! CLOSE_KEYWORD^ LPAREN! RPAREN!
  ;

tokenNameList:
    LPAREN^ (tokenNames)? RPAREN!
  ;

tokenNames:
    IDENT (COMMA! tokenNames)?
  ;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class NddlLexer extends Lexer;
options {
  k = 2;
  exportVocab = Nddl;
  charVocabulary = '\3'..'\177';
  defaultErrorHandler = false;
}

{
  protected boolean interp = false;
  private boolean prematureEOF = false;
  private File file = null;
  public NddlLexer(Reader reader, File file, boolean interp)
  {
    this(reader);
    this.file = file;
    this.interp = interp;
    if(file != null)
      setFilename(file.getName());
    else
      setFilename("eval");
  }
  public File getFile() {return file;}

  public void setPrematureEOF() {
    prematureEOF = true;
  }

  public boolean hasPrematureEOF() {
    return prematureEOF;
  }
}

LBRACKET: '[';
RBRACKET: ']';
LBRACE: '{';
RBRACE: '}';
LPAREN: '(';
RPAREN: ')';
DCOLON: "::";
DEQUALS: "==";
NEQUALS: "!=";
DAMP: "&&";
DPIPE: "||";
IS_A: "<:";
EQUALS: '=';
SEMICOLON: ';';
COMMA: ',';
INCLUDE_DECL: "#include";
// this is entirely handled by the Lexer here:
LINE_DECL: "#line" (SIMPLEWS)+ (DIGIT)+
         { // use declaration to set current line
            int nstart = getText().lastIndexOf(' ')+1;
           if(nstart == 0) nstart = getText().lastIndexOf('\t')+1;
           if(nstart == 0) nstart = getText().lastIndexOf('\f')+1; // freaks with formfeeds
           setLine(Integer.parseInt(getText().substring(nstart))-1); }
         ((SIMPLEWS)+ '"' (ESC|~('"'|'\\'))*
         { //use declaraction to set current file
           int sstart = getText().lastIndexOf('"')+1;
           setFilename(getText().substring(sstart));
         } '"')? {$setType(Token.SKIP);}
         ;
STRING: '"'! (ESC|~('"'|'\\'))* '"'!
      | '\''! (ESC|~('\''|'\\'))* '\''!
      ;
ESC: '\\' ('n' | 't' | 'b' | 'f' | '"' | '\'' | '\\' | UNICODE_ESC | NUMERIC_ESC);
protected UNICODE_ESC: 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT;
protected NUMERIC_ESC: DIGIT DIGIT DIGIT;
protected DIGIT: ('0'..'9');
protected HEX_DIGIT: (DIGIT|'A'..'F'|'a'..'f');
protected PLUS:  '+'  ;
protected MINUS:  '-'  ;
// the following returns tokens: DOT or FLOAT or INT
INT
  {boolean isDecimal=false;}
  : (PLUS | MINUS)?
  ( '.' {$setType(DOT);}
  ((DIGIT)+ (EXPONENT)?
      (FLOAT_SUFFIX)? { _ttype = FLOAT; })?
  |  (  '0' {isDecimal = true;} // special case for just '0'
      (  ('x'|'X')
        (                      // hex
          // the 'e'|'E' and float suffix stuff look
          // like hex digits, hence the (...)+ doesn't
          // know when to stop: ambig.  ANTLR resolves
          // it correctly by matching immediately.  It
          // is therefor ok to hush warning.
          options {
            warnWhenFollowAmbig=false;
          }
        :  HEX_DIGIT
        )+
      |  ('0'..'7')+                  // octal
      )?
    |  ('1'..'9') (DIGIT)*  {isDecimal=true;}    // non-zero decimal
    )
    (  ('l'|'L')
    
    // only check to see if it's a float if looks like decimal so far
    |  {isDecimal}?
      (  '.' (DIGIT)* (EXPONENT)? (FLOAT_SUFFIX)?
      |  EXPONENT (FLOAT_SUFFIX)?
      |  FLOAT_SUFFIX
      )
      {$setType(FLOAT);}
    )?
  )
  ;
// a few protected methods to assist in matching floating point numbers
protected EXPONENT:  ('e'|'E') ('+'|'-')? ('0'..'9')+;
protected FLOAT_SUFFIX:  'f'|'F'|'d'|'D';

PINF: PLUS "inf" (FLOAT_SUFFIX {_ttype = PINFF;})? ;
NINF: MINUS "inf" (FLOAT_SUFFIX {_ttype = NINFF;})? ;
IDENT: ('a'..'z'|'A'..'Z'|'_'|'$') ('a'..'z'|'A'..'Z'|'_'|'0'..'9'|'$')*
     ;

// Whitespace -- ignored
protected SIMPLEWS
  options {
    paraphrase = "simple whitespace: tabs, spaces (not newlines)";
  }
  : ( ' '
    | '\t'
    | '\f')
  ;
WS  
  options {
    paraphrase = "whitespace, such as a space, tab, or newline";
  }
  :  (  SIMPLEWS
    // handle newlines
    |  (  "\r\n"  // Evil DOS
      |  '\r'    // Macintosh
      |  '\n'    // Unix (the right way)
      )
      { newline(); }
    )
    {$setType(Token.SKIP);}
  ;

// Single-line comments
SL_COMMENT
  options {
    paraphrase = "a single line comment, delimited by //";
  }
  :  "//"
    (~('\n'|'\r'))* 
    {$setType(Token.SKIP);}
  ;

// multiple-line comments
ML_COMMENT
  options {
    paraphrase = "a multiple line comment, such as /* <comment> */";
  }
  : "/*"
    (options {generateAmbigWarnings=false;}:
      { LA(2)!='/' }? '*'
    | "\r\n"    {newline();}
    | '\r'      {newline();}
    | '\n'      {newline();}
    |  ~('*'|'\n'|'\r')
    )*
    "*/"
    {$setType(Token.SKIP);}
  ;
