#include <atlbase.h>
#include <initguid.h>

DEFINE_GUID(IID_ICatRegister, 0x0002E012, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(CLSID_CNotifyObject, 0xF69513F1, 0x8E1A, 0x4F35, 0x82, 0xD9, 0x04, 0x64, 0x06, 0x97, 0x0E, 0x6D);
DEFINE_GUID(IID_INetCfgComponentBindings, 0xC0E8AE9E, 0x306E, 0x11D1, 0xAA, 0xCF, 0x00, 0x80, 0x5F, 0xC1, 0x27, 0x0E);
DEFINE_GUID(CLSID_StdComponentCategoriesMgr, 0x0002E005, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

HRESULT __cdecl ATL::AtlRegisterClassCategoriesHelper(REFCLSID clsid, const _ATL_CATMAP_ENTRY *catmap, BOOL reg)
{
	if (!catmap)
		return S_OK;

	CComPtr<ICatRegister> catreg;

	HRESULT hResult = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&catreg);
	if (FAILED(hResult))
		return hResult;

	for (const _ATL_CATMAP_ENTRY *iter = catmap; iter->iType != _ATL_CATMAP_ENTRY_END; iter++)
	{
		CATID catid = *iter->pcatid;

		if (iter->iType == _ATL_CATMAP_ENTRY_IMPLEMENTED)
		{
			if (reg)
				hResult = catreg->RegisterClassImplCategories(clsid, 1, &catid);
			else
				hResult = catreg->UnRegisterClassImplCategories(clsid, 1, &catid);
		}
		else
		{
			if (reg)
				hResult = catreg->RegisterClassReqCategories(clsid, 1, &catid);
			else
				hResult = catreg->UnRegisterClassReqCategories(clsid, 1, &catid);
		}
		if (FAILED(hResult))
			return hResult;
	}

	if (!reg)
	{
		WCHAR reg_path[256] = L"CLSID\\";

		StringFromGUID2(clsid, reg_path + wcslen(reg_path), 64);
		wcscat(reg_path, L"\\");
		WCHAR* ptr = reg_path + wcslen(reg_path);

		wcscpy(ptr, L"Implemented Categories");
		RegDeleteKeyW(HKEY_CLASSES_ROOT, reg_path);

		wcscpy(ptr, L"Required Categories");
		RegDeleteKeyW(HKEY_CLASSES_ROOT, reg_path);
	}

	return hResult;
}
