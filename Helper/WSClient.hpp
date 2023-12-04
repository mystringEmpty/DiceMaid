#pragma once
#define LWS_ROLE_H1
#define LWS_WITH_NETWORK
#include "libwebsockets.h"
#include "json.hpp"
#include "HelperHttp.hpp"
#include <string>
#include <queue>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
using nlohmann::json;
using std::string;
using std::queue;
using std::cout;
using std::endl;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::this_thread;

#define MAX_PAYLOAD_SIZE  8 * 1024
class EventClient;
/*struct session_data {
    EventClient* client;
	unsigned char buf[LWS_PRE + MAX_PAYLOAD_SIZE];
	int len;
};*/
/**
 * 某个协议下的连接发生事件时，执行的回调函数
 * wsi：指向WebSocket实例的指针
 * reason：导致回调的事件
 * user 库为每个WebSocket会话分配的内存空间
 * in 指向与连接关联的SSL结构的指针
 * len 某些事件使用此参数，说明传入数据的长度
 */
//int callback_http(lws* wsi, enum lws_callback_reasons reasons, void* user, void* in, size_t len);
int callback_ws(lws* wsi, enum lws_callback_reasons reasons, void* user, void* in, size_t len);
class EventClient{
    struct lws_context* context;
	lws* wsi{ nullptr };
	bool isBuilt{ false };
	bool isBreak{ false };
	//bool isStoping{ false };
	int recvSum{ 0 };
	time_t lastGet{ 0 };
	std::queue<string> qSending;
    string rcvData;
    //friend int callback_http(lws*, enum lws_callback_reasons, void*, void*, size_t);
    friend int callback_ws(lws*, enum lws_callback_reasons, void*, void*, size_t);
public:
    std::unordered_map<string, std::pair<json, time_t>> mEcho;
    EventClient(string url) {
        bool ssl{ false }; //确认是否进行SSL加密
        string addr; //目标主机
        string path{ "/" }; //目标主机服务PATH
        int port{ CONTEXT_PORT_NO_LISTEN }; //端口号

        size_t pslash{ url.find("//", 0) };
        if (pslash != string::npos) {
            ssl = url.substr(0, pslash - 1) == "wss";
            url = url.substr(pslash + 2);
        }
        if ((pslash = url.find('/')) == string::npos) {   //没有path
            if ((pslash = url.find(':')) == string::npos) { //只有端口号
                if (strspn(url.c_str(), "0123456789") == url.length()) {
                    addr = "localhost";
                    port = stoi(url);
                }
                else addr = url;    //没有端口号
            }
            else {   //有端口号
                addr = url.substr(0, pslash);
                port = stoi(url.substr(pslash + 1));
            }
        }
        else {  //有path
            path = url.substr(pslash + 1);
            addr = url.substr(0, pslash);
            if ((pslash = url.find(':')) != string::npos) {  //有端口号
                addr = addr.substr(0, pslash);
                port = stoi(addr.substr(pslash + 1));
            }
        }
        /*cout << "ws init:" << url << endl
            << "addr " << addr << endl
            << "path " << path << endl
            << "port " << port << endl;*/
        struct lws_protocols lwsprotocol[]{
            //{ "http-only", callback_http, 0, 0 },
            {
                //协议名称，协议回调，接收缓冲区大小
                "ws-protocol-example", callback_ws, sizeof(EventClient), MAX_PAYLOAD_SIZE,
            },
            {
                NULL, NULL,   0 // 最后一个元素固定为此格式
            }
        };
        //lws初始化阶段
        lws_set_log_level(0xFF, NULL);
        struct lws_context_creation_info info { 0 }; //websocket 配置参数

        info.protocols = lwsprotocol;       //设置处理协议
        info.port = CONTEXT_PORT_NO_LISTEN; //作为ws客户端，无需绑定端口
        //ws和wss的初始化配置不同
        info.options = ssl ? LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT : 0; //如果是wss，需要做全局SSL初始化
        info.gid = -1;
        info.uid = -1;

        // 创建WebSocket语境
        context = lws_create_context(&info); //websocket 连接上下文
        while (context == NULL) {
            cout << "websocket连接上下文创建失败" << endl;
        }

        //初始化连接信息
        struct lws_client_connect_info conn_info { 0 };     //websocket 连接信息
        conn_info.context = context;      //设置上下文
        conn_info.address = addr.c_str(); //设置目标主机IP
        conn_info.port = port;            //设置目标主机服务端口
        conn_info.path = path.c_str();    //设置目标主机服务PATH
        conn_info.host = lws_canonical_hostname(context);      //设置目标主机header
        conn_info.origin = "origin";    //设置目标主机IP
        //ci.pwsi = &wsi;            //设置wsi句柄
        conn_info.userdata = this;        //userdata 指针会传递给callback的user参数，一般用作自定义变量传入
        conn_info.protocol = lwsprotocol[0].name;

        //ws/wss需要不同的配置
        conn_info.ssl_connection = ssl ? (LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK | LCCSCF_ALLOW_INSECURE) : 0;
        do {//使连接信息生效
            if (wsi = lws_client_connect_via_info(&conn_info))break;
            cout << "Client Connect Waiting: " << addr << ":" << port << "/" << path << endl;
            sleep_for(3s);
        } while (!wsi);
    }
    ~EventClient() {
        if (context) {
            lws_context_destroy(context);
        }
    }
    static string access_token;
    void keep() {
        //进入消息循环
        while (!isBreak) {
            lws_service(context, 20);
        }
        lws_context_destroy(context);
        context = nullptr;
    }
	void send_data(const string& data) {
        qSending.emplace(data);
    }
    void send(const json& data) {
        qSending.emplace(data.dump());
    }
    json get_data(const json& data) {
        string echo_key{ data["echo"] };
        if (auto it{ mEcho.find(echo_key) }; it != mEcho.end()) {
            if (time(nullptr) - it->second.second < 60
                && !it->second.first.is_null())
                return it->second.first;
            else mEcho.erase(it);
        }
        qSending.emplace(data.dump());
        size_t cntTry{ 0 };
        do {
            if (++cntTry > 100)break;
            std::this_thread::sleep_for(50ms);
        } while (!mEcho.count(echo_key));
        return mEcho[echo_key].first;
    }
	void shut() {
        isBreak = true;
    }
};
extern std::unique_ptr<EventClient> client;