#include "crow.h"
#include <string>
#include <vector>
#include <chrono>
#include <TP1/WebInterface.h>

using namespace std;

vector<string> msgs;
vector<pair<crow::response*, decltype(chrono::steady_clock::now())>> ress;

vector<string> Teste;

void broadcast(const string& msg)
{
    msgs.push_back(msg);
    crow::json::wvalue x;
    x["msgs"][0] = msgs.back();
    x["last"] = msgs.size();
    string body = crow::json::dump(x);
    for(auto p : ress)
    {
        auto* res = p.first;
        CROW_LOG_DEBUG << res << " replied: " << body;
        res->end(body);
    }
    ress.clear();
}


void teste(string out){
    std::cout << std::endl << std::endl << out << std::endl;
}

    crow::response testeArray(){
      crow::json::wvalue x;
      x = Teste;
      std::string json_message = crow::json::dump(x);
      CROW_LOG_INFO << " - MESSAGE: " << json_message;
        return crow::response(x);
    }
// To see how it works go on {ip}:40080 but I just got it working with external build (not directly in IDE, I guess a problem with dependency)
int main()
{
    Teste.push_back("1");
    Teste.push_back("1");

    crow::SimpleApp app;
    crow::mustache::set_base(".");

    CROW_ROUTE(app, "/")
    ([]{
        crow::mustache::context ctx;
        return crow::mustache::load("./html/index.html").render();
    });
        CROW_ROUTE(app, "/testeArray")
      ([]() {
        return testeArray();
      });
    CROW_ROUTE(app, "/banco")
    ([]{
        crow::mustache::context ctx;
        return crow::mustache::load("./html/Banco.html").render();
    });
        CROW_ROUTE(app, "/criarbanco")
    ([](const crow::request& req)
    {
        std::cout << "Params: " << req.url_params << std::endl; 
        if(req.url_params.get("Nome") != nullptr) {
            std::cout << "Nome do Banco: " << req.url_params.get("Nome") << std::endl;
        }
        if(req.url_params.get("CPF") != nullptr) {
            std::cout << "CPF do Banco: " << req.url_params.get("CPF") << std::endl;
        }
        // CROW_LOG_INFO << 
        // broadcast(req.body);
        crow::mustache::context ctx;

        return crow::mustache::load("./html/criarBanco.html").render();
    });
    CROW_ROUTE(app, "/cliente")
    ([]{
        crow::mustache::context ctx;
        return crow::mustache::load("./html/lCjiente.html").render();
    });

    CROW_ROUTE(app, "/teste/<string>")
    ([](const crow::request& /*req*/, string out){
        teste(out);
        return "";
    });

    CROW_ROUTE(app, "/json")
    ([]{
        crow::json::wvalue x;
        x["message"] = "Hello, World!";
        x["name"] = "Carlos";
        x["endereco"] = "rua xxxx,xxxx";
        return x;
    });
    CROW_ROUTE(app, "/params")
    ([](const crow::request& req){
        std::ostringstream os;

        // To get a simple string from the url params
        // To see it in action /params?foo='blabla'
        os << "Params: " << req.url_params << "\n\n"; 
        os << "The key 'foo' was " << (req.url_params.get("foo") == nullptr ? "not " : "") << "found.\n";

        // To get a double from the request
        // To see in action submit something like '/params?pew=42'
        if(req.url_params.get("pew") != nullptr) {
            double countD = boost::lexical_cast<double>(req.url_params.get("pew"));
            os << "The value of 'pew' is " <<  countD << '\n';
        }

        // To get a list from the request
        // You have to submit something like '/params?count[]=a&count[]=b' to have a list with two values (a and b)
        auto count = req.url_params.get_list("count");
        os << "The key 'count' contains " << count.size() << " value(s).\n";
        for(const auto& countVal : count) {
            os << " - " << countVal << '\n';
        }

        // To get a dictionary from the request
        // You have to submit something like '/params?mydict[a]=b&mydict[abcd]=42' to have a list of pairs ((a, b) and (abcd, 42))
        auto mydict = req.url_params.get_dict("mydict");
        os << "The key 'dict' contains " << mydict.size() << " value(s).\n";
        for(const auto& mydictVal : mydict) {
            os << " - " << mydictVal.first << " -> " << mydictVal.second << '\n';
        }

        return crow::response{os.str()};
    });   
    CROW_ROUTE(app, "/logs")
    ([]{
        CROW_LOG_INFO << "logs requested";
        crow::json::wvalue x;
        int start = max(0, (int)msgs.size()-100);
        for(int i = start; i < (int)msgs.size(); i++)
            x["msgs"][i-start] = msgs[i];
        x["last"] = msgs.size();
        CROW_LOG_INFO << "logs completed";
        return x;
    });

    CROW_ROUTE(app, "/logs/<int>")
    ([](const crow::request& /*req*/, crow::response& res, int after){
        CROW_LOG_INFO << "logs with last " << after;
        if (after < (int)msgs.size())
        {
            crow::json::wvalue x;
            for(int i = after; i < (int)msgs.size(); i ++)
                x["msgs"][i-after] = msgs[i];
            x["last"] = msgs.size();

            res.write(crow::json::dump(x));
            res.end();
        }
        else
        {
            vector<pair<crow::response*, decltype(chrono::steady_clock::now())>> filtered;
            for(auto p : ress)
            {
                if (p.first->is_alive() && chrono::steady_clock::now() - p.second < chrono::seconds(30))
                    filtered.push_back(p);
                else
                    p.first->end();
            }
            ress.swap(filtered);
            ress.push_back({&res, chrono::steady_clock::now()});
            CROW_LOG_DEBUG << &res << " stored " << ress.size();
        }
    });

    CROW_ROUTE(app, "/send")
        .methods("GET"_method, "POST"_method)
    ([](const crow::request& req)
    {
        CROW_LOG_INFO << "msg from client: " << req.body;
        broadcast(req.body);
        return "";
    });

    app.port(8080)
        .multithreaded()
        .run();
}