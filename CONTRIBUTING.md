# Contributing to Certifiable Harness

Thank you for your interest in contributing to certifiable-harness. This project is part of the Murray Deterministic Computing Platform (MDCP), designed for safety-critical ML systems.

## Contributor License Agreement

**All contributions require a signed CLA** before we can merge your changes.

Please read and sign the [Contributor License Agreement](CONTRIBUTOR-LICENSE-AGREEMENT.md) before submitting a pull request.

## Code Standards

### Language Requirements

- **C99** — No C11 or later features
- **No dynamic allocation** — All buffers statically allocated or caller-provided
- **No floating-point** — Fixed-point Q16.16 only
- **Bounded loops** — All loops must have provable termination
- **Full traceability** — Every function must trace to a specification

### Coding Style

```c
/**
 * @brief Brief description of function.
 * @param ctx Context pointer (must not be NULL)
 * @param value Input value in Q16.16
 * @param faults Fault flag accumulator
 * @return CH_OK on success, error code otherwise
 * @traceability CH-MATH-001 §X.Y
 */
ch_result_t ch_function_name(ch_context_t *ctx,
                              int32_t value,
                              ch_fault_flags_t *faults)
{
    if (!ctx || !faults) {
        return CH_ERR_NULL;
    }
    
    /* Implementation */
    
    return CH_OK;
}
```

### Key Principles

1. **NULL checks first** — Every pointer parameter checked at entry
2. **Fault propagation** — All operations that can fail must signal via fault flags
3. **Determinism** — Same inputs must produce same outputs across platforms
4. **No side effects** — Functions should not modify global state
5. **Constant-time where needed** — Hash comparisons must be constant-time

### Prohibited Constructs

- `malloc()`, `calloc()`, `realloc()`, `free()`
- `float`, `double`, or any floating-point operations
- `goto` (except for cleanup patterns)
- Variable-length arrays (VLAs)
- Recursion without bounded depth proof
- Platform-specific headers (except behind `#ifdef`)

## Commit Messages

Use the conventional commit format:

```
type(scope): brief description

Detailed explanation if needed.

Traceability: CH-MATH-001 §X.Y
```

**Types:**
- `feat` — New feature
- `fix` — Bug fix
- `docs` — Documentation only
- `test` — Adding or updating tests
- `refactor` — Code change that neither fixes nor adds
- `chore` — Build, CI, or tooling changes

**Examples:**
```
feat(golden): add cross-platform comparison tool

Implements Python script for comparing results from multiple platforms.
Reports first divergence point if hashes differ.

Traceability: CH-MATH-001 §5.2

fix(stages): correct hash chaining in training stage

Previous implementation did not include data commitment in training hash,
breaking the provenance chain.

Traceability: CH-MATH-001 §3.2
```

## Testing Requirements

### All Changes Must Pass

```bash
cd build
make clean && make
ctest --output-on-failure
```

### New Features Require Tests

Add tests to the appropriate test file in `tests/unit/`:

```c
static int test_new_feature(void)
{
    /* Setup */
    ch_config_t config = ch_config_default();
    
    /* Execute */
    ch_result_t rc = ch_new_function(&config, ...);
    
    /* Verify */
    ASSERT(rc == CH_OK);
    ASSERT(/* expected condition */);
    
    return 1;
}
```

### Cross-Platform Verification

For changes affecting determinism, test on at least two platforms:

1. Run on first platform, generate golden
2. Copy golden to second platform
3. Verify `Bit-identical: YES`

## Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feat/my-feature`)
3. **Implement** changes following code standards
4. **Test** thoroughly on your platform
5. **Commit** with proper message format
6. **Push** to your fork
7. **Open** pull request with:
   - Clear description of changes
   - Traceability to specification
   - Test results
   - Platforms tested

## Areas for Contribution

### High Priority

- **Cross-platform testing** — Test on ARM64, RISC-V, or other architectures
- **Full replay mode** — Implement verification with actual data replay
- **Harness self-hash** — Implement `/proc/self/exe` hashing

### Documentation

- Improve code comments
- Add examples to documentation
- Translate docs to other languages

### Testing

- Additional edge case tests
- Fuzz testing
- Performance benchmarks

## Questions?

Open an issue for:
- Clarification on requirements
- Discussion of implementation approaches
- Bug reports
- Feature requests

---

*Thank you for helping make certifiable-harness better!*

Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
