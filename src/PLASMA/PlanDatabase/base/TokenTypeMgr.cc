#include "TokenTypeMgr.hh"
#include "Schema.hh"

namespace EUROPA
{

/*
 * TokenTypeMgr.cc
 *
 *  Created on: Jun 29, 2009
 *      Author: mak
 */

TokenTypeMgr::TokenTypeMgr()
: m_id(this)
{
}

TokenTypeMgr::~TokenTypeMgr()
{
	cleanup(m_types);
	m_id.remove();
}

const TokenTypeMgrId& TokenTypeMgr::getId() const {return m_id;}

void TokenTypeMgr::registerType(const TokenTypeId& type) {
	check_error(type.isValid());

	// Ensure it is not present already
	check_error(m_types.find(type) == m_types.end()) ;

	m_types.insert(type);
	m_typesByPredicate.insert(std::make_pair(type->getSignature().getKey(), type));
}

/**
 * First try a hit for the predicate name as provided. If that does not work, extract the object,
 * and try each parent object until we get a hit.
 */
TokenTypeId TokenTypeMgr::getType(const SchemaId& schema, const LabelStr& predicateName){
	check_error(schema->isPredicate(predicateName), predicateName.toString() + " is undefined.");

	// Confirm it is present
	const std::map<edouble, TokenTypeId>::const_iterator pos = m_typesByPredicate.find(predicateName.getKey());

	if (pos != m_typesByPredicate.end()) // We have found what we are looking for
		return(pos->second);

	// If we are here, we have not found it, so build up a list of parents, and try each one. We have to use the schema
	// for this.

	// Call recursively if we have a parent
	if (schema->hasParent(predicateName)) {
		TokenTypeId type =  getType(schema, schema->getParent(predicateName));

		check_error(type.isValid(), "No type found for " + predicateName.toString());

		// Log the mapping in this case, from the original predicate, to make it faster the next time around
		m_typesByPredicate.insert(std::make_pair(predicateName, type));
		return(type);
	}

	// If we get here, it is an error
	check_error(ALWAYS_FAILS, "Failed in TokenTypeMgr::getType for " + predicateName.toString());

	return TokenTypeId::noId();
}

bool TokenTypeMgr::hasType()
{
	return (!m_types.empty());
}

void TokenTypeMgr::purgeAll()
{
	cleanup(m_types);
	m_typesByPredicate.clear();
}


} //namespace EUROPA

