# FreeRTOS Heap Allocation: A Comprehensive Guide

## Table of Contents
1. Introduction
2. Memory Allocation Schemes
3. Choosing the Right Scheme
4. Configuration and Setup
5. Utility Functions
6. Best Practices
7. Advanced Topics

## 1. Introduction

FreeRTOS offers flexible heap management through multiple allocation schemes. When FreeRTOS needs to allocate memory (for tasks, queues, semaphores, etc.), it uses `pvPortMalloc()` instead of the standard C `malloc()`. Similarly, it uses `vPortFree()` instead of `free()`. These functions can also be called directly from application code.

### Why Custom Heap Management?

Standard C library allocation functions may be unsuitable for embedded systems because:
- They're not always available on small embedded systems
- Their implementation can be large
- They're rarely thread-safe
- They're non-deterministic
- They can suffer from fragmentation
- They can complicate linker configuration
- They can cause difficult-to-debug errors

## 2. Memory Allocation Schemes

FreeRTOS provides five heap implementation options:

### Heap_1
- Simplest implementation
- Only allows allocation, no freeing
- No fragmentation possible
- Deterministic
- Best for systems that:
  - Only create tasks/objects before starting the scheduler
  - Never delete tasks or objects
  - Need deterministic behavior
  - Have safety-critical requirements

### Heap_2 (Legacy)
- Superseded by heap_4
- Uses best-fit algorithm
- Allows both allocation and freeing
- Does not combine adjacent free blocks
- More susceptible to fragmentation than heap_4
- Only suitable when allocated/freed blocks are always the same size

### Heap_3
- Wraps standard C library malloc/free
- Makes them thread-safe using critical sections
- Heap size defined by linker configuration
- Useful when:
  - Standard library allocation is acceptable
  - Thread-safety is needed
  - Existing allocation system must be used

### Heap_4 (Recommended for Most Applications)
- Uses first-fit algorithm
- Combines adjacent free blocks (coalescing)
- Allows both allocation and freeing
- Minimizes fragmentation
- Suitable for applications that:
  - Need to repeatedly allocate/free memory
  - Work with different-sized blocks
  - Require efficient memory use

### Heap_5
- Same algorithm as heap_4
- Can combine multiple non-contiguous memory regions into a single heap
- Requires initialization via `vPortDefineHeapRegions()`
- Perfect for systems with:
  - Discontinuous memory spaces
  - Multiple memory regions
  - Need to manage scattered RAM blocks

## 3. Choosing the Right Scheme

Selection criteria:

1. **For Safety-Critical Systems**
   - Use Heap_1 if dynamic allocation is necessary
   - Consider static allocation instead

2. **For Simple Systems**
   - Use Heap_1 if objects are only created at startup
   - Use Heap_4 if objects need to be created/deleted during runtime

3. **For Complex Systems**
   - Use Heap_4 for general-purpose dynamic allocation
   - Use Heap_5 if memory is non-contiguous

4. **When Using Existing Allocation**
   - Use Heap_3 to make existing malloc/free thread-safe

## 4. Configuration and Setup

### Basic Configuration
```c
// In FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE ( size_in_bytes )
```

### Heap_5 Initialization
```c
// Define memory regions
HeapRegion_t xHeapRegions[] = {
    { ucHeap, HEAP1_SIZE },
    { RAM2_START_ADDRESS, RAM2_SIZE },
    { RAM3_START_ADDRESS, RAM3_SIZE },
    { NULL, 0 }  // Terminator
};

// Initialize heap_5
vPortDefineHeapRegions( xHeapRegions );
```

### Custom Heap Location
```c
#define configAPPLICATION_ALLOCATED_HEAP 1

// GCC syntax
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] __attribute__((section(".my_heap")));

// IAR syntax
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] @ 0x20000000;
```

## 5. Utility Functions

### Memory Statistics
```c
// Get current free heap size
size_t remaining = xPortGetFreeHeapSize();

// Get minimum ever free heap size
size_t minimum = xPortGetMinimumEverFreeHeapSize();

// Get detailed heap stats (Heap_4 and Heap_5 only)
HeapStats_t stats;
vPortGetHeapStats(&stats);
```

### Allocation Failure Handling
```c
// In FreeRTOSConfig.h
#define configUSE_MALLOC_FAILED_HOOK 1

// In application code
void vApplicationMallocFailedHook(void)
{
    // Handle allocation failure
}
```

## 6. Best Practices

1. **Memory Management**
   - Use Heap_4 unless you have specific requirements
   - Monitor heap usage with utility functions
   - Implement meaningful malloc failed hooks
   - Consider static allocation for critical systems

2. **Fragmentation Prevention**
   - Allocate long-lived objects first
   - Try to allocate similar-sized blocks
   - Avoid frequent allocation/deallocation of different-sized blocks
   - Use fixed-size memory pools for frequent allocations

3. **Performance**
   - Place heap in fast memory for better performance
   - Use separate allocation for task stacks if needed
   - Consider memory alignment requirements

4. **Debugging**
   - Use heap stats to track memory usage
   - Implement allocation tracking
   - Set meaningful malloc failed hooks
   - Monitor minimum free heap size

## 7. Advanced Topics

### Task Stack Placement
```c
// In FreeRTOSConfig.h
#define pvPortMallocStack(x) pvMallocFastMemory(x)
#define vPortFreeStack(x) vPortFreeFastMemory(x)
```

### Per-Task Memory Tracking
- Implement trace macros for allocation tracking
- Monitor per-task heap usage
- Track maximum memory usage

### Static Allocation
```c
// Enable static allocation
#define configSUPPORT_STATIC_ALLOCATION 1

// Implement required callbacks
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                 StackType_t **ppxIdleTaskStackBuffer,
                                 uint32_t *pulIdleTaskStackSize);

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                  StackType_t **ppxTimerTaskStackBuffer,
                                  uint32_t *pulTimerTaskStackSize);
```
