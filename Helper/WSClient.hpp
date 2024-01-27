#pragma once
#include "json.hpp"
#include "HelperHttp.hpp"
#include <queue>
#include <memory>
#include "WSLinker.hpp"

using std::queue;

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
class WSClient : public WSLinker{
    //friend int callback_http(lws*, enum lws_callback_reasons, void*, void*, size_t);
    friend int callback_ws(lws*, enum lws_callback_reasons, void*, void*, size_t);
public:
    std::unordered_map<string, std::pair<json, time_t>> mEcho;
    WSClient(string url)//: ClientState(nullptr)
    {
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
                "ws-protocol-example", callback_ws, sizeof(this), MAX_PAYLOAD_SIZE,
            },
            {
                nullptr, nullptr,   0 // 最后一个元素固定为此格式
            }
        };
        //lws初始化阶段
        lws_set_log_level(0xFE, nullptr);
        struct lws_context_creation_info info { 0 }; //websocket 配置参数

        info.protocols = lwsprotocol;       //设置处理协议
        info.port = CONTEXT_PORT_NO_LISTEN; //作为ws客户端，无需绑定端口
        //ws和wss的初始化配置不同
        info.options = ssl ? LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT : 0; //如果是wss，需要做全局SSL初始化
        info.gid = -1;
        info.uid = -1;

        // 创建WebSocket语境
        context = lws_create_context(&info); //websocket 连接上下文
        while (!context) {
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
        conn_info.userdata = malloc(sizeof(ClientState));        //userdata 指针会传递给callback的user参数，一般用作自定义变量传入
        conn_info.protocol = lwsprotocol[0].name;

        //ws/wss需要不同的配置
        conn_info.ssl_connection = ssl ? (LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK | LCCSCF_ALLOW_INSECURE) : 0;
        while (!lws_client_connect_via_info(&conn_info)); {//使连接信息生效
            cout << "Client Connect Waiting: " << addr << ":" << port << "/" << path << endl;
            sleep_for(5s);
        } 
    }
    ~WSClient() {
        if (context) {
            lws_context_destroy(context);
        }
    }
    //ClientState* getClient(lws* wsi) override{ return this; }
};