#pragma once
#include "Anys.h"
enum class FileType { Unknown, Json, Toml, Yaml };
constexpr char* FileSuffix[] = { "",".json",".toml",".yaml" };

class AnyProfile :public Anys {
protected:
	FileType fileType = FileType::Unknown;
	std::filesystem::path pathFile;
public:
	AnyProfile(const std::filesystem::path& = {});
	virtual void read();
	virtual void save()const;
	virtual void updated() {
		save();
	}
};
class AnyProfileSet :public AnyObject {
protected:
	FileType suffix = FileType::Toml;
	std::filesystem::path fileDir;
	fifo_dict_ci<AnyProfile> table;
public:
	AnyClassType type()const override{ return AnyClassType::ProfileSet; }
	AnyProfileSet(const std::filesystem::path& p, FileType t) :AnyObject(AnyType::Table), fileDir(p), suffix(t) {
		fs::create_directories(p);
	}
	const fs::path& getDir() const{ return fileDir; }
	AnyProfile* get(const string& name) {
		if (!table.count(name)) {
			table.emplace(name, fileDir / (name + FileSuffix[(int)suffix]));
		}
		return table[name].as_anys<AnyProfile>();
	}
};