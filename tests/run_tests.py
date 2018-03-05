from __future__ import print_function

import os
import re
import subprocess
import sys


def gather_files(test_regex):
    """Collect files with name test_*.lox in test directory"""

    directory = os.path.dirname(os.path.abspath(__file__))
    filepaths = [os.path.join(dirpath, fname)
                 for dirpath, dirnames, fnames in os.walk(directory)
                 for fname in fnames if fname.endswith(".lox")]
    
    return [os.path.join(directory, path)
            for path in filepaths
            if re.findall(test_regex, path)]


def parse_test(test_path):
    """Parse test, returning return code and expected output"""

    with open(test_path) as f:
        lines = f.readlines()

    assert len(lines) >= 1

    header_length = 0

    for i, line in enumerate(lines):
        if line[:2] != "//":
            break
        header_length += 1

    output_lines = [re.findall("^//(.+)$", line)[0].strip()
                    for line in lines[:header_length - 1]]
    rvalue = int(re.findall("^// *(\d+)$", lines[header_length - 1])[0])

    return output_lines, rvalue


def print_failed_test(test_name, expected_data, actual_data):
    """Prints a failed test result to stdout"""

    expected_output, expected_retval = expected_data
    actual_output, actual_retval = actual_data
    
    red = "\033[31m" if sys.platform != "win32" else ""
    white = "\033[0m" if sys.platform != "win32" else ""
    
    print()
    print("Test {:.<50}{} FAILED{}"
          .format(test_name, red, white))
    print("-- Expected output:")
    
    for line in expected_output:
        print("    {}".format(line))
        
    print("-- Actual output:")
        
    for line in actual_output:
        print("    {}".format(line))

    print("-- Expected return value:")
    print("    {}".format(expected_retval))
    print("-- Actual return value:")
    print("    {}".format(actual_retval))
    print()


def print_succeeded_test(test_name):
    """Prints the a successful test result to stdout."""

    green = "\033[32m" if sys.platform != "win32" else ""
    white = "\033[0m" if sys.platform != "win32" else ""

    print("Test {:.<50}{} PASSED{}"
          .format(test_name, green, white))


def run_tests(interpreter_path, test_paths):
    """Run the specified interpreter over the provided test source files,
    reporting errors as necessary."""

    fail_counter = 0
    test_counter = 0

    directory = os.path.dirname(os.path.abspath(__file__))
    test_paths = sorted(test_paths, key=lambda p: os.path.relpath(p, directory))

    for test_path in test_paths:

        try:
            expected_output, expected_retval = parse_test(test_path)
        except (AssertionError, IndexError, ValueError):
            print("Unable to parse test file \"{}\". Skipping..."
                  .format(test_path))
            continue
        test_counter += 1

        process = subprocess.Popen([interpreter_path, test_path],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)

        actual_output = [
            line.strip()
            for line in "".join(process.communicate()).split('\n')
            if line.strip()]
        actual_retval = process.returncode

        test_name = os.path.relpath(test_path, directory)

        if (actual_retval == expected_retval and
                actual_output == expected_output):
            print_succeeded_test(test_name)
        else:
            print_failed_test(test_name,
                              (expected_output, expected_retval),
                              (actual_output, actual_retval))
            fail_counter += 1

    print()
    print("Ran {} tests, of which {} failed."
          .format(test_counter, fail_counter))

    return fail_counter


if __name__ == "__main__":

    try:
        exec_path = sys.argv[1]
    except IndexError:
        print("Usage: python {} <interpreter_path> [<test regex>]"
              .format(sys.argv[0]))
        sys.exit(1)

    try:
        test_regex = sys.argv[2]
    except IndexError:
        test_regex = ""

    num_failed_tests = run_tests(exec_path, gather_files(test_regex))
    sys.exit(num_failed_tests)
