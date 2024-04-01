#include <gtest/gtest.h>
#include <skiplist.h>
#include <string>

LevelGenerator levelGenerator{4};

using SL = SkipList<std::string, std::string, LevelGenerator>;

TEST(SkipListTests, SkipListConstructor) {

    SL skipList{levelGenerator};

    // Newly constructed skip list contains only a header node.
    const auto &nodes = skipList.getNodes();
    EXPECT_EQ(nodes.size(), 1);

    // The header node has level equal to MAX_LEVEL.
    const auto &node = nodes[0];
    EXPECT_EQ(node->d_level, SL::Constants::MAX_LEVEL);

    // The header node has all next pointers set to null.
    for(auto nextNode : node->d_nextNode) {
        EXPECT_EQ(nextNode, nullptr);
    }
}

TEST(SkipListTests, insertSingleKVPair) {

    SL skipList{levelGenerator};

    skipList.insert("key", "value");

    // After insert, the skip list contains one header node and one data node.
    const auto &nodes = skipList.getNodes();
    EXPECT_EQ(nodes.size(), 2);

    // The header node has level equal to MAX_LEVEL.
    const auto &header = nodes[0];
    EXPECT_EQ(header->d_level, SL::Constants::MAX_LEVEL);

    // The header points to the data node via it's level 1 next node pointer.
    EXPECT_EQ(header->d_nextNode[0], nodes[1].get());

    // The header has all other next nodes set to nullptr.
    for(size_t index = 1; index < SL::Constants::MAX_LEVEL; ++index) {
        EXPECT_EQ(header->d_nextNode[1], nullptr);
    }

    // The new node has level equal to 1.
    const SL::Node* newNode = nodes[1].get();
    EXPECT_EQ(newNode->d_level, 1);

    // The new node has key equal to "key".
    EXPECT_EQ(newNode->d_key, "key");

    // The new node has value equal to "value".
    EXPECT_EQ(newNode->d_value, "value");

    // The header node has all next pointers set to null.
    for(auto nextNode : newNode->d_nextNode) {
        EXPECT_EQ(nextNode, nullptr);
    }
}

bool nodesAreEquivalent(const SL::Node &lhs, const SL::Node &rhs) {
    if(lhs.d_key   != rhs.d_key   ||
       lhs.d_value != rhs.d_value || 
       lhs.d_level != rhs.d_level) {
        return false;
    }
    for(size_t index = 0; index < SL::Constants::MAX_LEVEL; ++index) {
        SL::Node *lhsNext = lhs.d_nextNode[index], 
                 *rhsNext = rhs.d_nextNode[index];
        if(lhsNext == nullptr && rhsNext == nullptr) {
            continue;
        }
        if(lhsNext == nullptr || rhsNext == nullptr) {
            return false;
        }
        if(lhsNext->d_key   != rhsNext->d_key ||
           lhsNext->d_value != rhsNext->d_value) {
            return false;
        }
    }
    return true;
}

TEST(SkipListTests, insertTwoKVPairs) {
    SL skipList{levelGenerator};
    skipList.insert("key1", "value1");
    skipList.insert("key2", "value2");
    SL::Node header{"", "", SL::Constants::MAX_LEVEL};
    SL::Node key1Node("key1", "value1", 1);
    SL::Node key2Node("key2", "value2", 2);
    header.d_nextNode[0]   = &key1Node;
    header.d_nextNode[1]   = &key2Node;
    key1Node.d_nextNode[0] = &key2Node;
    EXPECT_TRUE(nodesAreEquivalent(*skipList.getNodes()[0].get(), header));
    EXPECT_TRUE(nodesAreEquivalent(*skipList.getNodes()[1].get(), key1Node));
    EXPECT_TRUE(nodesAreEquivalent(*skipList.getNodes()[2].get(), key2Node));
}

TEST(SkipListTests, insertFiveKVPairs) {
    SL skipList{levelGenerator};
    skipList.insert("key1", "value1");
    skipList.insert("key2", "value2");
    skipList.insert("key3", "value3");
    skipList.insert("key4", "value4");
    skipList.insert("key5", "value5");

    // Node with "key5" has level 1 and doesn't point to any other node.
    SL::Node key5Node{"key5", "value5", 1};

    // Node with "key4" has level 4 and points to the node with "key5".
    SL::Node key4Node{"key4", "value4", 4};
    key4Node.d_nextNode[0] = &key5Node;

    // Node with "key3" has level 3 and points to the node with "key4".
    SL::Node key3Node{"key3", "value3", 3};
    key3Node.d_nextNode[0] = key3Node.d_nextNode[1] = key3Node.d_nextNode[2] = &key4Node;

    // Node with "key2" has level 2 and points to the node with "key3".
    SL::Node key2Node{"key2", "value2", 2};
    key2Node.d_nextNode[0] = key2Node.d_nextNode[1] = &key3Node;

    // Node with "key1" has level 1 and points to the node with "key2".
    SL::Node key1Node{"key1", "value1", 1};
    key1Node.d_nextNode[0] = &key2Node;

    // Header has level 4 and points to nodes with keys "key1", "key2", "key3" and "key4".
    SL::Node header{"", "", SL::Constants::MAX_LEVEL};
    header.d_nextNode[0] = &key1Node;
    header.d_nextNode[1] = &key2Node;
    header.d_nextNode[2] = &key3Node;
    header.d_nextNode[3] = &key4Node;

    const auto &nodes = skipList.getNodes();
    EXPECT_TRUE(nodesAreEquivalent(*nodes[0].get(), header));
}

