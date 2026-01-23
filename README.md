# Certifiable Harness

**End-to-end test harness for the certifiable-* deterministic ML pipeline.**

Proves bit-identity across platforms by running all seven pipeline stages and comparing cryptographic commitments.

[![Tests](https://img.shields.io/badge/tests-4%20passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-GPL--3.0-blue)]()
[![C Standard](https://img.shields.io/badge/C-C99-blue)]()

---

## The WOW Moment

```
Linux (GCC 12.2.0)                    macOS (Apple Clang)
──────────────────                    ───────────────────
data:      2f0c6228...        ═══     data:      2f0c6228...
training:  36b34d87...        ═══     training:  36b34d87...
quant:     8c78bae6...        ═══     quant:     8c78bae6...
deploy:    32296bbc...        ═══     deploy:    32296bbc...
inference: 48f4eceb...        ═══     inference: 48f4eceb...
monitor:   da7f4999...        ═══     monitor:   da7f4999...
verify:    33e41fca...        ═══     verify:    33e41fca...

                 Bit-identical: YES ✓
```

Different OS. Different compiler. **Same hashes.**

---

## Problem Statement

How do you prove that an ML model running on deployed hardware is *exactly* the same as what was tested and certified?

Traditional approaches fail:
- Floating-point arithmetic varies by platform
- Compiler optimizations change behaviour
- Hash-based verification only catches tampering, not drift

**certifiable-harness** solves this by:
1. Running all seven certifiable-* pipeline stages
2. Capturing a cryptographic commitment from each stage
3. Comparing commitments against a golden reference
4. Reporting exactly which stage (if any) diverged

---

## Pipeline Stages

| Stage | Project | Commitment |
|-------|---------|------------|
| 0 | [certifiable-data](https://github.com/SpeyTech/certifiable-data) | Merkle root of batches |
| 1 | [certifiable-training](https://github.com/SpeyTech/certifiable-training) | Training chain hash |
| 2 | [certifiable-quant](https://github.com/SpeyTech/certifiable-quant) | Quantization certificate |
| 3 | [certifiable-deploy](https://github.com/SpeyTech/certifiable-deploy) | Attestation root |
| 4 | [certifiable-inference](https://github.com/SpeyTech/certifiable-inference) | Predictions hash |
| 5 | [certifiable-monitor](https://github.com/SpeyTech/certifiable-monitor) | Ledger digest |
| 6 | [certifiable-verify](https://github.com/SpeyTech/certifiable-verify) | Report hash |

Each stage produces exactly one 32-byte SHA-256 commitment. If any stage produces a different hash on different hardware, the pipeline identifies exactly where determinism broke down.

---

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
make
ctest --output-on-failure
```

### Generate Golden Reference

```bash
./certifiable-harness --generate-golden --output result.json
```

This creates:
- `result.json` — Human-readable report
- `result.json.golden` — 368-byte binary for cross-platform comparison

### Verify on Another Platform

Copy the golden file to the target platform, then:

```bash
./certifiable-harness --golden result.json.golden --output their_result.json
```

If all hashes match: **Bit-identical: YES ✓**

---

## Golden Reference Format

The golden file is a 368-byte binary (CH-MATH-001 §4):

| Offset | Size | Field |
|--------|------|-------|
| 0x00 | 4 | Magic ("CHGR") |
| 0x04 | 4 | Version |
| 0x08 | 32 | Platform string |
| 0x28 | 8 | Timestamp |
| 0x30 | 32 | Config hash |
| 0x50 | 32 | Harness hash |
| 0x70 | 224 | Stage commitments (7 × 32) |
| 0x150 | 32 | File hash |

The file hash covers bytes 0x00–0x14F, ensuring integrity.

---

## CLI Reference

```
Usage: certifiable-harness [options]

Options:
  --data PATH        Path to test dataset (CSV)
  --policy PATH      Path to COE policy (JSON)
  --golden PATH      Path to golden reference (compare)
  --output PATH      Path for JSON report output
  --generate-golden  Generate golden reference
  --samples N        Number of samples (default: 1000)
  --batch-size N     Batch size (default: 32)
  --epochs N         Training epochs (default: 10)
  --verbose          Enable verbose output
  --help             Show this help

Examples:
  certifiable-harness --generate-golden --output result.json
  certifiable-harness --golden golden.bin --output result.json
```

---

## Cross-Platform Verification

Use the included Python tool to compare results from multiple platforms:

```bash
python3 tools/compare_platforms.py linux_result.json mac_result.json riscv_result.json
```

Output:
```
═══════════════════════════════════════════════════════════════
  Cross-Platform Bit-Identity Verification
  Platforms: x86_64, x86_64, riscv64
═══════════════════════════════════════════════════════════════

Reference platform: x86_64
Comparing x86_64 against x86_64:
  ✓ data: MATCH
  ✓ training: MATCH
  ...

═══════════════════════════════════════════════════════════════
  RESULT: ALL PLATFORMS BIT-IDENTICAL ✓
═══════════════════════════════════════════════════════════════
```

---

## Project Structure

```
certifiable-harness/
├── include/
│   ├── ch_types.h          Core types and constants
│   ├── ch_harness.h        Main harness API
│   ├── ch_golden.h         Golden reference I/O
│   ├── ch_report.h         Report generation
│   ├── ch_stages.h         Stage wrappers
│   └── ch_hash.h           Hash utilities
├── src/
│   ├── harness.c           Main orchestration
│   ├── golden.c            Golden load/save/compare
│   ├── report.c            JSON report generation
│   ├── stages.c            Stage implementations
│   ├── hash.c              SHA-256 utilities
│   └── main.c              CLI entry point
├── tests/unit/
│   ├── test_harness.c      Harness tests
│   ├── test_golden.c       Golden reference tests
│   ├── test_stages.c       Stage wrapper tests
│   └── test_report.c       Report generation tests
├── tools/
│   └── compare_platforms.py
└── docs/
    └── CH-MATH-001.md      Mathematical specification
```

---

## Documentation

| Document | Description |
|----------|-------------|
| [CH-MATH-001](docs/CH-MATH-001.md) | Mathematical specification |
| CH-STRUCT-001 | Data structure specification |
| SRS-HARNESS | Harness requirements |
| SRS-GOLDEN | Golden reference requirements |

---

## Wiring Up Real Libraries

By default, stages use deterministic stubs for testing. To wire up actual certifiable-* libraries:

1. Set compile flags in CMakeLists.txt:
```cmake
add_definitions(-DCH_LINK_CERTIFIABLE_DATA=1)
add_definitions(-DCH_LINK_CERTIFIABLE_TRAINING=1)
# ... etc
```

2. Link the libraries:
```cmake
target_link_libraries(certifiable-harness
    certifiable-data
    certifiable-training
    certifiable-quant
    certifiable-deploy
    certifiable-inference
    certifiable-monitor
    certifiable-verify
)
```

3. Rebuild and run

---

## Test Results

```
$ ctest --output-on-failure

Test project /home/william/certifiable-harness/build
    Start 1: test_harness
1/4 Test #1: test_harness .....................   Passed    0.00 sec
    Start 2: test_golden
2/4 Test #2: test_golden ......................   Passed    0.00 sec
    Start 3: test_stages
3/4 Test #3: test_stages ......................   Passed    0.00 sec
    Start 4: test_report
4/4 Test #4: test_report ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 4
```

---

## Verified Platforms

| Platform | OS | Compiler | Result |
|----------|-----|----------|--------|
| x86_64 | Linux (Ubuntu) | GCC 12.2.0 | ✓ Bit-identical |
| x86_64 | macOS 11.7 | Apple Clang | ✓ Bit-identical |
| aarch64 | — | — | Pending |
| riscv64 | — | — | Pending (Semper Victus) |

---

## Related Projects

| Project | Purpose |
|---------|---------|
| [certifiable-data](https://github.com/SpeyTech/certifiable-data) | Deterministic data pipeline |
| [certifiable-training](https://github.com/SpeyTech/certifiable-training) | Deterministic training |
| [certifiable-quant](https://github.com/SpeyTech/certifiable-quant) | Model quantization |
| [certifiable-deploy](https://github.com/SpeyTech/certifiable-deploy) | Bundle packaging |
| [certifiable-inference](https://github.com/SpeyTech/certifiable-inference) | Fixed-point inference |
| [certifiable-monitor](https://github.com/SpeyTech/certifiable-monitor) | Runtime monitoring |
| [certifiable-verify](https://github.com/SpeyTech/certifiable-verify) | Pipeline verification |

---

## License

**Dual License:**

- **Open Source:** GPL-3.0 — Free for open source projects
- **Commercial:** Contact william@fstopify.com for commercial licensing

**Patent:** UK Patent Application GB2521625.0

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

All contributions require a signed Contributor License Agreement.

---

## Author

**William Murray**  
The Murray Family Innovation Trust  
[SpeyTech](https://speytech.com) · [GitHub](https://github.com/SpeyTech)

---

*Built for safety. Designed for certification. Proven through testing.*

Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
