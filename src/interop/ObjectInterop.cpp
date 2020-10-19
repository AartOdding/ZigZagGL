#include <Application.hpp>
#include <interop/ObjectInterop.hpp>
#include <object/Node.hpp>
#include <object/NodeTemplateGroup.hpp>
#include <util/StringUtils.hpp>

#include <iostream>



namespace
{
    CreateNodeDelegate createNodeDelegate = nullptr;
    DestroyNodeDelegate destroyNodeDelegate = nullptr;
    SetNodeParentDelegate setNodeParentDelegate = nullptr;

}

namespace Host
{

    Identifier<Node> createNode(Identifier<NodeTemplate> templateID, Identifier<Node> parentID)
    {
        if (templateID && createNodeDelegate)
        {
            return Identifier<Node>(createNodeDelegate(static_cast<std::uint64_t>(templateID),
                                                       static_cast<std::uint64_t>(parentID)));
        }
        return Identifier<Node>();
    }

    bool destroyNode(Identifier<Node> nodeID)
    {
        if (nodeID && destroyNodeDelegate)
        {
            return destroyNodeDelegate(static_cast<std::uint64_t>(nodeID));
        }
        return false;
    }

    bool setNodeParent(Identifier<Node> nodeID, Identifier<Node> newParentID)
    {
        if (nodeID && setNodeParentDelegate)
        {
            return setNodeParentDelegate(static_cast<std::uint64_t>(nodeID),
                                         static_cast<std::uint64_t>(newParentID));
        }
        return false;
    }

}


ZIGZAG_API void installObjectDelegates(CreateNodeDelegate create,
    DestroyNodeDelegate destroy, SetNodeParentDelegate setParent)
{
    std::cout << "[editor dll] installing object delegates" << std::endl;
    createNodeDelegate = create;
    destroyNodeDelegate = destroy;
    setNodeParentDelegate = setParent;
}


ZIGZAG_API void onTemplateAdded(const char* name, std::uint64_t templateID_, NodeCategory category)
{
    std::cout << "[editor dll] object type added: " << name << " " << templateID_ << std::endl;
    auto templateID = Identifier<NodeTemplate>(templateID_);
    auto nameParts = StringUtils::split(name, '.');

    if (templateID && !nameParts.empty())
    {
        auto nodeTemplate = NodeTemplate::create(nameParts.back(), templateID);
        nodeTemplate->setCategory(category);
        nameParts.pop_back();

        NodeTemplateGroup* templateGroup = Application::getGlobalInstance()->getRootTypeNamespace();

        for (const auto& part : nameParts)
        {
            auto childNamespace = templateGroup->getChild(part);

            if (childNamespace)
            {
                templateGroup = childNamespace;
            }
            else
            {
                templateGroup = templateGroup->addChild(NodeTemplateGroup::create(part));
            }
        }

        templateGroup->addType(std::move(nodeTemplate));
    }
}


ZIGZAG_API void onNodeCreated(std::uint64_t nodeID_, std::uint64_t parentID_, std::uint64_t templateID_)
{
    std::cout << "[editor dll] object created: " << nodeID_ << std::endl;

    Identifier<Node> nodeID(nodeID_);
    Identifier<Node> parentID(parentID_);
    Identifier<NodeTemplate> templateID(templateID_);

    auto parent = const_cast<Node*>(IdentityMap<Node>::get(parentID));
    auto type = IdentityMap<NodeTemplate>::get(templateID);

    auto object = Node::create(nodeID);

    if (type)
    {
        object->setName(type->getName());
        object->setCategory(type->getCategory());

        if (parent)
        {
            parent->addChild(std::move(object));
        }
        else
        {
            Application::getGlobalInstance()->addParentlessNode(std::move(std::move(object)));
        }
    }
}


ZIGZAG_API void onNodeDestroyed(std::uint64_t objectID_)
{
    auto objectID = Identifier<Node>(objectID_);

    std::cout << "[editor dll] object destroyed: " << objectID << std::endl;

    auto objectConst = IdentityMap<Node>::get(Identifier<Node>(objectID));
    auto object = const_cast<Node*>(objectConst);

    if (object && object->getParent())
    {
        object->getParent()->removeChild(Identifier<Node>(objectID));
    }
    else if (object)
    {
        Application::getGlobalInstance()->takeParentlessNode(objectID);
    }
}


ZIGZAG_API void onNodeParentChanged(std::uint64_t nodeID_, std::uint64_t newParentID_)
{
    auto nodeID = Identifier<Node>(nodeID_);
    auto newParentID = Identifier<Node>(newParentID_);

    auto nodePtr = const_cast<Node*>(IdentityMap<Node>::get(nodeID));
    auto newParentPtr = const_cast<Node*>(IdentityMap<Node>::get(newParentID));
    auto oldParentPtr = nodePtr->getParent();

    if (nodePtr)
    {
        Node::Pointer object;

        if (nodePtr->getParent())
        {
            object = nodePtr->getParent()->removeChild(nodeID);
        }
        else
        {
            object = Application::getGlobalInstance()->takeParentlessNode(nodeID);
        }
        if (object)
        {
            if (newParentPtr)
            {
                newParentPtr->addChild(std::move(object));
            }
            else
            {
                Application::getGlobalInstance()->addParentlessNode(std::move(object));
            }
        }
    }
    std::cout << "[editor dll] object reparented: " << nodeID << std::endl;
}


ZIGZAG_API void setProjectRootNode(const char* name, std::uint64_t projectNodeID)
{
    std::cout << "[editor dll] project created: " << name << " " << projectNodeID << std::endl;

    auto node = Application::getGlobalInstance()->takeParentlessNode(Identifier<Node>(projectNodeID));

    if (node)
    {
        Application::getGlobalInstance()->setRootObject(std::move(node));
    }
    else
    {
        std::cout << "[editor dll] project root node did not exist" << std::endl;
    }
}
