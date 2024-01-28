#pragma once
#include <optional>
#include <variant>
#include <shared_mutex>
#include <filesystem>
#include "json.hpp"
#include "fifo_map.hpp"
#include "toml.hpp"
#include "HelperSTL.hpp"
using std::string;
using json = nlohmann::basic_json<fifo_map>;
using lock_writer = std::unique_lock<std::shared_mutex>;
namespace fs = std::filesystem;

enum class AnyType :size_t { Void, Empty, Boolean, Number, String, Table, Function, UserData, Anys };
enum class AnyClassType :size_t { Object, Anys, User, Group, ProfileSet, Driver, Runtime};
enum class LogLevel { Debug, Info, Notice, Warning, Error, Critical, Fatal };
using any_int = int64_t;

class Any;
class AnyEmpty;
class AnyObject;
using var_index = std::variant<std::monostate, double, string>;

std::string to_String(double num);
class AnyIndex {
    static std::vector<var_index> atoms;
    static std::unordered_map<string, size_t> hashed_str;
    static std::unordered_map<double, size_t> hashed_num;
    static std::shared_mutex exAnyIndex;
public:
    size_t idx{ 0 };
    AnyIndex() = default;
    AnyIndex(const string&);
    AnyIndex(const char* c) { new(this) AnyIndex(string(c)); }
    AnyIndex(const std::string_view& c) { new(this) AnyIndex(string(c)); }
    template<typename Num, std::enable_if_t<std::is_integral_v<Num>
    && !std::is_same_v<Num, char> && !std::is_same_v<Num, wchar_t>
        && !std::is_same_v<Num, bool>, int> = 0>
    AnyIndex(Num i) {
        double d{ (double)i };
        if (auto it{ hashed_num.find(d) }; it != hashed_num.end()) {
            idx = it->second;
        }
        else {
            lock_writer lock{ exAnyIndex };
            idx = hashed_num[d] = atoms.size();
            atoms.push_back(d);
        }
    }
    AnyIndex(const Any&);
    AnyIndex(const json&);
    bool operator==(const AnyIndex& other)const { return idx == other.idx; }
    explicit operator bool()const { return idx; }
    bool is_string()const { return idx && std::holds_alternative<string>(atoms[idx]); }
    string str()const;
    long long to_ll()const;
};
template<>
struct std::hash<AnyIndex> {
    size_t operator()(const AnyIndex& _Keyval) const noexcept {
        return hash<size_t>()(_Keyval.idx);
    }
};
template<>
struct std::equal_to<AnyIndex> {
    constexpr bool operator()(const AnyIndex& _Left, const AnyIndex& _Right) const {
        return _Left.idx == _Right.idx;
    }
};

class AnyRef {
    AnyObject* const obj{ nullptr };
    AnyIndex index; 
public:
    AnyRef() = default;
    AnyRef(AnyObject* p, AnyIndex i);
    ~AnyRef();
    operator Any()const;
    AnyEmpty* operator->();
    bool operator==(const AnyIndex& id)const;
    AnyRef operator[](const char* key)const;
};

class AnyEmpty {
public:
    const AnyType type{ AnyType::Empty };
    size_t cnt_ref{ 1 };
    AnyEmpty(AnyType t = AnyType::Empty) :type{ t } {}
    virtual ~AnyEmpty() {};
    AnyEmpty* copy() {
        ++cnt_ref;
        return this;
    }
    void drop() {
        if (0 == --cnt_ref)delete this;
    }
    virtual bool operator==(const Any&)const;
    virtual bool operator==(const AnyIndex&)const;
    virtual bool operator==(const char *)const { return false; }
    virtual operator bool()const { return type != AnyType::Empty; }
    //virtual string tname()const { static const string t{ "Empty" }; return t; }
    virtual void* ptr()const { return nullptr; }
    //virtual const char* c_str()const { return ""; }
    virtual any_int to_int()const { return 0; }
    virtual double to_num()const { return 0; }
    virtual operator double()const { return to_num(); }
    virtual std::string str()const { return {}; }
    std::string str_or(const string& els) {
        if (auto s{ str() }; !s.empty())return s;
        return els;
    }
    template<class DerivedAnys = Anys>
    DerivedAnys* as_anys() {
        return type == AnyType::Anys ? dynamic_cast<DerivedAnys*>(this) : nullptr;
    }
    virtual AnyIndex to_index()const { return {}; }
    //virtual lua_state* as_state() { return nullptr; }
    virtual bool incl(const AnyIndex&)const { return false; }
    //virtual bool incl(unsigned) { return false; }
    virtual AnyRef operator[](const string&) { return {}; }
    virtual AnyRef operator[](const char*) { return {}; }
    virtual AnyRef operator[](size_t) { return {}; }
    //virtual AnyRef operator[](unsigned) { return {}; }
    //virtual AnyRef operator[](const Any&) { return {}; }

