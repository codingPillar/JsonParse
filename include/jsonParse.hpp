#ifndef JSON_PARSE_H
#define JSON_PARSE_H

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <iostream>

namespace jsonParse {

/* TODO, ADD SUPPORT FOR NUMBERS AND BOOL */
enum JsonValueType{
    JSON_STRING,
    JSON_OBJ,
    JSON_LIST,
    JSON_NUMBER
};

struct JsonObj{
    /* VALUE POINTS AT EITHER JsonObj OR std::string OR std::vector<JsonValue> OR float */
    class JsonValue{
    public:
        JsonValue(enum JsonValueType type, void *ressource): type(type), value(ressource) {}
        JsonValue(JsonValue &&value): type(value.type), value(value.value) { value.value = nullptr; }
        JsonValue(const JsonValue &value) = delete;
        JsonValue& operator=(JsonValue &value) = delete;
        ~JsonValue(){ 
            if(!value) return; 
            if(type == JSON_LIST) delete (std::vector<JsonValue>*) value;
            else if(type == JSON_STRING) delete (std::string*) value;
            else if(type == JSON_OBJ) delete (JsonObj*) value;
            else if(type == JSON_NUMBER) delete (float*) value;
        }

        /* ATTRIBS */
        enum JsonValueType type;
        /* THE USAGE OF UNIONS AND STD::VARIANT IS WAYY TOO TRASH, THIS IS WHY C IS JUST BETTER */
        void *value = nullptr;
    };
    std::unordered_map<std::string, JsonValue> values;
};

struct JsonObj parseJson(const char *buffer, unsigned int length);

}

#endif //JSON_PARSE_H

//#define JSON_PARSE_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)
#ifdef JSON_PARSE_IMPLEMENTATION

#include <cstring>

