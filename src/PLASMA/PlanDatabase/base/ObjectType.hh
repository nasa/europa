/*
 * ObjectType.hh
 *
 *  Created on: Jan 30, 2009
 *      Author: javier
 */

#ifndef OBJECTTYPE_HH_
#define OBJECTTYPE_HH_

#include <map>
#include <vector>

#include "ObjectFactory.hh"
#include "TokenFactory.hh"

namespace EUROPA {

class ObjectType;
typedef Id<ObjectType> ObjectTypeId;

class ObjectType
{
public:
    ObjectType(const char* name, const char* parentClass, bool isNative=false);
    virtual ~ObjectType();

    const ObjectTypeId& getId() const;

    virtual const LabelStr& getName() const;
    virtual const LabelStr& getParent() const;
    virtual bool isNative() const;

    virtual void addMember(const char* type, const char* name); // TODO: use DataType instead
    virtual const std::map<std::string,std::string>& getMembers() const;

    virtual void addObjectFactory(const ObjectFactoryId& factory);
    virtual const std::map<double,ObjectFactoryId>& getObjectFactories() const;

    virtual void addTokenFactory(const TokenFactoryId& factory);
    virtual const std::map<double,TokenFactoryId>& getTokenFactories() const;

    virtual std::string toString() const;

    void purgeAll(); // TODO: make protected after Schema API is fixed

protected:
    ObjectTypeId m_id;
    LabelStr m_name;
    LabelStr m_parent;
    bool m_isNative;
    std::map<double,ObjectFactoryId> m_objectFactories;
    std::map<double,TokenFactoryId> m_tokenFactories;
    std::map<std::string,std::string> m_members;
};

}


#endif /* OBJECTTYPE_HH_ */
