#!/usr/bin/env bash

if [[ $# -eq 1 ]]; then
    TOP_DIR="$1"
    echo "Download to path $TOP_DIR"
else
    TOP_DIR="$(dirname $0)/neovim"
    echo "Download to default path $TOP_DIR"
fi

mkdir -p $TOP_DIR
# wget -O "$TOP_DIR/neovim-0.8.3.tar.gz" https://github.com/neovim/neovim/archive/refs/tags/v0.8.3.tar.gz

# These deps should go nvim-0.8.3/.deps/build/downloads/
URL_LIBUV="https://github.com/libuv/libuv/archive/v1.44.2.tar.gz"
PATH_LIBUV="libuv/v1.44.2.tar.gz"
SHA_LIBUV="e6e2ba8b4c349a4182a33370bb9be5e23c51b32efb9b9e209d0e8556b73a48da"

URL_MSGPACK="https://github.com/msgpack/msgpack-c/releases/download/c-4.0.0/msgpack-c-4.0.0.tar.gz"
PATH_MSGPACK="msgpack/msgpack-c-4.0.0.tar.gz"
SHA_MSGPACK="420fe35e7572f2a168d17e660ef981a589c9cbe77faa25eb34a520e1fcc032c8"

URL_LUAJIT="https://github.com/LuaJIT/LuaJIT/archive/633f265f67f322cbe2c5fd11d3e46d968ac220f7.tar.gz"
PATH_LUAJIT="luajit/633f265f67f322cbe2c5fd11d3e46d968ac220f7.tar.gz"
SHA_LUAJIT="2681f0a6f624a64a8dfb70a5a377d494daf38960442c547d9c468674c1afa3c2"

URL_LUA="https://www.lua.org/ftp/lua-5.1.5.tar.gz"
PATH_LUA="lua/lua-5.1.5.tar.gz"
SHA_LUA="2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333"

URL_LUAROCKS="https://github.com/luarocks/luarocks/archive/v3.8.0.tar.gz"
PATH_LUAROCKS="luarocks/v3.8.0.tar.gz"
SHA_LUAROCKS="ab6612ca9ab87c6984871d2712d05525775e8b50172701a0a1cabddf76de2be7"

URL_UNIBILIUM="https://github.com/neovim/unibilium/archive/92d929f.tar.gz"
PATH_UNIBILIUM="unibilium/92d929f.tar.gz"
SHA_UNIBILIUM="29815283c654277ef77a3adcc8840db79ddbb20a0f0b0c8f648bd8cd49a02e4b"

URL_LIBTERMKEY="https://www.leonerd.org.uk/code/libtermkey/libtermkey-0.22.tar.gz"
PATH_LIBTERMKEY="libtermkey/libtermkey-0.22.tar.gz"
SHA_LIBTERMKEY="6945bd3c4aaa83da83d80a045c5563da4edd7d0374c62c0d35aec09eb3014600"

URL_LIBVTERM="https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.1.tar.gz"
PATH_LIBVTERM="libvterm/libvterm-0.3.1.tar.gz"
SHA_LIBVTERM="25a8ad9c15485368dfd0a8a9dca1aec8fea5c27da3fa74ec518d5d3787f0c397"

URL_LUV="https://github.com/luvit/luv/archive/1.44.2-1.tar.gz"
PATH_LUV="luv/1.44.2-1.tar.gz"
SHA_LUV="f8c69908e17ec8ab370253d1508e23deaecfc0c4752d2efb77e427e579501104"

URL_LUA_COMPAT53="https://github.com/keplerproject/lua-compat-5.3/archive/v0.9.tar.gz"
PATH_LUA_COMPAT53="lua-compat-5.3/v0.9.tar.gz"
SHA_LUA_COMPAT53="ad05540d2d96a48725bb79a1def35cf6652a4e2ec26376e2617c8ce2baa6f416"

URL_LIBICONV="https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz"
PATH_LIBICONV="libiconv/libiconv-1.15.tar.gz"
SHA_LIBICONV="ccf536620a45458d26ba83887a983b96827001e92a13847b45e4925cc8913178"

URL_TREESITTER_C="https://github.com/tree-sitter/tree-sitter-c/archive/v0.20.2.tar.gz"
PATH_TREESITTER_C="treesitter-c/v0.20.2.tar.gz"
SHA_TREESITTER_C="af66fde03feb0df4faf03750102a0d265b007e5d957057b6b293c13116a70af2"

URL_TREESITTER_LUA="https://github.com/MunifTanjim/tree-sitter-lua/archive/v0.0.13.tar.gz"
PATH_TREESITTER_LUA="treesitter-lua/v0.0.13.tar.gz"
SHA_TREESITTER_LUA="564594fe0ffd2f2fb3578a15019b723e1bc94ac82cb6a0103a6b3b9ddcc6f315"

URL_TREESITTER_VIM="https://github.com/vigoux/tree-sitter-viml/archive/v0.2.0.tar.gz"
PATH_TREESITTER_VIM="treesitter-vim/v0.2.0.tar.gz"
SHA_TREESITTER_VIM="608dcc31a7948cb66ae7f45494620e2e9face1af75598205541f80d782ec4501"

URL_TREESITTER_HELP="https://github.com/neovim/tree-sitter-vimdoc/archive/v1.3.0.tar.gz"
PATH_TREESITTER_HELP="treesitter-help/v1.3.0.tar.gz"
SHA_TREESITTER_HELP="f33f6d49c7d71feb2fd68ef2b2684da150f9f8e486ad9726213631d673942331"

URL_TREESITTER="https://github.com/tree-sitter/tree-sitter/archive/v0.20.7.tar.gz"
PATH_TREESITTER="tree-sitter/v0.20.7.tar.gz"
SHA_TREESITTER="b355e968ec2d0241bbd96748e00a9038f83968f85d822ecb9940cbe4c42e182e"

deps=(LIBUV MSGPACK LUAJIT LUA LUAROCKS UNIBILIUM LIBTERMKEY LIBVTERM LUV LUA_COMPAT53 LIBICONV TREESITTER_C TREESITTER_LUA TREESITTER_VIM TREESITTER_HELP TREESITTER)

for dep in ${deps[@]}; do
    url=$(eval echo \$URL_$dep)
    path=$(eval echo \$PATH_$dep)
    sha=$(eval echo \$SHA_$dep)
    target="$TOP_DIR/deps/${path}"
    if [[ -f "$target" ]]; then
        osha=$(sha256sum "$target" | awk '{printf $1}')
        if [[ "$sha" == "$osha" ]]; then
            echo "Skipping for existence: $target"
            continue
        fi
    fi
    mkdir -p $(dirname "$target")
    cmd="wget -O $target $url"
    echo "$cmd"
    $cmd
done

# make CMAKE_BUILD_TYPE=RelWithDebInfo CMAKE_EXTRA_FLAGS="-DCMAKE_INSTALL_PREFIX=$(realpath ./install)"
# make install
