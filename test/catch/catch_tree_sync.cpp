#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"

#include "Arduino.h"

#include "catch_utils.hpp"

#include "painlessmesh/plugin.hpp"
#include "plugin/performance.hpp"

// Testing the tree syncing method
// Previously we would send over the whole network the layout when syncing with
// neighbours, which would result into very large messages, potentially too
// large to keep in memory. New approach is to send it in parts, with first
// sending the root, and then sending updates
//
// Steps needed
// - Test update_subtree
// - Snip a tree into parts
// - Producing packages
// - Write a plugin to do it.
// - Replace old sync with the new plugin
// - Run the tests (and boost tests as well to make sure syncing works)

using namespace painlessmesh;

logger::LogClass Log;

/**
 * Split a tree into parts of the (max) size given by no_nodes
 */
std::list<painlessmesh::protocol::NodeTree> split_tree(
    painlessmesh::protocol::NodeTree tree, size_t no_nodes) {
  std::list<painlessmesh::protocol::NodeTree> parts;
  // parse tree, if size is bigger, cut rest off? Would be very inefficient?,
  // Might end up with very small leaves? Should I start counting from a leave?
  return parts;
}

SCENARIO("We can overwrite old subtree with new subtree") {
  auto expected_size = 6;
  auto tree1 = createNodeTree(6, true);
  REQUIRE(tree1.size() == 6);
  auto tree2 = createNodeTree(6, false);

  tree2.nodeId = tree1.subs.begin()->nodeId;
  expected_size += 6 - tree1.subs.begin()->size();

  tree1 = layout::update_subtree(std::move(tree1), tree2);
  REQUIRE(tree1.size() == expected_size);
}
