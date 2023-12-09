#include <iostream>

#include <gtest/gtest.h>

#define JSON_PARSE_IMPLEMENTATION
#include "../include/jsonParse.hpp"

using namespace std;
using namespace jsonParse;

TEST(JsonParse, simpleObj){
    const string simpleObj = "{\"name\":\"mister\"}"; 
    /*
    struct JsonObj obj = parseJson(simpleObj.c_str(), simpleObj.size());
    EXPECT_EQ(obj.values.size(), 1);
    const auto &iter = obj.values.find("name");
    EXPECT_FALSE(iter == obj.values.end());
    EXPECT_TRUE(*((std::string*)(iter->second.value)) == "mister");
    */
}

TEST(JsonParse, simpleList){
    EXPECT_FALSE(true);
}

int main(){
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}