#include <iostream>
#include <stdlib.h>

#include <PaVT/avl.h>

namespace pavt {

void AVL::insert(const int& key) {
  Node* new_node = new Node(key);
  auto return_node = (Node*)BinarySearchTree::insert(new_node);
  if (return_node == nullptr) {
    delete new_node;
  } else {
    rebalance(return_node);
  }
}

void AVL::remove(const int& key) {
  auto balance_nodes = BinarySearchTree::remove((Node*)root, key);
  if (balance_nodes->first != nullptr) {
    rebalance((Node*)balance_nodes->first);
    if (balance_nodes->second != nullptr) rebalance((Node*)balance_nodes->second);
  } 
  delete balance_nodes;
}

// Rotates node to the left. Child becomes nodes parent.
void AVL::rotateLeft(Node *child, Node *node, Node *parent) {

  // Grab the nodes right child
  Node *newRoot = child;

  // Give node the left child of the rotated node since the
  // key is greater than node
  Node *temp = (Node*)newRoot->left;
  node->right = (temp);

  // The node's right child (temp) now moves up to take the place of
  // node
  newRoot->left = (node);

  // Update parents
  if(temp!=nullptr) temp->parent =(node);

  Node *rootParent = parent;

  if (rootParent->right == node) {
    rootParent->right = (newRoot);
  } else {
    rootParent->left = (newRoot);
  }

  newRoot->parent = (rootParent);
  node->parent = (newRoot);

  // Update the tree heights
  int leftHeight = height((Node*)node->left);
  int rightHeight = height((Node*)node->right);
  node->height = (1 + std::max(leftHeight, rightHeight));

  int newRootLeftHeight = height((Node*)newRoot->left);
  int newRootRightHeight = height((Node*)newRoot->right);
  newRoot->height = (1 + std::max(newRootLeftHeight, newRootRightHeight));
}

//Rotates node to the right. Child becomes nodes parent
void AVL::rotateRight(Node *child, Node *node, Node *parent) {

  // Grab the nodes left child
  Node* newRoot = child;

  // Give node the left child of newRoot since the key
  // is less than node
  Node *temp = (Node*)newRoot->right;
  node->left = (temp);

  // The new Root moves up to take the place of node
  // Now set newNodes right pointer to node
  newRoot->right = (node);

  // Update parents
  if(temp!=nullptr) temp->parent = (node);

  Node *rootParent = parent;
  if (rootParent->right == node) {
    rootParent->right = (newRoot);
  } else {
    rootParent->left = (newRoot);
  }

  newRoot->parent = (rootParent);
  node->parent = (newRoot);

  // Update the tree heights
  int leftHeight = height((Node*)node->left);
  int rightHeight = height((Node*)node->right);
  node->height = (1 + std::max(leftHeight, rightHeight));

  int newRootLeftHeight = height((Node*)newRoot->left);
  int newRootRightHeight = height((Node*)newRoot->right);
  newRoot->height = (1 + std::max(newRootLeftHeight, newRootRightHeight));
}

/*
 * Returns the height of node
 */
int AVL::height(Node *node) {
  return (node == nullptr) ? -1 : node->height;
}

/*
 * Check the balance factor at this node, it does not meet requirements
 * perform tree rebalance
 *
 * @param node
 */
void AVL::rebalance(Node *node) {

  if (node==root) {
    return;
  }

  Node *parent = (Node*)node->parent;

  while(node!=root) {

    // lock parent
    parent->lock.lock();
    if (node->parent!=parent) {
      parent->lock.unlock();
      if (node->mark) {
        return;
      }

      parent = (Node*)node->parent;
      continue;
    }

    // lock node
    node->lock.lock();
    if (node->mark) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    Node *left = (Node*)node->left;
    Node *right= (Node*)node->right;

    int leftHeight = height(left);
    int rightHeight = height(right);

    int currHeight = std::max(leftHeight, rightHeight) + 1;
    int prevHeight = node->height;

    int bf = leftHeight - rightHeight;
    if (currHeight != prevHeight) {
      node->height = (currHeight);
    } else if (bf <= 1) {
      node->lock.unlock();
      parent->lock.unlock();
      return;
    }

    Node *child;
    // The node's right subtree is too tall
    if (bf < MINBF) {
      child = right;
      child->lock.lock();

      Node *childLeft = (Node*)child->left;
      Node *childRight = (Node*)child->right;

      int childLeftHeight = height(childLeft);
      int childRightHeight = height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childLeft;

      // Need to do double rotation
      if (childBf > 0) {
        grandChild->lock.lock();
        rotateRight(grandChild, child, node);
        rotateLeft(grandChild, node, parent);
        child->lock.unlock();
        node->lock.unlock();
        grandChild->lock.unlock();
        parent->lock.unlock();

        node = grandChild;

      } else {
        rotateLeft(child, node, parent);
        node->lock.unlock();
        child->lock.unlock();
        parent->lock.unlock();

        node = child;
      }

      // The node's left subtree is too tall
    } else if (bf > MAXBF) {
      child = left;
      child->lock.lock();

      Node *childLeft = (Node*)child->left;
      Node *childRight = (Node*)child->right;

      int childLeftHeight = height(childLeft);
      int childRightHeight = height(childRight);

      int childBf = childLeftHeight - childRightHeight;

      Node *grandChild = childRight;

      if (childBf < 0) {
        grandChild->lock.lock();

        rotateLeft(grandChild, child, node);
        rotateRight(grandChild, node, parent);
        node->lock.unlock();
        child->lock.unlock();
        grandChild->lock.unlock();
        parent->lock.unlock();

        node = grandChild;
      } else {

        rotateRight(child, node, parent);

        node->lock.unlock();
        child->lock.unlock();
        parent->lock.unlock();

        node = child;
      }

    } else {

      node->lock.unlock();
      parent->lock.unlock();

      // Traverse back up tree
      node = parent;
      parent = (Node*)node->parent;
    }
  }
}
} // namespace pavt