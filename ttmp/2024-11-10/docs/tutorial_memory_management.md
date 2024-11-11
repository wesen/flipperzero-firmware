# FURI Memory Management Tutorial

## Introduction
This tutorial will guide you through using FURI's memory management system effectively. We'll start with basic concepts and progress to advanced usage patterns.

## Prerequisites
- Basic C programming knowledge
- FURI development environment set up
- Understanding of memory concepts (heap, stack, allocation)

## Core Concepts

### Memory Architecture Overview

FURI's memory management system is built on a dual-pool architecture designed for embedded systems:

1. **Main Heap**
   - Uses a modified FreeRTOS heap4 implementation
   - Block-based allocation with coalescing
   - Suitable for general-purpose allocations
   - Managed using standard malloc/free interfaces

2. **Memory Pool**
   - Fixed-size block allocator
   - Zero fragmentation
   - Fast allocation/deallocation
   - Ideal for critical system components

### Block Management

Memory blocks in FURI are managed using headers:

```c
typedef struct BlockLink {
    struct BlockLink* next;     // Next block in list
    size_t size;               // Size including header
    bool is_free;              // Block status
} BlockLink_t;
```

Key concepts:
- Minimum block size is sizeof(BlockLink_t)
- Size includes header overhead
- Blocks are aligned to CPU word size
- Free blocks are coalesced automatically

### Memory Regions

FURI divides memory into distinct regions:

```c
typedef enum {
    FuriMemRegionHeap,     // Main dynamic heap
    FuriMemRegionPool,     // Fixed-size pool
    FuriMemRegionTotal     // Combined memory
} FuriMemRegion;
```

Each region has:
- Dedicated allocation strategy
- Separate free list
- Independent fragmentation tracking
- Region-specific statistics

### Allocation Strategies

1. **Heap Allocation**
   - First-fit algorithm
   - Automatic block splitting
   - Coalescing of adjacent free blocks
   - O(n) allocation time

2. **Pool Allocation**
   - O(1) allocation time
   - Fixed block sizes
   - No fragmentation
   - Ideal for frequent allocations/deallocations

### Understanding Memory Pools

Memory pools work by pre-allocating a set of fixed-size blocks:

```c
// Internal pool structure (simplified)
typedef struct {
    void* start_address;        // Pool beginning
    size_t block_size;         // Size of each block
    size_t total_blocks;       // Total blocks available
    BitArray_t* free_blocks;   // Bitmap of free blocks
    FuriMutex* mutex;         // Thread safety
} FuriMemPool;
```

Benefits:
- No fragmentation
- Predictable allocation time
- Efficient for same-sized allocations
- Better cache utilization

## Part 1: Basic Memory Management

### Step 1: Understanding FURI's Memory Architecture

FURI uses a dual-pool architecture:
1. Main Heap - For general dynamic allocations
2. Memory Pool - For fixed-size critical allocations

Let's start with a simple example:

```c
// Basic allocation example
void* basic_allocation(void) {
    // Allocate 100 bytes from heap
    void* data = malloc(100);
    
    // Always check allocation result
    furi_check(data != NULL);
    
    return data;
}
```

### Step 2: Memory Pool Usage

The memory pool is ideal for critical allocations:

```c
void* critical_allocation(size_t size) {
    // Try pool first
    void* data = memmgr_alloc_from_pool(size);
    if(!data) {
        // Fallback to heap if pool is full
        data = malloc(size);
        furi_check(data != NULL);
    }
    return data;
}
```

### Exercise 1: Basic Memory Management
Create a simple structure and manage its memory:

```c
typedef struct {
    uint8_t* buffer;
    size_t size;
} DataBuffer;

// Create buffer
DataBuffer* create_buffer(size_t size) {
    DataBuffer* db = malloc(sizeof(DataBuffer));
    furi_check(db != NULL);
    
    db->buffer = malloc(size);
    if(!db->buffer) {
        free(db);
        return NULL;
    }
    
    db->size = size;
    return db;
}

// Free buffer
void free_buffer(DataBuffer* db) {
    if(db) {
        free(db->buffer);
        free(db);
    }
}
```

### Memory Alignment

Memory alignment is crucial for:
- Hardware requirements
- CPU efficiency
- DMA operations
- Cache performance

FURI ensures proper alignment:
```c
// Internal alignment calculation
size_t calculate_alignment(size_t size, size_t alignment) {
    return (size + (alignment - 1)) & ~(alignment - 1);
}
```

### Memory Tracking System

FURI's memory tracking provides:
1. Per-thread allocation tracking
2. Memory leak detection
3. Usage statistics
4. High water mark tracking

