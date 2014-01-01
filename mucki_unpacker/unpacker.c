#include <stdio.h>
#include <string.h>
#include <windows.h>

/*

https://github.com/Cr4sh/SimpleUnpacker/blob/master/SimpleUnpacker/SimpleUnpacker.cpp

raw offset = rva - section virtual address + section raw offset
rva = raw offset + section virtual address - section raw offset
*/

void dump(const unsigned char *data_buffer, const unsigned int length)
{
	unsigned char byte;
	unsigned int i, j;
	for(i=0; i < length; i++) {
		byte = data_buffer[i];
		printf("%02x ", data_buffer[i]);
		if(((i%16)==15) || (i==length-1)) {
			for(j=0; j < 15-(i%16); j++)
				printf("   ");
			printf("| ");
			for(j=(i-(i%16)); j <= i; j++) {
				byte = data_buffer[j];
				if((byte > 31) && (byte < 127))
					printf("%c", byte);
				else
					printf(".");
			}
			printf("\n");
		}
	}
}

IMAGE_SECTION_HEADER FindRVASection(IMAGE_SECTION_HEADER *sections, DWORD numOfSections, DWORD rva)
{     
        int i;
        for(i = 0; i < numOfSections; i++) {
                DWORD sectionVA = sections[i].VirtualAddress;
                DWORD sectionVSize = sections[i].Misc.VirtualSize;
                if(rva <= sectionVA+sectionVSize && rva >= sectionVA) {
                        return sections[i]; 
                }
        }
}

// Calculate raw (file) offset from RVA relative to the section
// it belongs to
DWORD RVAtoRO(IMAGE_SECTION_HEADER RVASection, DWORD rva)
{
        return rva-RVASection.VirtualAddress+RVASection.PointerToRawData;
}

// Calculate RVA from raw offset and the section it belongs to
DWORD ROtoRVA(IMAGE_SECTION_HEADER RVASection, DWORD ro)
{
        return ro+RVASection.VirtualAddress-RVASection.PointerToRawData;
}

// Calculate RVA from VA
// Assuming ImageBase = 0x400000
DWORD VAtoRVA(DWORD va)
{
        return va-0x400000;
}

// Calculate VA from RVA
// ASsuming ImageBase = 0x400000
DWORD RVAtoVA(DWORD rva)
{
        return rva+0x400000;
}

// Convert num of bytes in little endian to dword (4 byte)
DWORD LittleEndianToDWORD(BYTE *bytes, DWORD size)
{
        DWORD ret = 0;
        BYTE c = 0;
        int i;
        for(i = 0; i < size; i++) {
                c = bytes[i];
                ret |= c<<(i*8);
        }
        return ret;
}

DWORD htoi(BYTE *str)
{
        DWORD n = 0, t = 0;
        while(*str != '\0') {
                if(*str >= 'A' && *str <= 'F') {
                        t = *str-'A'+10;
                } else if(*str >= 'a' && *str <= 'f') {
                        t = *str-'a'+10;
                } else {
                        t = *str-'0';
                }
                n = (n<<4)+t;
                *str++;
        }
        return n;
}

