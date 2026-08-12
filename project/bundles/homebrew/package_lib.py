#!/usr/bin/env python3
from __future__ import print_function
import sys, os, traceback, shutil, subprocess, re, pickle, glob
from pathlib import Path
from ast import literal_eval

"""Copy dylibs into the build folder.

Copyright (c)2024 Michael Miller michael underscore miller at msn dot com

Based heavily on copy_dylibs.py by Andy Duplain trojanfoe at gmail dot com

"""

# Any dependencies outside of these directories need copying and fixing
good_dirs = ["/System/", "/usr/lib/"]

# preferred directories to find libraries, in order
preferred_dir_prefix = ["lib/", "share/", "opt/", "bin/", "Cellar/"]
preferred_dirs = []

# Turn on for more output
debug = True

homebrew_prefix = None

custom_lib_dir = None

lib_dir = "@executable_path/../"

frameworks_dir = None

bundle_dir = None

# Module and library install_name changes:
# {
#    dylib_path : [ old_name, new_name ], ..., [old_name, new_name]
#    ...
# }
install_names = set()
install_names_cache = set()

# List of dylibs copied
processed_dylibs = set()
found_dylibs = dict()
copied_dylibs = set()

sign_binaries = False
copy_binaries = False
follow_symlinks = False
preserve_rpath = False
force = False
force_depth = 1
current_depth = 0

def echo(message):
    print(message)
    sys.stdout.flush()


def echon(message):
    sys.stdout.write(message)
    sys.stdout.flush()

def normalize_path(path):
    new_path = os.path.abspath(path)
    if not new_path.endswith("/"):
        new_path = new_path + "/"
    return new_path

def is_file_good(file):
    for dir in good_dirs:
        if file.startswith(dir):
            return True
    return False

def add_rpath(dylib):
    global bundle_dir
    global install_names
    global install_names_cache
    global sign_binaries

    if not sign_binaries or (install_names_cache and dylib in install_names_cache):
        return
    
    install_names.add(dylib)
    
    

    # change id based on the new rpath of the binary
    rpath = dylib[len(bundle_dir):]
    echo("Changing ID :@rpath/" + rpath)
    cmdline = ["install_name_tool", "-id", "@rpath/" + rpath, dylib]
    if debug:
        echo("Running: " + " ".join(cmdline))
    exitcode = subprocess.call(cmdline)

    # depth = rpath.count("/")
    # rpath = "@loader_path"
    # for i in range(depth):
    #     rpath = rpath + "/.."
    
    echo("Adding @rpath: @executable_path/../")
    cmdline = ["install_name_tool", "-add_rpath", "@executable_path/../", dylib]
    if debug:
        echo("Running: " + " ".join(cmdline))
    exitcode = subprocess.call(cmdline)
    
    # path to ./lib relative to the executable
    echo("Adding @rpath: @executable_path/../lib")
    cmdline = ["install_name_tool", "-add_rpath", "@executable_path/../lib", dylib]
    if debug:
        echo("Running: " + " ".join(cmdline))
    exitcode = subprocess.call(cmdline)

    # # path to ./lib relative to the executable
    # dylib_dir = dylib[len(bundle_dir):]
    # if not dylib_dir.startswith("/"):
    #     dylib_dir = "/" + dylib_dir

    # rpath = "@loader_path/"

    # depth = dylib_dir.count("/")
    # for i in range(dylib_dir.count("/")-1):
    #     echo("Adding @rpath: " + rpath)
    #     cmdline = ["install_name_tool", "-add_rpath", rpath, dylib]
    #     if debug:
    #         echo("Running: " + " ".join(cmdline))
    #     exitcode = subprocess.call(cmdline)
    #     cmdline = ["install_name_tool", "-add_rpath", rpath + "lib", dylib]
    #     if debug:
    #         echo("Running: " + " ".join(cmdline))
    #     exitcode = subprocess.call(cmdline)
    #     cmdline = ["install_name_tool", "-add_rpath", rpath + "opt", dylib]
    #     if debug:
    #         echo("Running: " + " ".join(cmdline))
    #     exitcode = subprocess.call(cmdline)
    #     rpath = rpath + "../"
    
