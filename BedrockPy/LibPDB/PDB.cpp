#include "PDB.h"
#include "PDBCallback.h"

#include <dia2.h>       // IDia* interfaces
#include <atlcomcli.h>

#include <cassert>

#include <string>
#include <memory>

#include <Dbghelp.h>


SymbolModuleBase::SymbolModuleBase() {
    HRESULT hr = CoInitialize(nullptr);

    assert(hr == S_OK);
}

HRESULT
SymbolModuleBase::LoadDiaViaCoCreateInstance() {
    return CoCreateInstance(
            __uuidof(DiaSource),
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IDiaDataSource),
            (void **) &m_DataSource
    );
}

HRESULT
SymbolModuleBase::LoadDiaViaLoadLibrary() {
    HRESULT Result;
    HMODULE Module = LoadLibrary(TEXT("msdia140.dll"));

    if (!Module) {
        Result = HRESULT_FROM_WIN32(GetLastError());
        return Result;
    }

    using PDLLGETCLASSOBJECT_ROUTINE = HRESULT(WINAPI *)(REFCLSID, REFIID, LPVOID);
    auto DllGetClassObject = reinterpret_cast<PDLLGETCLASSOBJECT_ROUTINE>(GetProcAddress(Module, "DllGetClassObject"));

    if (!DllGetClassObject) {
        Result = HRESULT_FROM_WIN32(GetLastError());
        return Result;
    }

    CComPtr<IClassFactory> ClassFactory;
    Result = DllGetClassObject(__uuidof(DiaSource), __uuidof(IClassFactory), &ClassFactory);

    if (FAILED(Result)) {
        return Result;
    }

    return ClassFactory->CreateInstance(nullptr, __uuidof(IDiaDataSource), (void **) &m_DataSource);
}

BOOL
SymbolModuleBase::Open(
        IN const CHAR *Path
) {
    HRESULT Result = S_OK;
    LPCOLESTR PDBSearchPath = L"srv*.\\Symbols*https://msdl.microsoft.com/download/symbols";

    //
    // Load msdia140.dll.
    // First try registered COM class, if it fails,
    // do LoadLibrary() directly.
    //

    if (FAILED(Result = LoadDiaViaCoCreateInstance()) &&
        FAILED(Result = LoadDiaViaLoadLibrary())) {
        return FALSE;
    }

    //
    // Convert Path to WCHAR string.
    //

    int PathUnicodeLength = MultiByteToWideChar(CP_UTF8, 0, Path, -1, NULL, 0);
    auto PathUnicode = std::make_unique<WCHAR[]>(PathUnicodeLength);
    MultiByteToWideChar(CP_UTF8, 0, Path, -1, PathUnicode.get(), PathUnicodeLength);

    //
    // Parse the file extension.
    //

    WCHAR FileExtension[8] = {0};
    _wsplitpath_s(
            PathUnicode.get(),
            nullptr,
            0,
            nullptr,
            0,
            nullptr,
            0,
            FileExtension,
            _countof(FileExtension));

    //
    // If PDB file is specified, load it directly.
    // Otherwise, try to find the corresponding PDB for
    // the specified file (locally / symbol server).
    //

    if (_wcsicmp(FileExtension, L".pdb") == 0) {
        Result = m_DataSource->loadDataFromPdb(PathUnicode.get());
    } else {
        PDBCallback Callback;
        Callback.AddRef();

        Result = m_DataSource->loadDataForExe(PathUnicode.get(), PDBSearchPath, &Callback);
    }

    //
    // Check if PDB is open.
    //

    if (FAILED(Result)) {
        goto Error;
    }

    //
    // Open DIA session.
    //

    Result = m_DataSource->openSession(&m_Session);

    if (FAILED(Result)) {
        goto Error;
    }

    //
    // Get root symbol.
    //

    Result = m_Session->get_globalScope(&m_GlobalSymbol);

    if (FAILED(Result)) {
        goto Error;
    }

    return TRUE;

    Error:
    Close();
    return FALSE;
}

