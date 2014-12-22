#include "AlsongAPI.h"
#include "AlsongAPIService1SoapProxy.h"
CAlsongAPI::CAlsongAPI(void)
{
	proxy = new Service1SoapProxy(SOAP_C_UTFSTRING);
}


CAlsongAPI::~CAlsongAPI(void)
{
	if(proxy)
	{
		delete proxy;
		proxy = NULL;
	}
}

Service1SoapProxy* CAlsongAPI::Get() const
{
	return proxy;
}
