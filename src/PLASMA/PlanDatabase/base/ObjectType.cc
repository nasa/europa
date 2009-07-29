/*
 * ObjectType.cc
 *
 *  Created on: Jan 30, 2009
 *      Author: javier
 */

#include "ObjectType.hh"
#include "Object.hh"

namespace EUROPA {

ObjectType::ObjectType(const char* name, const ObjectTypeId& parent, bool isNative)
    : m_id(this)
    , m_varType((new ObjectDT(name))->getId())
    , m_name(name)
    , m_parent(parent)
    , m_isNative(isNative)
{
}

ObjectType::~ObjectType()
{
    // TODO: enable this when Schema API is cleaned up to reflect the fact that object factories and token factories are owned by the object type
    // purgeAll();
    delete (ObjectDT*)m_varType;
    m_id.remove();
}

const DataTypeId& ObjectType::getVarType() const
{
    return m_varType;
}

void ObjectType::purgeAll()
{
    for(std::map<double, ObjectFactoryId>::const_iterator it = m_objectFactories.begin(); it != m_objectFactories.end(); ++it)
        delete (ObjectFactory*) it->second;
    m_objectFactories.clear();

    for(std::map<double, TokenFactoryId>::const_iterator it = m_tokenFactories.begin(); it != m_tokenFactories.end(); ++it)
        delete (TokenFactory*) it->second;
    m_tokenFactories.clear();
}


const ObjectTypeId& ObjectType::getId() const
{
    return m_id;
}

const LabelStr& ObjectType::getName() const
{
    return m_name;
}

const ObjectTypeId& ObjectType::getParent() const
{
    return m_parent;
}

const std::map<std::string,DataTypeId>& ObjectType::getMembers() const
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

void ObjectType::addMember(const DataTypeId& type, const char* name)
{
    m_members[name] = type;
}

const DataTypeId& ObjectType::getMemberType(const char* name) const
{
    std::map<std::string,DataTypeId>::const_iterator it = m_members.find(name);

    if (it != m_members.end())
        return it->second;

    if (std::string(name) == "this")
        return getVarType();

    if (m_parent.isId())
        return m_parent->getMemberType(name);

    return DataTypeId::noId();
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

const TokenFactoryId& ObjectType::getTokenFactory(const LabelStr& signature) const
{
    check_error(signature.getElement(0,".")==getName(),
            "Can't look for a token factory I don't own");

    std::map<double,TokenFactoryId>::const_iterator it = m_tokenFactories.find((double)signature);
    if (it != m_tokenFactories.end())
        return it->second;

    if (m_parent.isId()) {
        std::string parentSignature = m_parent->getName().toString()+"."+signature.getElement(1,".").toString();
        return m_parent->getTokenFactory(parentSignature);
    }

    return TokenFactoryId::noId();
}

const TokenFactoryId& ObjectType::getParentFactory(const TokenFactoryId& factory) const
{
    if (m_parent.isId()) {
        std::string parentSignature = m_parent->getName().toString()+"."+factory->getPredicateName().toString();
        return m_parent->getTokenFactory(parentSignature);
    }

    return TokenFactoryId::noId();
}

std::string ObjectType::toString() const
{
    std::ostringstream os;
    std::string extends = (m_parent.isId() ? std::string("extends ")+m_parent->getName().c_str() : "");

    os << "class " << m_name.c_str() << " extends " << extends << " {" << std::endl;

    {
        std::map<std::string,DataTypeId>::const_iterator it = m_members.begin();
        for(;it != m_members.end(); ++it)
            os << "    "
               << it->second->getName().toString() /*type*/ << " "
               <<  it->first/*name*/
               << std::endl;
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
            std::map<LabelStr,DataTypeId>::const_iterator paramIt = tokenFactory->getArgs().begin();
            for(;paramIt != tokenFactory->getArgs().end();++paramIt)
                os<< " " << paramIt->second->getName().c_str() /*type*/ << "->" << paramIt->first.c_str()/*name*/;
            os << std::endl;
        }
    }

    os << "}" << std::endl;

    return os.str();
}


}

