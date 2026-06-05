#include <iostream>
#include <fstream>
#include "ext/json.hpp"
#include "headers/materialdesc.hpp"
#include "headers/fntmat.hpp"

namespace fs = std::filesystem;

EBlendMode ProcedureBlendMode(const std::string& strBlendMode) {
	if (strBlendMode == "Opaque") return EBlendMode::Opaque;
	if (strBlendMode == "AlphaBlend") return EBlendMode::AlphaBlend;
	if (strBlendMode == "Additive") return EBlendMode::Additive;
	else { return EBlendMode::Opaque; }
}

ECullMode ProcedureCullMode(const std::string& strCullMode) {
	if (strCullMode == "Back") return ECullMode::Back;
	if (strCullMode == "Front") return ECullMode::Front;
	if (strCullMode == "None") return ECullMode::None;
	else { return ECullMode::Back; }
}

EDepthMode ProcedureDepthMode(const std::string& strDepthMode) {
	if (strDepthMode == "Enabled") return EDepthMode::Enabled;
	if (strDepthMode == "Disabled") return EDepthMode::Disabled;
	if (strDepthMode == "ReadOnly") return EDepthMode::ReadOnly;
	else { return EDepthMode::Enabled; }
}

MaterialDesc_t ParseMaterialFromFile(const std::string& strFileName) {
	std::ifstream fin(strFileName);
	std::string strLine;
	MaterialDesc_t Material = { "", 0 };

	if (!fin.is_open()) { 
		std::cerr << "Failed to open file.";
		return Material;
	}

	while (std::getline(fin, strLine)) {
		std::stringstream ss(strLine);
		std::string strCommand;

		ss >> strCommand;

		if (strCommand == "$DiffuseTexture") {
			ss >> Material.strDiffuseTexture;
		}
		else if (strCommand == "$Ambient") {
			ss >> Material.flAmbient[0] >> Material.flAmbient[1] >> Material.flAmbient[2];
		}
		else if (strCommand == "$Diffuse") {
			ss >> Material.flDiffuse[0] >> Material.flDiffuse[1] >> Material.flDiffuse[2];
		}
		else if (strCommand == "$Specular") {
			ss >> Material.flSpecular[0] >> Material.flSpecular[1] >> Material.flSpecular[2];
		}
		else if (strCommand == "$Shininess") {
			ss >> Material.flShininess;
		}
		else if (strCommand == "$Opacity") {
			ss >> Material.flOpacity;
		}
		else if (strCommand == "$BlendMode") {
			std::string strBlendMode;
			ss >> strBlendMode;
			Material.BlendMode = ProcedureBlendMode(strBlendMode);
		}
		else if (strCommand == "$CullMode") {
			std::string strCullMode;
			ss >> strCullMode;
			Material.CullMode = ProcedureCullMode(strCullMode);
		}
		else if (strCommand == "$DepthMode") {
			std::string strDepthMode;
			ss >> strDepthMode;
			Material.DepthMode = ProcedureDepthMode(strDepthMode);
		}
		else if (strCommand.empty() || strCommand.starts_with("//")) {
			continue;
		}
		else {
			std::cout << "Unknown command: " << strCommand << "\n";
		}
	}

	return Material;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Unknown amount of arguments. Usage ./fntmat.exe <input_desc.txt>\n";
		return 0;
	}

	std::string strFile = argv[1];
	MaterialDesc_t Material = ParseMaterialFromFile(strFile);

	std::cout << "Path: " << Material.strDiffuseTexture << "\n";
	std::cout << "Dif: " << Material.flDiffuse[0] << " "
		<< Material.flDiffuse[1] << " "
		<< Material.flDiffuse[2] << "\n";
	std::cout << "BlendMode: " << (Material.BlendMode == EBlendMode::AlphaBlend ? "AlphaBlend" : "Other") << "\n";

	return 0;
}