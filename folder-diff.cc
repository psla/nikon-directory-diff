#include <chrono>
#include <iostream>
#include <filesystem>

#include <stdio.h>
#include <stdlib.h>


const int BUFFER_SIZE = 81920;

// Compares two files from Nikon camera from two memory cards.
// Nikon uses a scheme were one file has a 00 byte and the other a 01 byte.
// There is only one byte like this, and it's in a different position in JPG and in NEF file. 
// 
// In JPG, it seems to always be a ninth (9) byte, while in NEF
// it's 33684-th byte. However, for simplicity, we are going to assume that 00 can become 01 only once in the file, without a restriction on position.
//
// Note, this is a first attempt, and it's unfortunately not ideal.
// Some files have other differences, see the screenshot.
//
//
bool nikonCompare(const char * file1, const char * file2) {

	FILE * fp1;
	FILE * fp2;
	fp1 = fopen(file1, "rb");
	if(fp1 == NULL) {
		std::cerr << "Unable to open file: " << file1 << std::endl;
		return false;
	}
	
	fp2 = fopen(file2, "rb");
	if(fp2 == NULL) {
		std::cerr << "Unable to open file: " << file2 << std::endl;
		return false;
	}

	char buffer1[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];
	
	int zeroToOneByteCount = 0;
	int64_t bytesRead1, bytesRead2;
	do {
		bytesRead1 = fread(buffer1, sizeof(char), BUFFER_SIZE, fp1);
		bytesRead2 = fread(buffer2, sizeof(char), BUFFER_SIZE, fp2);
		if(bytesRead1 != bytesRead2) {
			fclose(fp1);
			fclose(fp2);
			return false;
		}
		
		for(int i = 0; i < bytesRead1; i++) {
			if(buffer1[i] != buffer2[i]) {
				if((buffer1[i] ^ buffer2[i]) == (0 ^ 1)) {
					zeroToOneByteCount++;
				} else {
					// TODO: RAII
					fclose(fp1);
					fclose(fp2);
					return false;
				}
			}
		}
			
		if(zeroToOneByteCount > 1) {
			fclose(fp1);
			fclose(fp2);
			return false;
		}
	} while(bytesRead1 == BUFFER_SIZE && bytesRead2 == BUFFER_SIZE);

	fclose(fp1);
	fclose(fp2);
		
	return true;
}

// Verifies if all files from the left directory are present in the right one, and then compares if they are (almost) the same.
	
bool nikonCompareDirs(const char * leftDir, const char * rightDir) {
	
	bool result = true;
	for (const auto & entry : std::experimental::filesystem::directory_iterator(leftDir)) 
	{
		std::experimental::filesystem::path file(entry.path());
		std::experimental::filesystem::path targetDir(rightDir);
		std::experimental::filesystem::path comparedFile(targetDir.append(file.filename()));
		bool are_identical = nikonCompare(entry.path().string().c_str(), comparedFile.string().c_str());
		result &= are_identical;
        std::cout << entry.path() << " ==> " << file.filename() << " identical=" << are_identical << std::endl;
	}
	return result;
}

int main(int argc, char *argv[])
{
	if(argc != 3) {
		std::cout << "Provide left and right directory";
		return 2;
	}
	
	std::cout << "Comparing " << argv[1] << " with " << argv[2] << std::endl;
	
	if(nikonCompareDirs(argv[1], argv[2])) {
		std::cout << "All files identical" << std::endl;
		return 0;
	}
	
	std::cout << "Some files were not identical" << std::endl;
	return 1;
}