Internal implementation:
```c
typedef struct {
    size_t current_usage;     // Current allocations
    size_t peak_usage;        // Maximum usage seen
    uint32_t alloc_count;     // Number of allocations
    uint32_t free_count;      // Number of frees
} ThreadMemStats;
```

### Resource Management Patterns

FURI implements several resource management patterns:

1. **RAII-like Pattern**
   - Resource acquisition is initialization
   - Automatic cleanup on scope exit
   - Used in mutex and thread management

2. **Guard Pattern**
   - Protects resources from leaks
   - Ensures cleanup on error paths
   - Manages complex resource dependencies

3. **Pool Pattern**
   - Pre-allocated resources
   - Fast allocation/deallocation
   - Prevents fragmentation

## Part 2: Advanced Memory Patterns

### Step 1: Using Aligned Memory

Some hardware requires aligned memory access:

```c
void aligned_memory_example(void) {
    // Allocate 256 bytes aligned to 8-byte boundary
    void* aligned_data = aligned_malloc(256, 8);
    furi_check(aligned_data != NULL);
    
    // Use aligned memory
    // ...
    
    // Free aligned memory
    aligned_free(aligned_data);
}
```

### Step 2: Memory Tracking

Track memory usage in your thread:

```c
void memory_tracking_example(void) {
    // Enable tracking
    FuriThreadId thread_id = furi_thread_get_current_id();
    memmgr_heap_enable_thread_trace(thread_id);
    
    // Allocate some memory
    void* data = malloc(1000);
    
    // Check thread's memory usage
    size_t used = memmgr_heap_get_thread_memory(thread_id);
    FURI_LOG_I("Memory", "Thread using %d bytes", (int)used);
    
    free(data);
}
```

### Exercise 2: Memory Pool Manager
Create a simple memory pool manager:

```c
typedef struct {
    void* pool_memory;
    size_t total_size;
    size_t used_size;
    FuriMutex* mutex;
} PoolManager;

PoolManager* pool_manager_create(size_t size) {
    PoolManager* pm = malloc(sizeof(PoolManager));
    if(!pm) return NULL;
    
    pm->pool_memory = memmgr_alloc_from_pool(size);
    if(!pm->pool_memory) {
        free(pm);
        return NULL;
    }
    
    pm->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!pm->mutex) {
        free(pm->pool_memory);
        free(pm);
        return NULL;
    }
    
    pm->total_size = size;
    pm->used_size = 0;
    
    return pm;
}

void pool_manager_free(PoolManager* pm) {
    if(pm) {
        if(pm->mutex) furi_mutex_free(pm->mutex);
        if(pm->pool_memory) free(pm->pool_memory);
        free(pm);
    }
}
```

## Part 3: Best Practices and Patterns

### Pattern 1: Resource Guard

Create a safe resource allocation pattern:

```c
typedef struct {
    void* data;
    size_t size;
    bool is_pool_allocation;
} SafeResource;

SafeResource* safe_resource_alloc(size_t size) {
    SafeResource* sr = malloc(sizeof(SafeResource));
    if(!sr) return NULL;
    
    // Try pool first
    sr->data = memmgr_alloc_from_pool(size);
    sr->is_pool_allocation = true;
    
    // Fallback to heap
    if(!sr->data) {
        sr->data = malloc(size);
        sr->is_pool_allocation = false;
        
        if(!sr->data) {
            free(sr);
            return NULL;
        }
    }
    
    sr->size = size;
    return sr;
}

void safe_resource_free(SafeResource* sr) {
    if(sr) {
        if(sr->data) {
            if(sr->is_pool_allocation) {
                // Pool memory can be freed with regular free
                free(sr->data);
            } else {
                free(sr->data);
            }
        }
        free(sr);
    }
}
```

### Pattern 2: Memory Statistics Monitor

Create a memory usage monitor:

```c
typedef struct {
    size_t heap_used;
    size_t pool_used;
    size_t heap_free;
    size_t pool_free;
} MemoryStats;

void get_memory_stats(MemoryStats* stats) {
    if(!stats) return;
    
    // Get heap statistics
    stats->heap_free = memmgr_get_free_heap();
    stats->heap_used = memmgr_get_total_heap() - stats->heap_free;
    
    // Get pool statistics
    stats->pool_free = memmgr_pool_get_free();
    stats->pool_used = memmgr_get_total_heap() - stats->pool_free;
}

void monitor_memory(void) {
    MemoryStats stats;
    get_memory_stats(&stats);
    
    FURI_LOG_I("Memory", "Heap: %d used, %d free", 
        (int)stats.heap_used, 
        (int)stats.heap_free);
    
    FURI_LOG_I("Memory", "Pool: %d used, %d free", 
        (int)stats.pool_used, 
        (int)stats.pool_free);
}
```

