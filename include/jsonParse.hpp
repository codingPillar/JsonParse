#ifndef JSON_PARSE_H
#define JSON_PARSE_H

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace jsonParse {

/* TODO, ADD SUPPORT FOR NUMBERS AND BOOL */
enum JsonValueType{
    JSON_STRING,
    JSON_OBJ,
    JSON_LIST
};

struct JsonObj{
    /* VALUE POINT AT EITHER JsonObj OR std::string OR std::vector<JsonValue> */
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
        }

        /* ATTRIBS */
        enum JsonValueType type;
        /* THE USAGE OF UNIONS AND STD::VARIANT IS WAYY TOO TRASH, THIS IS WHY C IS JUST BETTER */
        void *value = nullptr;
    };
    std::unordered_map<std::string, struct JsonValue> values;
};

struct JsonObj parseJson(const char *buffer, unsigned int length);

}

#endif //JSON_PARSE_H

#define JSON_PARSE_IMPLEMENTATION 

// NOLINTBEGIN(misc-definitions-in-headers)
#ifdef JSON_PARSE_IMPLEMENTATION

#include <cstring>

namespace jsonParse {

/* CONSTANTS */
constexpr unsigned int MAX_KEY_VALUE_BUFFER_SIZE = 128;

/* PRIVATE FUNCTIONS */
static char whiteChars[] = {' ', '\n', '\r'};
static bool isWhiteSpace(char element){
    for(unsigned int i = 0; i < sizeof(whiteChars); i++)
        if(element == whiteChars[i]) return true;
    return false;
}

/* RETURN JSON VALUE AND THE LENGTH OF THE STRING THAT HAS BEEN HANDLED */
static std::pair<struct JsonObj::JsonValue, unsigned int> parseList(const char *buffer, unsigned int length){
    std::vector<struct JsonObj::JsonValue> list;
    unsigned int i = 0;
    for(;i < length; i++){
        if(i == ']') break;
        /* TODO, PARSE LIST */
    }
    std::pair<JsonObj::JsonValue, unsigned int> pair{JsonObj::JsonValue(JSON_LIST, new std::vector(std::move(list))), i};
    return pair;
}

/* GLOBAL FUNCTIONS */

/* SUPPORTS ONLY VALUES OF TYPE STRING AND LIST OF NUMBERS */
enum PARSE_OBJECT_STATE{
    WAITING_INIT,
    READING_KEY,
    WAITING_KEY_VALUE_SEP,
    READING_VALUE,
    WAITING_SEP,
};
struct JsonObj parseJson(const char *buffer, unsigned int length){
    struct JsonObj obj;
    enum PARSE_OBJECT_STATE state = WAITING_INIT;
    bool reading = false;
    char keyBuffer[MAX_KEY_VALUE_BUFFER_SIZE] = {0};
    unsigned int start = 0;
    for(unsigned int i = 0; i < length; i++){
        switch (state) {
            case WAITING_INIT:{
                if(buffer[i] == '{') state = READING_KEY;
                else if(isWhiteSpace(buffer[i])) continue;
                else{
                    std::cout << "ERROR WHILE PARSING JSON, FIRST CHAR NOT {" << std::endl;
                    return {};
                }
            }break;
            case READING_KEY:{
                if(isWhiteSpace(buffer[i])) continue;
                else if(!reading && buffer[i] == '"'){
                    start = i;
                    reading = true;
                }else if(reading && buffer[i] == '"'){
                    strncpy(keyBuffer, &buffer[start + 1], i - start - 1);
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
                    return {};
                }
            }break;
            case READING_VALUE:{
                if(!reading && isWhiteSpace(buffer[i])) continue;
                if(!reading && buffer[i] == '{'){
                    /* TODO, IMPLEMENT SUBOBJECTS HANDLING */
                }else if(!reading && buffer[i] == '['){
                    std::pair<JsonObj::JsonValue, unsigned int> pair = parseList(&buffer[i], length - i);
                    if(pair.second == 0) return obj;
                    obj.values.insert({std::string(keyBuffer), std::move(pair.first)});
                    i += pair.second;
                    state = WAITING_SEP;
                    reading = false;
                }else if(!reading && buffer[i] == '\"'){
                    reading = true;
                    start = i;
                }else if(reading && buffer[i] == '\"'){
                    reading = false;
                    std::string key = std::string(keyBuffer);
                    strncpy(keyBuffer, &buffer[start + 1], i - start - 1);
                    keyBuffer[i - start - 1] = '\0';
                    std::string value = std::string(keyBuffer);
                    obj.values.insert({key, JsonObj::JsonValue{JSON_STRING, new std::string(value)}});
                }
            }break;
            case WAITING_SEP:{
                if(isWhiteSpace(buffer[i])) continue;
                else if(buffer[i] == ',') state = READING_KEY;
                else if(buffer[i] == '}') return obj;
                else{
                    std::cout << "WAITING FOR SEPERATOR BUT ENCOUTERED INVALID VALUE, ABORT" << std::endl;
                    return {};
                }
            }break;
        }
    }
    return obj;
}

}

#endif //JSON_PARSE_IMPLEMENTATION
// NOLINTEND(misc-definitions-in-headers)