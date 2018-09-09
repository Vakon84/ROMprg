#include "flash.hpp"
#include <Arduino.h>
#include "am29f040.hpp"

static uint8_t onesCount(uint32_t ulData) {
	uint8_t ubCnt = 0;
	while(ulData) {
		if(ulData & 1) {
			++ubCnt;
		}
		ulData >>= 1;
	}
	return ubCnt;
}

bool tFlash::processCommand(const char *szCmd) {
	// Read message type
	char szCmdType[10];
	int32_t lCntRead = sscanf(szCmd, "%s", szCmdType);
	if(!lCntRead) {
		return false;
	}
	szCmd += strlen(szCmdType);
	bool isOk = false;
	uint32_t pArgs[4];
	uint8_t ubArgCnt = sscanf( // Hackty hack
		szCmd, "%lu %lu %lu %lu", &pArgs[0], &pArgs[1], &pArgs[2], &pArgs[3]
	);

	Serial.print("START "); Serial.println(szCmdType);
  if(!strcmp(szCmdType, "read") && ubArgCnt == 3) {
		isOk = cmdRead(pArgs[0], pArgs[1], pArgs[2]);
  }
  else if(!strcmp(szCmdType, "mask") && ubArgCnt == 3) {
		isOk = cmdMask(pArgs[0], pArgs[1], pArgs[2]);
  }
  else if(!strcmp(szCmdType, "chkerase") && ubArgCnt == 3) {
		isOk = cmdChkErase(pArgs[0], pArgs[1], pArgs[2]);
  }
	else if(!strcmp(szCmdType, "write")) {
		ubArgCnt = sscanf( // Hackty hack
			szCmd, "%lu %lu %lx %lu", &pArgs[0], &pArgs[1], &pArgs[2], &pArgs[3]
		);
		if(ubArgCnt == 3) {
			isOk = cmdWrite(pArgs[0], pArgs[1], pArgs[2]);
		}
	}
	else if(!strcmp(szCmdType, "erase_all") && ubArgCnt == 255) {
		isOk = cmdErase();
	}
	else {
		Serial.println("ERR: Unknown command: '");
		Serial.print(szCmdType);
		Serial.print("', arg cnt: ");
		Serial.println(ubArgCnt);
	}
	relax();
	return isOk;
}

tFlash *tFlash::fromString(const char *szChip) {
	if(!strcmp(szChip, "am29f040")) {
		return new tAm29f040();
	}
	return nullptr;
}

bool tFlash::cmdRead(uint8_t ubDepth, uint32_t ulAddr, uint32_t ulLength) {
	// Read mode
	if(ubDepth == 1) {
		char szBfr[4];
		for(uint32_t i = ulAddr; i < ulAddr + ulLength; ++i) {
			uint8_t ubRead = readByte(i);
			sprintf(szBfr, "%02x ", ubRead);
			Serial.print(szBfr);
		}
		Serial.println();
	}
	else {
		// TODO: add read depth 2
		// TODO: add read depth 4
		Serial.print("ERR: Unsupported depth: ");
		Serial.println(ubDepth);
		return false;
	}
	return true;
}

bool tFlash::cmdWrite(uint8_t ubDepth, uint32_t ulAddr, uint32_t ulData) {
	if(ubDepth == 1) {
		return writeByte(ulAddr, ulData);
	}
	else {
		// TODO: add write depth 2
		// TODO: add write depth 4
		Serial.print("ERR: Unsupported depth: ");
		Serial.println(ubDepth);
		return false;
	}
}

bool tFlash::cmdMask(uint8_t ubDepth, uint32_t ulAddr, uint32_t ulLength) {
	if(ubDepth == 1) {
		uint8_t ubMask = 0;
		for(uint32_t i = ulAddr; i < ulAddr + ulLength; ++i) {
			uint8_t ubRead = readByte(i);
			ubMask |= ubRead;
		}
		char szBfr[3];
		sprintf(szBfr, "%02x", ubMask);
		Serial.print("Mask: ");
		Serial.println(szBfr);
	}
	else {
		// TODO: add mask depth 2
		// TODO: add mask depth 4
		Serial.print("ERR: Unsupported depth: ");
		Serial.println(ubDepth);
		return false;
	}
	return true;
}

bool tFlash::cmdChkErase(uint8_t ubDepth, uint32_t ulAddr, uint32_t ulLength) {
	// Erase check mode
	uint32_t ulOnesCnt = 0;
	for(uint32_t i = ulAddr; i < ulAddr + ulLength; ++i) {
		uint8_t ubRead = readByte(i);
		ulOnesCnt += onesCount(ubRead);
	}
	uint32_t ulBitCnt = (ulLength*8);
	uint32_t ulZerosCnt = ulBitCnt - ulOnesCnt;
	double percentage = (ulZerosCnt *100.0)/ulBitCnt;
	char szDbl[10];
	dtostrf(percentage, 0, 2, szDbl);
	char bfr[255];
	sprintf(bfr, "Zeros: %lu / %lu (%s%%)", ulZerosCnt, ulBitCnt, szDbl);
	Serial.println(bfr);
	return true;
}

bool tFlash::cmdErase(void) {
	return eraseAll();
}

