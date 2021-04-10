#include <string>
#include <sys/_types/_mach_port_t.h>
#include <vector>
#include <sstream>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <mach/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/i386/kern_return.h>
#include <iostream>
#include <unistd.h>

#define JUMP(addr) ((void (*)(void)) addr)();

class Mocha {
    public: 
        struct m_Pointer {
            uintptr_t m_address;
            uintptr_t m_offset;
            std::vector<Mocha::m_Pointer> children;

            m_Pointer(uintptr_t address, uintptr_t offset) {
                this->m_address = address;
                this->m_offset = offset;
            }
        };

        Mocha();

        // Mocha Internals at Construct time to default scanning base addresses
        // to its internal process. 
        uintptr_t m_base;
        uintptr_t m_top;

        uintptr_t FindPattern(uintptr_t, std::string, uintptr_t);
        bool InlineHook(uintptr_t*, void*, int);
        template<typename RT, typename ...ARGS>
        RT Invoke(uintptr_t address, ARGS... args) {
            RT (*func)(ARGS...) = (RT (*)(ARGS...))address;
            return func(args...);
        }
        mach_port_t PIDToTask(pid_t);
        vm_address_t BaseAddress(mach_port_t);
        vm_address_t TopAddress(mach_port_t);
        template<typename T>
        void Write(uintptr_t address, T value) {
            *(T*)address = value;
        }
        template<typename T>
        T Read(uintptr_t address) {
            return *(T*)address;
        }
        bool IsAligned(uintptr_t, uintptr_t);
        std::vector<m_Pointer> PointerScan(uintptr_t, uintptr_t, uintptr_t, int = 1, int = 0);
        void SpiderScan(uintptr_t, uintptr_t, uintptr_t, int, uintptr_t, uintptr_t);
        bool InReadableMemory(uintptr_t);
    private:
};