    //virtual void set_type(const string&) {}

    //virtual void push(lua_State*);

    virtual json to_json()const { return json(); }
};

class AnyBool :public AnyEmpty {
    const bool data{ false };
public:
    AnyBool(bool b) :AnyEmpty(AnyType::Boolean), data(b) {}
    bool operator==(const Any& other)const override;
    //std::string tname()const override { static const std::string t{ "Boolean" }; return t; }
    operator bool() const override { return data; }
    std::string str()const override { return std::to_string(data); }
    json to_json()const override { return data; }
};

class AnyInt :public AnyEmpty {
    const any_int data;
public:
    AnyInt(double val) :AnyEmpty(AnyType::Number), data(val) {}
    bool operator==(const Any& other)const override;
    //bool operator==(const string& other)const override;
    //string tname()const override { static const string t{ "Number" }; return t; }
    operator double()const override { return data; }
    any_int to_int()const override { return data; }
    std::string str()const override { return std::to_string(data); }
    //void push(lua_State*)override;
    json to_json()const override { return data; }
};
class AnyNumber :public AnyEmpty {
    const double data;
public:
    AnyNumber(double val) :AnyEmpty(AnyType::Number), data(val) {}
    bool operator==(const Any& other)const override;
    //bool operator==(const string& other)const override;
    //string tname()const override { static const string t{ "Number" }; return t; }
    operator double()const override { return data; }
    long long to_int()const override { return (long long)data; }
    std::string str()const override { return to_String(data); }
    //void push(lua_State*)override;
    json to_json()const override { return data; }
};

class AnyString :public AnyEmpty {
    const string data;
public:
    AnyString(const char* val) :AnyEmpty(AnyType::String), data(val) {}
    AnyString(const string& val) :AnyEmpty(AnyType::String), data(val) {}
    bool operator==(const Any& other)const override;
    bool operator==(const AnyIndex& other)const override;
    bool operator==(const char* other)const override;
    //string tname()const override { static const string t{ "String" }; return t; }
    std::string str()const override { return data; }
    operator double()const override { return stod(data); }
    //const char* c_str()const override { return data.c_str(); }
    operator const string& ()const { return data; }
    long long to_int()const override {
        try {
            return stoll(data);
        }catch(...){}
        return 0;
    };
    //void push(lua_State*)override;
    json to_json()const override { return data; }
};

//int lua_obj_tonumber(lua_State* L);
//int lua_obj_tostring(lua_State* L);
//int lua_obj_newindex(lua_State* L);
//int lua_obj_gc(lua_State* L);
//int lua_obj_len(lua_State* L);

//typedef int (*lua_CFunction) (lua_State* L);

