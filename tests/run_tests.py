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

    header_length = 0

    while lines[header_length][:2] == "//":
        header_length += 1

    output_lines = [re.findall("^//(.+)$", line)[0].strip()
                    for line in lines[:header_length - 1]]
    rvalue = int(re.findall("^// *(\d+)$", lines[header_length - 1])[0])

    return output_lines, rvalue


def run_tests(interpreter_path, test_paths):

    success_counter = 0

    test_paths = sorted(test_paths, key=lambda p: os.path.basename(p))

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

        actual_output = [line.strip()
                         for line in "".join(process.communicate()).split('\n')
                         if line.strip()]
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

            for line in expected_output:
                print("---- {}".format(line))

            print("-- Actual output:")

            for line in actual_output:
                print("---- {}".format(line))

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