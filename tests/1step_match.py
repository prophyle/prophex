#! /usr/bin/env python3

"""Single-command ProPhex wrapper.

Author: Karel Brinda <kbrinda@hsph.harvard.edu>

Licence: MIT
"""

import argparse
import sys
import os
import datetime
import subprocess
import time

log_file = None

sys.path.append(os.path.dirname(__file__))

script_dir = os.path.dirname(os.path.realpath(__file__))
bwa = os.path.join(script_dir, "../src/", "bwa", "bwa")
prophyle_index = os.path.join(script_dir, "../", "prophex")

def message(*msg, subprogram='', upper=False, only_log=False):
    """Print a ProPhyle message to stderr.

    Args:
        *msg: Message.
        subprogram (str): Subprogram.
        upper (bool): Transform text to upper cases.
        only_log (bool): Don't print the message to screen (i.e., it would be only logged).
    """

    global log_file

    dt = datetime.datetime.now()
    fdt = dt.strftime("%Y-%m-%d %H:%M:%S")

    if upper:
        msg = map(str, msg)
        msg = map(str.upper, msg)

    log_line = '[prophyle{}] {} {}'.format(subprogram, fdt, " ".join(msg))

    if not only_log:
        print(log_line, file=sys.stderr)
    if log_file is not None:
        log_file.write(log_line)
        log_file.write("\n")
        log_file.flush()

def run_safe(command, output_fn=None, output_fo=None, err_msg=None, thr_exc=True, silent=False):
    """Run a shell command safely.

    Args:
        command (list of str): Command to execute.
        output_fn (str): Name of a file for storing the output.
        output_fo (fileobject): Output file object. If both params are None, the standard output is used.
        err_msg (str): Error message if the command fails.
        thr_exc (bool): Through exception if the command fails. error_msg or thr_exc must be set.
        silent (bool): Silent mode (print messages only if the command fails).

    Raises:
        RuntimeError: Command exited with a non-zero code.
    """

    assert output_fn is None or output_fo is None
    assert err_msg is not None or thr_exc
    assert len(command) > 0

    command_safe = []

    for part in command:
        part = str(part)
        if " " in part:
            part = '"{}"'.format(part)
        command_safe.append(part)

    command_str = " ".join(command_safe)

    if not silent:
        message("Shell command:", command_str)

    if output_fn is None:
        if output_fo is None:
            out_fo = sys.stdout
        else:
            out_fo = output_fo
    else:
        out_fo = open(output_fn, "w+")

    if out_fo == sys.stdout:
        p = subprocess.Popen("/bin/bash -e -o pipefail -c '{}'".format(command_str), shell=True)
    else:
        p = subprocess.Popen("/bin/bash -e -o pipefail -c '{}'".format(command_str), shell=True, stdout=out_fo)

    stdout, stderr = p.communicate()
    error_code = p.returncode

    if output_fn is not None:
        out_fo.close()

    if error_code == 0 or error_code == 141:
        if not silent:
            message("Finished")
    else:
        message("Unfinished, an error occurred (error code {}): {}".format(error_code, command_str))

        if err_msg is not None:
            print('Error: {}'.format(err_msg), file=sys.stderr)

        if thr_exc:
            raise RuntimeError("A command failed, see messages above.")

        sys.exit(1)

def create_bwa_index(fa):
    # cmd('"{bwa}" index "{fa}"'.format(bwa=bwa,fa=fa))
    run_safe([bwa, 'fa2pac', fa, fa])
    run_safe([bwa, 'pac2bwtgen', fa + ".pac", fa + ".bwt", ">", "/dev/null"])
    run_safe([bwa, 'bwtupdate', fa + ".bwt"])
    run_safe([bwa, 'bwt2sa', fa + ".bwt", fa + ".sa"])


def create_klcp(fa, k):
    run_safe([prophyle_index, 'build', '-k', k, fa, ">", "/dev/null"])


def query(fa, fq, k, u=False, v=False, t=1):
    params = ""
    cmd = [prophyle_index, 'query', "-v" if v else "", "-u" if u else "", '-k', k, '-t', t, fa, fq]
    run_safe(cmd)


def main():
    parser = argparse.ArgumentParser(description='Single-command ProPhex matching.')
    parser.add_argument(
        '-k',
        type=int,
        metavar='int',
        dest='k',
        required=True,
        help='k-mer length',
    )
    parser.add_argument(
        '-t',
        type=int,
        default=1,
        metavar='int',
        dest='t',
        required=False,
        help='number of threads',
    )
    parser.add_argument(
        '-v',
        action='store_true',
        help='verbose output format',
    )
    parser.add_argument(
        '-u',
        action='store_true',
        help='use rolling window',
    )
    parser.add_argument(
        'in_fasta',
        type=str,
        help='Input FASTA reference.',
    )
    parser.add_argument(
        'in_fq',
        type=str,
        help='Reads to be matched.',
    )

    args = parser.parse_args()

    fa = args.in_fasta
    fq = args.in_fq
    k = args.k
    u = args.u
    v = args.v
    t = args.t

    create_bwa_index(fa)

    if u:
        create_klcp(fa, k)

    query(fa, fq, k, u=u, v=v, t=t)


if __name__ == "__main__":
    main()