TEST(SkipListsTests, insertNineKVPairsRandomly) {
    SL skipList{levelGenerator};
    skipList.insert("key5", "value5");
    skipList.insert("key1", "value1");
    skipList.insert("key3", "value3");
    skipList.insert("key7", "value7");
    skipList.insert("key8", "value8");
    skipList.insert("key4", "value4");
    skipList.insert("key2", "value2");
    skipList.insert("key9", "value9");
    skipList.insert("key6", "value6");

    SL::Node key1Node{"key1", "value1", 2};
    SL::Node key2Node{"key2", "value2", 3};
    SL::Node key3Node{"key3", "value3", 3};
    SL::Node key4Node{"key4", "value4", 2};
    SL::Node key5Node{"key5", "value5", 1};
    SL::Node key6Node{"key6", "value6", 1};
    SL::Node key7Node{"key7", "value7", 4};
    SL::Node key8Node{"key8", "value8", 1};
    SL::Node key9Node{"key9", "value9", 4};

    SL::Node header{"", "", SL::Constants::MAX_LEVEL};
    header.d_nextNode[0] = header.d_nextNode[1] = &key1Node;
    header.d_nextNode[2] = &key2Node;
    header.d_nextNode[3] = &key7Node;

    const auto &nodes = skipList.getNodes();
    EXPECT_TRUE(nodesAreEquivalent(*nodes[0].get(), header));

    key1Node.d_nextNode[0] = key1Node.d_nextNode[1] = &key2Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[2].get(), key1Node));

    key2Node.d_nextNode[0] = key2Node.d_nextNode[1] = key2Node.d_nextNode[2] = &key3Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[7].get(), key2Node));

    key3Node.d_nextNode[0] = key3Node.d_nextNode[1] = &key4Node;
    key3Node.d_nextNode[2] = &key7Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[3].get(), key3Node));

    key4Node.d_nextNode[0] = &key5Node;
    key4Node.d_nextNode[1] = &key7Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[6].get(), key4Node));

    key5Node.d_nextNode[0] = &key6Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[1].get(), key5Node));

    key6Node.d_nextNode[0] = &key7Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[9].get(), key6Node));

    key7Node.d_nextNode[0] = &key8Node;
    key7Node.d_nextNode[1] = key7Node.d_nextNode[2] = key7Node.d_nextNode[3] = &key9Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[4].get(), key7Node));

    key8Node.d_nextNode[0] = &key9Node;
    EXPECT_TRUE(nodesAreEquivalent(*nodes[5].get(), key8Node));

    EXPECT_TRUE(nodesAreEquivalent(*nodes[8].get(), key9Node));
}

TEST(SkipListTests, findLocatesExistingKey) {
    SL skipList;
    skipList.insert("key5", "value5");
    skipList.insert("key1", "value1");
    skipList.insert("key3", "value3");
    skipList.insert("key7", "value7");
    skipList.insert("key8", "value8");
    skipList.insert("key4", "value4");
    skipList.insert("key2", "value2");
    skipList.insert("key9", "value9");
    skipList.insert("key6", "value6");
    EXPECT_EQ(skipList.find("key4"), "value4");
    EXPECT_EQ(skipList.find("key5"), "value5");
    EXPECT_EQ(skipList.find("key6"), "value6");
}

TEST(SkipListTests, findDoesNotFindNonExistingKey) {
    SL skipList;
    skipList.insert("key5", "value5");
    skipList.insert("key1", "value1");
    skipList.insert("key3", "value3");
    skipList.insert("key7", "value7");
    skipList.insert("key8", "value8");
    skipList.insert("key4", "value4");
    skipList.insert("key2", "value2");
    skipList.insert("key9", "value9");
    skipList.insert("key6", "value6");
    EXPECT_EQ(skipList.find("NON-EXISTING-KEY"), std::nullopt);
}

TEST(SkipListTests, removeRemovesExistingKey) {
    SL skipList;
    skipList.insert("key5", "value5");
    skipList.insert("key1", "value1");
    skipList.insert("key3", "value3");
    skipList.insert("key7", "value7");
    skipList.insert("key8", "value8");
    skipList.insert("key4", "value4");
    skipList.insert("key2", "value2");
    skipList.insert("key9", "value9");
    skipList.insert("key6", "value6");
    EXPECT_EQ(skipList.find("key5"), "value5");
    skipList.remove("key5");
    EXPECT_EQ(skipList.find("key5"), std::nullopt);
    skipList.insert("key5", "value15");
    EXPECT_EQ(skipList.find("key5"), "value15");
    skipList.remove("key5");
    EXPECT_EQ(skipList.find("key5"), std::nullopt);
}