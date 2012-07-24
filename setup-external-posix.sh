#!/bin/sh

#User configuration
MADGRAPH_REMOTE_TARBALL="https://launchpad.net/madgraph5/trunk/1.4.0/+download/MadGraph5_v1.4.7.tar.gz"
LLVM_REMOTE_TARBALL="http://llvm.org/releases/3.1/llvm-3.1.src.tar.gz"
CLANG_REMOTE_TARBALL="http://llvm.org/releases/3.1/clang-3.1.src.tar.gz"
COMPILER_RT_REMOTE_TARBALL="http://llvm.org/releases/3.1/compiler-rt-3.1.src.tar.gz"

#----------- DO NOT MODIFY BELOW THIS LINE -----------#

#Fixed configuration
SOURCE_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
EXTERNALS_PATH="$SOURCE_PATH/externals"
EXTERNALS_SOURCE_PATH="$EXTERNALS_PATH/src"
EXTERNALS_INSTALL_MARKER_PATH="$EXTERNALS_PATH/installed.txt"
MADGRAPH_LOCAL_TARBALL="$EXTERNALS_SOURCE_PATH/MadGraph.tar.gz"
LLVM_LOCAL_TARBALL="$EXTERNALS_SOURCE_PATH/llvm.tar.gz"
CLANG_LOCAL_TARBALL="$EXTERNALS_SOURCE_PATH/clang.tar.gz"
COMPILER_RT_LOCAL_TARBALL="$EXTERNALS_SOURCE_PATH/compiler-rt.tar.gz"
MADGRAPH_SOURCE_PATH="$EXTERNALS_SOURCE_PATH/MadGraph"
LLVM_SOURCE_PATH="$EXTERNALS_SOURCE_PATH/llvm"
CLANG_SOURCE_PATH="$LLVM_SOURCE_PATH/tools/clang"
COMPILER_RT_SOURCE_PATH="$LLVM_SOURCE_PATH/projects/compiler-rt"
LLVM_BUILD_PATH="$LLVM_SOURCE_PATH/build"

#Only install the software if it hasn't been already
if [ ! -f "$EXTERNALS_INSTALL_MARKER_PATH" ]; then

    echo "Installing external software..."

    #Find a download command
    #Sets GET_COMMAND to a command that can be used as:
    #   $GET_COMMAND destination URL
    POTENTIAL_GET_COMMANDS=("curl --location --output" "wget --output-document")
    for POTENTIAL_GET_COMMAND in "${POTENTIAL_GET_COMMANDS[@]}"; do
        if command -v $POTENTIAL_GET_COMMAND > /dev/null 2>&1; then
            GET_COMMAND=$POTENTIAL_GET_COMMAND
            break
        fi
    done
    if [ -z "$GET_COMMAND" ]; then
        echo >&2 "Unable to find any viable HTTP download commands.  Aborting."
        exit 1
    else
        echo "Using \"$GET_COMMAND\" to download files."
    fi

    #A few useful functions
    function download_file_if_necessary()
    {
        #Usage: download_file_if_necessary URL OUTPUT_PATH
        if [ ! -f "$2" ]; then
            $GET_COMMAND "$2" "$1"
        fi
    }

    function extract_tarball_if_necessary()
    {
        #Usage extract_tarball_if_necessary TARBALL_PATH OUTPUT_PATH
        #Note that this will strip the first directory component of
        #the tarball, so that its main contents go in the specified
        #output path.  No files are overwritten.
        mkdir -p "$2"
        tar -C "$2" --strip-components 1 -xkzf "$1"
    }

    #Create the externals directory (if it doesn't exist)
    mkdir -p "$EXTERNALS_PATH"
    mkdir -p "$EXTERNALS_SOURCE_PATH"

    #Download tarballs
    download_file_if_necessary "$MADGRAPH_REMOTE_TARBALL" "$MADGRAPH_LOCAL_TARBALL"
    download_file_if_necessary "$LLVM_REMOTE_TARBALL" "$LLVM_LOCAL_TARBALL"
    download_file_if_necessary "$CLANG_REMOTE_TARBALL" "$CLANG_LOCAL_TARBALL"
    download_file_if_necessary "$COMPILER_RT_REMOTE_TARBALL" "$COMPILER_RT_LOCAL_TARBALL"

    #Set up MadGraph
    extract_tarball_if_necessary "$MADGRAPH_LOCAL_TARBALL" "$MADGRAPH_SOURCE_PATH"

    #Extract LLVM/Clang
    extract_tarball_if_necessary "$LLVM_LOCAL_TARBALL" "$LLVM_SOURCE_PATH"
    extract_tarball_if_necessary "$CLANG_LOCAL_TARBALL" "$CLANG_SOURCE_PATH"
    extract_tarball_if_necessary "$COMPILER_RT_LOCAL_TARBALL" "$COMPILER_RT_SOURCE_PATH"

    #Build LLVM/Clang
    mkdir -p "$LLVM_BUILD_PATH"
    pushd "$LLVM_BUILD_PATH"
    "$LLVM_SOURCE_PATH"/configure --prefix="$EXTERNALS_PATH" --enable-optimized --disable-assertions
    make
    make install
    popd

    #Mark the externals as installed
    touch "$EXTERNALS_INSTALL_MARKER_PATH"

else

    echo "External software already installed, just setting up environment."

fi #End of install conditional

#Set up the environment
echo "Setting up environment..."
export PATH="$EXTERNALS_PATH/bin:$PATH"
export LD_LIBRARY_PATH="$(llvm-config --libdir):$LD_LIBRARY_PATH"
export PYTHONPATH="$CLANG_SOURCE_PATH/bindings/python:$PYTHONPATH"
