# FURI Memory Management Documentation

## Overview
The FURI Memory Management System provides a robust and efficient memory allocation framework built on FreeRTOS's heap implementation. It features heap management, memory pools, allocation tracking, and debugging capabilities. The system is designed to prevent fragmentation while providing detailed memory usage insights.

## Core Concepts

### Memory Architecture
FURI uses a dual-pool memory architecture:
- **Main Heap**: Dynamic allocations using a modified FreeRTOS heap4 implementation
- **Memory Pool**: Fixed-size allocations for critical system components

```c
// Memory regions
typedef enum {
    FuriMemRegionHeap,     // Main dynamic heap
    FuriMemRegionPool,     // Fixed-size pool
    FuriMemRegionTotal     // Combined memory stats
} FuriMemRegion;
```

### Block Structure
Memory blocks are managed using a linked list structure:

```c
typedef struct A_BLOCK_LINK {
    struct A_BLOCK_LINK* pxNextFreeBlock; // Next free block
    size_t xBlockSize;                    // Block size including header
} BlockLink_t;
```

Key concepts:
- **Block Header**: Contains size and allocation status
- **Alignment**: All blocks are aligned to portBYTE_ALIGNMENT
- **Minimum Size**: Blocks must be at least heapMINIMUM_BLOCK_SIZE

### Memory Allocation Types

The system provides several allocation mechanisms:

```c
// Standard allocations
void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void* ptr, size_t size);

// Aligned allocations
void* aligned_malloc(size_t size, size_t alignment);

// Pool allocations
void* memmgr_alloc_from_pool(size_t size);
```

## Memory Pool System

### Pool Architecture
The memory pool provides fast, fragmentation-free allocations:

```c
// Pool allocation example
void* critical_alloc = memmgr_alloc_from_pool(size);
if(!critical_alloc) {
    // Fallback to heap
    critical_alloc = malloc(size);
}
```

Key features:
- Fixed block sizes
- No fragmentation
- Fast allocation/deallocation
- Automatic fallback to heap

### Pool Statistics
```c
// Get pool statistics
size_t free_pool = memmgr_pool_get_free();
size_t max_block = memmgr_pool_get_max_block();
```

## Heap Management

### Heap Implementation
The heap uses a modified FreeRTOS heap4 implementation with additional features:

```c
// Heap structure
static BlockLink_t xStart;           // List start marker
static BlockLink_t* pxEnd = NULL;    // List end marker
static size_t xFreeBytesRemaining;   // Free bytes counter
```

### Memory Coalescing
The system automatically combines adjacent free blocks:

```c
// Simplified coalescing logic
static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert) {
    BlockLink_t* pxIterator;
    uint8_t* puc;
    
    // Find insertion point
    for(pxIterator = &xStart; 
        pxIterator->pxNextFreeBlock < pxBlockToInsert;
        pxIterator = pxIterator->pxNextFreeBlock) {}
        
    // Check if blocks can be merged
    puc = (uint8_t*)pxIterator;
    if((puc + pxIterator->xBlockSize) == (uint8_t*)pxBlockToInsert) {
        // Merge with previous block
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }
    
    // Similar check for next block...
}
```

## Memory Tracking

### Thread Memory Tracking
FURI can track per-thread memory allocations:

```c
// Enable tracking for current thread
void enable_thread_allocation_tracking(void) {
    FuriThreadId thread_id = furi_thread_get_current_id();
    memmgr_heap_enable_thread_trace(thread_id);
}

// Get thread memory usage
size_t get_thread_memory(void) {
    FuriThreadId thread_id = furi_thread_get_current_id();
    return memmgr_heap_get_thread_memory(thread_id);
}
```

### Memory Statistics
The system provides detailed memory statistics:

```c
// Get memory statistics
size_t total_heap = memmgr_get_total_heap();
size_t free_heap = memmgr_get_free_heap();
size_t minimum_free = memmgr_get_minimum_free_heap();
```

## Best Practices

### Memory Allocation
```c
// CORRECT: Check allocation results
void* data = malloc(size);
furi_check(data != NULL);

// CORRECT: Use calloc for zero-initialization
void* zeroed_data = calloc(1, size);

// CORRECT: Aligned allocation for hardware requirements
void* aligned_data = aligned_malloc(size, 8);
```

### Memory Deallocation
```c
// CORRECT: NULL check before free
void safe_free(void* ptr) {
    if(ptr != NULL) {
        free(ptr);
    }
}

// CORRECT: Clear pointer after free
void clear_and_free(void** ptr) {
    if(ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}
```

### Resource Management
```c
// CORRECT: Proper cleanup order
void cleanup_resources(void) {
    // 1. Free allocated memory
    free(dynamic_memory);
    
    // 2. Free aligned memory
    aligned_free(aligned_memory);
    
    // 3. Clear pointers
    dynamic_memory = NULL;
    aligned_memory = NULL;
}
```

## Example Usage

Here's a complete example showing common memory management patterns:

```c
typedef struct {
    void* heap_data;
    void* pool_data;
    void* aligned_data;
} MemoryExample;

bool memory_example_init(MemoryExample* example, size_t size) {
    // Regular heap allocation
    example->heap_data = malloc(size);
    if(!example->heap_data) {
        return false;
    }
    
    // Pool allocation for critical data
    example->pool_data = memmgr_alloc_from_pool(size);
    if(!example->pool_data) {
        example->pool_data = malloc(size); // Fallback
        if(!example->pool_data) {
            free(example->heap_data);
            return false;
        }
    }
    
    // Aligned allocation
    example->aligned_data = aligned_malloc(size, 8);
    if(!example->aligned_data) {
        free(example->heap_data);
        free(example->pool_data);
        return false;
    }
    
    return true;
}

void memory_example_free(MemoryExample* example) {
    if(example) {
        free(example->heap_data);
        free(example->pool_data);
        aligned_free(example->aligned_data);
        
        example->heap_data = NULL;
        example->pool_data = NULL;
        example->aligned_data = NULL;
    }
}
```

## Advanced Topics

### Memory Protection
FURI implements several memory protection mechanisms:

1. **Heap Corruption Detection**
   - Block header validation
   - Alignment checks
   - Size validation

2. **Memory Tracking**
   - Per-thread allocation tracking
   - Memory leak detection
   - High water mark tracking

### Performance Considerations
- Use pool allocations for critical paths
- Minimize allocations in tight loops
- Consider block alignment for performance
- Use appropriate allocation sizes to reduce fragmentation

### Debugging Tools
The system provides several debugging capabilities:

```c
// Print free block information
void debug_memory(void) {
    memmgr_heap_printf_free_blocks();
}

// Get maximum contiguous free block
size_t max_block = memmgr_heap_get_max_free_block();

// Track thread allocations
memmgr_heap_enable_thread_trace(thread_id);
size_t thread_memory = memmgr_heap_get_thread_memory(thread_id);
```

The memory management system provides a robust foundation for building reliable applications while maintaining efficient memory usage and providing powerful debugging capabilities. 