### Exercise 3: Memory-Safe Container

Create a dynamic array with memory safety:

```c
typedef struct {
    void** items;
    size_t capacity;
    size_t size;
    FuriMutex* mutex;
} SafeArray;

SafeArray* safe_array_create(size_t initial_capacity) {
    SafeArray* array = malloc(sizeof(SafeArray));
    if(!array) return NULL;
    
    array->items = malloc(sizeof(void*) * initial_capacity);
    if(!array->items) {
        free(array);
        return NULL;
    }
    
    array->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!array->mutex) {
        free(array->items);
        free(array);
        return NULL;
    }
    
    array->capacity = initial_capacity;
    array->size = 0;
    
    return array;
}

bool safe_array_add(SafeArray* array, void* item) {
    if(!array || !item) return false;
    
    if(furi_mutex_acquire(array->mutex, FuriWaitForever) != FuriStatusOk) {
        return false;
    }
    
    if(array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        void** new_items = realloc(array->items, 
            sizeof(void*) * new_capacity);
            
        if(!new_items) {
            furi_mutex_release(array->mutex);
            return false;
        }
        
        array->items = new_items;
        array->capacity = new_capacity;
    }
    
    array->items[array->size++] = item;
    furi_mutex_release(array->mutex);
    
    return true;
}

void safe_array_free(SafeArray* array) {
    if(array) {
        if(array->mutex) furi_mutex_free(array->mutex);
        if(array->items) free(array->items);
        free(array);
    }
}
```

## Common Pitfalls and Solutions

### 1. Memory Leaks
```c
// WRONG: Memory leak
void leak_example(void) {
    void* data = malloc(100);
    if(error_condition) {
        return;  // Memory leaked!
    }
    free(data);
}

// RIGHT: Proper cleanup
void no_leak_example(void) {
    void* data = malloc(100);
    if(error_condition) {
        free(data);  // Clean up before return
        return;
    }
    free(data);
}
```

### 2. Double Free
```c
// WRONG: Double free
void double_free_example(void* ptr) {
    free(ptr);
    free(ptr);  // Crash!
}

// RIGHT: Clear pointer after free
void safe_free_example(void** ptr) {
    if(ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;  // Prevent double free
    }
}
```

### 3. Memory Fragmentation
```c
// WRONG: Frequent small allocations
void fragmentation_example(void) {
    for(int i = 0; i < 1000; i++) {
        void* small = malloc(10);  // Creates fragmentation
        // ... use small ...
        free(small);
    }
}

// RIGHT: Pool for small allocations
void pool_example(void) {
    void* pool = memmgr_alloc_from_pool(1000);
    // Use pool for small allocations
    // Single free at end
    free(pool);
}
```

## Best Practices Summary

1. Always check allocation results
2. Use pool for critical allocations
3. Clean up resources in reverse order
4. Track memory usage in long-running threads
5. Use aligned memory when required
6. Implement proper error handling
7. Monitor memory statistics regularly

## Advanced Topics

### Memory Manager Internals

The memory manager consists of several key components:

1. **Block Lists**
```c
typedef struct {
    BlockLink_t* free_list;    // List of free blocks
    BlockLink_t* used_list;    // List of used blocks
    size_t total_size;        // Total managed size
    size_t free_size;         // Available memory
} HeapRegion;
```

2. **Allocation Strategy**
The allocator uses a modified first-fit algorithm with optimizations:
- Quick-fit for small sizes
- Segregated free lists
- Boundary tag coalescing

3. **Thread Safety**
Memory management is thread-safe through:
- Critical sections
- Atomic operations
- Lock-free operations where possible

## Debugging and Profiling

### Memory Debugging Tools

FURI provides several debugging tools:

1. **Heap Walking**
```c
void debug_heap_walk(void) {
    BlockLink_t* block = get_first_block();
    while(block) {
        FURI_LOG_D("Heap", 
            "Block: %p, Size: %d, Free: %d",
            block,
            (int)block->size,
            block->is_free);
        block = block->next;
    }
}
```

2. **Memory Validation**
```c
bool validate_heap(void) {
    // Check block integrity
    // Verify free list consistency
    // Validate guard patterns
    // Return true if heap is valid
}
```

3. **Usage Statistics**
```c
void print_memory_stats(void) {
    MemoryStats stats;
    get_memory_stats(&stats);
    // Print detailed statistics
}
```

## Next Steps

1. Experiment with the exercises
2. Create a memory monitoring system
3. Implement a custom allocator
4. Practice debugging memory issues
5. Learn about memory optimization techniques

The FURI memory management system provides robust tools for efficient memory usage. Understanding and following these patterns will help create reliable and efficient applications. 