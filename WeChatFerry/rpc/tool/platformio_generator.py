import os
import hashlib
import pathlib
import shlex
import subprocess

import SCons.Action
from platformio import fs

Import("env")

# We don't use `env.Execute` because it does not handle spaces in path
# See https://github.com/nanopb/nanopb/pull/834
# So, we resolve the path to the executable and then use `subprocess.run`
python_exe = env.subst("$PYTHONEXE")

try:
    import protobuf
except ImportError:
    print("[nanopb] Installing Protocol Buffers dependencies");

    # We need to specify protobuf version. In other case got next (on Ubuntu 20.04):
    # Requirement already satisfied: protobuf in /usr/lib/python3/dist-packages (3.6.1)
    subprocess.run([python_exe, '-m', 'pip', 'install', "protobuf>=3.19.1"])

try:
    import grpc_tools.protoc
except ImportError:
    print("[nanopb] Installing gRPC dependencies");
    subprocess.run([python_exe, '-m', 'pip', 'install', "grpcio-tools>=1.43.0"])


nanopb_root = os.path.join(os.getcwd(), '..')

project_dir = env.subst("$PROJECT_DIR")
build_dir = env.subst("$BUILD_DIR")

generated_src_dir = os.path.join(build_dir, 'nanopb', 'generated-src')
generated_build_dir = os.path.join(build_dir, 'nanopb', 'generated-build')
md5_dir = os.path.join(build_dir, 'nanopb', 'md5')

nanopb_protos = env.GetProjectOption("custom_nanopb_protos", "")
nanopb_plugin_options = env.GetProjectOption("custom_nanopb_options", "")

if not nanopb_protos:
    print("[nanopb] No generation needed.")
else:
    if isinstance(nanopb_plugin_options, (list, tuple)):
        nanopb_plugin_options = " ".join(nanopb_plugin_options)

    nanopb_plugin_options = shlex.split(nanopb_plugin_options)

    protos_files = fs.match_src_files(project_dir, nanopb_protos)
    if not len(protos_files):
        print("[nanopb] ERROR: No files matched pattern:")
        print(f"custom_nanopb_protos: {nanopb_protos}")
        exit(1)

    nanopb_generator = os.path.join(nanopb_root, 'generator', 'nanopb_generator.py')

    nanopb_options = []
    nanopb_options.extend(["--output-dir", generated_src_dir])
    for opt in nanopb_plugin_options:
        nanopb_options.append(opt)

    try:
        os.makedirs(generated_src_dir)
    except FileExistsError:
        pass

    try:
        os.makedirs(md5_dir)
    except FileExistsError:
        pass

    # Collect include dirs based on
    proto_include_dirs = set()
    for proto_file in protos_files:
        proto_file_abs = os.path.join(project_dir, proto_file)
        proto_dir = os.path.dirname(proto_file_abs)
        proto_include_dirs.add(proto_dir)

    for proto_include_dir in proto_include_dirs:
        nanopb_options.extend(["--proto-path", proto_include_dir])

    for proto_file in protos_files:
        proto_file_abs = os.path.join(project_dir, proto_file)

        proto_file_path_abs = os.path.dirname(proto_file_abs)
        proto_file_basename = os.path.basename(proto_file_abs)
        proto_file_without_ext = os.path.splitext(proto_file_basename)[0]

        proto_file_md5_abs = os.path.join(md5_dir, proto_file_basename + '.md5')
        proto_file_current_md5 = hashlib.md5(pathlib.Path(proto_file_abs).read_bytes()).hexdigest()

        options_file = proto_file_without_ext + ".options"
        options_file_abs = os.path.join(proto_file_path_abs, options_file)
        options_file_md5_abs = None
        options_file_current_md5 = None
        if pathlib.Path(options_file_abs).exists():
            options_file_md5_abs = os.path.join(md5_dir, options_file + '.md5')
            options_file_current_md5 = hashlib.md5(pathlib.Path(options_file_abs).read_bytes()).hexdigest()
        else:
            options_file = None

        header_file = proto_file_without_ext + ".pb.h"
        source_file = proto_file_without_ext + ".pb.c"

        header_file_abs = os.path.join(generated_src_dir, source_file)
        source_file_abs = os.path.join(generated_src_dir, header_file)

        need_generate = False

        # Check proto file md5
        try:
            last_md5 = pathlib.Path(proto_file_md5_abs).read_text()
            if last_md5 != proto_file_current_md5:
                need_generate = True
        except FileNotFoundError:
            need_generate = True

        if options_file:
            # Check options file md5
            try:
                last_md5 = pathlib.Path(options_file_md5_abs).read_text()
                if last_md5 != options_file_current_md5:
                    need_generate = True
            except FileNotFoundError:
                need_generate = True

        options_info = f"{options_file}" if options_file else "no options"

        if not need_generate:
            print(f"[nanopb] Skipping '{proto_file}' ({options_info})")
        else:
            print(f"[nanopb] Processing '{proto_file}' ({options_info})")
            cmd = [python_exe, nanopb_generator] + nanopb_options + [proto_file_basename]
            action = SCons.Action.CommandAction(cmd)
            result = env.Execute(action)
            if result != 0:
                print(f"[nanopb] ERROR: ({result}) processing cmd: '{cmd}'")
                exit(1)
            pathlib.Path(proto_file_md5_abs).write_text(proto_file_current_md5)
            if options_file:
                pathlib.Path(options_file_md5_abs).write_text(options_file_current_md5)

    #
    # Add generated includes and sources to build environment
    #
    env.Append(CPPPATH=[generated_src_dir])

    # Fix for ESP32 ESP-IDF https://github.com/nanopb/nanopb/issues/734#issuecomment-1001544447
    global_env = DefaultEnvironment()
    already_called_env_name = "_PROTOBUF_GENERATOR_ALREADY_CALLED_" + env['PIOENV'].replace("-", "_")
    if not global_env.get(already_called_env_name, False):
        env.BuildSources(generated_build_dir, generated_src_dir)
    global_env[already_called_env_name] = True
