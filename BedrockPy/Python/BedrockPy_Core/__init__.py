import sys
import os
import prettytable
from prettytable import PrettyTable
import cppyy
import cppyy.ll
import pandas as pd

if __name__ != '__main__':
    system_info_table = PrettyTable(["Var", "Value"], header=True, encoding="UTF-8", hrules=prettytable.ALL)
    system_info_table.align["Value"] = "l"
    system_info_table.add_row(["Ver", f"{sys.version}"])
    system_info_table.add_row(["CWD", f"{os.getcwd()}"])
    system_info_table.add_row(["PATH", "{}".format('\n'.join([os.path.abspath(rp) for rp in sys.path]))])
    print(system_info_table.get_string(title="Embeded Python Information"))

    # cppyy.load_library("LibHook.dll")
    # cppyy.load_library("LibPDB.dll")
    # cppyy.cppdef('''
    # #include <unordered_map>
    # #include <tuple>
    # #include <algorithm>
    # using FunctionRAVList   = std::vector<DWORD>;
    # using FunctionSymList   = std::vector<std::string>;
    # using FunctionDecList   = std::vector<std::string>;
    # using FunctionMeta      = std::tuple<FunctionRAVList, FunctionSymList, FunctionDecList>;
    # void RegisterStaticHook(unsigned int sym, void* hook, void** org);
    # FunctionMeta &extract_symbols(const std::string &pdb_path);
    # ''')
    # print(cppyy.gbl.extract_symbols("./bedrock_server.pdb"))
    from _BedrockPy_Core_Tools import extract_symbols, HookBase, call_cppyy_test
    import pandas as pd
    from time import time
    syms = None
    pdb_proposals = map(os.path.abspath, ("./bedrock_server.pdb", "../bedrock_server.pdb"))
    for pdb_file in pdb_proposals:
        if os.path.isfile(pdb_file):
            print(f"Loading symbols from {os.path.abspath(pdb_file)}...", end="\t", flush=True)
            ravs, syms, decs = extract_symbols(os.path.abspath(pdb_file))
            print(f"{len(ravs)} functions found!", flush=True)
            symbols = pd.DataFrame(
                data=dict(
                    ravs=ravs,
                    syms=syms,
                    decs=decs
                )
            ).set_index('syms')
            # symbols.to_excel("symbols.xlsx")
            def dummy(a, b, c):
                return None
            hb = HookBase()
            hb.func_arg(dummy)

            cppyy.cppdef("""
            typedef unsigned long long addr_t;
            void cppyy_func (std::string what) {std::cout << what << std::endl;}
            addr_t address() {
                auto addr = reinterpret_cast<addr_t>(&cppyy_func);
                std::cout << addr << std::endl;
                return addr;
            }
            """)
            cppyy_func_addr = cppyy.gbl.address()
            print(cppyy_func_addr)
            call_cppyy_test(cppyy_func_addr, "hello from python!")


# if __name__ == '__main__':
#     os.system("../../bedrock_server.exe")
