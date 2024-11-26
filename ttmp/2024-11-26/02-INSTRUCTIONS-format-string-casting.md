### Format String and Type Casting Standards

1. **Format Specifiers**

   - Use `%lu` for unsigned values with `(uint32_t)` cast
   - Use `%d` for signed values with `(int32_t)` cast

2. **Type Casting**

   - For unsigned values:
     - Add explicit `(uint32_t)` cast
     - Use with `%lu` format specifier
     - Example: `(uint32_t)duration`
   - For signed values:
     - Add explicit `(int32_t)` cast
     - Use with `%d` format specifier
     - Example: `(int32_t)signed_value`

3. **Arithmetic Operations**

   - When casting expressions with arithmetic, wrap the operation in parentheses
   - Example: `(uint32_t)(elapsed_time + 1)`
   - Example: `(int32_t)(signed_value - 1)`

4. **Example Transformations**

```cpp
// Before:
FURI_LOG_D(TAG, "Value: %u", some_value);
FURI_LOG_D(TAG, "Size: %zu", size);
FURI_LOG_D(TAG, "Unsigned: %u", unsigned_value);
FURI_LOG_D(TAG, "Signed: %d", signed_value);

// After:
FURI_LOG_D(TAG, "Value: %lu", (uint32_t)some_value);
FURI_LOG_D(TAG, "Size: %lu", (uint32_t)size);
FURI_LOG_D(TAG, "Unsigned: %lu", (uint32_t)unsigned_value);
FURI_LOG_D(TAG, "Signed: %d", (int32_t)signed_value);
```

5. **Common Patterns to Look For**
   - Log statements using `FURI_LOG_*`
   - Format strings containing `%lu`, `%zu`
   - Uncasted values with `%u` or `%d`
   - Arithmetic operations in log parameters
   - Size or length variables being logged

This standardization ensures consistent 32-bit integer handling across the codebase, making type conversions explicit and safe, while properly handling both signed and unsigned values using the correct fixed-width types.
