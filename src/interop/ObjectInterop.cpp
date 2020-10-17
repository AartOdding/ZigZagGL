#include <Application.hpp>
#include <interop/ObjectInterop.hpp>
#include <object/ZObject.hpp>
#include <object/ObjectTypeNamespace.hpp>
#include <util/StringUtils.hpp>

#include <iostream>



namespace
{
    AddObjectDelegate addObjectDelegate = nullptr;
    RemoveObjectDelegate removeObjectDelegate = nullptr;
}


bool addObject(Identifier<ObjectType> type, Identifier<ZObject> object)
{
    if (type && object && addObjectDelegate)
    {
        return addObjectDelegate(static_cast<std::uint64_t>(type),
                                 static_cast<std::uint64_t>(object));
    }
    return false;
}

bool removeObject(Identifier<ZObject> object)
{
    if (object && removeObjectDelegate)
    {
        return removeObjectDelegate(static_cast<std::uint64_t>(object));
    }
    return false;
}

ZIGZAG_API void installObjectDelegates(AddObjectDelegate add, RemoveObjectDelegate rm)
{
    std::cout << "[editor dll] installing object creation / destruction delegates" << std::endl;
    addObjectDelegate = add;
    removeObjectDelegate = rm;
}

ZIGZAG_API void onNewObjectTypeAdded(const char* name, std::uint64_t uniqueID, ObjectTypeCategory category)
{
    std::cout << "[editor dll] object type added: " << name << " " << uniqueID << std::endl;
    auto identifier = Identifier<ObjectType>(uniqueID);
    auto nameParts = StringUtils::split(name, '.');
    
    if (identifier && !nameParts.empty())
    {
        auto objectType = ObjectType::create(nameParts.back(), identifier);
        objectType->setCategory(category);
        nameParts.pop_back();

        ObjectTypeNamespace* typeNamespace = Application::getGlobalInstance()->getRootTypeNamespace();

        for (const auto& namespaceName : nameParts)
        {
            auto childNamespace = typeNamespace->getChild(namespaceName);

            if (childNamespace)
            {
                typeNamespace = childNamespace;
            }
            else
            {
                typeNamespace = typeNamespace->addChild(ObjectTypeNamespace::create(namespaceName));
            }
        }

        typeNamespace->addType(std::move(objectType));
    }
}

ZIGZAG_API void onObjectCreated(std::uint64_t newObject, std::uint64_t parentObject, std::uint64_t objectType)
{
    std::cout << "[editor dll] object created: " << newObject << std::endl;

    Identifier<ZObject> objectID(newObject);
    Identifier<ZObject> parentID(parentObject);
    Identifier<ObjectType> typeID(objectType);

    auto parent = const_cast<ZObject*>(IdentityMap<ZObject>::get(parentID));
    auto type = IdentityMap<ObjectType>::get(typeID);

    auto object = ZObject::create(objectID);

    if (type)
    {
        object->setName(type->getName());
        object->setNodeCategory(type->getCategory());

        if (parent)
        {
            parent->addChild(std::move(object));
        }
    }
}

ZIGZAG_API void onObjectDestroyed(std::uint64_t objectID)
{
    auto objectConst = IdentityMap<ZObject>::get(Identifier<ZObject>(objectID));
    auto object = const_cast<ZObject*>(objectConst);

    if (object && object->getParent())
    {
        object->getParent()->removeChild(Identifier<ZObject>(objectID));
    }

    //object->stealFromParent(); // not storing the returned unique_ptr will delete it.
}