VOID
SymbolModuleBase::Close() {
    m_GlobalSymbol.Release();
    m_Session.Release();
    m_DataSource.Release();

    CoUninitialize();
}

BOOL
SymbolModuleBase::IsOpen() const {
    return m_DataSource && m_Session && m_GlobalSymbol;
}

//////////////////////////////////////////////////////////////////////////
// SymbolModule
//



SymbolModule::SymbolModule() {

}

SymbolModule::~SymbolModule() {
    Close();
}

BOOL
SymbolModule::Open(
        IN const CHAR *Path
) {
    BOOL Result;

    Result = SymbolModuleBase::Open(Path);

    if (Result == FALSE) {
        return FALSE;
    }

    m_GlobalSymbol->get_machineType(&m_MachineType);

    DWORD Language;
    m_GlobalSymbol->get_language(&Language);
    m_Language = static_cast<CV_CFL_LANG>(Language);

    BuildSymbolMap();

    return TRUE;
}

BOOL
SymbolModule::IsOpen() const {
    return SymbolModuleBase::IsOpen();
}

VOID
SymbolModule::Close() {
    SymbolModuleBase::Close();

    for (auto &&Symbol : m_SymbolSet) {
        DestroySymbol(Symbol);
        delete Symbol;
    }

    m_Path.clear();
    m_SymbolMap.clear();
    m_SymbolNameMap.clear();
    m_FunctionDecList.clear();
    m_FunctionSymList.clear();
    m_FunctionRAVList.clear();
    m_SymbolSet.clear();
}

CHAR *
SymbolModule::GetSymbolName(
        IN IDiaSymbol *DiaSymbol
) {
    BSTR SymbolNameBstr;

    if (DiaSymbol->get_name(&SymbolNameBstr) != S_OK) {
        //
        // Not all symbols have the name.
        //

        return nullptr;
    }

    //
    // BSTR is essentially a wide char string.
    // Since we work in multibyte character set,
    // we need to convert it.
    //

    CHAR *SymbolNameMb;
    size_t SymbolNameLength;

    SymbolNameLength = (size_t) SysStringLen(SymbolNameBstr) + 1;
    SymbolNameMb = new CHAR[SymbolNameLength];
    wcstombs(SymbolNameMb, SymbolNameBstr, SymbolNameLength);

    //
    // BSTR is supposed to be freed by this call.
    //

    SysFreeString(SymbolNameBstr);

    return SymbolNameMb;
}

SYMBOL *
SymbolModule::GetSymbol(
        IN IDiaSymbol *DiaSymbol
) {
    DWORD TypeId;
    DiaSymbol->get_symIndexId(&TypeId);

    auto it = m_SymbolMap.find(TypeId);

    if (it != m_SymbolMap.end()) {
        return it->second;
    }

    SYMBOL *Symbol;
    Symbol = new SYMBOL;
    m_SymbolMap[TypeId] = Symbol;
    m_SymbolSet.insert(Symbol);

    InitSymbol(DiaSymbol, Symbol);

    if (Symbol->Name) {
        m_SymbolNameMap[Symbol->Name] = Symbol;
    }

    return Symbol;
}

VOID
SymbolModule::BuildSymbolMapFromEnumerator(
        IN IDiaEnumSymbols *DiaSymbolEnumerator
) {
    IDiaSymbol *Result;
    ULONG FetchedSymbolCount = 0;

    while (SUCCEEDED(DiaSymbolEnumerator->Next(1, &Result, &FetchedSymbolCount)) && (FetchedSymbolCount == 1)) {
        CComPtr<IDiaSymbol> DiaChildSymbol(Result);

        GetSymbol(DiaChildSymbol);
    }
}