def expand_symlink_path(path):
    # subdirs = path.split("/")
    # for subdir in subdirs:
    #     if 
    return

def expand_symlinks(dylib):
    return


def change_install_names(old_name, new_name, dylib):
    global install_names
    global sign_binaries
    global preserve_rpath
    # check if anything needs to change
    if str(old_name).lower().startswith("@loader_path") or not sign_binaries or (install_names_cache and dylib in install_names_cache):
        return
    
    if str(old_name).startswith("@"):
        if preserve_rpath:
            return
        # check the old rpath matches the new path or it doesn't have any path
        if str(old_name).startswith("@rpath"):
            prefix = "@rpath/"
        else:
            if str(old_name).startswith("@@executable_path"):
                prefix = "@executable_path/"
        
        file = old_name[len(prefix):]
        rel_path = new_name[len(bundle_dir):]

        # nothing to do, return
        if file == rel_path:
            return
    
    # dylib_dir = dylib[len(bundle_dir):]
    # if not dylib_dir.startswith("/"):
    #     dylib_dir = "/" + dylib_dir

    new_lib_name = new_name[len(bundle_dir):]
    if not new_lib_name.startswith("/"):
        new_lib_name = "/" + new_lib_name

    # depth = dylib_dir.count("/")
    # for i in range(dylib_dir.count("/")-1):
    #     new_lib_name = "/.." + new_lib_name
    new_lib_name = "@rpath" + new_lib_name
    
    install_names.add(os.path.realpath(dylib))

    # new_lib_name = simplify_path(new_lib_name, dylib)

    cmdline = ["install_name_tool", "-change", old_name, new_lib_name, dylib]
    if debug:
        echo("Running: " + " ".join(cmdline))
    exitcode = subprocess.call(cmdline)
    if exitcode != 0:
        raise RuntimeError(
            "Failed to change '{0}' to '{1}' in '{2}".format(
                old_name, new_lib_name, dylib
            )
        )
            
def find_file(file, parent):
    global found_dylibs
    global bundle_dir
    global follow_symlinks
    global preferred_dirs

    dylib = found_dylibs.get(file)
    if dylib:
        return dylib

    # # return the same file if it exists, but avoid things in the Cellar if we can
    # if file.find("Cellar") == -1 and os.path.exists(file) and (copy_binaries or file.startswith(bundle_dir)):
    #     # check if file lives in a valid lib path
    #     for path in preferred_dirs:
    #         if file.startswith(path):
    #             found_dylibs[file] = file
    #             return file
    
    # if file.startswith("@loader_path"):
    #     file = file.replace("@loader_path", os.path.dirname(parent))
    #     if os.path.exists(file):
    #         found_dylibs[os.path.basename(file)] = file
    #         return file

    echo("Finding: " + os.path.basename(file))

    # Shortcut for bundle path to avoid duplicates
    found_files = glob.glob(os.path.join(bundle_dir + "/**/", os.path.basename(file)), recursive=True)
    for found_file in found_files:
        dylib_path = os.path.dirname(found_file)
        if not "dSYM" in dylib_path and not "include" in dylib_path and not "bindings" in dylib_path and not "Headers" in dylib_path and os.path.isfile(found_file):
    #    if not "dSYM" in dylib_path and not "include" in dylib_path and not "bindings" in dylib_path and os.path.isfile(found_file):
            found_dylibs[file] = found_file
            return found_file
        
    if os.path.exists(file):
        found_dylibs[file] = file
        return file

    new_file = os.path.basename(file)

    parent_path = os.path.dirname(parent)
    if os.path.exists(os.path.join(parent_path, new_file)) and os.path.isfile(os.path.join(parent_path, new_file)):
        echo("Found in parent: " + os.path.basename(file))
        found_dylibs[file] = os.path.join(parent_path, new_file)
        return os.path.join(parent_path, new_file)

    tmp_preferred_dirs = []
    if copy_binaries:
        tmp_preferred_dirs = preferred_dirs
    else:
        tmp_preferred_dirs.append(bundle_dir)

    for pref_dir in tmp_preferred_dirs:
        search_path = pref_dir + "/**/" + new_file
        echo("Searching: " + search_path)
        found_files = glob.glob(search_path, recursive=True)
        for found_file in found_files:
            dylib_path = os.path.dirname(found_file)
            if not "dSYM" in dylib_path and not "include" in dylib_path and not "bindings" in dylib_path and not "Headers" in dylib_path and os.path.isfile(found_file):
                found_dylibs[file] = found_file
                return found_file
    
    raise RuntimeError("Unable to find file: '{0}'\nDo you need --copy=true?".format(file))

