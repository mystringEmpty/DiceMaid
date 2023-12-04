#include <fstream>
#include "AnysDriver.h"

AnyProfile::AnyProfile(const std::filesystem::path& p) :pathFile(p) {
	if (!p.empty()) {
		if (auto suf{ p.extension() }; suf == ".toml") {
			fileType = FileType::Toml;
		}
		else if (suf == ".json") {
			fileType = FileType::Json;
		}
		else if (suf == ".yaml" || suf == ".yml") {
			fileType = FileType::Yaml;
		}
		fs::create_directories(p.parent_path());
		read();
	}
}
void AnyProfile::read() {
	if (fs::exists(pathFile)) {
		if (fileType == FileType::Toml) {
			if (auto t = toml::parse_file(pathFile.u8string()); t.is_table()) {
				any_table new_table;
				for (auto& item : *t.as_table()) {
					new_table.emplace(item.first.str(), item.second);
				}
				fields.swap(new_table);
			}
			else app.error(pathFile.u8string() + "内数据类型不为table！");
		}
		else if (fileType == FileType::Json) {
			if (std::ifstream fin{ pathFile }) {
				json obj;
				fin >> obj;
				if (obj.is_object()) {
					any_table new_table;
					for (auto& [key,val] : obj.items()) {
						new_table.emplace(key, val);
					}
					fields.swap(new_table);
				}
				else app.error(pathFile.u8string() + "内数据类型不为object！");
			}
		}
	}
}
void AnyProfile::save() const {
	if (!fs::exists(pathFile))fs::create_directories(pathFile.parent_path());
	if (fileType == FileType::Toml) {
		if (std::ofstream fout{ pathFile })fout << to_toml();
	}
	else if (fileType == FileType::Json) {
		if (std::ofstream fout{ pathFile })fout << to_json().dump(1);
	}
}