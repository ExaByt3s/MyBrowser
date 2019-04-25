// EnDeCode.cpp: implementation of the CEnDeCode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EnDeCode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const CStdString CEnDeCode::Encode(char* plainText)
{
	const char * tmpStrKey = "test";
	const char * tmpStrPosKey = "9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF9876543210ABCDEF";

	char* tmpPlainText = plainText;
    int len = ::strlen(tmpPlainText);
	char* tmpOutString = new char[len + 1];
	int lenKey = ::strlen(tmpStrKey);
    int lenSecondKey = ::strlen(tmpStrPosKey);
    int m = 0;

    for(int i = 0; i < len; i++) {
        if (i >= lenSecondKey) {
            m = i - lenSecondKey;
        } else {
            m = i;
        }

        tmpOutString[i] = tmpPlainText[i];
        tmpOutString [i] = tmpOutString[i] ^ tmpStrPosKey[m];
        for(int j = 0; j < lenKey; j++) {
            tmpOutString[i] = tmpOutString[i] ^ tmpStrKey[j];
        }
    }
	CStdString strOut;
    for(int i = 0; i < len; i++) {        
		strOut.AppendFormat(_T("%02X"), tmpOutString[i]);
    }
    return strOut;
}