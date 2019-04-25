#include "EncryptByMd5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CStdString CEncryptByMd5::Digest(CStdString strPlainText)
{
	char *cTemp = CEncryptByMd5::Unicode2ANSI(strPlainText);
	unsigned int len = strPlainText.GetLength();
	char *cIdentity;
	CMd5A md5;
	cIdentity = md5.MDString(cTemp, len);
	return CStdString(cIdentity);
}

char * CEncryptByMd5::Unicode2ANSI(CStdString strSource)
{
	if (strSource.IsEmpty()) { return NULL; }
	char *pBuffer = NULL;
	int nBufferSize = 0;

	nBufferSize = WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strSource, -1, NULL, 0, NULL, NULL) + 1;
	pBuffer = new char[nBufferSize];
	memset(pBuffer, 0, sizeof(char)*nBufferSize);

	WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)strSource, -1, pBuffer, nBufferSize, NULL, NULL);

	return pBuffer;
}

// ½øÐÐUrl±àÂë UTF-8 
CStdString CEncryptByMd5::UrlEncode(CStdString strUnicode)
{
	LPCWSTR unicode = T2CW(strUnicode);
	int len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, 0, 0, 0, 0);
	if (!len) { return strUnicode; }
	char *utf8 = new char[len + 1];
	char *utf8temp = utf8;
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, len + 1, 0, 0);
	utf8[len] = NULL;
	CStdString strTemp, strEncodeData;
	while (*utf8 != NULL) {
		if (isalnum(*utf8) ||
			'-' == *utf8 ||
			'_' == *utf8 ||
			'.' == *utf8) {
			strEncodeData += *utf8;
		}
		else if (isspace(*utf8)) {
			strEncodeData += '+';
		}
		else {
			strTemp.Format(_T("%%%2X"), (BYTE)*utf8);
			strEncodeData += strTemp;
		}
		++utf8;
	}

	delete[] utf8temp;

	return CStdString(strEncodeData);
}
