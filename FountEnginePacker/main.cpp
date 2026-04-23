#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "fntpk_header.hpp"
#include "fntpk_block.hpp"

namespace fs = std::filesystem;

std::vector<std::string> GetFilesFromDir(const fs::path& RootPath) {
	std::vector<std::string> vecFiles;

	if (!fs::exists(RootPath) || !fs::is_directory(RootPath))
		throw std::runtime_error("directory path is not exists or not is directory.");

	std::cout << "Getting files from directory....\n\n";

	for (const auto& FileIterator : fs::recursive_directory_iterator(RootPath)) {
		if (!FileIterator.is_regular_file()) continue;
		
		fs::path FileAbsolutePath = FileIterator.path();
		fs::path FileRelativePath = fs::relative(FileAbsolutePath, RootPath);
		std::string strRelativePath = FileRelativePath.generic_string();
		std::cout << strRelativePath << std::endl;
		vecFiles.push_back(strRelativePath);
	}

	std::cout << "\nTotal blocks - " << vecFiles.size() << "\n";

	return vecFiles;
}

void PackDirectory(const std::string& strInputDir, const std::string& strOutputFile) {
	FNTPK_HEADER Header = { 0 };
	Header.nMagic = 0x50544E46;
	Header.nVersion = 1;
	Header.nBlocksCount = 0;

	std::vector<std::string> vecFiles = GetFilesFromDir(strInputDir);
	Header.nBlocksCount = static_cast<uint32_t>(vecFiles.size());

	std::vector<FNTPK_BLOCK> vecBlocks(Header.nBlocksCount);
	uint64_t nCurrentOffset = sizeof(FNTPK_HEADER) + sizeof(FNTPK_BLOCK) * Header.nBlocksCount;

	std::vector<std::vector<char>> vecFilesData;

	for (uint32_t nIndex = 0; nIndex < Header.nBlocksCount; nIndex++) {
		fs::path strPath = fs::path(strInputDir) / vecFiles[nIndex];
		std::ifstream BinaryFile(strPath, std::ios::binary | std::ios::ate);

		if (!BinaryFile.is_open()) {
			throw std::runtime_error("Failed to open " + strPath.string() + ". File invalid or doesn't exists...\n");
		}

		std::streamsize nFileSize = BinaryFile.tellg();
		BinaryFile.seekg(0);

		std::vector<char> vecBuffer(nFileSize);
		if (!BinaryFile.read(vecBuffer.data(), nFileSize)) {
			throw std::runtime_error("Failed to read " + strPath.string() + ".\n");
		}

		FNTPK_BLOCK FileBlock = { 0 };
		strncpy_s(FileBlock.szResourceName, sizeof(FileBlock.szResourceName), vecFiles[nIndex].c_str(), sizeof(FileBlock.szResourceName) - 1);
		FileBlock.nBlockDataOffset = nCurrentOffset;
		FileBlock.nBlockDataSize = static_cast<uint32_t>(nFileSize);

		nCurrentOffset += static_cast<uint64_t>(nFileSize);
		vecBlocks[nIndex] = FileBlock;
		vecFilesData.push_back(std::move(vecBuffer));
	}

	std::ofstream OutputFile(strOutputFile, std::ios::binary);
	if (!OutputFile.is_open())
		throw std::runtime_error("failed to open output file");

	OutputFile.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	OutputFile.write(reinterpret_cast<const char*>(vecBlocks.data()), sizeof(FNTPK_BLOCK) * Header.nBlocksCount);

	for (const auto& Data : vecFilesData) {
		OutputFile.write(Data.data(), Data.size());
	}
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "Usage: " << argv[0] << " <input_dir> <output.fntpk>\n";
		return 1;
	}

	std::string strInputDir = argv[1];
	std::string strOutputFile = argv[2];

	try {
		PackDirectory(strInputDir, strOutputFile);
	}
	catch (const std::exception& e) {
		std::cout << "[ERROR] Failed to pack files from directory: " << e.what() << "\n";
		return 1;
	}

	std::cout << "File " << strOutputFile << " successfully created!\n\n";
	return 0;
}