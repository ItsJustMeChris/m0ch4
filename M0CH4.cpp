#include "M0CH4.hpp"

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

void Mocha::PointerScan(uintptr_t address, uintptr_t alignment, uintptr_t scanSize) {
    std::cout << "Pointer scan started" << std::endl;
    for (int offset = 0; offset < scanSize; offset += alignment) {
        uintptr_t* fnd = (uintptr_t*)(address + offset);
        
        if ((uintptr_t)*fnd > this->base && (uintptr_t)*fnd < this->top) {
            std::cout << "Potential Pointer Found @ 0x" << std::hex << *fnd << " [OFF] - " << offset << std::endl;
        } 
    }
}

Mocha::Mocha() {
    pid_t pid = getpid();
    mach_port_t task = this->PIDToTask(pid);
    this->base = this->BaseAddress(task);
    this->top = this->TopAddress(task);
}