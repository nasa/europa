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
    for(std::map<edouble, ObjectFactoryId>::const_iterator it = m_objectFactories.begin(); it != m_objectFactories.end(); ++it)
        delete (ObjectFactory*) it->second;
    m_objectFactories.clear();

    for(std::map<edouble, TokenTypeId>::const_iterator it = m_tokenTypes.begin(); it != m_tokenTypes.end(); ++it)
        delete (TokenType*) it->second;
    m_tokenTypes.clear();
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

const std::map<edouble,ObjectFactoryId>& ObjectType::getObjectFactories() const
{
    return m_objectFactories;
}

const std::map<edouble,TokenTypeId>& ObjectType::getTokenTypes() const
{
    return m_tokenTypes;
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
    m_objectFactories[(edouble)(factory->getSignature())] = factory;
}

void ObjectType::addTokenType(const TokenTypeId& factory)
{
    // TODO: allow redefinition of old one
    m_tokenTypes[(edouble)(factory->getSignature())] = factory;
}

const TokenTypeId& ObjectType::getTokenType(const LabelStr& signature) const
{
    check_error(signature.getElement(0,".")==getName(),
            "Can't look for a token factory I don't own");

    std::map<edouble,TokenTypeId>::const_iterator it = m_tokenTypes.find((edouble)signature);
    if (it != m_tokenTypes.end())
        return it->second;

    if (m_parent.isId()) {
        std::string parentSignature = m_parent->getName().toString()+"."+signature.getElement(1,".").toString();
        return m_parent->getTokenType(parentSignature);
    }

    return TokenTypeId::noId();
}

const TokenTypeId& ObjectType::getParentType(const TokenTypeId& type) const
{
    if (m_parent.isId()) {
        std::string parentSignature = m_parent->getName().toString()+"."+type->getPredicateName().toString();
        return m_parent->getTokenType(parentSignature);
    }

    return TokenTypeId::noId();
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
        std::map<edouble,ObjectFactoryId>::const_iterator it = m_objectFactories.begin();
        for(;it != m_objectFactories.end(); ++it)
            os << "    " << it->second->getSignature().c_str() << std::endl;
    }

    os << std::endl;

    {
        std::map<edouble,TokenTypeId>::const_iterator it = m_tokenTypes.begin();
        for(;it != m_tokenTypes.end(); ++it) {
            TokenTypeId tokenType = it->second;
            os << "    " << tokenType->getSignature().c_str();
            std::map<LabelStr,DataTypeId>::const_iterator paramIt = tokenType->getArgs().begin();
            for(;paramIt != tokenType->getArgs().end();++paramIt)
                os<< " " << paramIt->second->getName().c_str() /*type*/ << "->" << paramIt->first.c_str()/*name*/;
            os << std::endl;
        }
    }

    os << "}" << std::endl;

    return os.str();
}


}

