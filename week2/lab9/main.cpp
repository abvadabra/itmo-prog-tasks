#include <libxml++/libxml++.h>
#include <curl/curl.h>
#include <iostream>

using namespace std;

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

int main(int arg_c, char** args){
    if(arg_c < 3) {
        cout << "Invalid argument: " << arg_c << endl;
        return 1;
    }

    char *fromArg, *toArg;

    for(int i = 0; i < arg_c; i++){
        char* arg = args[i];
        if(startsWith("--from=", arg)){
            fromArg = arg + 7;
        } else if(startsWith("--to=", arg)){
            toArg = arg + 5;
        }
    }

    if(!fromArg || !toArg){
        cout << "Input invalid" << endl;
        return 1;
    }

    string from(fromArg);
    string to(toArg);

    cout << "Converting from " << from << " to " << to << endl;

    CURL* curl = curl_easy_init();
    std::string readBuffer;

    curl_easy_setopt(curl, CURLOPT_URL, "http://www.cbr-xml-daily.ru/daily_eng.xml");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    xmlpp::DomParser parser;
    parser.parse_memory(readBuffer);
    auto root = parser.get_document()->get_root_node();
    auto children = root->get_children();

    double original_value = 0.0;
    double target_value = 0.0;

    for(auto it = children.begin(); it != children.end(); it++){
        auto char_code_node = reinterpret_cast<xmlpp::Element*>((*it)->get_first_child("CharCode"));
        auto value_node = reinterpret_cast<xmlpp::Element*>((*it)->get_first_child("Value"));
        auto code = char_code_node->get_child_text()->get_content();
        auto string_value = value_node->get_child_text()->get_content();
        string_value.replace(string_value.find(","), sizeof(".") - 1, ".");

        double numeric_value = stod(string_value.c_str());

        if(code == from)
            original_value = numeric_value;
        if(code == to)
            target_value = numeric_value;
    }

    if(original_value == 0.0){
        cout << "Unable to find exchange rate for " << from << endl;
        return 1;
    } else if(target_value == 0.0){
        cout << "Unable to find exchange rate for " << to << endl;
        return 1;
    }

    cout << "Exchange rate from RUB to " << from << " is " << original_value << endl;
    cout << "Exchange rate from RUB to " << to << " is " << target_value << endl;

    double k = original_value / target_value;
    cout << "Exchange rate from " << from << " to " << to << " is " << k << endl;
    return 0;
}