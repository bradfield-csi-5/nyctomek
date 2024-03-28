#include <gtest/gtest.h>
#include <level2db_database.h>
#include <level2db_iterator.h>

TEST(Level2DBTest, HasReturnsFalse) {
    Level2DB::Database db;
    EXPECT_FALSE(db.Has("key"));
}

TEST(Level2DBTest, HasReturnsTrue) {
    Level2DB::Database db;
    db.Put("key", "value");
    EXPECT_TRUE(db.Has("key"));
}

TEST(Level2DBTest, GetReturnsValue) {
    Level2DB::Database db;
    db.Put("key", "value");
    std::variant<Level2DB::ErrorCode, Level2DB::Bytes>
        result = db.Get("key");
    Level2DB::Bytes expectedValue;
    EXPECT_NO_THROW(expectedValue = std::get<Level2DB::Bytes>(result));
    EXPECT_EQ(expectedValue, "value");
}

TEST(Level2DBTest, GetReturnsKeyNotFoundError) {
    Level2DB::Database db;
    using GetResult = std::variant<Level2DB::ErrorCode, Level2DB::Bytes>;
    GetResult result = db.Get("key");
    Level2DB::ErrorCode errorCode;
    EXPECT_NO_THROW(errorCode = std::get<Level2DB::ErrorCode>(result));
    EXPECT_EQ(errorCode, Level2DB::ErrorCode::KEY_NOT_FOUND);
}

TEST(Level2DBTest, DeleteRemovesExistingKey) {
    Level2DB::Database db;
    db.Put("key", "value");
    EXPECT_EQ(std::nullopt, db.Delete("key"));
    EXPECT_FALSE(db.Has("key"));
}

TEST(Level2DBTest, DeleteReturnsKeyNotFoundError) {
    Level2DB::Database db;
    EXPECT_EQ(Level2DB::ErrorCode::KEY_NOT_FOUND, db.Delete("key"));
}

TEST(Level2DBTest, RangeScanReturnsExpectedRange) {
    Level2DB::Database db;
    db.Put("key1", "value1");
    db.Put("key2", "value2");
    db.Put("key3", "value3");
    db.Put("key4", "value4");
    db.Put("key5", "value5");
    using namespace Level2DB;
    using RangeScanResult = std::variant<ErrorCode, IteratorInterface*>;
    RangeScanResult result = db.RangeScan("key1", "key4");
    Level2DB::IteratorInterface *iteratorInterface = nullptr;
    EXPECT_NO_THROW(iteratorInterface = std::get<IteratorInterface*>(result));
    Iterator *iterator = dynamic_cast<Iterator*>(iteratorInterface);
    EXPECT_TRUE(iterator);
    EXPECT_EQ(iterator->Key(), "key1");
    EXPECT_EQ(iterator->Value(), "value1");
    EXPECT_TRUE(iterator->Next());
    EXPECT_EQ(iterator->Key(), "key2");
    EXPECT_EQ(iterator->Value(), "value2");
    EXPECT_TRUE(iterator->Next());
    EXPECT_EQ(iterator->Key(), "key3");
    EXPECT_EQ(iterator->Value(), "value3");
    EXPECT_TRUE(iterator->Next());
    EXPECT_EQ(iterator->Key(), "key4");
    EXPECT_EQ(iterator->Value(), "value4");
    EXPECT_FALSE(iterator->Next());
    delete iterator;
}