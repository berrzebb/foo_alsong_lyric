#pragma once
#include "AlsongAPIService1SoapProxy.h"
class Service1SoapProxy;
class CAlsongAPI
{
public:
	CAlsongAPI(void);
	~CAlsongAPI(void);
	Service1SoapProxy* Get() const;
private:
	Service1SoapProxy* proxy;
};

