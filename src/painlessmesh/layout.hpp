#ifndef _PAINLESS_MESH_LAYOUT_HPP_
#define _PAINLESS_MESH_LAYOUT_HPP_

#include <algorithm>
#include <list>
#include <memory>

#include "painlessmesh/protocol.hpp"

namespace painlessmesh {
namespace layout {

#include <memory>

/*
 * Generic function to find if an element of any type exists in list
 */
template <typename T>
bool contains(std::list<T>& listOfElements, const T& element) {
  // Find the iterator if element in list
  auto it = std::find(listOfElements.begin(), listOfElements.end(), element);
  // return if iterator points to end or not. It points to end then it means
  // element
  // does not exists in list
  return it != listOfElements.end();
}

/**
 * Whether the tree contains the given nodeId
 */
inline bool contains(protocol::NodeTree nodeTree, uint32_t nodeId) {
  if (nodeTree.nodeId == nodeId) {
    return true;
  }
  return contains(nodeTree.knownNodes, nodeId);
}

template <class T>
class Layout {
 public:
  size_t stability = 0;

  // T is expected to behave like a NodeTree/Neighbour

  std::list<std::shared_ptr<T> > subs;

  /** Return the nodeId of the node that we are running on.
   *
   * On the ESP hardware nodeId is uniquely calculated from the MAC address of
   * the node.
   */
  uint32_t getNodeId() { return nodeId; }

 protected:
  uint32_t nodeId = 0;
  bool root = false;
};

template <typename T>
inline Layout<T> excludeRoute(Layout<T> layout, uint32_t exclude) {
  // Make sure to exclude any subs with nodeId == 0,
  // even if exlude is not set to zero
  layout.subs.remove_if(
      [exclude](auto s) { return s.nodeId == 0 || s.nodeId == exclude; });
  return layout;
}

template <class T>
void syncLayout(Layout<T>& layout, uint32_t changedId) {
  // TODO: this should be called from changed connections and dropped
  // connections events
  for (auto&& sub : layout.subs) {
    if (sub->connected() && !sub->newConnection && sub->nodeId != 0 &&
        sub->nodeId != changedId) {  // Exclude current
      sub->nodeSyncTask.forceNextIteration();
    }
  }
  layout.stability /= 2;
}

class Neighbour : public protocol::NodeTree {
 public:
  // Inherit constructors
  using protocol::NodeTree::NodeTree;

  /**
   * Is the passed nodesync valid
   *
   * If not then the caller of this function should probably disconnect
   * this neighbour.
   */
  bool validSubs(protocol::NodeTree tree) {
    if (nodeId == 0)  // Cant really know, so valid as far as we know
      return true;
    if (nodeId != tree.nodeId) return false;
    // Detect a loop, if the knownNodes contain the nodeId, then that can't be
    // good
    return !layout::contains(tree.knownNodes, nodeId);
  }

  /**
   * Update subs with the new subs
   *
   * \param tree The possible new tree with this node as base
   *
   * Generally one probably wants to call validSubs before calling this
   * function.
   *
   * \return Whether we adopted the new tree
   */
  bool updateSubs(protocol::NodeTree tree) {
    if (nodeId == 0 || tree != (*this)) {
      nodeId = tree.nodeId;
      knownNodes = tree.knownNodes;
      root = tree.root;
      containsRoot = tree.containsRoot;
      return true;
    }
    return false;
  }

  /**
   * Create a request
   */
  template <typename T>
  protocol::NodeSyncRequest request(layout::Layout<T> layout) {
    auto subTree = excludeRoute(std::move(layout), nodeId);
    return protocol::NodeSyncRequest(subTree.nodeId, nodeId, subTree.subs,
                                     subTree.root);
  }

  /**
   * Create a reply
   */
  template <typename T>
  protocol::NodeSyncReply reply(layout::Layout<T>&& layout) {
    auto subTree = excludeRoute(std::move(layout), nodeId);
    return protocol::NodeSyncReply(subTree.nodeId, nodeId, subTree.subs,
                                   subTree.root);
  }
};

/**
 * The size of the mesh (the number of nodes)
 */
inline uint32_t size(protocol::NodeTree nodeTree) {
  return nodeTree.knownNodes.size() + 1;
}

template <typename T>
inline uint32_t size(const Layout<T>& layout) {
  uint32_t no = 1;
  for (auto&& s : layout.subs) no += size((*s));
  return no;
}

/**
 * Whether the top node in the tree is also the root of the mesh
 */
inline bool isRoot(protocol::NodeTree nodeTree) {
  if (nodeTree.root) return true;
  return false;
}

/**
 * Whether any node in the tree is also root of the mesh
 */
inline bool isRooted(protocol::NodeTree nodeTree) {
  return (isRoot(nodeTree) || nodeTree.containsRoot);
}

/**
 * Return all nodes in a list container
 */
template <typename T>
inline std::list<uint32_t> asList(layout::Layout<T> nodeTree,
                                  bool includeSelf = true) {
  std::list<uint32_t> lst;
  if (includeSelf) lst.push_back(nodeTree.nodeId);
  for (auto&& s : nodeTree.subs) {
    lst.splice(lst.end(), asList(s));
  }
  return lst;
}

}  // namespace layout
}  // namespace painlessmesh

#endif
