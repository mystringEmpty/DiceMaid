#include <unordered_set>
#include "Anys.h"
struct lua_State;
typedef int (*lua_CFunction) (lua_State* L);
struct AnysClass {
	AnyClassType type{ AnyClassType::Object };
	string name;
	dict<lua_CFunction>lua_lib;
};
extern umap<AnyClassType, AnysClass> MetaList;

class AnyLuaState: public AnyObject {
	lua_State* L;
	std::unordered_set<AnyClassType> class_registy;
	//void push(const Any& var);
public:
	AnyLuaState();
	~AnyLuaState();
	AnyClassType type()const override { return AnyClassType::Runtime; }
	bool registed(AnyClassType);
	void regist(const AnysClass&);
	bool dofile(const fs::path&, const Any& ctx = {});
};