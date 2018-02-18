from __future__ import print_function

import os
import re
import subprocess
import sys


fname_regex = re.compile("^test_(\w+).lox$")


def gather_files(test_regex):
    """Collect files with name test_*.lox in test directory"""

    directory = os.path.dirname(os.path.abspath(__file__))

    return [os.path.join(directory, fname)
            for fname in os.listdir(directory)
            if fname_regex.findall(fname) and re.findall(test_regex, fname)]


def parse_test(test_path):
    """Parse test, returning return code and expected output"""

    with open(test_path) as f:
        lines = f.readlines()

    assert len(lines) >= 2

    output = re.findall("^//(.+)$", lines[0])[0].strip()
    rvalue = int(re.findall("^// *(\d+)$", lines[1])[0])

    return output, rvalue


def run_tests(interpreter_path, test_paths):

    success_counter = 0

    for test_path in test_paths:

        try:
            expected_output, expected_rvalue = parse_test(test_path)
        except (AssertionError, IndexError, ValueError):
            print("Unable to parse test file \"{}\". Skipping..."
                  .format(test_path))
            continue

        process = subprocess.Popen([interpreter_path, test_path],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)

        actual_output = "".join(process.communicate()).strip()
        actual_rvalue = process.returncode

        test_fname = os.path.basename(test_path)
        test_name = fname_regex.findall(test_fname)[0]

        if (actual_rvalue == expected_rvalue and
                actual_output == expected_output):
            print("Test \"{}\" passed!".format(test_name))
            success_counter += 1
        else:
            print()
            print("Test \"{}\" failed!".format(test_name))
            print("-- Expected output:")
            print("---- {}".format(expected_output))
            print("-- Actual output:")
            print("---- {}".format(actual_output))
            print("-- Expected rvalue:")
            print("---- {}".format(expected_rvalue))
            print("-- Actual rvalue:")
            print("---- {}".format(actual_rvalue))
            print()

    print()
    print("Ran {} tests, of which {} failed."
          .format(len(test_paths), len(test_paths) - success_counter))


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

    run_tests(exec_path, gather_files(test_regex))