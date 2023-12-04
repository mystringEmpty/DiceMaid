#pragma once
#include <filesystem>
#include "Anys.h"
namespace fs = std::filesystem;

class AnyProfile :public Anys {
protected:
	enum class FileType { Unknown, Json, Toml, Yaml };
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