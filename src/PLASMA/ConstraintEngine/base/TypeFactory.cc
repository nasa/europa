#include "TypeFactory.hh"
#include "Debug.hh"

namespace EUROPA 
{
  TypeFactory::TypeFactory(const std::string& typeName)
    : m_id(this)
    , m_typeName(typeName)
  {
  }

  TypeFactory::~TypeFactory()
  {
      m_id.remove();
  }

  const TypeFactoryId& TypeFactory::getId() const
  {
      return m_id;
  }

  const LabelStr& TypeFactory::getTypeName() const
  {
      return m_typeName;
  }
} // namespace EUROPA
