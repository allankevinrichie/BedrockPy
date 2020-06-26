#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <atlcomcli.h>
#include <dia2.h>

#include <set>
#include <unordered_set>
#include <unordered_map>

typedef struct _SYMBOL SYMBOL, *PSYMBOL;
typedef struct _SYMBOL_ENUM_FIELD
{
	//
	// Name of the enumeration field.
	//
	CHAR*                Name;

	//
	// Assigned value of the enumeration field.
	//
	VARIANT              Value;

	//
	// Parent enumeration.
	//
	SYMBOL*              Parent;

} SYMBOL_ENUM_FIELD, *PSYMBOL_ENUM_FIELD;
typedef struct _SYMBOL_UDT_FIELD
{
	//
	// Name of the UDT field.
	//
	CHAR*                Name;

	//
	// Type of the field.
	//
	SYMBOL*              Type;

	//
	// Offset from the start of the structure/class/union.
	//
	DWORD                Offset;

	//
	// Amount of bits this field takes.
	// If this value is 0, this field takes
	// all of the space of the field type (Type->Size bytes).
	//
	DWORD                Bits;

	//
	// Which bit this fields starts at (relative to the Offset).
	//
	DWORD                BitPosition;

	//
	// Parent UDT symbol.
	//
	SYMBOL*              Parent;

} SYMBOL_UDT_FIELD, *PSYMBOL_UDT_FIELD;
typedef struct _SYMBOL_ENUM
{
	//
	// Count of fields in the enumeration.
	//
	DWORD                FieldCount;

	//
	// Pointer to the continuous array of structures of the enumeration fields.
	//
	SYMBOL_ENUM_FIELD*   Fields;

} SYMBOL_ENUM, *PSYMBOL_ENUM;
typedef struct _SYMBOL_TYPEDEF
{
	//
	// Underlying type of the type definition.
	//
	SYMBOL*              Type;

} SYMBOL_TYPEDEF, *PSYMBOL_TYPEDEF;
typedef struct _SYMBOL_POINTER
{
	//
	// Underlying type of the pointer definition.
	//
	SYMBOL*              Type;

	//
	// Specifies if the pointer represents the reference.
	//
	BOOL                 IsReference;

} SYMBOL_POINTER, *PSYMBOL_POINTER;
typedef struct _SYMBOL_ARRAY
{
	//
	// Type of the array element.
	//
	SYMBOL*              ElementType;

	//
	// Array size in elements.
	//
	DWORD                ElementCount;

} SYMBOL_ARRAY, *PSYMBOL_ARRAY;
typedef struct _SYMBOL_FUNCTION
{
	//
	// Return type of the function.
	//
	SYMBOL*              ReturnType;

	//
	// Calling convention of the function.
	//
	CV_call_e            CallingConvention;

	//
	// Number of arguments.
	//
	DWORD                ArgumentCount;

	//
	// Pointer to the continuous array of pointers to the symbol structure for arguments.
	// These symbols are of type SYMBOL_FUNCTIONARG and has tag SymTagFunctionArgType.
	//
	SYMBOL**             Arguments;

} SYMBOL_FUNCTION, *PSYMBOL_FUNCTION;
typedef struct _SYMBOL_FUNCTIONARG
{
	//
	// Underlying type of the argument.
	//
	SYMBOL*              Type;

} SYMBOL_FUNCTIONARG, *PSYMBOL_FUNCTIONARG;
typedef struct _SYMBOL_UDT
{
	//
	// Kind of the UDT.
	// It may be either UdtStruct, UdtClass or UdtUnion.
	//
	UdtKind              Kind;

	//
	// Number of fields (members) in the UDT.
	//
	DWORD                FieldCount;

	//
	// Pointer to the continuous array of structures of the UDT.
	//
	SYMBOL_UDT_FIELD*    Fields;

} SYMBOL_UDT, *PSYMBOL_UDT;
struct _SYMBOL
{
	//
	// Type of the symbol.
	//
	enum SymTagEnum      Tag;

	//
	// Data kind.
	// Only sef it Tag == SymTagData.
	//
	enum DataKind        DataKind;

	//
	// Base type.
	// Only set if Tag == SymTagBaseType.
	//
	BasicType            BaseType;

	//
	// Internal ID of the type.
	//
	DWORD                TypeId;

	//
	// Total size of the type which this symbol represents.
	//
	DWORD                Size;

	//
	// Specifies constness.
	//
	BOOL                 IsConst;

	//
	// Specifies volatileness.
	//
	BOOL                 IsVolatile;

	//
	// Name of the type.
	//
	CHAR*                Name;

	union
	{
		SYMBOL_ENUM        Enum;
		SYMBOL_TYPEDEF     Typedef;
		SYMBOL_POINTER     Pointer;
		SYMBOL_ARRAY       Array;
		SYMBOL_FUNCTION    Function;
		SYMBOL_FUNCTIONARG FunctionArg;
		SYMBOL_UDT         Udt;
	} u;
};

class SymbolModule;

using SymbolMap     = std::unordered_map<DWORD, SYMBOL*>;
using SymbolNameMap = std::unordered_map<std::string, SYMBOL*>;
using SymbolSet     = std::unordered_set<SYMBOL*>;
using FunctionSet   = std::set<std::string>;
using FunctionRAVList   = std::vector<DWORD>;
using FunctionSymList   = std::vector<std::string>;
using FunctionDecList   = std::vector<std::string>;
using FunctionMeta      = std::tuple<FunctionRAVList, FunctionSymList, FunctionDecList>;

class SymbolModuleBase {
public:
    SymbolModuleBase();

    virtual BOOL
    Open(
            IN const CHAR *Path
    );

    virtual VOID
    Close();

    virtual BOOL
    IsOpen() const;

private:
    HRESULT
    LoadDiaViaCoCreateInstance();

    HRESULT
    LoadDiaViaLoadLibrary();

protected:
    CComPtr<IDiaDataSource> m_DataSource;
    CComPtr<IDiaSession> m_Session;
    CComPtr<IDiaSymbol> m_GlobalSymbol;
};
class SymbolModule
        : public SymbolModuleBase {
public:
    SymbolModule();

    ~SymbolModule();

    BOOL
    Open(
            IN const CHAR *Path
    ) override;

    BOOL
    IsOpen() const override;

    VOID
    Close() override;

    SYMBOL *
    GetSymbol(
            IN IDiaSymbol *DiaSymbol
    );

    CHAR *
    GetSymbolName(
            IN IDiaSymbol *DiaSymbol
    );

    VOID
    BuildSymbolMapFromEnumerator(
            IN IDiaEnumSymbols *DiaSymbolEnumerator
    );

    VOID
    BuildFunctionSetFromEnumerator(
            IN IDiaEnumSymbols *DiaSymbolEnumerator
    );

    VOID
    BuildSymbolMap();

    FunctionRAVList m_FunctionRAVList;
    FunctionSymList m_FunctionSymList;
    FunctionDecList m_FunctionDecList;
private:
    VOID
    InitSymbol(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    static VOID
    DestroySymbol(
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolBase(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolEnum(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolTypedef(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolPointer(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolArray(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolFunction(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolFunctionArg(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

    VOID
    ProcessSymbolUdt(
            IN IDiaSymbol *DiaSymbol,
            IN SYMBOL *Symbol
    );

private:
    std::string m_Path;
    SymbolMap m_SymbolMap;
    SymbolNameMap m_SymbolNameMap;
    SymbolSet m_SymbolSet;
    FunctionSet m_FunctionSet;


    DWORD m_MachineType = 0;
    CV_CFL_LANG m_Language = CV_CFL_C;

};