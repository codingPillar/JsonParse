#include <iostream>

#include <gtest/gtest.h>
#include <string>
#include <vector>

#define JSON_PARSE_IMPLEMENTATION
#include "../include/jsonParse.hpp"

using namespace std;
using namespace jsonParse;

TEST(JsonParse, simpleObj){
    const string simpleObj = "{\"name\":\"mister\"}"; 
    struct JsonObj obj = parseJson(simpleObj.c_str(), simpleObj.size());
    EXPECT_EQ(obj.values.size(), 1);
    const auto &iter = obj.values.find("name");
    EXPECT_FALSE(iter == obj.values.end());
    EXPECT_TRUE(*((std::string*)(iter->second.value)) == "mister");
}

TEST(JsonParse, simpleList){
    const string simpleObj = "{\"names\": [\"adam\", \"patnais\"] }"; 
    struct JsonObj obj = parseJson(simpleObj.c_str(), simpleObj.size());
    EXPECT_EQ(obj.values.size(), 1);
    const auto &iter = obj.values.find("names");
    EXPECT_TRUE(iter != obj.values.end());
    EXPECT_EQ(iter->second.type, JSON_LIST);
    const std::vector<JsonObj::JsonValue> &value = *((std::vector<JsonObj::JsonValue>*)iter->second.value);
    EXPECT_EQ(value.size(), 2);
    EXPECT_EQ(value[0].type, JSON_STRING);
    EXPECT_EQ(*((std::string*)value[0].value), "adam");
    EXPECT_EQ(value[1].type, JSON_STRING);
    EXPECT_EQ(*((std::string*)value[1].value), "patnais");
}

TEST(JsonParse, complexObj){
    const string simpleObj = "{\"person\": {\"name\":\"adam\", \"age\":\"25\"} }"; 
    struct JsonObj obj = parseJson(simpleObj.c_str(), simpleObj.size());
    EXPECT_EQ(obj.values.size(), 1);
    const auto &iter = obj.values.find("person");
    EXPECT_TRUE(iter != obj.values.end());
    EXPECT_EQ(iter->second.type, JSON_OBJ);

    const JsonObj &value = *((JsonObj*)iter->second.value);
    const auto &secondIter = value.values.find("name");
    EXPECT_FALSE(secondIter == value.values.end());
    EXPECT_EQ(secondIter->second.type, JSON_STRING); 
    EXPECT_EQ(*((std::string*)secondIter->second.value), "adam"); 

    const auto &thirdIter = value.values.find("age");
    EXPECT_FALSE(thirdIter == value.values.end());
    EXPECT_EQ(thirdIter->second.type, JSON_STRING); 
    EXPECT_EQ(*((std::string*)thirdIter->second.value), "25"); 
}

int main(){
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}