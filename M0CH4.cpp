#include "M0CH4.hpp"
#include <mach/kern_return.h>
#include <sys/_types/_uintptr_t.h>

inline std::vector<std::string> split(std::string text, char delim) {
    std::string line;
    std::vector<std::string> vec;
    std::stringstream ss(text);
    while(std::getline(ss, line, delim)) {
        vec.push_back(line);
    }
    return vec;
}

uintptr_t Mocha::FindPattern(uintptr_t base, std::string pattern, uintptr_t size) 
{
    std::vector<char>chars;
    std::vector<std::string> patternSplit = split(pattern, ' ');

    for(int i=0; i< patternSplit.size(); i++) {
        if (patternSplit.at(i) == "?") {
            chars.push_back('\00');
        } else {
            chars.push_back(std::stoi(patternSplit.at(i), nullptr, 16));
        }
    }

	uintptr_t patternLength = (uintptr_t)chars.size();

	for (uintptr_t i = 0; i < size - patternLength; i++)
	{
		bool found = true;
		for (uintptr_t j = 0; j < patternLength; j++)
		{
			found &= chars.at(j) == '\00' || chars.at(j) == *(char*)(base + i + j);
		}
		if (found)
			return base + i;
	}
	return 0;
}

bool Mocha::InlineHook(uintptr_t* toHook, void* ourFunct, int len)
{
    mach_vm_protect(mach_task_self(), (mach_vm_address_t)toHook, (mach_vm_size_t)len, FALSE, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE | VM_PROT_COPY);
    memset(toHook, 0x90, len);

    uintptr_t relativeAddress = ((uintptr_t)ourFunct - (uintptr_t)toHook) - 5;

    *(uintptr_t*)toHook = 0xE9;
    *(uintptr_t*)((uintptr_t)toHook + 1) = relativeAddress;

    mach_vm_protect(mach_task_self(), (mach_vm_address_t)toHook, (mach_vm_size_t)len, FALSE, VM_PROT_READ | VM_PROT_EXECUTE);

    return true;

}

mach_port_t Mocha::PIDToTask(pid_t pid)
{
    kern_return_t kern_return;
    mach_port_t task;
    kern_return = task_for_pid(mach_task_self(), pid, &task);
    if (kern_return != KERN_SUCCESS) {
        return 0;
    }
    return task;
}

vm_address_t Mocha::BaseAddress(mach_port_t task)
{
    kern_return_t kret;
    vm_region_basic_info_data_t info;
    mach_vm_size_t size;

    mach_port_t object_name;
    mach_msg_type_number_t count;
    vm_address_t firstRegionBegin;
    vm_address_t lastRegionEnd;
    vm_size_t fullSize = 0;
    count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_vm_address_t address = 1;

    int regionCount = 0;
    int flag = 0;
    while (flag == 0) {
        //Attempts to get the region info for given task
        kret = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &count, &object_name);
        if (kret == KERN_SUCCESS) {
            if (regionCount == 0) {
                firstRegionBegin = address;
                regionCount += 1;
            }
            fullSize += size;
            address += size;
        } else
            flag = 1;
    }
    lastRegionEnd = address;

    return firstRegionBegin;
}

vm_address_t Mocha::TopAddress(mach_port_t task)
{
    kern_return_t kret;
    vm_region_basic_info_data_t info;
    mach_vm_size_t size;

    mach_port_t object_name;
    mach_msg_type_number_t count;
    vm_address_t firstRegionBegin;
    vm_address_t lastRegionEnd;
    vm_size_t fullSize = 0;
    count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_vm_address_t address = 1;

    int regionCount = 0;
    int flag = 0;
    while (flag == 0) {
        //Attempts to get the region info for given task
        kret = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &count, &object_name);
        if (kret == KERN_SUCCESS) {
            if (regionCount == 0) {
                firstRegionBegin = address;
                regionCount += 1;
            }
            fullSize += size;
            address += size;
        } else
            flag = 1;
    }
    lastRegionEnd = address;

    return lastRegionEnd;
}

