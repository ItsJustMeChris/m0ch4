# m0ch4

M0ch4 (Mocha) is a useful memory manipulation toolkit for MacOS/Unix systems.

Mocha currently features a small but versatile set of features which are useful for in process memory manipulation tasks, such as software cracking, and video game hacking.

# Features

- IDA style pattern scanning.
  - Useful for finding and maintaining miscellaneous offsets / memory addresses based on opcode patterns dumped from IDA, without the need to mess with 'masks' like most implementations.
- Inline-function hooking jmp placement.
  - Useful code to insert JMP instructions to our function in the middle of a remote function in a process, allowing us to execute our own code, or assembly, before returning to normal programmatic operation. See [This GIST](https://gist.github.com/ItsJustMeChris/1aeff828b43e6aa00a477b5c79da164c) for more information.
- Wrapper to invoke functions from memory addresses.
  - Useful when wanting to call functions from a memory address, without the need to declare and manage a typedef of the function.
- Wrapper function to invoke virtual functions in object Virtual Method Tables. 
   - Useful for when you need to call a function inside of an object, without needing to reverse out the function address every time (VMT indexes rarely change in most software)  
- Get mach-task port from PID.
  - Used to get a task from a process ID.
- Get base address for a mach-task port.
  - Used to get the base address of a Mach task/process.
- Wrapper function to write a value at a memory address.
  - Use to write an arbitrary value at a memory address.
- Wrapper function to read a value at a memory address.
  - Used to read an arbitrary value at a memory address.
- Method to find DYLIB image base address in memory. 
  - Useful for targetting dylibs inside of other processes.
  - Useful for targetting your own dylib inside a process to deploy CRC style integrity checks on your own code to avoid dynamic manipulation. 
- Macro to insert a JMP to a memory address.
  - Useful when GCC inline assembly doesn't want to cooperate or be simplistic.
- MachO Binary information extractor.
  - Used to dump MachO memory segments, and much more useful information. See [Mach-Observer](https://github.com/ItsJustMeChris/mach-observer)
- Address Alignment Sanity Function
  - Used to ensure that a memory address is aligned to a specific amount of bytes, useful for things like structure dissecting.
- Pointer Scanning
  - Scan around a memory address for other potential pointers, this is useful for dissecting complex data structures like Linked Lists.
- Spider Scanning
  - Scan around a memory address for potential offset pointer chains that lead to a segment of memory that you want, this is useful for finding sub-data in a custom data structure that you may need to gather more information about.
- Helper function to determine if an address is withint readable memory
  - Useful for helping narrow down if a pointer MIGHT be valid, of course there's no certainty that the pages permission won't change, or that that is a valid pointer. All we can do is try to mitigate invalid pointers. 
- Helper method to freeze / unfreeze all threads.

# Notes

Not everything is implemented yet, and the feature set grows as I find the need to implement new and or difficult to find programming problems for Mac reverse engineering.
