#!/usr/bin/env python3
"""Build orchestrator for prebuilt tools.

Edit the PACKAGES list below to add/remove/update tools.
Run: python3 ext/build-all.py
Env: PREFIX (install dir), JOBS (parallel make jobs)
"""

import os
import sys
import platform
import subprocess
from pathlib import Path

PACKAGES = [
    {"name": "patchelf", "version": "0.18.0", "order": 0, "link": "static",
     "url": "https://github.com/NixOS/patchelf/releases/download/0.18.0/patchelf-0.18.0.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64"]},

    {"name": "ncurses", "version": "6.4", "order": 1, "link": "static",
     "url": "https://ftp.gnu.org/gnu/ncurses/ncurses-6.4.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "libevent", "version": "2.1.12", "order": 1, "link": "static",
     "url": "https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64"]},

    {"name": "readline", "version": "8.2", "order": 1, "link": "static",
     "url": "https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "zsh", "version": "5.9", "order": 2, "link": "static", "depends": ["ncurses"],
     "url": "https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "utf8proc", "version": "2.9.0", "order": 1, "link": "static",
     "url": "https://github.com/JuliaStrings/utf8proc/releases/download/v2.9.0/utf8proc-2.9.0.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "tmux", "version": "3.5a", "order": 2, "link": "static", "depends": ["ncurses", "libevent", "utf8proc"],
     "url": "https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "fmt", "version": "9.1.0", "order": 1, "link": "shared",
     "url": "https://github.com/fmtlib/fmt/releases/download/9.1.0/fmt-9.1.0.zip",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "git", "version": "2.47.0", "order": 2, "link": "static",
     "url": "https://github.com/git/git/archive/refs/tags/v2.47.0.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "htop", "version": "3.3.0", "order": 2, "link": "static", "depends": ["ncurses"],
     "url": "https://github.com/htop-dev/htop/releases/download/3.3.0/htop-3.3.0.tar.xz",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    {"name": "numactl", "version": "2.0.18", "order": 1, "link": "static",
     "url": "https://github.com/numactl/numactl/releases/download/v2.0.18/numactl-2.0.18.tar.gz",
     "platforms": ["linux-x86_64", "linux-arm64"]},

    {"name": "gitstatus", "version": "1.5.4", "order": 1, "link": "prebuilt",
     "url": "https://github.com/romkatv/gitstatus/releases/download/v1.5.4/gitstatusd",
     "platforms": ["linux-x86_64", "linux-arm64", "macos-arm64"]},

    # Add more packages here...
]


def detect_platform():
    system = platform.system().lower()
    machine = platform.machine().lower()
    if system == "linux":
        if machine in ("x86_64", "amd64"):
            return "linux-x86_64"
        elif machine in ("aarch64", "arm64"):
            return "linux-arm64"
    elif system == "darwin":
        if machine in ("arm64", "aarch64"):
            return "macos-arm64"
        elif machine == "x86_64":
            return "macos-x86_64"
    print(f"ERROR: Unsupported platform: {system}-{machine}", file=sys.stderr)
    sys.exit(1)


def snapshot(prefix):
    """Get set of all files under prefix (relative paths)."""
    files = set()
    prefix_path = Path(prefix)
    if prefix_path.exists():
        for f in prefix_path.rglob("*"):
            if f.is_file():
                files.add(str(f.relative_to(prefix_path)))
    return files


def build_package(pkg, ext_dir, prefix, jobs, current_platform, force=False):
    """Build a single package and record its manifest."""
    name = pkg["name"]
    script = ext_dir / f"build-{name}.sh"
    if not script.exists():
        print(f"WARNING: {script} not found, skipping {name}")
        return False

    manifest_file = prefix / "manifests" / f"{name}.txt"
    if not force and manifest_file.exists():
        print(f"  [{name}] already built (manifests/{name}.txt exists), skipping")
        return True

    print(f"\n{'='*60}")
    print(f"Building {name} {pkg['version']} (link={pkg.get('link', 'static')})")
    print(f"{'='*60}\n")

    before = snapshot(prefix)

    env = os.environ.copy()
    env["PREFIX"] = str(prefix)
    env["JOBS"] = str(jobs)
    env["URL"] = pkg["url"]
    env["PKG_VERSION"] = pkg["version"]
    env["PKG_LINK"] = pkg.get("link", "static")

    result = subprocess.run(["bash", str(script)], env=env, cwd=str(ext_dir))
    if result.returncode != 0:
        print(f"ERROR: build-{name}.sh failed with exit code {result.returncode}", file=sys.stderr)
        sys.exit(1)

    after = snapshot(prefix)
    new_files = sorted(after - before)

    manifest_dir = prefix / "manifests"
    manifest_dir.mkdir(parents=True, exist_ok=True)
    manifest_file = manifest_dir / f"{name}.txt"
    manifest_file.write_text("\n".join(new_files) + "\n")
    print(f"  Manifest: {len(new_files)} files recorded in manifests/{name}.txt")
    return True


def main():
    ext_dir = Path(__file__).resolve().parent
    prefix = Path(os.environ.get("PREFIX", str(ext_dir / "output"))).resolve()
    jobs = int(os.environ.get("JOBS", os.cpu_count() or 4))
    force = "--force" in sys.argv
    current_platform = detect_platform()

    print(f"Platform:  {current_platform}")
    print(f"PREFIX:    {prefix}")
    print(f"JOBS:      {jobs}")
    print(f"Force:     {force}")
    print(f"EXT_DIR:   {ext_dir}")

    prefix.mkdir(parents=True, exist_ok=True)

    packages = [p for p in PACKAGES if current_platform in p.get("platforms", [])]
    packages.sort(key=lambda p: p.get("order", 99))

    print(f"\nPackages to build ({len(packages)}):")
    for p in packages:
        print(f"  [{p.get('order', '?')}] {p['name']} {p['version']} ({p.get('link', 'static')})")

    for pkg in packages:
        build_package(pkg, ext_dir, prefix, jobs, current_platform, force=force)

    # Run fixup-rpath as final step
    fixup_script = ext_dir / "fixup-rpath.sh"
    if fixup_script.exists():
        print(f"\n{'='*60}")
        print("Running fixup-rpath.sh")
        print(f"{'='*60}\n")
        env = os.environ.copy()
        env["PREFIX"] = str(prefix)
        result = subprocess.run(["bash", str(fixup_script)], env=env, cwd=str(ext_dir))
        if result.returncode != 0:
            print("WARNING: fixup-rpath.sh returned non-zero exit code", file=sys.stderr)

    print(f"\nDone. Output in: {prefix}")


if __name__ == "__main__":
    main()
