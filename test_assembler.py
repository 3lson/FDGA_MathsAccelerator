import os
import subprocess
import filecmp
from pathlib import Path

# --- Configuration ---
# Get the absolute path of the directory this script is in.
SCRIPT_DIR = Path(__file__).parent.resolve()

# Define all other paths relative to the script's location.
ASSEMBLER_EXEC = SCRIPT_DIR / "assembler" / "assembler"
TEST_DIR = SCRIPT_DIR / "assembler" / "tests" / "asm_files"
EXPECTED_DIR = SCRIPT_DIR / "assembler" / "tests" / "expected_output"
TEMP_DIR = SCRIPT_DIR / "assembler" / "tests" / "temp_output"

def run_tests():
    """Finds all .asm tests, runs them through the assembler, and compares the output."""

    # Ensure the assembler executable exists and is runnable
    if not (ASSEMBLER_EXEC.exists() and os.access(ASSEMBLER_EXEC, os.X_OK)):
        print(f"[FAIL] Assembler executable not found or not executable at: {ASSEMBLER_EXEC}")
        print("       Please compile assembler.cpp first (e.g., g++ -std=c++17 -o assembler/assembler assembler/assembler.cpp)")
        return False

    # Create a temporary directory for the actual output
    TEMP_DIR.mkdir(exist_ok=True)
    print(f"Using temporary output directory: {TEMP_DIR}\n")

    # Find all test assembly files
    test_files = sorted(list(TEST_DIR.glob("*.asm"))) # Sorted for consistent order
    if not test_files:
        print(f"[WARN] No .asm files found in {TEST_DIR}")
        return True

    passed_count = 0
    failed_count = 0

    for asm_path in test_files:
        base_name = asm_path.stem
        print("-" * 50)
        print(f"Running test: {base_name}")

        # Define all expected and actual file paths using absolute paths
        expected_instr_path = EXPECTED_DIR / f"{base_name}.instr.hex"
        expected_data_path = EXPECTED_DIR / f"{base_name}.data.hex"
        actual_instr_path = TEMP_DIR / f"{base_name}.instr.hex"
        actual_data_path = TEMP_DIR / f"{base_name}.data.hex"

        if not expected_instr_path.exists():
            print(f"[SKIP] Missing expected instruction file: {expected_instr_path}")
            failed_count += 1
            continue

        # --- Run the Assembler ---
        command = [
            str(ASSEMBLER_EXEC),
            str(asm_path),
            str(actual_instr_path),
            str(actual_data_path),
        ]
        
        # --- NEW: Added more detailed printing for debugging ---
        print(f"  > Executing: {' '.join(command)}")
        result = subprocess.run(command, capture_output=True, text=True)

        # Print assembler output regardless of exit code to see what happened
        if result.stdout:
            print(f"  > Assembler STDOUT:\n{result.stdout.strip()}")
        if result.stderr:
            print(f"  > Assembler STDERR:\n{result.stderr.strip()}")

        if result.returncode != 0:
            print(f"[FAIL] Assembler exited with a non-zero error code ({result.returncode}).")
            failed_count += 1
            continue
        
        # --- NEW: Check if output files were actually created ---
        if not actual_instr_path.exists():
            print(f"[FAIL] Assembler ran successfully but did NOT create the instruction output file.")
            failed_count += 1
            continue

        # --- Compare the Output Files ---
        # Data file comparison is optional, as some tests might not have a .data section
        instr_match = filecmp.cmp(expected_instr_path, actual_instr_path, shallow=False)
        data_match = True 
        if expected_data_path.exists():
            if actual_data_path.exists():
                data_match = filecmp.cmp(expected_data_path, actual_data_path, shallow=False)
            else:
                data_match = False # Expected a data file, but none was created.

        if instr_match and data_match:
            print(f"[PASS] {base_name} - All output files match.")
            passed_count += 1
        else:
            failed_count += 1
            if not instr_match:
                print(f"[FAIL] {base_name} - Instruction hex output does NOT match expected.")
            if not data_match:
                print(f"[FAIL] {base_name} - Data hex output does NOT match expected (or was not created).")
    
    # Final Summary
    print("\n" + "=" * 50)
    print("Test Summary:")
    print(f"  PASSED: {passed_count}")
    print(f"  FAILED: {failed_count}")
    print("=" * 50)

    return failed_count == 0

if __name__ == "__main__":
    if run_tests():
        print("\nAll tests passed successfully!")
        exit(0)
    else:
        print("\nSome tests failed.")
        exit(1)