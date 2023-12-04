#include "AnysDriver.h"

AnyProfile::AnyProfile(const std::filesystem::path& p) :pathFile(p) {
	if (!p.empty()) {
		if (auto suf{ p.extension() }; suf == ".toml") {
			fileType = FileType::Toml;
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

	}
}
void AnyProfile::save() const {
	if (fs::exists(pathFile)) {

	}
}