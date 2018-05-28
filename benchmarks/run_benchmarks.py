from __future__ import division, print_function

import argparse
import os
import re
import subprocess
import time

import numpy as np


def gather_files(bench_regex, exclude=False):
    """Collect files with name *.lox in benchmarks directory"""

    directory = os.path.dirname(os.path.abspath(__file__))
    filepaths = [os.path.join(dirpath, fname)
                 for dirpath, dirnames, fnames in os.walk(directory)
                 for fname in fnames if fname.endswith(".lox")]

    modifier = lambda r: (not r if exclude else r)

    return [os.path.join(directory, path)
            for path in filepaths
            if modifier(re.findall(bench_regex, path))]


def run_benchmarks(interpreter_path, bench_paths, num_iters=100, verbose=False):
    """Run the specified interpreter over the provided benchmark source files,
    reporting errors as necessary."""

    directory = os.path.dirname(os.path.abspath(__file__))
    bench_paths = sorted(bench_paths,
                         key=lambda p: os.path.relpath(p, directory))

    stats = {}

    for bench_path in bench_paths:

        benchmark_name = os.path.relpath(bench_path, directory)
        print("Running benchmark file '{}'...".format(benchmark_name))
        durations = np.zeros(num_iters)

        for i in range(num_iters):
            if verbose:
                print("  - Iteration {} of {}:".format(i + 1, num_iters),
                      end='', flush=True)

            start = time.time()
            process = subprocess.Popen([interpreter_path, bench_path],
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)
            process.communicate()
            durations[i] = time.time() - start

            if verbose:
                print(" {:>4.3f} s".format(durations[i]))

        print("Done!")

        bench_stats = (
            ("Min.", np.min(durations)),
            ("Mean", np.mean(durations)),
            ("Med.", np.median(durations)),
            ("Max.", np.max(durations)),
            ("Std.", np.std(durations)))

        stats[benchmark_name] = bench_stats

    return stats


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Run benchmarks tests for loxx.")
    parser.add_argument("interpreter", help="Path to interpreter to bench.")
    parser.add_argument("bench_regex", nargs="?", default="",
                        help="Regex to filter tests with")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Print iteration numbers when benchmarking.")
    parser.add_argument("-x", "--exclude", action="store_true",
                        help="Exclude tests matching regex.")
    parser.add_argument("-n", "--num-iters", type=int, default=100,
                        help="Number of iterations to run each benchmark for.")
    args = parser.parse_args()

    files = gather_files(args.bench_regex, args.exclude)
    results = run_benchmarks(args.interpreter, files,
                             args.num_iters, args.verbose)

    for name, stats in results.items():
        print("Results for benchmark '{}':".format(name))

        for label, stat in stats:
            print("* {:<20} = {:>4.3f} s".format(label, stat))