VOID
SymbolModule::BuildFunctionSetFromEnumerator(
        IN IDiaEnumSymbols *DiaSymbolEnumerator
) {
    IDiaSymbol *Result;
    ULONG FetchedSymbolCount = 0;

    while (SUCCEEDED(DiaSymbolEnumerator->Next(1, &Result, &FetchedSymbolCount)) && (FetchedSymbolCount == 1)) {
        CComPtr<IDiaSymbol> DiaChildSymbol(Result);

        BOOL IsFunction;
        DiaChildSymbol->get_function(&IsFunction);

        if (IsFunction) {
            CHAR *FunctionName = GetSymbolName(DiaChildSymbol);
            DWORD DwordResult;
            DWORD LongResult;
            auto undec_func = new char[1024];
            DiaChildSymbol->get_symTag(&DwordResult);
            DiaChildSymbol->get_addressOffset(&LongResult);
            // auto Tag = static_cast<enum SymTagEnum>(DwordResult);
            m_FunctionRAVList.push_back(LongResult);
            m_FunctionSymList.push_back(FunctionName);
            m_FunctionSet.insert(FunctionName);
            UnDecorateSymbolName(
                    FunctionName, undec_func, 1024,
                    UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_MS_KEYWORDS
                    );
            m_FunctionDecList.push_back(undec_func);
            delete[] FunctionName;
            delete[] undec_func;
        }
    }
}

VOID
SymbolModule::BuildSymbolMap() {
    if (CComPtr<IDiaEnumSymbols> DiaSymbolEnumerator;
            SUCCEEDED(m_GlobalSymbol->findChildren(SymTagPublicSymbol, nullptr, nsNone, &DiaSymbolEnumerator))) {
        BuildFunctionSetFromEnumerator(DiaSymbolEnumerator);
    }

    if (CComPtr<IDiaEnumSymbols> DiaSymbolEnumerator;
            SUCCEEDED(m_GlobalSymbol->findChildren(SymTagEnum, nullptr, nsNone, &DiaSymbolEnumerator))) {
        BuildSymbolMapFromEnumerator(DiaSymbolEnumerator);
    }

    if (CComPtr<IDiaEnumSymbols> DiaSymbolEnumerator;
            SUCCEEDED(m_GlobalSymbol->findChildren(SymTagUDT, nullptr, nsNone, &DiaSymbolEnumerator))) {
        BuildSymbolMapFromEnumerator(DiaSymbolEnumerator);
    }
}

VOID
SymbolModule::InitSymbol(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    DWORD DwordResult;
    ULONGLONG UlonglongResult;
    BOOL BoolResult;

    DiaSymbol->get_symTag(&DwordResult);
    Symbol->Tag = static_cast<enum SymTagEnum>(DwordResult);

    DiaSymbol->get_dataKind(&DwordResult);
    Symbol->DataKind = static_cast<enum DataKind>(DwordResult);

    DiaSymbol->get_baseType(&DwordResult);
    Symbol->BaseType = static_cast<BasicType>(DwordResult);

    DiaSymbol->get_typeId(&DwordResult);
    Symbol->TypeId = DwordResult;

    DiaSymbol->get_length(&UlonglongResult);
    Symbol->Size = static_cast<DWORD>(UlonglongResult);

    DiaSymbol->get_constType(&BoolResult);
    Symbol->IsConst = static_cast<BOOL>(BoolResult);

    DiaSymbol->get_volatileType(&BoolResult);
    Symbol->IsVolatile = static_cast<BOOL>(BoolResult);

    Symbol->Name = GetSymbolName(DiaSymbol);

    switch (Symbol->Tag) {
        case SymTagUDT:
            ProcessSymbolUdt(DiaSymbol, Symbol);
            break;
        case SymTagEnum:
            ProcessSymbolEnum(DiaSymbol, Symbol);
            break;
        case SymTagFunctionType:
            ProcessSymbolFunction(DiaSymbol, Symbol);
            break;
        case SymTagPointerType:
            ProcessSymbolPointer(DiaSymbol, Symbol);
            break;
        case SymTagArrayType:
            ProcessSymbolArray(DiaSymbol, Symbol);
            break;
        case SymTagBaseType:
            ProcessSymbolBase(DiaSymbol, Symbol);
            break;
        case SymTagTypedef:
            ProcessSymbolTypedef(DiaSymbol, Symbol);
            break;
        case SymTagFunctionArgType:
            ProcessSymbolFunctionArg(DiaSymbol, Symbol);
            break;
        default:
            break;
    }
}

