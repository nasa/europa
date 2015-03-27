#ifndef CONSTRAINTTYPECHECKING_H_
#define CONSTRAINTTYPECHECKING_H_

#include "ConstraintType.hh"
#include "DataType.hh"

#include <vector>

/**
 * @file   ConstraintTypeChecking.hh
 * @author Michael Iatauro <Michael.J.Iatauro@nasa.gov>
 * 
 * @brief  Defines template expressions for checking of constraint type signatures.
 *
 * Constraints often have restrictions on the number and types of the variables in their
 * scope in much the same way that functions have on their parameters.  This header 
 * provides a system for concisely stating and composing those restrictions
 * so that constraint implementors don't have to manually write code to enforce them or to
 * duplicate code between constraints with shared type signatures.
 * 
 * It achieves this through an expression template system that consists of predicates,
 * accessors, and iterators.
 *
 * Predicates
 *
 * Predicates are (mostly) unary functions that evaluate to a boolean value indicating
 * a successful (true) or failed (false) check.  
 * There are two predicates that check the number of arguments rather than properties of
 * specific arguments:
 *
 *   `NArgs<N>` specifies an exact number of arguments to check.  `NArgs<3>` will check 
 *   that a constraint has three arguments.
 *
 *   `AtLeastNArgs<N>`specified a minimum number of arguments to check.  `AtLeastNArgs<3>`
 *   will check that a constraint has no fewer than three arguments.
 *
 * The rest of the predicates expect to get the type to check from an accessor or an
 * iterator.
 *
 *   `CanBePositive` asserts that a numeric argument has a positive upper bound.
 * 
 *   `Numeric` asserts that an argument is numeric @see DataType::isNumeric
 *
 *   `IsEntity` asserts that an argument is of some Entity type (e.g. Object) 
 *    @see DataType::isEntity
 *
 *   `Type<T>` asserts that an argument is of some specific DataType.  T must be a subclass
 *   of DataType.
 *
 *   `Same` asserts that consecutive arguments have the same type. 
 *    (TODO: Redesign this to be consistent with Assignable and Comparable.)
 * 
 *
 * The last two predicates, Assignable and Comparable, specify a relationship between two
 * argument types.  As a result,
 * they have two forms: one with no parameter that takes a pair of arguments at checking-time
 * (@see Mutually) and one with an accessor parameter for a particular argument.  The 
 * single-parameter form performs the same check as the parameterless, but with the parameter
 * as the first argument.
 *
 *   `Comparable` asserts that its first argument can be compared to its second.  
 *    @see DataType::canBeCompared
 *
 *   `Assignable` asserts that its first argument is assignable from its second.
 *   @see DataType::isAssignableFrom
 *
 * Accessors
 *
 * A set of named accessors for specific arguments are provided.  Currently they are `First`
 * through `Fourth`, as well as an Nth for any access whose index is known at compile time.
 * There are two special relative accessors, Last and End, for the final argument and
 * one-past-the-end, respectively.
 *
 * Accessors other than End have two forms, one without parameters that simply acts as an
 * accessor and one with a single predicate parameter that asserts a property of a specific
 * parameter.  `First<Type<BoolDT> >` asserts that the first argument to the constraint is
 * a boolean variable.  
 *
 * Iterators
 *
 *   `All` asserts that some predicate is true for all arguments in a range.  By default,
 *   the range is from First to End.
 *
 *   `Mutually` asserts that some predicate is true for all pairs of arguments in a range.
 *   By default, the range is from First to End.
 *
 * Assertions can be combined with the logical operator `And`.
 * And takes two template expressions.  When evaluated, it will evaluate both expressions
 * and return the logical conjunction of their evaluation.  It always evaluates both of its
 * arguments in order to present error messages from both checks so the user doesn't have
 * to re-run the checking process after fixing the error message from one.
 *
 * Composition
 * 
 * There are a couple of ways to create a type check intended for composition into larger
 * checks (or simply to save typing).  If the type check is intended to be used directly,
 * a simple typedef will do:
 *
 * typedef And<NArgs<2>, And<Mutually<Assignable<> >, All<Numeric> > > TwoAssignableNumeric;
 *
 * The TwoAssignableNumeric type can then be used directly:
 *
 * typedef DataTypeCheck<AbsoluteValue, TwoAssignableNumeric>  AbsoluteValueCT;
 *
 * or as part of a conjunction:
 *
 * typedef DataTypeCheck<SomeConstraint, And<TwoAssignableNumeric, First<Type<IntDT> > >  AbsoluteValueCT;
 *
 * On the other hand, some constraints have the same type requirements as other constraints,
 * but shifted to account for additional variables in the scope.  `allDiff` and `condAllDiff`
 * are an excellent example: condAllDiff has a leading boolean parameter that allDiff doesn't,
 * but the type requirements on the rest of the parameters are the same.  Because typedefs
 * aren't allowed to take template arguments, a struct is required:
 *
 * template<typename Start = First<>, typename End = End>
 * struct AllAssignable : Mutually<Assignable<>, Start, End> {
 *   AllAssignable() : Mutually<Assignable<>, Start, End>() {}
 *   AllAssignable(type_iterator start, type_iterator end)
 *     : Mutually<Assignable<>, Start, End>(start, end){}
 * };
 * 
 * The first line declares the parameters for the argument range to check, defaulting to
 * the whole range with AllAssignable<>.  The second declares AllAssignable as a subclass
 * of Mutually<Assignable<> > over that range.  Lines three through five are boilerplate.
 * Creating this struct allows the allDiff constraint check to be declared as AllAssignable
 * and the condAllDiff to be declared as 
 * And<And<AtLeastNArgs<3>, First<Type<BoolDT> >, AllAssignable<Second<>, End> >
 * (note that this isn't exactly what's done in the source--the 
 * And<AtLeastNArgs<3>, First<Type<BoolDT> > is declared separately to be reused in other
 * constraint types)
 */

