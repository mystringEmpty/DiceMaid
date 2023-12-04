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
	case json::value_t::boolean:
		value = new AnyBool(j);
		break;
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
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
bool Any::incl(const AnyIndex& key) const {
	return value && value->incl(key);
}

Any AnyObject::get(const AnyIndex&, const Any&) { return Any(); }
//Anys
bool Anys::incl(const AnyIndex& key)const {
	return fields.count(key);
}