bool Mocha::IsAligned(uintptr_t address, uintptr_t offset) {
    return address % offset == 0x0;
}

std::vector<Mocha::m_Pointer> Mocha::PointerScan(uintptr_t address, uintptr_t alignment, uintptr_t scanSize, int depth, int cur) {
    if (cur >= depth) {
        return {};
    }

    std::vector<Mocha::m_Pointer> pointers;
    for (int offset = 0; offset < scanSize; offset += alignment) {
        uintptr_t* fnd = (uintptr_t*)(address + offset);
        
        if ((uintptr_t)*fnd > this->m_base && (uintptr_t)*fnd < this->m_top) {
            if (this->InReadableMemory(*fnd)) {
                Mocha::m_Pointer p = Mocha::m_Pointer(*fnd, offset);
                p.children = this->PointerScan(p.m_address, alignment, scanSize, depth, cur + 1);
                pointers.push_back(p);
            }
        } 
    }

    return pointers;
}

bool Mocha::InReadableMemory(uintptr_t address) {
    pid_t pid = getpid();
    mach_port_t task = this->PIDToTask(pid);

    kern_return_t kret = KERN_SUCCESS;
    vm_region_basic_info_data_t info;
    mach_vm_size_t size;

    mach_port_t object_name;
    mach_msg_type_number_t count;
    count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_vm_address_t _address = 1;

    int regionCount = 0;
    int flag = 0;
    while (kret == KERN_SUCCESS) {
        kret = mach_vm_region(task, &_address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &count, &object_name);
        if (info.protection & VM_PROT_READ )
        {
            if (address > _address  && address < _address + size) {
                return true;
            }
        }                
        _address += size;
    }
    return false;
}

bool Mocha::m_Pointer::SearchChain(Mocha::m_Pointer* p, uintptr_t* scanSize, uintptr_t* alignment, uintptr_t* scanLow, uintptr_t* scanHigh, std::vector<Mocha::m_Pointer>* chain) {
    for (Mocha::m_Pointer pointer : p->children) {
        chain->push_back(pointer);        

        for (int offset = 0; offset < *scanSize; offset += *alignment) {
            uintptr_t fnd = (uintptr_t)(pointer.m_address + offset);

            if ((uintptr_t)fnd >= *scanLow && (uintptr_t)fnd <= *scanHigh) {
                return true;
            } 
        }
        Mocha::m_Pointer::SearchChain(&pointer, scanSize, alignment, scanLow, scanHigh, chain);
    }
    return false;
}

void Mocha::SpiderScan(uintptr_t address, uintptr_t alignment, uintptr_t scanSize, int depth, uintptr_t scanLow, uintptr_t scanHigh) {
    std::vector<Mocha::m_Pointer> pointers = this->PointerScan(address, alignment, scanSize, depth);
    std::vector<Mocha::m_Pointer>* temp = new std::vector<Mocha::m_Pointer>();

    for (Mocha::m_Pointer pointer : pointers) {
        temp->push_back(pointer);
        std::cout << "Looping pointers - " << pointer.children.size() << std::endl;
        bool test = pointer.SearchChain(&pointer, &scanSize, &alignment, &scanLow, &scanHigh, temp);
        if (test) {
            std::cout << "Chain Size: " << temp->size() << std::endl;
            std::cout << std::hex << "Chain Base: " << address << std::endl;
            for (Mocha::m_Pointer p : *temp) {
                // std::cout <<  std::hex << " Address: " << p.m_address << std::endl;
                std::cout << std::hex << "  Offsets - " << p.m_offset << std::endl;
            }
            std::cout << std::endl;
            std::cout << std::endl;
        }
        temp->clear();

        std::cout << std::endl;
        std::cout << std::endl;
    }
}

Mocha::Mocha() {
    pid_t pid = getpid();
    mach_port_t task = this->PIDToTask(pid);
    this->m_base = this->BaseAddress(task);
    this->m_top = this->TopAddress(task);
}
/*

stage: 1
    scan for poitners

stage2:
    scan those pointers for pointers
    repeat stage2  to depth

engage:
    main routine to scan
        

*/