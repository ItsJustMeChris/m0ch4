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
        Mocha();

        // Mocha Internals at Construct time to default scanning base addresses
        // to its internal process. 
        uintptr_t base;
        uintptr_t top;

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
        bool IsAligned(uintptr_t address, uintptr_t offset);
        void PointerScan(uintptr_t address, uintptr_t alignment, uintptr_t scanSize);
    private:
};