namespace EUROPA {

//TODO: put this in a namespace?
typedef std::vector<DataTypeId> type_vector;
typedef std::vector<DataTypeId>::const_iterator type_iterator;

template<type_vector::size_type N>
struct NArgs {
  NArgs(){}
  NArgs(type_iterator, type_iterator) {}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                  std::ostream& os) const {
    if(std::distance(start, end) != N) {
      os << "Constraint " << name << " takes " << N << " args, not " <<
          std::distance(start, end) << std::endl;
      return false;
    }
    return true;
  }
};

template<int N>
struct AtLeastNArgs {
  AtLeastNArgs(){}
  AtLeastNArgs(type_iterator, type_iterator){}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                  std::ostream& os) const {
    if(std::distance(start, end) < N) {
      os << "Constraint " << name << " takes at least " << N << " args, not " <<
          std::distance(start, end) << std::endl;
      return false;
    }
    return true;
  }
};

struct None;

template<int N, typename Check = None>
struct Nth {
  Nth(type_iterator, type_iterator) {}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                  std::ostream& os) const {
    
    Check c;
    if(start == end) {
      os << "Constraint " << name << " ran out of arguments before checking " <<
          c.description() << std::endl;
      return false;
    }
    std::advance(start, N);
    if(!c(name, *start, os)) {
      os << "Constraint " << name << " requires that argument " << N << " be " <<
          c.description() << std::endl;
      return false;
    }
    return true;
  }
};

template<int N>
struct Nth<N, None> {
  Nth() {}
  type_iterator operator()(type_iterator start, type_iterator end) const {
    std::advance(start, N);
    return start;
  }
};

#define INDEXED_CHECKER(name, number) \
  template<typename Check = None> \
  struct name : public Nth<number, Check> { \
  name(type_iterator a, type_iterator b) : Nth<number, Check>(a, b) {} \
  }; \
  template<> \
  struct name<None> : public Nth<number, None> { \
  name() : Nth<number, None>() {} \
  };

INDEXED_CHECKER(First, 0);
INDEXED_CHECKER(Second, 1);
INDEXED_CHECKER(Third, 2);
INDEXED_CHECKER(Fourth, 3);

