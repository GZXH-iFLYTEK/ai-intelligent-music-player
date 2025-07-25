#ifndef SPARK_CLIENT_H
#define SPARK_CLIENT_H

#include <string>

class SparkClient {
public:
    SparkClient(std::string appid, std::string api_key, std::string api_secret);
    std::string ask(const std::string& question);

private:
    std::string _appid;
    std::string _api_key;
    std::string _api_secret;
};

#endif // SPARK_CLIENT_H