VOID
SymbolModule::ProcessSymbolBase(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {

}

VOID
SymbolModule::ProcessSymbolEnum(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    CComPtr<IDiaEnumSymbols> DiaSymbolEnumerator;

    if (FAILED(DiaSymbol->findChildren(SymTagNull, nullptr, nsNone, &DiaSymbolEnumerator))) {
        return;
    }

    LONG ChildCount;
    DiaSymbolEnumerator->get_Count(&ChildCount);

    Symbol->u.Enum.FieldCount = static_cast<DWORD>(ChildCount);
    Symbol->u.Enum.Fields = new SYMBOL_ENUM_FIELD[ChildCount];

    IDiaSymbol *Result;
    ULONG FetchedSymbolCount = 0;
    DWORD Index = 0;

    while (SUCCEEDED(DiaSymbolEnumerator->Next(1, &Result, &FetchedSymbolCount)) && (FetchedSymbolCount == 1)) {
        CComPtr<IDiaSymbol> DiaChildSymbol(Result);

        SYMBOL_ENUM_FIELD *EnumValue = &Symbol->u.Enum.Fields[Index];

        EnumValue->Parent = Symbol;
        EnumValue->Name = GetSymbolName(DiaChildSymbol);

        VariantInit(&EnumValue->Value);
        DiaChildSymbol->get_value(&EnumValue->Value);

        Index += 1;
    }
}

VOID
SymbolModule::ProcessSymbolTypedef(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    CComPtr<IDiaSymbol> DiaTypedefSymbol;

    DiaSymbol->get_type(&DiaTypedefSymbol);

    Symbol->u.Typedef.Type = GetSymbol(DiaTypedefSymbol);
}

VOID
SymbolModule::ProcessSymbolPointer(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    CComPtr<IDiaSymbol> DiaPointerSymbol;

    DiaSymbol->get_type(&DiaPointerSymbol);
    DiaSymbol->get_reference(&Symbol->u.Pointer.IsReference);

    Symbol->u.Pointer.Type = GetSymbol(DiaPointerSymbol);

    if (m_MachineType == 0) {

        //
        // Sometimes the Machine type is not stored in the PDB.
        // If this is our case, try to guess the machine type
        // by pointer size.
        //

        switch (Symbol->Size) {
            case 4:
                m_MachineType = IMAGE_FILE_MACHINE_I386;
                break;
            case 8:
                m_MachineType = IMAGE_FILE_MACHINE_AMD64;
                break;
            default:
                m_MachineType = 0;
                break;
        }
    }
}

VOID
SymbolModule::ProcessSymbolArray(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    CComPtr<IDiaSymbol> DiaDataTypeSymbol;

    DiaSymbol->get_type(&DiaDataTypeSymbol);
    Symbol->u.Array.ElementType = GetSymbol(DiaDataTypeSymbol);

    DiaSymbol->get_count(&Symbol->u.Array.ElementCount);
}

VOID
SymbolModule::ProcessSymbolFunction(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    //
    // Calling convention.
    //

    DWORD CallingConvention;
    DiaSymbol->get_callingConvention(&CallingConvention);

    Symbol->u.Function.CallingConvention = static_cast<CV_call_e>(CallingConvention);

    //
    // Return type.
    //

    CComPtr<IDiaSymbol> DiaReturnTypeSymbol;
    DiaSymbol->get_type(&DiaReturnTypeSymbol);
    Symbol->u.Function.ReturnType = GetSymbol(DiaReturnTypeSymbol);

    //
    // Arguments.
    //

    CComPtr<IDiaEnumSymbols> DiaSymbolEnumerator;

    if (FAILED(DiaSymbol->findChildren(SymTagNull, nullptr, nsNone, &DiaSymbolEnumerator))) {
        return;
    }

    LONG ChildCount;

    DiaSymbolEnumerator->get_Count(&ChildCount);

    Symbol->u.Function.ArgumentCount = static_cast<DWORD>(ChildCount);
    Symbol->u.Function.Arguments = new SYMBOL *[ChildCount];

    IDiaSymbol *Result;
    ULONG FetchedSymbolCount = 0;
    DWORD Index = 0;

    while (SUCCEEDED(DiaSymbolEnumerator->Next(1, &Result, &FetchedSymbolCount)) && (FetchedSymbolCount == 1)) {
        CComPtr<IDiaSymbol> DiaChildSymbol(Result);

        SYMBOL *Argument;
        Argument = GetSymbol(DiaChildSymbol);
        Symbol->u.Function.Arguments[Index] = Argument;

        Index += 1;
    }
}

VOID
SymbolModule::ProcessSymbolFunctionArg(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    CComPtr<IDiaSymbol> DiaArgumentTypeSymbol;

    DiaSymbol->get_type(&DiaArgumentTypeSymbol);
    Symbol->u.FunctionArg.Type = GetSymbol(DiaArgumentTypeSymbol);
}

VOID
SymbolModule::ProcessSymbolUdt(
        IN IDiaSymbol *DiaSymbol,
        IN SYMBOL *Symbol
) {
    DWORD Kind;
    DiaSymbol->get_udtKind(&Kind);
    Symbol->u.Udt.Kind = static_cast<UdtKind>(Kind);

    CComPtr<IDiaEnumSymbols> DiaSymbolEnumerator;

    if (FAILED(DiaSymbol->findChildren(SymTagData, nullptr, nsNone, &DiaSymbolEnumerator))) {
        return;
    }

    LONG ChildCount;

    DiaSymbolEnumerator->get_Count(&ChildCount);

    Symbol->u.Udt.FieldCount = static_cast<DWORD>(ChildCount);
    Symbol->u.Udt.Fields = new SYMBOL_UDT_FIELD[ChildCount + 1];

    IDiaSymbol *Result;
    ULONG FetchedSymbolCount = 0;
    DWORD Index = 0;

    while (SUCCEEDED(DiaSymbolEnumerator->Next(1, &Result, &FetchedSymbolCount)) && (FetchedSymbolCount == 1)) {
        CComPtr<IDiaSymbol> DiaChildSymbol(Result);

        SYMBOL_UDT_FIELD *Member = &Symbol->u.Udt.Fields[Index];

        Member->Name = GetSymbolName(DiaChildSymbol);
        Member->Parent = Symbol;

        LONG Offset = 0;
        DiaChildSymbol->get_offset(&Offset);
        Member->Offset = static_cast<DWORD>(Offset);

        ULONGLONG Bits = 0;
        DiaChildSymbol->get_length(&Bits);
        Member->Bits = static_cast<DWORD>(Bits);

        DiaChildSymbol->get_bitPosition(&Member->BitPosition);

        CComPtr<IDiaSymbol> MemberTypeDiaSymbol;
        DiaChildSymbol->get_type(&MemberTypeDiaSymbol);
        Member->Type = GetSymbol(MemberTypeDiaSymbol);

        Index += 1;
    }

    //
    // Padding.
    //
    if (Symbol->u.Udt.Kind == UdtStruct && Symbol->u.Udt.FieldCount > 0 &&
        Symbol->u.Udt.Fields[Symbol->u.Udt.FieldCount - 1].Type != nullptr) {
        SYMBOL_UDT_FIELD *LastUdtField = &Symbol->u.Udt.Fields[Symbol->u.Udt.FieldCount - 1];
        SYMBOL_UDT_FIELD *PaddingUdtField = &Symbol->u.Udt.Fields[Symbol->u.Udt.FieldCount];
        DWORD PaddingSize = Symbol->Size - (LastUdtField->Offset + LastUdtField->Type->Size);

        if (PaddingSize > 0) {
            auto *PaddingSymbolArrayElement = new SYMBOL;
            PaddingSymbolArrayElement->Tag = SymTagBaseType;
            PaddingSymbolArrayElement->BaseType = !(PaddingSize % 4) ? btLong : btChar;
            PaddingSymbolArrayElement->TypeId = 0;
            PaddingSymbolArrayElement->Size = PaddingSymbolArrayElement->BaseType == btLong ? 4 : 1;
            PaddingSymbolArrayElement->IsConst = FALSE;
            PaddingSymbolArrayElement->IsVolatile = FALSE;
            PaddingSymbolArrayElement->Name = nullptr;

            auto *PaddingSymbolArray = new SYMBOL;
            PaddingSymbolArray->Tag = SymTagArrayType;
            PaddingSymbolArray->BaseType = btNoType;
            PaddingSymbolArray->TypeId = 0;
            PaddingSymbolArray->Size = PaddingSize;
            PaddingSymbolArray->IsConst = FALSE;
            PaddingSymbolArray->IsVolatile = FALSE;
            PaddingSymbolArray->Name = nullptr;
            PaddingSymbolArray->u.Array.ElementType = PaddingSymbolArrayElement;
            PaddingSymbolArray->u.Array.ElementCount =
                    PaddingSymbolArrayElement->BaseType == btLong ? PaddingSize / 4 : PaddingSize;

            PaddingUdtField->Name = new CHAR[64];
            PaddingUdtField->Type = PaddingSymbolArray;
            PaddingUdtField->Offset = LastUdtField->Offset + LastUdtField->Type->Size;

            PaddingUdtField->Bits = 0;
            PaddingUdtField->BitPosition = 0;
            PaddingUdtField->Parent = Symbol;

            strcpy(PaddingUdtField->Name, "__PADDING__");

            Symbol->u.Udt.FieldCount++;

            m_SymbolSet.insert(PaddingSymbolArray);
            m_SymbolSet.insert(PaddingSymbolArrayElement);
        }
    }
}

void SymbolModule::DestroySymbol(
        IN SYMBOL *Symbol
) {
    delete[] Symbol->Name;

    switch (Symbol->Tag) {
        case SymTagUDT:
            for (DWORD i = 0; i < Symbol->u.Udt.FieldCount; i++) {
                delete[] Symbol->u.Udt.Fields[i].Name;
            }

            delete[] Symbol->u.Udt.Fields;
            break;

        case SymTagEnum:
            for (DWORD i = 0; i < Symbol->u.Enum.FieldCount; i++) {
                delete[] Symbol->u.Enum.Fields[i].Name;
            }

            delete[] Symbol->u.Enum.Fields;
            break;

        case SymTagFunctionType:
            delete[] Symbol->u.Function.Arguments;
            break;
        case SymTagNull:
        case SymTagExe:
        case SymTagCompiland:
        case SymTagCompilandDetails:
        case SymTagCompilandEnv:
        case SymTagFunction:
        case SymTagBlock:
        case SymTagData:
        case SymTagAnnotation:
        case SymTagLabel:
        case SymTagPublicSymbol:
        case SymTagPointerType:
        case SymTagArrayType:
        case SymTagBaseType:
        case SymTagTypedef:
        case SymTagBaseClass:
        case SymTagFriend:
        case SymTagFunctionArgType:
        case SymTagFuncDebugStart:
        case SymTagFuncDebugEnd:
        case SymTagUsingNamespace:
        case SymTagVTableShape:
        case SymTagVTable:
        case SymTagCustom:
        case SymTagThunk:
        case SymTagCustomType:
        case SymTagManagedType:
        case SymTagDimension:
        case SymTagCallSite:
        case SymTagInlineSite:
        case SymTagBaseInterface:
        case SymTagVectorType:
        case SymTagMatrixType:
        case SymTagHLSLType:
        case SymTagCaller:
        case SymTagCallee:
        case SymTagExport:
        case SymTagHeapAllocationSite:
        case SymTagCoffGroup:
        case SymTagInlinee:
        case SymTagMax:
            break;
    }
}
