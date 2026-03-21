import sys
import subprocess
import os.path

def has_grpcio_protoc():
    # type: () -> bool
    """ checks if grpcio-tools protoc is installed"""

    try:
        import grpc_tools.protoc
    except ImportError:
        return False
    return True


def invoke_protoc(argv):
    # type: (list) -> typing.Any
    """
    Invoke protoc.

    This routine will use grpcio-provided protoc if it exists,
    using system-installed protoc as a fallback.

    Args:
        argv: protoc CLI invocation, first item must be 'protoc'
    """

    # Add current directory to include path if nothing else is specified
    if not [x for x in argv if x.startswith('-I')]:
        argv.append("-I.")

    # Add default protoc include paths
    nanopb_include = os.path.dirname(os.path.abspath(__file__))
    argv.append('-I' + nanopb_include)

    if has_grpcio_protoc():
        import grpc_tools.protoc as protoc
        import pkg_resources
        proto_include = pkg_resources.resource_filename('grpc_tools', '_proto')
        argv.append('-I' + proto_include)

        return protoc.main(argv)
    else:
        return subprocess.call(argv)

def print_versions():
    try:
        if has_grpcio_protoc():
            import grpc_tools.protoc
            sys.stderr.write("Using grpcio-tools protoc from " + grpc_tools.protoc.__file__ + "\n")
        else:
            sys.stderr.write("Using protoc from system path\n")

        invoke_protoc(['protoc', '--version'])
    except Exception as e:
        sys.stderr.write("Failed to determine protoc version: " + str(e) + "\n")

    try:
        import google.protobuf
        sys.stderr.write("Python version " + sys.version + "\n")
        sys.stderr.write("Using python-protobuf from " + google.protobuf.__file__ + "\n")
        sys.stderr.write("Python-protobuf version: " + google.protobuf.__version__ + "\n")
    except Exception as e:
        sys.stderr.write("Failed to determine python-protobuf version: " + str(e) + "\n")

if __name__ == '__main__':
    print_versions()
