/**
 * @file main.c
 * @brief Certifiable Harness entry point
 * @traceability CH-STRUCT-001
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_harness.h"
#include "ch_golden.h"
#include "ch_report.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  --data PATH        Path to test dataset (CSV)\n");
    printf("  --policy PATH      Path to COE policy (JSON)\n");
    printf("  --golden PATH      Path to golden reference (compare)\n");
    printf("  --output PATH      Path for JSON report output\n");
    printf("  --generate-golden  Generate golden reference\n");
    printf("  --samples N        Number of samples (default: 1000)\n");
    printf("  --batch-size N     Batch size (default: 32)\n");
    printf("  --epochs N         Training epochs (default: 10)\n");
    printf("  --verbose          Enable verbose output\n");
    printf("  --help             Show this help\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s --data mnist.csv --generate-golden --output result.json\n", prog);
    printf("  %s --data mnist.csv --golden golden.bin --output result.json\n", prog);
    printf("\n");
}

int main(int argc, char **argv)
{
    ch_config_t config = ch_config_default();

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            config.data_path = argv[++i];
        }
        else if (strcmp(argv[i], "--policy") == 0 && i + 1 < argc) {
            config.policy_path = argv[++i];
        }
        else if (strcmp(argv[i], "--golden") == 0 && i + 1 < argc) {
            config.golden_path = argv[++i];
        }
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            config.output_path = argv[++i];
        }
        else if (strcmp(argv[i], "--generate-golden") == 0) {
            config.generate_golden = true;
        }
        else if (strcmp(argv[i], "--samples") == 0 && i + 1 < argc) {
            config.num_samples = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--batch-size") == 0 && i + 1 < argc) {
            config.batch_size = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc) {
            config.epochs = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--verbose") == 0) {
            config.verbose = true;
        }
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Run harness */
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  Certifiable Harness v%s\n", CH_VERSION_STRING);
    printf("  Platform: %s\n", ch_get_platform());
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");

    ch_result_t rc = ch_harness_run(&config, &result, &faults);

    if (rc != CH_OK) {
        fprintf(stderr, "Harness failed: %s\n", ch_result_string(rc));
        return 1;
    }

    /* Load or generate golden */
    ch_golden_t golden;
    ch_golden_t *golden_ptr = NULL;

    if (config.golden_path && !config.generate_golden) {
        rc = ch_golden_load(config.golden_path, &golden, &faults);
        if (rc == CH_OK) {
            golden_ptr = &golden;

            bool bit_identical;
            int first_mismatch;
            ch_golden_compare(&result, &golden, &bit_identical, &first_mismatch, &faults);
            result.bit_identical = bit_identical;
        } else {
            fprintf(stderr, "Warning: Could not load golden reference: %s\n",
                    ch_result_string(rc));
        }
    }

    /* Print summary */
    ch_report_print_summary(&result, golden_ptr);

    /* Generate golden if requested */
    if (config.generate_golden && config.output_path) {
        ch_golden_generate(&result, &config, &golden, &faults);

        char golden_path[256];
        snprintf(golden_path, sizeof(golden_path), "%s.golden", config.output_path);

        rc = ch_golden_save(&golden, golden_path, &faults);
        if (rc == CH_OK) {
            printf("Golden reference saved: %s\n", golden_path);
        }
    }

    /* Write JSON report */
    if (config.output_path) {
        rc = ch_report_write_json(&result, golden_ptr, config.output_path, &faults);
        if (rc == CH_OK) {
            printf("Report saved: %s\n", config.output_path);
        }
    }

    return result.all_passed ? 0 : 1;
}
