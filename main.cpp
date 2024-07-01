#include <iostream>
#include <hiredis/hiredis.h>
#include <thread>
#include <variant>
#include <vector>
#include <string>
#include <map>
#include <set>

using ReplyResult = std::variant<
    std::string,
    std::vector<std::string>,
    long long,
    double,
    bool,
    nullptr_t,
    std::map<std::string, std::string>,
    std::set<std::string>
>;

ReplyResult checkReply(redisContext *c, redisReply *reply) {
    if (!reply) {
        if (c->err) {
            printf("Error: %s | Code: %d\n", c->errstr, c->err);
        } else {
            printf("Unknown error occurred.\n");
        }
        return nullptr;
    }

    switch(reply->type) {
        case REDIS_REPLY_STRING:
            std::cout << "Received string: " << reply->str << std::endl;
            return std::string(reply->str);
        case REDIS_REPLY_ARRAY: {
            std::cout << "Received array of length: " << reply->elements << std::endl;
            // Optionally, loop through the array to print elements
            std::vector<std::string> result;
            for (size_t i = 0; i < reply->elements; i++) {
                std::cout << "Element " << i << ": " << reply->element[i]->str << std::endl;
                result.push_back(reply->element[i]->str);
            }
            return result;
        }
        case REDIS_REPLY_INTEGER:
            std::cout << "Received integer: " << reply->integer << std::endl;
            return reply->integer;

        case REDIS_REPLY_NIL:
            std::cout << "Received nil." << std::endl;
            return nullptr;
        case REDIS_REPLY_STATUS:
            std::cout << "Received status: " << reply->str << std::endl;
            return std::string(reply->str);
        case REDIS_REPLY_ERROR:
            std::cout << "Received error: " << reply->str << std::endl;
            return std::string(reply->str);
            // Note: Below types might not be available in older versions of hiredis.
        case REDIS_REPLY_DOUBLE:
            std::cout << "Received double: " << reply->dval << std::endl;
            return reply->dval;
        case REDIS_REPLY_BOOL:
            std::cout << "Received bool: " << (reply->integer ? "true" : "false") << std::endl;
            return static_cast<bool>(reply->integer);
        case REDIS_REPLY_MAP: {
            std::cout << "Received map of length: " << reply->elements << std::endl;
            std::map<std::string, std::string> resultMap;
            for (size_t i = 0; i < reply->elements; i += 2) {
                std::cout << "Key " << i / 2 << ": " << reply->element[i]->str
                          << ", Value " << i / 2 << ": " << reply->element[i + 1]->str << std::endl;
                resultMap[reply->element[i]->str] = reply->element[i + 1]->str;
            }
            return resultMap;
        }
        case REDIS_REPLY_SET: {
            std::cout << "Received set of length: " << reply->elements << std::endl;
            std::set<std::string> resultSet;
            for (size_t i = 0; i < reply->elements; i++) {
                std::cout << "Element " << i << ": " << reply->element[i]->str << std::endl;
                resultSet.insert(reply->element[i]->str);
            }
            return resultSet;
        }
        case REDIS_REPLY_ATTR:
            std::cout << "Received attributes. Handling depends on the context." << std::endl;
            return nullptr;
        case REDIS_REPLY_PUSH:
            std::cout << "Received push message. Handling depends on the context." << std::endl;
            return nullptr;
        case REDIS_REPLY_BIGNUM:
            std::cout << "Received bignum: " << reply->str << std::endl;  // Bignum is a large integer value, often represented as a string.
            return std::string(reply->str);
        case REDIS_REPLY_VERB:
            std::cout << "Received verb reply: " << reply->str << std::endl;
            return std::string(reply->str);
        default:
            std::cout << "Unknown reply type received: " << reply->type << std::endl;
            return nullptr;
    }
    freeReplyObject(reply);
}

int main() {
    redisContext *c = redisConnect("192.168.20.204", 6379);

    if (c == NULL || c->err) {
        if (c) {
            std::cout << "Error: " << c->errstr << std::endl;
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
        return 1;
    } else {
        std::cout << "Hello, hiredis!" << std::endl;
    }

    //std::this_thread::sleep_for(std::chrono::seconds(10));
    redisReply* reply1 = (redisReply*) redisCommand(c, "AUTH wonchul!1");
    checkReply(c, reply1);
    redisReply* reply2 = (redisReply*) redisCommand(c, "smembers victor");
    auto rst = std::get<std::vector<std::string>>(checkReply(c, reply2));
    if( rst.empty() ) {
        std::cout << "empty" << std::endl;
    }
    for(auto & iter : rst) {
        std::cout << iter << std::endl;
    }
    std::cout << "---" << std::endl;
    redisReply* reply3 = (redisReply*) redisCommand(c, "get wonchul");
    auto rst2 = std::get<std::string>(checkReply(c, reply3));
    std::cout << rst2 << std::endl;

    freeReplyObject(reply1);
    freeReplyObject(reply2);
    freeReplyObject(reply3);

    redisFree(c);

    return 0;
}
