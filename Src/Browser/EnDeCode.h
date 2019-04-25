// EnDeCode.h: interface for the CEnDeCode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENDECODE_H_)
#define AFX_ENDECODE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdString.h"

class CEnDeCode
{
public:
    const static CStdString Encode(char* plainText);
};

#endif // !defined(AFX_ENDECODE_H_)