class Any {
    AnyEmpty* value = nullptr;
    static AnyEmpty nil;
public:
    Any() = default;
    Any(nullptr_t null) :value(nil.copy()) {}
    Any(const Any& val) :value(val.value ? val.value->copy() : nullptr) {}
    Any(AnyEmpty* val) :value(val ? val->copy() : nullptr) {}
    //Any(const AnyRef& ref) :value(ref.var().val()) { value->copy(); }
    Any& operator=(const Any& val) {
        if (value)value->drop();
        new(this)Any(val);
        return *this;
    }
    /*Any& operator=(const AnyRef& ref) {
        if (value)value->drop();
        new(this)Any(ref);
        return *this;
    }*/
    /*Any(Any&& val) :value(val.value) {
        val.value = nullptr;
    }*/
    explicit Any(bool b) :value(new AnyBool(b)) {}
    //Any(double d) :value(new AnyNumber(d)) {}
    template<typename Num, std::enable_if_t<std::is_integral_v<Num>
    && !std::is_same_v<Num, char> && !std::is_same_v<Num, wchar_t>
        && !std::is_same_v<Num, bool>, int> = 0>
    Any(Num d) : value(new AnyInt((any_int)d)) {}
    template<typename Num, std::enable_if_t<std::is_floating_point_v<Num>, int> = 0>
    Any(Num d) : value(new AnyNumber((double)d)) {}
    Any(const char* s) :value(s ? new AnyString(s) : nullptr) {}
    Any(const string& s) :value(new AnyString(s)) {}
    //Any(void* u, const string& t = {}) :value(u ? new lua_userdata(u) : nullptr) { if (u)set_type(t); }
    Any(AnyObject& obj);
    //template<class DerivedAnys = Anys, AnyClassType (DerivedAnys::*T)() = &DerivedAnys::type>
    //Any(DerivedAnys* obj) {
    //    value(obj ? obj->copy() : nullptr)
    //}
    Any(const json& j);
    Any(const toml::node& j);
    using Dict = fifo_map<string, Any>;
    using List = std::vector<Any>;
    //Any(Dict&& tab, const string& t = {});
    //Any(lua_CFunction f) :value(f ? new lua_function(f) : nullptr) {}
    //Any(lua_State* L, int idx = -1);
    ~Any() {
        if (value)value->drop();
    }

    //static Any new_empty();
    //static Any new_table();
    //static Any new_state();

    AnyEmpty* val()const { return value; }
    AnyEmpty* operator->()const { return value; }
    AnyType type()const { return value ? value->type : AnyType::Void; }
    bool is_void()const { return !value; }
    bool is_null()const { return value ? (value->type == AnyType::Empty) : true; }
    bool is_empty()const { return value ? (value->type == AnyType::Empty) : true; }
    bool is_boolean()const { return type() == AnyType::Boolean; }
    bool is_number()const { return type() == AnyType::Number; }
    bool is_int()const { return type() == AnyType::Number && double(*value) == value->to_int(); }
    bool is_string()const { return type() == AnyType::String; }
    bool is_table()const { return type() == AnyType::Table; }
    bool is_udata()const { return type() == AnyType::UserData; }
    bool is_object()const { return type() == AnyType::Table || type() == AnyType::Anys || type() == AnyType::UserData; }
    bool is_function()const { return type() == AnyType::Function; }

    //Any& set_type(const string& s) { if (value)value->set_type(s); return *this; }
    //string tname()const { return value ? value->tname() : "Void";}

    //void push(lua_State* L)const;
    //void push_var(lua_State* L)const;
    string str()const {
        if (value) {
            return value->str();
        }
        return "";
    }
    string to_string()const { return str(); }
    operator string()const {
        return str();
    }
    operator const string& ()const {
        static string Empty;
        if (type() == AnyType::String) {
            return *(AnyString*)value;
        }
        return Empty;
    }
    bool to_boolean()const {
        return value ? *value : false;
    }
    explicit operator bool()const { return to_boolean(); }
    double to_number()const { return value ? value->to_num() : 0; }
    long long to_int()const{ return value ? value->to_int() : 0; }
    //Any to_table()const;
    //lua_table* as_table()const;
    AnyObject* as_object()const {
        return is_object() ? (AnyObject*)value : nullptr;
    }
    template<class DerivedObject = Anys>
    DerivedObject* as_table()const {
        return type() == AnyType::Table ? dynamic_cast<DerivedObject*>(value) : nullptr;
    }
    template<class DerivedAnys = Anys>
    DerivedAnys* as_anys()const {
        return type() == AnyType::Anys ? dynamic_cast<DerivedAnys*>(value) : nullptr;
    }
    //lua_state* as_state()const;
    bool operator==(const string& other)const {
        return value ? *value == AnyIndex(other) : false;
    }
    bool operator==(const char* other)const {
        return value ? *value == AnyIndex(other) : false;
    }
    bool operator==(const AnyIndex& other)const {
        return value ? *value == other : !other;
    }
    bool operator==(const Any& other)const {
        return value ? *value == other : other.is_void();
    }
    template<class C = void>
    C* to_udata()const {
        if (type() == AnyType::UserData) {
            return reinterpret_cast<C*>(value->ptr());
        }
        return dynamic_cast<C*>(value);
    }
    template<class C = void>
    C* as()const {
        return dynamic_cast<C*>(value);
    }
    bool incl(const AnyIndex& key) const;
    AnyRef operator[](const Any& key)const {
        if (is_object()) {
            return (*value)[key];
        }
        return {};
    }
    AnyRef operator[](const string& key)const {
        if (is_object()) {
            return (*value)[key];
        }
        return {};
    }
    AnyRef operator[](const char* key)const {
        if (is_object()) {
            return (*value)[string(key)];
        }
        return {};
    }
    AnyRef operator[](size_t key)const {
        if (is_object()) {
            return (*value)[key];
        }
        return {};
    }
    json to_json()const { return value ? value->to_json() : json(); }
};
//using lua_init = any(*)(Any);
//any lua_to_any(Any);


