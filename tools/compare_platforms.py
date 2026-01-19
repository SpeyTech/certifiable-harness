#!/usr/bin/env python3
"""
Cross-platform bit-identity verification tool.

Usage: compare_platforms.py result1.json result2.json [result3.json ...]
"""

import json
import sys

def load_result(path):
    with open(path) as f:
        return json.load(f)

def compare_results(results):
    reference = results[0]
    ref_platform = reference['platform']
    
    print(f"Reference platform: {ref_platform}")
    print("Reference hashes:")
    for stage in reference['stages']:
        print(f"  {stage['name']}: {stage['hash'][:16]}...")
    print()
    
    all_match = True
    for result in results[1:]:
        platform = result['platform']
        print(f"Comparing {platform} against {ref_platform}:")
        
        for i, stage in enumerate(result['stages']):
            ref_hash = reference['stages'][i]['hash']
            cur_hash = stage['hash']
            
            if ref_hash == cur_hash:
                print(f"  ✓ {stage['name']}: MATCH")
            else:
                print(f"  ✗ {stage['name']}: DIFFERS")
                print(f"      Reference: {ref_hash}")
                print(f"      {platform}: {cur_hash}")
                all_match = False
        print()
    
    return all_match

def main():
    if len(sys.argv) < 3:
        print("Usage: compare_platforms.py result1.json result2.json [result3.json ...]")
        print()
        print("Compare harness results from multiple platforms for bit-identity.")
        sys.exit(1)
    
    results = [load_result(path) for path in sys.argv[1:]]
    platforms = [r['platform'] for r in results]
    
    print("═" * 60)
    print("  Cross-Platform Bit-Identity Verification")
    print(f"  Platforms: {', '.join(platforms)}")
    print("═" * 60)
    print()
    
    if compare_results(results):
        print("═" * 60)
        print("  RESULT: ALL PLATFORMS BIT-IDENTICAL ✓")
        print("═" * 60)
        sys.exit(0)
    else:
        print("═" * 60)
        print("  RESULT: PLATFORM DIVERGENCE DETECTED ✗")
        print("═" * 60)
        sys.exit(1)

if __name__ == "__main__":
    main()
