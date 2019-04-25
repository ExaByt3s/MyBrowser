
#if !defined(AFX_ENCRYPTBYMD5_H__19C260E7_3A3A_48D4_9085_0409B037CC2E__INCLUDED_)
#define AFX_ENCRYPTBYMD5_H__19C260E7_3A3A_48D4_9085_0409B037CC2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Md5A.h"
#include "StdString.h"

class CEncryptByMd5
{
public:	
	static char * Unicode2ANSI(CStdString strSource);
	static CStdString UrlEncode(CStdString strUnicode);
	static CStdString Digest(CStdString strPlainText);
};

#endif // !defined(AFX_ENCRYPTBYMD5_H__19C260E7_3A3A_48D4_9085_0409B037CC2E__INCLUDED_)