def copy_dylib(src):
    global copied_dylibs
    global copy_binaries

    #(dylib_path, dylib_filename) = os.path.split(src)

    dest = None  

    src = src.replace("//", "/")

    # shortcut for Hoembrew libs
    if homebrew_prefix and src.startswith(homebrew_prefix):        
        dest = src.replace(homebrew_prefix, bundle_dir)
    else:  
        # figure out where we found the file
        if src.startswith(bundle_dir):
            dest = src
        else:
            for path in preferred_dirs:
                if src.startswith(path):
                    dest = src.replace(path, bundle_dir)
                    break

    # if dest.find("Cellar") != -1:
    #     dest = dest.replace("Cellar", "share")
    echo("Source file: " + src)
    #if dest:
    dest = os.path.abspath(dest)

    if not dest in copied_dylibs and not os.path.exists(dest) and copy_binaries:
        if debug:
            echo("Copying {0} into bundle".format(src))
        dest_dir = os.path.dirname(dest)
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        attrs = os.stat(src)
        shutil.copyfile(src, dest)
        os.chmod(dest, attrs.st_mode)

    
    if dest in copied_dylibs:
        ret = True
    else:
        ret = False
        copied_dylibs.add(dest)

    return dest, ret
    
def copy_dependencies(file, dep_file):
    global processed_dylibs
    global force
    global force_depth
    global current_depth

    if not file in processed_dylibs and current_depth < force_depth:
        if (force):
            current_depth += 1
            echo("Current depth: " + str(current_depth))
        # process the file once if it's a symlinked file
        if file.startswith("@"):
            processed_dylibs.add(file)
        else:
            processed_dylibs.add(os.path.realpath(file))

        dep_lines = []

        file_filename = os.path.basename(file)
        echo("Examining {0}".format(file_filename))
        #found = 0
        pipe = subprocess.Popen(["otool", "-L", file], stdout=subprocess.PIPE)
        while True:
            line = pipe.stdout.readline().decode("utf-8")
            if line != "":
                dep_lines.append(line)
            else:
                break

        for line in dep_lines:
            m = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", line)
            if m:
                dep = m.group(1)
                # process only good file, and avoid reprocessing current file
                if not is_file_good(dep) and os.path.split(file)[1] != os.path.split(dep)[1]:
                    #(dep_path, dep_filename) = os.path.split(dep)
                    # echo(dep)
                    found_dep = find_file(dep, dep_file)
                    if found_dep :
                        (bundled_dep, processed) = copy_dylib(found_dep)
                        if bundled_dep:
                            change_install_names(dep, bundled_dep, file)
                        if not processed:
                            # use the dep moved to the bundle. Don't update the original
                            copy_dependencies(bundled_dep, found_dep)



def codesign():
    global sign_binaries
    if not sign_binaries:
        return
    
    for dylib in install_names:
        if not install_names_cache or not dylib in install_names_cache:
            add_rpath(dylib)                    

            echo("Codesigning {0}".format(os.path.basename(dylib)))
            cmdline = [
                "/usr/bin/codesign",
                "--force",
                "-s",
                "-",
                dylib,
            ]
            if debug:
                echo("Running: " + " ".join(cmdline))
            exitcode = subprocess.call(cmdline)
            if exitcode != 0:
                raise RuntimeError("Failed to codesign '{0}'".format(dylib))
            
