#include "Domain.hh"
#include "Entity.hh"
#include "Error.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "DataType.hh"

namespace EUROPA {

  ostream& operator<<(ostream& os, const Domain& dom) {
    dom >> os;
    return(os);
  }

  /**
   * @brief Constructor overwrites prior static instance, effecting a last-writer wins policy
   * for establishing the shared allocator.
   */
  DomainComparator::DomainComparator(){
  }

  /**
   * @brief On destruction, if the static instance is this object, then set it to null.
   */
  DomainComparator::~DomainComparator(){
  }

  /**
   * @brief Implements the default tests for comparison.
   */
  bool DomainComparator::canCompare(const Domain& domx, const Domain& domy) const
  {
    return domx.getDataType()->canBeCompared(domy.getDataType());
  }

  DomainListener::DomainListener()
    : m_id(this) {
  }

  DomainListener::~DomainListener() {
    m_id.remove();
  }

  const DomainListenerId DomainListener::getId() const {
    return(m_id);
  }

  Domain::Domain(const DataTypeId dt, bool enumerated, bool closed)
    : m_dataType(dt)
    , m_enumerated(enumerated)
    , m_closed(closed)
    , m_listener()
  {
  }

  Domain::Domain(const Domain& org)
    : m_dataType(org.m_dataType)
    , m_enumerated(org.m_enumerated)
    , m_closed(org.m_closed)
    , m_listener()
  {
  }

  Domain::~Domain() {}

  bool Domain::operator==(const Domain& dom) const {
    return(m_closed == dom.m_closed && isFinite() == dom.isFinite());
  }

  bool Domain::operator!=(const Domain& dom) const {
    return(!operator==(dom));
  }

  void Domain::close() {
    // Benign if already closed
    if(m_closed == true)
      return;

    m_closed = true;
    notifyChange(DomainListener::CLOSED);
    if (isEmpty()) // Empty initially, want to generate the event
      empty();
  }

  void Domain::open() {
    check_error(isClosed(), "Attempted to re-open a domain that is already open.");
    m_closed = false;
    notifyChange(DomainListener::OPENED);
  }

  void Domain::touch() {
    notifyChange(DomainListener::BOUNDS_RESTRICTED);
  }

  bool Domain::isClosed()     const { return(m_closed); }
  bool Domain::isOpen()       const { return(!m_closed); }
  bool Domain::isEnumerated() const { return(m_enumerated); }
  bool Domain::isInterval()   const { return(!m_enumerated); }
  bool Domain::isInfinite()   const { return(!isFinite()); }

  void Domain::setListener(const DomainListenerId listener) {
    check_error(m_listener.isNoId()); // Only set once
    m_listener = listener;

    // Now we want to send any events, since we added the listener a bit late in the game
    if (isClosed()) {
      notifyChange(DomainListener::CLOSED);
      if (isEmpty())
        notifyChange(DomainListener::EMPTIED); // Have to do this to force inconsistency
    }
  }

  const DomainListenerId Domain::getListener() const {
    return(m_listener);
  }

  void Domain::notifyChange(const DomainListener::ChangeType& changeType) {
    checkError(m_listener.isNoId() || m_listener.isValid(), m_listener);

    if (m_listener.isNoId())
      return;

    //Ensure we get the tightes notification if restricted to a singleton
    if(DomainListener::isRestriction(changeType) && isSingleton())
      m_listener->notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
    else
      m_listener->notifyChange(changeType);
  }

  void Domain::operator>>(ostream& os) const {
    os << getTypeName() << (m_closed ? ":CLOSED" : ":OPEN");
  }

  edouble Domain::translateNumber(edouble number, bool) const {
    return(number);
  }

  bool Domain::check_value(edouble value) const {
    checkPrecision(value);
    return(!isNumeric() || (value >= MINUS_INFINITY && value <= PLUS_INFINITY));
  }

  /**
   * @see DomainComparator::comparator
   */
  bool Domain::canBeCompared(const Domain& domx, const Domain& domy) {
    debugMsg("Domain:canBeCompared", "domx.isBool " << domx.isBool() << " domx.isNumeric " << domx.isNumeric() << " domx.isString " << domx.isString() << " domx.isSymbolic " << domx.isSymbolic());
    debugMsg("Domain:canBeCompared", "domy.isBool " << domy.isBool() << " domy.isNumeric " << domy.isNumeric() << " domy.isString " << domy.isString() << " domy.isSymbolic " << domy.isSymbolic());
    debugMsg("Domain:canBeCompared", "type of domx " << domx.getTypeName() << " type of domy " << domy.getTypeName());
    debugMsg("Domain:canBeCompared", domx.toString());
    debugMsg("Domain:canBeCompared", domy.toString());
    bool result = domx.getDataType()->canBeCompared(domy.getDataType());
    debugMsg("Domain:canBeCompared", "returning " << result);
    return result;
  }


  bool Domain::areBoundsFinite() const {
    return (!isNumeric() ||
	    (!isEmpty() && isClosed() &&
	     getUpperBound() < PLUS_INFINITY && getLowerBound() > MINUS_INFINITY));
  }

  std::string Domain::toString() const {
    std::stringstream s_stream;

    // Use fixed position notation on output for reals
    if(isNumeric() && minDelta() < 1)
      s_stream.setf(std::ios::fixed);

    s_stream << *this;
    return s_stream.str();
  }

  void Domain::assertSafeComparison(const Domain& domA, const Domain& domB){
    check_error(canBeCompared(domA, domB),
		domA.getTypeName() + " cannot be compared with " + domB.getTypeName());

  }

  std::string  Domain::toString(edouble value) const
  {
    checkError(isMember(value),  value << " not in " << toString());
    return getDataType()->toString(value);
  }


  const DataTypeId Domain::getDataType() const { return m_dataType; }
  void Domain::setDataType(const DataTypeId dt) { m_dataType=dt; }

  // TODO: all these just delegate to the data type, should be dropped eventually, preserved for now for backwards compatibility
const std::string& Domain::getTypeName() const { return getDataType()->getName(); }
  bool Domain::isSymbolic() const { return getDataType()->isSymbolic(); }
  bool Domain::isEntity() const { return getDataType()->isEntity(); }
  bool Domain::isNumeric() const { return getDataType()->isNumeric(); }
  bool Domain::isBool() const { return getDataType()->isBool(); }
  bool Domain::isString() const { return getDataType()->isString(); }
  bool Domain::isRestricted() const { return getDataType()->getIsRestricted(); }
  edouble Domain::minDelta() const {return getDataType()->minDelta();}
}
