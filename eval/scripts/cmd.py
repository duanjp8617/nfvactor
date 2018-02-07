import subprocess
import sys

def cmd(cmd):
    proc = subprocess.Popen(cmd,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            shell=True)

    # err should be None
    out, err = proc.communicate()

    if proc.returncode:
        print >> sys.stderr, 'Log:\n', out
        print >> sys.stderr, 'Error has occured running command: %s' % cmd
        sys.exit(proc.returncode)


def cmd_success(cmd):
    try:
        subprocess.check_call(cmd,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT,
                              shell=True)
        return True
    except subprocess.CalledProcessError:
        return False

def cmd_with_output(cmd):
    proc = subprocess.Popen(cmd,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            shell=True)

    # err should be None
    out, err = proc.communicate()

    if proc.returncode:
        print >> sys.stderr, 'Log:\n', out
        print >> sys.stderr, 'Error has occured running command: %s' % cmd
        sys.exit(proc.returncode)

    return out