namespace jsonParse {

/* CONSTANTS */
constexpr unsigned int MAX_KEY_VALUE_BUFFER_SIZE = 256;

/* PRIVATE FUNCTIONS */
static char whiteChars[] = {' ', '\n', '\r'};
static bool isWhiteSpace(char element){
    for(unsigned int i = 0; i < sizeof(whiteChars); i++)
        if(element == whiteChars[i]) return true;
    return false;
}

static bool isNumeric(char value){
    return (value >= '0' && value <= '9') || value == '-';
}

std::string copyStr(const char *buffer, unsigned int length){
    std::string value;
    for(unsigned int i = 0; i < length; i++){
        if(buffer[i] == '\0') break;
        value += buffer[i];
    }
    return value;
}

static std::pair<struct JsonObj, int> parseJsonObj(const char *buffer, unsigned int length);

static std::pair<struct JsonObj::JsonValue, unsigned int> parseJsonString(const char *buffer, unsigned int length){
    bool started = false;
    unsigned int start = 0;
    unsigned int i = 0;
    for(; i < length; i++){
        if(!started && isWhiteSpace(buffer[i])) continue;
        if(!started && buffer[i] != '\"'){
            std::cout << "ENCOUTERED CHARACTER OTHER THAN \" ON READING STRING, ABORT" << std::endl;
            return {JsonObj::JsonValue{JSON_STRING, nullptr}, 0};
        } 
        else if(!started){
            started = true;
            start = i;
        }else if(started && buffer[i] == '\"'){
            std::string str = copyStr(&buffer[start + 1], i - start - 1);
            return {JsonObj::JsonValue{JSON_STRING, new std::string(std::move(str))}, i};
        }
    }
    std::cout << "NEVER ENCOUTERED END OF STRING, ABORT" << std::endl;
    return {JsonObj::JsonValue{JSON_STRING, nullptr}, 0};
}

static std::pair<struct JsonObj::JsonValue, int> parseJsonNumber(const char *buffer, unsigned int length){
    bool started = false;
    unsigned int start = 0;
    unsigned int i = 0;
    for(; i < length; i++){
        if(!started && isWhiteSpace(buffer[i])) continue;
        if(!started && !isNumeric(buffer[i])){
            std::cout << "ENCOUTERED UNVALID FIRST CHARACTER FOR NUMBER, ABORT" << std::endl;
            return {JsonObj::JsonValue{JSON_NUMBER, nullptr}, -1};
        } 
        if(!started){
            start = i;
            started = true;
        }else if(started && (isNumeric(buffer[i]) || buffer[i] == '.')) continue;
        else{
            std::string str = copyStr(&buffer[start], i - start);
            const float value = atoi(str.c_str());
            return {JsonObj::JsonValue{JSON_NUMBER, new float(value)}, i - start - 1};
        }
    }
    std::cout << "NEVER ENCOUTERED END OF NUMBER, ABORT" << std::endl;
    return {JsonObj::JsonValue{JSON_NUMBER, nullptr}, -1};
}

enum JSON_LIST_PARSING_STATE{
    LIST_WAITING_START,
    LIST_READING_VALUE,
    LIST_WAITING_SEP,
};
/* RETURN JSON VALUE AND THE LENGTH OF THE STRING THAT HAS BEEN HANDLED */
#define LIST_RET std::pair<struct JsonObj::JsonValue, int>
static LIST_RET parseList(const char *buffer, unsigned int length){
    std::vector<struct JsonObj::JsonValue> list;
    bool started = false;
    enum JSON_LIST_PARSING_STATE state = LIST_WAITING_START;
    unsigned int i = 0;
    for(;i < length; i++){
        switch (state) {
        case LIST_WAITING_START:{
            if(isWhiteSpace(buffer[i])) continue;
            else if(buffer[i] != '['){
                std::cout << "FIRST CHARACTER OF LIST IS NOT [, ABORT" << std::endl;
                return {JsonObj::JsonValue(JSON_LIST, nullptr), -1};
            } 
            state = LIST_READING_VALUE;
        }break;
        case LIST_READING_VALUE:{
            if(!started && isWhiteSpace(buffer[i])) continue;
            if(buffer[i] == '{'){
                std::pair<struct JsonObj, int> pair = parseJsonObj(&buffer[i], length - i - 1);
                if(pair.second < 0) return {JsonObj::JsonValue(JSON_LIST, nullptr), -1}; 
                i += pair.second;
                list.push_back({JSON_OBJ, new JsonObj(std::move(pair.first))});
            }else if(buffer[i] == '['){
                LIST_RET pair = parseList(&buffer[i], length - i);
                if(pair.second < 0) return {JsonObj::JsonValue(JSON_LIST, nullptr), -1};
                i += pair.second;
                list.push_back(std::move(pair.first));
            }else if(buffer[i] == '\"'){
                LIST_RET pair = parseJsonString(&buffer[i], length - i);
                if(pair.second < 0) return {JsonObj::JsonValue(JSON_LIST, nullptr), -1};
                i += pair.second;
                list.push_back(std::move(pair.first));
            }else if(isNumeric(buffer[i])){
                LIST_RET pair = parseJsonNumber(&buffer[i], length - i);
                if(pair.second < 0) return {JsonObj::JsonValue(JSON_LIST, nullptr), -1};
                i += pair.second;
                list.push_back(std::move(pair.first));
            }
            state = LIST_WAITING_SEP;
        }break;
        case LIST_WAITING_SEP:{
            if(isWhiteSpace(buffer[i])) continue;
            if(buffer[i] == ',') state = LIST_READING_VALUE;
            else if(buffer[i] == ']') return {JsonObj::JsonValue(JSON_LIST, new std::vector<JsonObj::JsonValue>(std::move(list))), i};
            else{
                std::cout << "NON VALID LIST SEPERATOR, ENCOUTERED: " << buffer[i] << " ABORT" << std::endl;
                return {JsonObj::JsonValue(JSON_LIST, nullptr), -1};
            }
        }break;
        }
    }
    std::cout << "NEVER ENCOUTERED END OF LIST, ABORT" << std::endl;
    return {JsonObj::JsonValue(JSON_LIST, nullptr), -1};
}

/* SUPPORTS ONLY VALUES OF TYPE STRING AND LIST OF NUMBERS */
enum PARSE_OBJECT_STATE{
    WAITING_INIT,
    READING_KEY,
    WAITING_KEY_VALUE_SEP,
    READING_VALUE,
    WAITING_SEP,
};
static std::pair<struct JsonObj, int> parseJsonObj(const char *buffer, unsigned int length){
    struct JsonObj obj;
    enum PARSE_OBJECT_STATE state = WAITING_INIT;
    bool reading = false;
    char keyBuffer[MAX_KEY_VALUE_BUFFER_SIZE] = {0};
    unsigned int start = 0;
    unsigned int i = 0;
    for(; i < length; i++){
        switch (state) {
            case WAITING_INIT:{
                if(buffer[i] == '{') state = READING_KEY;
                else if(isWhiteSpace(buffer[i])) continue;
                else{
                    std::cout << "ERROR WHILE PARSING JSON, FIRST CHAR NOT {" << std::endl;
                    return {JsonObj{}, -1};
                }
            }break;
            case READING_KEY:{
                if(isWhiteSpace(buffer[i])) continue;
                else if(!reading && buffer[i] == '"'){
                    start = i;
                    reading = true;
                }else if(reading && buffer[i] == '"'){
                    strncpy(keyBuffer, &buffer[start + 1], i - start);
                    keyBuffer[i - start - 1] = '\0';
                    state = WAITING_KEY_VALUE_SEP;
                    reading = false;
                }
            }break;
            case WAITING_KEY_VALUE_SEP:{
                if(isWhiteSpace(buffer[i])) continue;
                if(buffer[i] == ':') state = READING_VALUE;
                else{
                    std::cout << "ERROR WHILE PARSING, WAITING FOR : BUT ENCOUTERED " << buffer[i] << std::endl;
                    return {JsonObj{}, -1};
                }
            }break;
            case READING_VALUE:{
                if(isWhiteSpace(buffer[i])) continue;
                if(buffer[i] == '{'){
                    std::pair<struct JsonObj, int> pair = parseJsonObj(&buffer[i], length - i);
                    if(pair.second < 0) return {JsonObj{}, -1};
                    i += pair.second;
                    obj.values.insert({std::string(keyBuffer), JsonObj::JsonValue(JSON_OBJ, new JsonObj(std::move(pair.first)))});
                }else if(buffer[i] == '['){
                    std::pair<JsonObj::JsonValue, int> pair = parseList(&buffer[i], length - i);
                    if(pair.second < 0) return {JsonObj{}, -1};
                    obj.values.insert({std::string(keyBuffer), std::move(pair.first)});
                    i += pair.second;
                }else if(buffer[i] == '\"'){
                    std::pair<struct JsonObj::JsonValue, int> pair = parseJsonString(&buffer[i], length - i);
                    if(pair.second < 0) return {JsonObj{}, -1};
                    i += pair.second;
                    obj.values.insert({std::string(keyBuffer), std::move(pair.first)});
                }else if(isNumeric(buffer[i])){
                    std::pair<struct JsonObj::JsonValue, int> pair = parseJsonNumber(&buffer[i], length - i);
                    if(pair.second < 0) return {JsonObj{}, -1};
                    i += pair.second;
                    obj.values.insert({std::string(keyBuffer), std::move(pair.first)});
                }else{
                    std::cout << "NOT VALID JSON OBJECT VALUE FIRST CHARACTER: " << buffer[i] << std::endl;
                    return {JsonObj{}, -1};
                }
                state = WAITING_SEP;
            }break;
            case WAITING_SEP:{
                if(isWhiteSpace(buffer[i])) continue;
                else if(buffer[i] == ',') state = READING_KEY;
                else if(buffer[i] == '}') return {std::move(obj), i};
                else{
                    std::cout << "WAITING FOR SEPERATOR BUT ENCOUTERED INVALID VALUE, ABORT" << std::endl;
                    return {JsonObj{}, -1};
                }
            }break;
        }
    }
    std::cout << "COULD NOT FIND END OF JSON OBJECT" << std::endl;
    return {JsonObj{}, -1};
}

/* GLOBAL FUNCTIONS */
struct JsonObj parseJson(const char *buffer, unsigned int length){
    return parseJsonObj(buffer, length).first;
}

}

#endif //JSON_PARSE_IMPLEMENTATION
// NOLINTEND(misc-definitions-in-headers)