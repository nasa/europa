/*
 * CFunction.cc
 *
 *  Created on: Oct 2, 2009
 *      Author: jbarreir
 */

#include "CFunction.hh"

namespace EUROPA {

  CFunction::CFunction(const char* name)
      : m_id(this)
      , m_name(name)
  {
  }

  CFunction::~CFunction()
  {
	  m_id.remove();
  }

  const CFunctionId& CFunction::getId() const
  {
	  return m_id;
  }

  const LabelStr& CFunction::getName()
  {
     return m_name;
  }

}

