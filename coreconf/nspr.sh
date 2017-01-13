#!/usr/bin/env bash
# This script builds NSPR for NSS.
#
# This build system is still under development.  It does not yet support all
# the features or platforms that the regular NSPR build supports.

# variables
nspr_cflags=
nspr_cxxflags=
nspr_ldflags=

# Try to avoid bmake on OS X and BSD systems
if hash gmake 2>/dev/null; then
    make() { command gmake "$@"; }
fi

nspr_sanitizer()
{
    local extra=$(python $cwd/coreconf/sanitizers.py "$@")
    nspr_cflags="$nspr_cflags $extra"
    nspr_cxxflags="$nspr_cxxflags $extra"
    nspr_ldflags="$nspr_ldflags $extra"
}

nspr_build()
{
    local nspr_dir="$cwd"/../nspr/$target
    mkdir -p "$nspr_dir"

    # These NSPR options are directory-specific, so they don't need to be
    # included in nspr_opt and changing them doesn't force a rebuild of NSPR.
    extra_params=(--prefix="$dist_dir"/$target)
    if [ "$opt_build" = 1 ]; then
        extra_params+=(--disable-debug --enable-optimize)
    fi

    echo "NSPR [1/3] configure ..."
    pushd "$nspr_dir" >/dev/null
    CFLAGS="$nspr_cflags" CXXFLAGS="$nspr_cxxflags" \
          LDFLAGS="$nspr_ldflags" CC="$CC" CXX="$CCC" \
          run_verbose ../configure "${extra_params[@]}" "$@"
    popd >/dev/null
    echo "NSPR [2/3] make ..."
    run_verbose make -C "$nspr_dir"
    echo "NSPR [3/3] install ..."
    run_verbose make -C "$nspr_dir" install
}

nspr_clean()
{
    rm -rf "$cwd"/../nspr/$target
}