template<typename T = Any>
using dict = umap<string, T>;
using any_dict = Any::Dict;
template<typename T = Any>
using fifo_dict_ci = std::unordered_map<string, T, hash_ci, equal_ci>;
using any_dict_ci = fifo_dict_ci<>;
using any_list = Any::List;
using any_table = fifo_map<AnyIndex, Any>;
//json to_json(const any_dict&);
class AnyObject :public AnyEmpty {
public:
    AnyObject(AnyType t) :AnyEmpty(t) {}
    AnyRef operator[](const string& key) override {
        return AnyRef{ this,key };
    }
    AnyRef operator[](const char* key) override {
        return AnyRef{ this,string(key) };
    }
    AnyRef operator[](size_t key) override { return AnyRef{ this,key }; }
    //AnyRef operator[](const Any& key)const override { return AnyRef{ this,key }; }
    //bool incl(const AnyIndex& key) const override { return false; }
    virtual AnyClassType type()const = 0;
    virtual Any get(const AnyIndex& key, const Any& val = {});
};
class AnyCITable :public AnyObject {
protected:
    friend class Any;
    any_dict_ci table;
public:
    AnyCITable() :AnyObject(AnyType::Table) {}
    AnyCITable(const any_dict& d) :AnyObject(AnyType::Table) {
        for (auto& [k, v] : d) {
            if(!v.is_void())table.emplace(k, v);
        }
    }
    //void set_type(const string&)override;
    //const lua_mod* get_type()const;
    virtual AnyClassType type()const override { return AnyClassType::Object; }
    bool incl(const AnyIndex& key) const override;
    virtual bool count(unsigned)const { return false; }
    Any rawget(const string& key, const Any& val = {})const {
        return table.count(key) ? table.at(key) : Any();
    }
    Any get(const AnyIndex& key, const Any& val = {})override {
        return rawget(key.str(), val);
    }
    void rawset(const string& key, const Any& val = {}) {
        if (!val.is_void())table[key] = val;
        else table.erase(key);
    }
    json to_json()const override;
    toml::table to_toml()const;
};
class Anys :public AnyObject {
protected:
    friend class Any;
    any_table fields;
public:
    Anys():AnyObject(AnyType::Anys){}
    Anys(const any_dict& d):AnyObject(AnyType::Anys) {
        for (auto& [k, v] : d) {
            if (!v.is_void())fields.insert(k, v);
        }
    }
    //void set_type(const string&)override;
    //const lua_mod* get_type()const;
    virtual AnyClassType type()const override { return AnyClassType::Object; }
    bool incl(const AnyIndex& key) const override;
    virtual bool count(unsigned)const { return false; }
    Any rawget(const AnyIndex& key, const Any& val = {})const {
        return fields.count(key) ? fields.at(key) : val;
    }
    Any get(const AnyIndex& key, const Any& val = {})override {
        return rawget(key, val);
    }
    void rawset(const AnyIndex& key, const Any& val = {}) {
        if (!val.is_void())fields[key] = val;
        else fields.erase(key);
    }
    json to_json()const override;
    toml::table to_toml()const;
};
//int lua_tab_pairs(lua_State* L);
//int lua_tab_ipairs(lua_State* L);