def main(args):
    global frameworks_dir
    global bundle_dir
    global homebrew_prefix
    global preferred_dirs
    global install_names
    global install_names_cache
    global processed_dylibs
    global sign_binaries
    global copy_binaries
    global found_dylibs
    global follow_symlinks
    global preserve_rpath
    global force
    global force_depth

    processed_dylib_cache_mode = "none"
    found_dylib_cache_mode = "none"
    signed_dylib_cache_mode = "none"


    # process command line parameters
    for arg in args:
        param = str(arg)
        if param.find("=") == -1:
            continue
        (arg_name, arg_value) = param.split("=")
        arg_value = arg_value.replace("\"", "")
        match arg_name.lower():
            case "--file":
                root_full_path = os.path.abspath(arg_value)
            case "--bundle-root":
                bundle_dir = normalize_path(arg_value)
                preferred_dirs.append(bundle_dir)
            case "--homebrew":
                homebrew_prefix = normalize_path(arg_value)        
                for pref_dir in preferred_dir_prefix:
                    preferred_dirs.append(normalize_path(os.path.join(homebrew_prefix, pref_dir)))
            case "--other-libs":
                preferred_dirs.append(normalize_path(arg_value))
            case "--processed-cache":
                processed_dylib_cache_mode = arg_value
            case "--found-cache":
                found_dylib_cache_mode = arg_value
            case "--signed-cache":
                signed_dylib_cache_mode = arg_value
            case "--update-binary":
                sign_binaries = (arg_value.lower() == "true" or arg_value == "1")
            case "--copy":
                copy_binaries = (arg_value.lower() == "true" or arg_value == "1")
            case "--preserve_rpath":
                preserve_rpath = (arg_value.lower() == "true" or arg_value == "1")
            case "--force":
                force = (arg_value.lower() == "true" or arg_value == "1")
            case _:
                echo("Invalid parameter: " + arg_name)
                quit()

    # check we have the minimum params
    if not root_full_path or not bundle_dir:
        # we need a file to process and the bundle path at a minimum
        echo( "Please supply --file and --bundle-path")

    # get the path to the script to store the cache files
    script_path = os.path.dirname(os.path.realpath(__file__))

    # load the caches
    processed_dylibs_file = script_path + "/processed_dylibs_cache.pkl"
    if (processed_dylib_cache_mode == "update" or processed_dylib_cache_mode == "use") and os.path.exists(processed_dylibs_file):
        with open(processed_dylibs_file, 'rb') as f:
            processed_dylibs = pickle.load(f)

    found_dylibs_file = script_path + "/found_dylibs_cache.pkl"
    if (found_dylib_cache_mode == "update" or found_dylib_cache_mode == "use") and os.path.exists(found_dylibs_file):
        with open(found_dylibs_file, 'rb') as f:
            found_dylibs = pickle.load(f)

    signed_dylibs_file = script_path + "/signed_dylibs_cache.pkl"
    if (signed_dylib_cache_mode == "update" or signed_dylib_cache_mode == "use") and os.path.exists(signed_dylibs_file):
        with open(signed_dylibs_file, 'rb') as f:
             install_names_cache = pickle.load(f)
    

    # force remove the file from the caches to reprocess it but preserving the rest of the cache
    if (force):
        for file in glob.glob(root_full_path):
            try:
                processed_dylibs.remove(file)
            except:
                print(file + " not in processed cache")
            try:
                found_dylibs.remove(file)
            except:
                print(file + " not in found cache")
            try:
                install_names_cache.remove(file)
            except:
                print(file + " not in signed cache")
    
    # main loop to process files
    for file in glob.glob(root_full_path):
        print(file)
        copy_dependencies(file, file)

    # codesign the binaries
    codesign()

    # persist working data for the caches
    if processed_dylib_cache_mode == "new" or processed_dylib_cache_mode == "update":
        with open(processed_dylibs_file, 'wb') as f:
            pickle.dump(processed_dylibs, f)

    if found_dylib_cache_mode == "new" or found_dylib_cache_mode == "update":
        with open(found_dylibs_file, 'wb') as f:
            pickle.dump(found_dylibs, f)

    if signed_dylib_cache_mode == "new" or signed_dylib_cache_mode == "update":
        install_names_cache = install_names_cache.union(install_names)
        with open(signed_dylibs_file, 'wb') as f:
            pickle.dump(install_names_cache, f)

if __name__ == "__main__":
    exitcode = 99
    try:
        exitcode = main(sys.argv)
    except Exception as e:
        echo(traceback.format_exc())
    sys.exit(exitcode)