#include <iostream>
#include <fstream>
#include <sstream>
#include "headers/materialdesc.hpp"
#include "headers/fntmat.hpp"

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
	MaterialDesc_t Material = {};

	std::cout << "Parsing .txt file...\n";

	if (!fin.is_open()) { 
		std::cerr << "Failed to open file.\n";
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

bool CheckMaterialDesc(const MaterialDesc_t& Material) {
	bool bSuccess = true;

	std::cout << "\n";

	if (Material.strDiffuseTexture.empty()) {
		std::cerr << "Texture is missing or empty. Please check material config file.\n";
		bSuccess &= false;
	}

	for (int i = 0; i < 3; i++) {
		if (Material.flAmbient[i] < 0.f || Material.flAmbient[i] > 1.f) {
			std::cerr << "Ambient component " << i << " is out of range [0..1]\n";
			bSuccess &= false;
		}

		if (Material.flDiffuse[i] < 0.f || Material.flDiffuse[i] > 1.f) {
			std::cerr << "Diffuse component " << i << " is out of range [0..1]\n";
			bSuccess &= false;
		}

		if (Material.flSpecular[i] < 0.f || Material.flSpecular[i] > 1.f) {
			std::cerr << "Specular component " << i << " is out of range [0..1]\n";
			bSuccess &= false;
		}
	}

	if (Material.flOpacity < 0.f || Material.flOpacity > 1.f) {
		std::cerr << "Opacity is out of range. [0..1]\n";
		bSuccess &= false;
	}

	if (Material.flShininess < 1.f || Material.flShininess > 256.f) {
		std::cerr << "Shininess is out of range. [1..256]\n";
		bSuccess &= false;
	}

	return bSuccess;
}

int main(int argc, char* argv[]) {
	if (argc < 2 || argc > 3) {
		std::cerr << "Unknown amount of arguments. Usage ./fntmat.exe <input_desc.txt> [output_file.fntmat]\n";
		return 1;
	}

	std::string strFile = argv[1];
	MaterialDesc_t Material = ParseMaterialFromFile(strFile);

	if (!strFile.ends_with(".txt")) {
		std::cerr << "Invalid file extension. File should be in '.txt' format.\n";
		return 1;
	}

	if (!CheckMaterialDesc(Material)) {
		std::cerr << "\nFailed to bake material: fix errors above to bake material.";
		return 1;
	}

	FNTMatHeader_t Header = {};
	Header.nMagic = 0x544D5446;
	Header.nVersion = 1;

	FNTMatData_t Data = {};
	strncpy_s(Data.szDiffuseTexture, sizeof(Data.szDiffuseTexture), Material.strDiffuseTexture.c_str(), sizeof(Data.szDiffuseTexture) - 1);
	for (int i = 0; i < 3; i += 1) {
		Data.flAmbient[i] = Material.flAmbient[i];
		Data.flDiffuse[i] = Material.flDiffuse[i];
		Data.flSpecular[i] = Material.flSpecular[i];
	}

	Data.flOpacity = Material.flOpacity;
	Data.flShininess = Material.flShininess;

	Data.nBlendMode = static_cast<uint32_t>(Material.BlendMode);
	Data.nCullMode = static_cast<uint32_t>(Material.CullMode);
	Data.nDepthMode = static_cast<uint32_t>(Material.DepthMode);

	std::string strFileOut = (argc == 3 ? argv[2] : strFile.substr(0, strFile.size() - 4));
	if (!strFileOut.ends_with(".fntmat")) strFileOut = strFileOut + ".fntmat";

	std::ofstream fout(strFileOut, std::ios::binary);
	if (!fout.is_open()) {
		std::cerr << "Failed to open or create output file.\n";
		return 1;
	}

	fout.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	fout.write(reinterpret_cast<const char*>(&Data), sizeof(Data));

	std::cout << "Successfully baket material to " << strFileOut << "\n";

	return 0;
}