int main(int argc, char **argv)
{
        if(argc < 2) {
                printf("Usage: %s <file>", argv[0]);
                exit(1);
        }
        
        HANDLE hFile = CreateFile(argv[1], GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        
        if(hFile == INVALID_HANDLE_VALUE) {
                printf("File doesn't exist or cannot be opened");
                exit(1);
        }        
        
        IMAGE_DOS_HEADER DOSHeader;
        IMAGE_NT_HEADERS NTHeader;
        IMAGE_FILE_HEADER FileHeader;
        IMAGE_OPTIONAL_HEADER64 OptionalHeader;
        // IMAGE_OPTIONAL_HEADER32 OptionalHeader;
        DWORD NTHeaderSize;
        
        DWORD bytesRead;
        DWORD bytesWritten;
        int i;
        
        // Read DOS Header
        ReadFile(hFile, &DOSHeader, sizeof(IMAGE_DOS_HEADER), &bytesRead, NULL);
        
        // Read NT Header
        SetFilePointer(hFile, DOSHeader.e_lfanew, FILE_BEGIN, 0);
        ReadFile(hFile, &NTHeader, sizeof(IMAGE_NT_HEADERS), &bytesRead, NULL);
        
        FileHeader = NTHeader.FileHeader;
        OptionalHeader = NTHeader.OptionalHeader;
        
        // Size of NTHeader differs between 32-bit and 64-bit machines
        if(FileHeader.Machine == IMAGE_FILE_MACHINE_I386) {
                NTHeaderSize = sizeof(NTHeader)-sizeof(OptionalHeader)+sizeof(IMAGE_OPTIONAL_HEADER32);
        } else if(FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
                NTHeaderSize = sizeof(NTHeader);
        }
        
        DWORD numOfSections = FileHeader.NumberOfSections;
        DWORD fileAlignment = OptionalHeader.FileAlignment;
        
        // Load sections into an array of IMAGE_SECTION_HEADER
        IMAGE_SECTION_HEADER sections[ numOfSections ];
        IMAGE_SECTION_HEADER section;
        for(i = 0; i < numOfSections; i++) {
                SetFilePointer(hFile, DOSHeader.e_lfanew+NTHeaderSize+i*sizeof(IMAGE_SECTION_HEADER), FILE_BEGIN, 0);
                ReadFile(hFile, &section, sizeof(IMAGE_SECTION_HEADER), &bytesRead, NULL);
                sections[i] = section;
        }
        
        DWORD entryPoint = OptionalHeader.AddressOfEntryPoint;
        IMAGE_SECTION_HEADER entryPointSection = FindRVASection(sections, numOfSections, entryPoint);
        DWORD entryPointRO = RVAtoRO(entryPointSection, entryPoint);
        
        printf("Entry point RVA: 0x%08X\n", entryPoint);
        printf("Entry point file offset: 0x%08X\n", entryPointRO);
        
        // Read packer decryption routine
        BYTE decryptRoutine[24] = {0,};
        SetFilePointer(hFile, entryPointRO, FILE_BEGIN, 0);
        ReadFile(hFile, decryptRoutine, sizeof(decryptRoutine), &bytesRead, NULL);
        
        printf("Dumping decryption routine..\n");
        dump((char *)decryptRoutine, 24);
        
        // Check if we read it correctly
        if(decryptRoutine[0] != 0xBE) {
                printf("invalid decryption routine, found 0x%02X, expected 0x%02X\n", decryptRoutine[0], 0xBE);
                CloseHandle(hFile);
                return 1;
        }
        
        printf("Decryption routine found at: 0x%08X\n", entryPointRO);
        
        // Check what block of data it's decrypting
        // and convert the VA to raw offset
        BYTE source[4] = {
                decryptRoutine[1], decryptRoutine[2],
                decryptRoutine[3], decryptRoutine[4]
        };
        DWORD sourceVA = LittleEndianToDWORD(source, sizeof(source));
        DWORD sourceRVA = VAtoRVA(sourceVA);
        IMAGE_SECTION_HEADER sourceSection = FindRVASection(sections, numOfSections, sourceRVA);
        DWORD sourceRO = RVAtoRO(sourceSection, sourceRVA);
        
        // Check how many bytes to decrypt
        // and create an array of that bytes
        BYTE b_sizeToDecrypt[4] = {
                decryptRoutine[6], decryptRoutine[7],
                decryptRoutine[8], decryptRoutine[9]
        };
        DWORD sizeToDecrypt = LittleEndianToDWORD(b_sizeToDecrypt, sizeof(b_sizeToDecrypt));
        
        printf("Decrypting block at 0x%08X of size 0x%08X\n", sourceRO, sizeToDecrypt);
        
        BYTE Block[sizeToDecrypt];
        
        // Read sizeToDecrypt bytes to Block
        SetFilePointer(hFile, sourceRO, FILE_BEGIN, 0);
        ReadFile(hFile, Block, sizeToDecrypt, &bytesRead, NULL);
        
        // Decrypt the block
        for(i = 0; i < sizeToDecrypt; i++) {
                Block[i] = (~Block[i])&0xFF;
        }
        
        // Check if the jump to OEP is valid
        if(decryptRoutine[19] != 0xE9) {
                printf("invalid jump to OEP, found 0x%02X, expected 0x%02X\n", decryptRoutine[19], 0xE9);
                CloseHandle(hFile);
                return 1;
        }
        
        // Read the offset where the packer is jumping after decrypting
        BYTE jumpToOEP[4] = {
                decryptRoutine[20], decryptRoutine[21],
                decryptRoutine[22], decryptRoutine[23]
        };
        
        // Convert it to dword from little endian
        DWORD jumpOffset = LittleEndianToDWORD(jumpToOEP, sizeof(jumpToOEP));
        
        // Calculate address of the instruction after the jump (+sizeof(decryptRoutine))
        DWORD jumpRVA = ROtoRVA(entryPointSection, entryPointRO)+sizeof(decryptRoutine);
        DWORD jumpVA = RVAtoVA(jumpRVA);
        
        // Calculate OEP RVA and VA
        DWORD oepVA = jumpVA+jumpOffset;
        DWORD oepRVA = VAtoRVA(oepVA);
        
        printf("New OEP: 0x%08X\n", oepRVA);
        
        NTHeader.OptionalHeader.AddressOfEntryPoint = oepRVA;
        
        // Write the decrypted block
        SetFilePointer(hFile, sourceRO, FILE_BEGIN, 0);
        WriteFile(hFile, Block, sizeToDecrypt, &bytesWritten, NULL);
        
        // Fix the OEP
        SetFilePointer(hFile, DOSHeader.e_lfanew, FILE_BEGIN, 0);
        WriteFile(hFile, (IMAGE_NT_HEADERS *)&NTHeader, sizeof(NTHeader), &bytesWritten, NULL);
        
        CloseHandle(hFile);
        return 0;
}
