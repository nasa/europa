/*
 * ObjectType.cc
 *
 *  Created on: Jan 30, 2009
 *      Author: javier
 */

#include "ObjectType.hh"


namespace EUROPA {

ObjectType::ObjectType(const char* name, const char* parent, bool isNative)
    : m_id(this)
    , m_name(name)
    , m_parent(parent)
    , m_isNative(isNative)
{
}

ObjectType::~ObjectType()
{
    m_id.remove();
}

const ObjectTypeId& ObjectType::getId() const
{
    return m_id;
}

const LabelStr& ObjectType::getName() const
{
    return m_name;
}

const LabelStr& ObjectType::getParent() const
{
    return m_parent;
}

const std::map<std::string,std::string>& ObjectType::getMembers() const
{
    return m_members;
}

const std::map<double,ObjectFactoryId>& ObjectType::getObjectFactories() const
{
    return m_objectFactories;
}

const std::map<double,TokenFactoryId>& ObjectType::getTokenFactories() const
{
    return m_tokenFactories;
}

bool ObjectType::isNative() const
{
    return m_isNative;
}

void ObjectType::addMember(const char* type, const char* name)
{
    // TODO: error checking
    m_members[name] = type;
}

void ObjectType::addObjectFactory(const ObjectFactoryId& factory)
{
    // TODO: allow redefinition of old one
    m_objectFactories[(double)(factory->getSignature())] = factory;
}

void ObjectType::addTokenFactory(const TokenFactoryId& factory)
{
    // TODO: allow redefinition of old one
    m_tokenFactories[(double)(factory->getSignature())] = factory;
}

std::string ObjectType::toString() const
{
    std::ostringstream os;

    os << "class " << m_name.c_str() << " extends " << m_parent.c_str() << " {" << std::endl;

    {
        std::map<std::string,std::string>::const_iterator it = m_members.begin();
        for(;it != m_members.end(); ++it)
            os << "    " << it->second /*type*/ << " " <<  it->first/*name*/ << std::endl;
    }

    os << std::endl;

    {
        std::map<double,ObjectFactoryId>::const_iterator it = m_objectFactories.begin();
        for(;it != m_objectFactories.end(); ++it)
            os << "    " << it->second->getSignature().c_str() << std::endl;
    }

    os << std::endl;

    {
        std::map<double,TokenFactoryId>::const_iterator it = m_tokenFactories.begin();
        for(;it != m_tokenFactories.end(); ++it) {
            TokenFactoryId tokenFactory = it->second;
            os << "    " << tokenFactory->getSignature().c_str();
            std::map<LabelStr,LabelStr>::const_iterator paramIt = tokenFactory->getArgs().begin();
            for(;paramIt != tokenFactory->getArgs().end();++paramIt)
                os<< " " << paramIt->second.c_str() /*type*/ << "->" << paramIt->first.c_str()/*name*/;
            os << std::endl;
        }
    }

    os << "}" << std::endl;

    return os.str();
}


}

