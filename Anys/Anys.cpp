#include "Anys.h"

//AnyIndex
std::vector<var_index> AnyIndex::atoms{ var_index{} };
std::unordered_map<string, size_t> AnyIndex::hashed_str;
std::unordered_map<double, size_t> AnyIndex::hashed_num;
std::shared_mutex AnyIndex::exAnyIndex;
AnyIndex::AnyIndex(const string& s) {
	if (auto it{ hashed_str.find(s) }; it != hashed_str.end()) {
		idx = it->second;
	}
	else {
		lock_writer lock{ exAnyIndex };
		idx = hashed_str[s] = atoms.size();
		atoms.push_back(s);
	}
}
AnyIndex::AnyIndex(const Any& a) {
	if (a.is_number()) {
		auto num{ a.to_number() };
		if (auto it{ hashed_num.find(num) }; it != hashed_num.end()) {
			idx = it->second;
		}
		else {
			lock_writer lock{ exAnyIndex };
			idx = hashed_num[num] = atoms.size();
			atoms.push_back(num);
		}
	}
	else if (auto s{ a.str() };!s.empty()) {
		if (auto it{ hashed_str.find(s) }; it != hashed_str.end()) {
			idx = it->second;
		}
		else {
			lock_writer lock{ exAnyIndex };
			idx = hashed_str[s] = atoms.size();
			atoms.push_back(s);
		}
	}
}
string AnyIndex::str() const {
	if (auto atom{ atoms[idx] }; std::holds_alternative<string>(atom))
		return std::get<string>(atom);
	else if (std::holds_alternative<double>(atom))
		return std::to_string(std::get<double>(atom));
	return {};
}

//AnyRef
AnyRef::AnyRef(AnyObject* p, AnyIndex i) :obj(p), index(i) {
	if (obj)++obj->cnt_ref;
}
AnyRef::~AnyRef() {
	if (obj)obj->drop();
}
AnyRef::operator Any()const {
	return obj && obj->incl(index) ? obj->get(index) : Any();
}
AnyEmpty* AnyRef::operator->() {
	return obj && obj->incl(index) ? obj->get(index).val() : nullptr;
}
bool AnyRef::operator==(const AnyIndex& id)const {
	return operator Any() == id;
}
AnyRef AnyRef::operator[](const char* key)const {
	if (auto sub = operator Any(); sub.is_object()) {
		return { sub.as_object(),key };
	}
	return {};
}

//Any
AnyEmpty Any::nil;
bool AnyEmpty::operator==(const Any& other)const {
	return other.is_null();
}
bool AnyEmpty::operator==(const AnyIndex& other)const {
	return !other;
}
bool AnyBool::operator==(const Any& other)const {
	return other.is_boolean() && data == other.to_boolean();
}
bool AnyInt::operator==(const Any& other)const {
	return other.is_number() && data == other.to_number();
}
bool AnyNumber::operator==(const Any& other)const {
	return other.is_number() && data == other.to_number();
}
bool AnyString::operator==(const Any& other)const {
	return other.is_string() && data == other.str();
}
bool AnyString::operator==(const AnyIndex& other)const {
	return other == AnyIndex(data);
}
bool AnyString::operator==(const char* other)const {
	return data == other;
}

Any::Any(const json& j) {
	switch (j.type()) {
	case json::value_t::null:
		value = new AnyEmpty(j);
		break;
	case json::value_t::boolean:
		value = new AnyBool(j);
		break;
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		value = new AnyInt(j);
		break;
	case json::value_t::number_float:
		value = new AnyNumber(j);
		break;
	case json::value_t::string:
		value = new AnyString(j);
		break;
	case json::value_t::object:
		if (auto obj = (Anys*)(value = new Anys())) {
			for (auto& item : j.items()) {
				obj->rawset(item.key(), item.value());
			}
		}
		break;
	default:
		break;
	}
}
Any::Any(const toml::node& t) {
	switch (t.type()) {
	case toml::node_type::none:
		value = new AnyEmpty();
		break;
	case toml::node_type::boolean:
		value = new AnyBool(bool(*t.as_boolean()));
		break;
	case toml::node_type::integer:
		value = new AnyInt(int64_t(*t.as_integer()));
		break;
	case toml::node_type::floating_point:
		value = new AnyNumber(double(*t.as_floating_point()));
		break;
	case toml::node_type::string:
		value = new AnyString(string(*t.as_string()));
		break;
	case toml::node_type::table:
		if (auto obj = (Anys*)(value = new Anys())) {
			for (auto& item : *t.as_table()) {
				obj->rawset(item.first.str(), item.second);
			}
		}
		break;
	default:
		break;
	}
}

bool Any::incl(const AnyIndex& key) const {
	return value && value->incl(key);
}

Any AnyObject::get(const AnyIndex&, const Any&) { return Any(); }
//AnyCITable
bool AnyCITable::incl(const AnyIndex& key)const {
	return key.is_string() && table.count(key.str());
}
//Anys
bool Anys::incl(const AnyIndex& key)const {
	return fields.count(key);
}
json AnyCITable::to_json()const {
	json tab = json::object();
	for (auto& [key, val] : table) {
		tab.emplace(key, val.to_json());
	}
	return tab;
}
json Anys::to_json()const {
	json tab = json::object();
	for (auto& [key, val] : fields) {
		tab.emplace(key.str(), val.to_json());
	}
	return tab;
}
toml::table Anys::to_toml()const {
	toml::table tab;
	for (auto& [key, val] : fields) {
		switch (val.type()) {
		case AnyType::Boolean:
			tab.emplace(key.str(), bool(val.val()));
			break;
		case AnyType::Number:
			tab.emplace(key.str(), val.val()->to_num());
			break;
		case AnyType::String:
			tab.emplace(key.str(), val.val()->str());
			break;
		case AnyType::Table:
		case AnyType::Anys:
			tab.emplace(key.str(), ((Anys*)val.val())->to_toml());
			break;
		}
	}
	return tab;
}