template<typename Check = None>
struct Last {
  Last(type_iterator, type_iterator) {}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                    std::ostream& os) {
    checkError(start != end);
    std::advance(start, std::distance(start, end) - 1);
    Check c;
    if(!c(name, *start, os)) {
      os << "Constraint " << name << " requires that its last argument be " <<
          c.description() << std::endl;
    }
    return true;
  }
};

template<>
struct Last<None> {
  type_iterator operator()(type_iterator start, type_iterator end) const {
    checkError(start != end);
    std::advance(start, std::distance(start, end) - 1);
    return start;
  }
};

struct End {
  type_iterator operator()(type_iterator start, type_iterator end) const {
    return end;
  }
};

template<typename Check, typename Start = First<>, typename End = End>
struct All {
  All(){}
  All(type_iterator, type_iterator){}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                  std::ostream& os) const {
    bool retval = true;
    type_iterator begin = Start()(start, end);
    type_iterator finish = End()(start, end);
    Check c(start, end);
    for(; begin != finish; ++begin) {
      if(!c(name, *begin, os)) {
        os << "Constraint " << name << " takes a " << c.description() << " argument at "
            << std::distance(begin, finish) << ", not a " << (*begin)->getName() <<
            std::endl;
        retval = false;
      }
    }
    return retval;
  }

};

template<typename A, typename B>
struct And {
  And(){}
  And(type_iterator, type_iterator) {}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                  std::ostream& os) const {
    A m_a(start, end);
    B m_b(start, end);
    bool a = m_a(name, start, end, os);
    bool b = m_b(name, start, end, os);
    return a && b;
  }
  bool operator()(const std::string& name, DataTypeId dt, std::ostream& os) const {
    bool retval = true;
    A m_a;
    B m_b;
    if(!m_a(name, dt, os)) {
      os << "Constraint " << name << " requires that " << dt->getName() <<
          " be " << m_a.description() << std::endl;
      retval = false;
    }
    if(!m_b(name, dt, os)) {
      os << "Constraint " << name << " requires that " << dt->getName() <<
          " be " << m_b.description() << std::endl;
      retval = false;
    }
    return retval;
  }
  std::string description() const {
    std::ostringstream str;
    A m_a;
    B m_b;
    str << m_a.description() << " and " << m_b.description();
    return str.str();
  }
};

template<typename Check, typename Start = First<>, typename End = End>
struct Mutually {
  Mutually(){}
  Mutually(type_iterator, type_iterator){}
  bool operator()(const std::string& name, type_iterator start, type_iterator end,
                  std::ostream& os) const {
    Check m_c(start, end);
    type_iterator begin = Start()(start, end);
    type_iterator finish = End()(start, end);
    type_iterator next = begin;
    std::advance(next, 1);
    bool retval = true;
    for(; next != finish; ++begin, ++next) {
      retval = (*this)(name, *begin, *next, os) && retval;
    }
    return retval;
  }

  bool operator()(const std::string& name, DataTypeId one, DataTypeId two,
                  std::ostream& os) const {
    Check m_c;
    bool a = m_c(name, one, two, os);
    bool b = m_c(name, two, one, os);
    if(!a || !b) {
      os << "Constraint " << name << " requires that " <<
          one->getName() << " and " << two->getName() <<
          " be mutually " << m_c.description() << std::endl;
      return false;
    }
    return true;
  }
};

//predicates
//Bounds accessors?
struct CanBePositive {
  CanBePositive(){}
  CanBePositive(type_iterator, type_iterator){}
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const;
  std::string description() const {return "possibly positive";}
};

struct Numeric {
  Numeric(){}
  Numeric(type_iterator, type_iterator){}
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const {
    return dt->isNumeric();
  }
  std::string description() const {return "numeric";}
};

struct IsEntity {
  IsEntity(){}
  IsEntity(type_iterator, type_iterator){}
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const {
    return dt->isEntity();
  }
  std::string description() const {return "entity";}
};

template<typename Accessor = None>
struct Assignable {
  Assignable(type_iterator start, type_iterator end) : m_d() {
    checkError(start != end);
    Accessor a;
    m_d = *a(start, end);
    checkError(m_d.isValid());
  }
  Assignable() : m_d(Accessor::instance()) {}
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const {
    return m_d->isAssignableFrom(dt);
  }
  std::string description() const {return "assignable";}

