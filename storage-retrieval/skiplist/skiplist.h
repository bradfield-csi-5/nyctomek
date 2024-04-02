#ifndef SKIP_LIST
#define SKIP_LIST

#include <vector>
#include <memory>
#include <optional>
#include <cstdlib>
#include <algorithm>


class LevelGenerator {
public:
    LevelGenerator(size_t maxLevel) : d_level{0}, d_maxLevel{maxLevel} {}
    size_t computeLevel() {
        d_level %= d_maxLevel;
        return ++d_level;      
    }
private:
    size_t d_level;
    size_t d_maxLevel;
};

template<typename Key, typename Value, typename LevelGenerator>
class SkipList {

public:
    enum Constants {
        MAX_LEVEL = 16
    };

    struct Node {

        Node(const Key &key, const Value &value, size_t level) : 
            d_key{key},
            d_value{value},
            d_level{level},
            d_nextNode{0} {}

        Key    d_key;
        Value  d_value;
        size_t d_level;
        Node*  d_nextNode[Constants::MAX_LEVEL];
    };

    explicit SkipList(std::optional<LevelGenerator> levelGenerator = std::nullopt);

    void remove(const Key &key);

    void insert(const Key &key, const Value& value);

    std::optional<Value> find(const Key &key) const;

    const std::vector<std::unique_ptr<Node>>& getNodes() const;
private:

    size_t computeLevel();

    std::vector<std::unique_ptr<Node>> d_nodes;
    std::optional<LevelGenerator> d_levelGenerator;

};

template<typename Key, typename Value, typename LevelGenerator>
SkipList<Key, Value, LevelGenerator>::SkipList(std::optional<LevelGenerator> levelGenerator) :
    d_nodes{},
    d_levelGenerator{levelGenerator} {
    d_nodes.emplace_back(new Node(Key{}, Value{}, SkipList::Constants::MAX_LEVEL));  // Header.
}

template<typename Key, typename Value, typename LevelGenerator>
void SkipList<Key, Value, LevelGenerator>::remove(const Key &key) {
    Node *node = d_nodes[0].get();
    size_t level = Constants::MAX_LEVEL;
    Node *previousNodes[Constants::MAX_LEVEL] = {0};
    while(level) {
        size_t levelIndex = level - 1;
        Node *nextNode = node->d_nextNode[levelIndex];
        if(nextNode && nextNode->d_key < key) {
            node = nextNode;
        }
        else {
            previousNodes[levelIndex] = node;
            level--;
        }
    }
    Node *nodeToRemove = nullptr;
    if(node->d_nextNode[0] && node->d_nextNode[0]->d_key == key) {
        nodeToRemove = node->d_nextNode[0];
        // At this point, level is 1.  Walk back up the levels and
        // update our `d_nextNode` pointers to skip over the deleted node.
        while(level <= Constants::MAX_LEVEL) {
            size_t levelIndex = level - 1;
            if(previousNodes[levelIndex]->d_nextNode[levelIndex] == nodeToRemove) {
                previousNodes[levelIndex]->d_nextNode[levelIndex] = nodeToRemove->d_nextNode[levelIndex];
            }
            ++level;
        }
    }
    auto iter = std::find_if(d_nodes.begin(), d_nodes.end(),
        [&nodeToRemove](auto &currentNode){return currentNode.get() == nodeToRemove;});
    d_nodes.erase(iter);
}

template<typename Key, typename Value, typename LevelGenerator>
void SkipList<Key, Value, LevelGenerator>::insert(const Key &key, const Value &value) {
    Node *node = d_nodes[0].get();
    size_t level = Constants::MAX_LEVEL;
    Node *previousNodes[Constants::MAX_LEVEL] = {0};
    while(level) {
        size_t levelIndex = level-1;
        previousNodes[levelIndex] = node;
        Node *nextNode = node->d_nextNode[levelIndex];
        if(nextNode != nullptr && nextNode->d_key < key) {
            node = nextNode;
        }
        else {
            --level;
        }
    }
    Node *newNode = new Node{key, value, computeLevel()};
    for(size_t newNodeLevel = newNode->d_level; newNodeLevel > 0; --newNodeLevel) {
        size_t newNodeLevelIndex = newNodeLevel-1;
        Node *previousNodeAtLevel = previousNodes[newNodeLevelIndex];
        if(previousNodeAtLevel) {
            newNode->d_nextNode[newNodeLevelIndex] = previousNodeAtLevel->d_nextNode[newNodeLevelIndex];
            previousNodeAtLevel->d_nextNode[newNodeLevelIndex] = newNode;
        }
        else {
            // If there is no previous node at the given level
            // to connect to the new node, we connect the header.
            d_nodes[0].get()->d_nextNode[newNodeLevelIndex] = newNode;
        }
    }
    d_nodes.push_back(std::unique_ptr<Node>(newNode));
}

template<typename Key, typename Value, typename LevelGenerator>
std::optional<Value> SkipList<Key, Value, LevelGenerator>::find(const Key &key) const {
    Node *currentNode = d_nodes[0].get();
    size_t level = currentNode->d_level;
    while(level) {
        size_t levelIndex = level - 1;
        Node *nextNode = currentNode->d_nextNode[levelIndex];
        if(nextNode && nextNode->d_key == key) {
            return nextNode->d_value;
        }
        if(nextNode && nextNode->d_key < key) {
            currentNode = nextNode;
            level = currentNode->d_level;
        }
        else {
            --level;
        }
    }
    return std::nullopt;
}

template<typename Key, typename Value, typename LevelGenerator>
size_t SkipList<Key, Value, LevelGenerator>::computeLevel() {
    if(d_levelGenerator != std::nullopt) {
        return d_levelGenerator->computeLevel();
    }
    size_t level = 1;
    while(std::rand() % 2 && level < Constants::MAX_LEVEL) {
        level++;
    }
    return level;
}

template<typename Key, typename Value, typename LevelGenerator>
const std::vector<std::unique_ptr<typename SkipList<Key, Value, LevelGenerator>::Node>>& 
SkipList<Key, Value, LevelGenerator>::getNodes() const {
    return d_nodes;
}

#endif