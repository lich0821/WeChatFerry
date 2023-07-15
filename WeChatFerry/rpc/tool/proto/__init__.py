'''This file dynamically builds the proto definitions for Python.'''
from __future__ import absolute_import

import os
import os.path
import sys
import tempfile
import shutil
import traceback
import pkg_resources
from ._utils import has_grpcio_protoc, invoke_protoc, print_versions

# Compatibility layer to make TemporaryDirectory() available on Python 2.
try:
    from tempfile import TemporaryDirectory
except ImportError:
    class TemporaryDirectory:
        '''TemporaryDirectory fallback for Python 2'''
        def __init__(self, prefix = 'tmp', dir = None):
            self.prefix = prefix
            self.dir = dir

        def __enter__(self):
            self.dir = tempfile.mkdtemp(prefix = self.prefix, dir = self.dir)
            return self.dir

        def __exit__(self, *args):
            shutil.rmtree(self.dir)

def build_nanopb_proto(protosrc, dirname):
    '''Try to build a .proto file for python-protobuf.
    Returns True if successful.
    '''

    cmd = [
        "protoc",
        "--python_out={}".format(dirname),
        protosrc,
        "-I={}".format(dirname),
    ]

    if has_grpcio_protoc():
        # grpcio-tools has an extra CLI argument
        # from grpc.tools.protoc __main__ invocation.
        _builtin_proto_include = pkg_resources.resource_filename('grpc_tools', '_proto')
        cmd.append("-I={}".format(_builtin_proto_include))

    try:
        invoke_protoc(argv=cmd)
    except:
        sys.stderr.write("Failed to build nanopb_pb2.py: " + ' '.join(cmd) + "\n")
        sys.stderr.write(traceback.format_exc() + "\n")
        return False

    return True

def load_nanopb_pb2():
    # To work, the generator needs python-protobuf built version of nanopb.proto.
    # There are three methods to provide this:
    #
    # 1) Load a previously generated generator/proto/nanopb_pb2.py
    # 2) Use protoc to build it and store it permanently generator/proto/nanopb_pb2.py
    # 3) Use protoc to build it, but store only temporarily in system-wide temp folder
    #
    # By default these are tried in numeric order.
    # If NANOPB_PB2_TEMP_DIR environment variable is defined, the 2) is skipped.
    # If the value of the $NANOPB_PB2_TEMP_DIR exists as a directory, it is used instead
    # of system temp folder.

    build_error = None
    proto_ok = False
    tmpdir = os.getenv("NANOPB_PB2_TEMP_DIR")
    temporary_only = (tmpdir is not None)
    dirname = os.path.dirname(__file__)
    protosrc = os.path.join(dirname, "nanopb.proto")
    protodst = os.path.join(dirname, "nanopb_pb2.py")
    proto_ok = False

    if tmpdir is not None and not os.path.isdir(tmpdir):
        tmpdir = None # Use system-wide temp dir

    if os.path.isfile(protosrc):
        src_date = os.path.getmtime(protosrc)
        if not os.path.isfile(protodst) or os.path.getmtime(protodst) < src_date:
            # Outdated, rebuild
            proto_ok = False
        else:
            try:
                from . import nanopb_pb2 as nanopb_pb2_mod
                proto_ok = True
            except Exception as e:
                sys.stderr.write("Failed to import nanopb_pb2.py: " + str(e) + "\n"
                                "Will automatically attempt to rebuild this.\n"
                                "Verify that python-protobuf and protoc versions match.\n")
                print_versions()

    # Try to rebuild into generator/proto directory
    if not proto_ok and not temporary_only:
        proto_ok = build_nanopb_proto(protosrc, dirname)

        try:
            from . import nanopb_pb2 as nanopb_pb2_mod
        except:
            sys.stderr.write("Failed to import generator/proto/nanopb_pb2.py:\n")
            sys.stderr.write(traceback.format_exc() + "\n")

    # Try to rebuild into temporary directory
    if not proto_ok:
        with TemporaryDirectory(prefix = 'nanopb-', dir = tmpdir) as protodir:
            proto_ok = build_nanopb_proto(protosrc, protodir)

            if protodir not in sys.path:
                sys.path.insert(0, protodir)

            try:
                import nanopb_pb2 as nanopb_pb2_mod
            except:
                sys.stderr.write("Failed to import %s/nanopb_pb2.py:\n" % protodir)
                sys.stderr.write(traceback.format_exc() + "\n")

    # If everything fails
    if not proto_ok:
        sys.stderr.write("\n\nGenerating nanopb_pb2.py failed.\n")
        sys.stderr.write("Make sure that a protoc generator is available and matches python-protobuf version.\n")
        print_versions()
        sys.exit(1)

    return nanopb_pb2_mod