  DataTypeId m_d;
};

template<>
struct Assignable<None> {
  Assignable(){}
  Assignable(type_iterator, type_iterator){}
  bool operator()(const std::string&, DataTypeId dt1, DataTypeId dt2, std::ostream&) const {
    return dt1->isAssignableFrom(dt2) || (dt1->isNumeric() && dt2->isNumeric());
  }
  std::string description() const {return "assignable";}
};

template<typename Accessor = None>
struct Comparable {
  Comparable(type_iterator start, type_iterator end) : m_d() {
    checkError(start != end);
    Accessor a;
    m_d = *a(start, end);
    checkError(m_d.isValid());
  }
  Comparable() : m_d(Accessor::instance()) {}
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const {
    return m_d->canBeCompared(dt);
  }
  std::string description() const {return "assignable";}

  DataTypeId m_d;
};

template<>
struct Comparable<None> {
  Comparable(){}
  Comparable(type_iterator, type_iterator){}
  bool operator()(const std::string&, DataTypeId dt1, DataTypeId dt2, std::ostream&) const {
    return dt1->canBeCompared(dt2);
  }
  std::string description() const {return "assignable";}
};

/**
 * @struct Same
 * 
 */

struct Same {
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const {
    if(!dtSaved.isValid())
      dtSaved = dt;
    return dtSaved == dt;
  }
  std::string description() const {return "same";}
  mutable DataTypeId dtSaved;
};

/**
 * @struct Type
 * Predicate.  Checks that a data type is of a specific type by comparing DataType pointers.
 */
template<typename T>
struct Type {
  Type(){}
  Type(type_iterator, type_iterator) {}
  bool operator()(const std::string&, DataTypeId dt, std::ostream&) const {
    return T::instance() == dt;
  }
  std::string description() const {return T::instance()->getName();}
};

/**
 * @class DataTypeCheckBase
 * DataTypeCheckBase checks the vector of argument types for the constraint against the
 * check expression given in the TypeCheck parameter, throwing a message string if
 * the check fails (returns false).
 */
//TODO: Make this not use throw.
template<typename TypeCheck>
class DataTypeCheckBase : public ConstraintType {
 public:
  DataTypeCheckBase(const std::string& _name, const std::string& _propagatorName,
                bool systemDefined = false)
      : ConstraintType(_name, _propagatorName, systemDefined), m_check() {}
  
  virtual ConstraintId createConstraint(const ConstraintEngineId constraintEngine,
                                const std::vector<ConstrainedVariableId>& scope,
                                const std::string& violationExpl) = 0;

  void checkArgTypes(const std::vector<DataTypeId>& argTypes) const {
    std::ostringstream str;
    if(!m_check(this->m_name, argTypes.begin(), argTypes.end(), str))
      throw str.str();
  }
 private:
  TypeCheck m_check;
};

/**
 * @class DataTypeCheck
 * This class is the constraint-developer-level interface to the type-check generator code.
 * The Constr type parameter is the type of the constraint class to create and the TypeCheck
 * parameter is the statement of the checks to be performed.  
 *
 * All the implementation really does is extend DataTypeCheckBase and implement the 
 * ConstraintType::createConstraint method.
 */
template<typename Constr, typename TypeCheck>
class DataTypeCheck : public DataTypeCheckBase<TypeCheck> {
 public:
  DataTypeCheck(const std::string& _name, const std::string& _propagatorName,
                bool systemDefined = false)
      : DataTypeCheckBase<TypeCheck>(_name, _propagatorName, systemDefined) {}
  
  ConstraintId createConstraint(const ConstraintEngineId constraintEngine,
                                const std::vector<ConstrainedVariableId>& scope,
                                const std::string& violationExpl) {
    return makeConstraintInstance<Constr>(this->m_name, this->m_propagatorName,
                                          constraintEngine,scope, violationExpl);
    
  }
};
}

#endif /* CONSTRAINTTYPECHECKING_H_ */
