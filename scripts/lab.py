"""Build and verify the finished lab, using only Python's standard library."""

import argparse
import contextlib
import csv
import hashlib
import json
import os
import platform
import stat
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path, PurePosixPath

from eco_build import build, digest_tree, discover_toolchain

ROOT = Path(__file__).resolve().parent.parent
ARCHIVE_SHA256 = "2f23fffb9e81d4e5423153e2ff7236578ae5e947501a1e5c4168d3dc2b3c08b3"
SDK_SHA256 = "75d6d2171f845f356a2c9f445b963217fca0f8c4d741ffcea08f04ec9146464a"
VARIANTS = {"short": "short-1", "detailed": "detailed-1"}
COM_CASES = [
    "matrix", "ownership", "secondary_lifetime", "null_iid_output",
    "factory_null_output", "factory_null_iid", "factory_null_me", "factory_refcount",
    "aggregation", "alloc_null_system", "alloc_null_iid", "alloc_null_output",
    "alloc_null_me", "system_failure", "bus_failure", "memory_failure",
    "allocation_failure", "second_bus_query_failure", "second_memory_query_failure",
    "optional_extension_absent", "manager_identifier_lifetime",
]


def save_json(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def setup(archive):
    discover_toolchain()
    if hashlib.sha256(archive.read_bytes()).hexdigest() != ARCHIVE_SHA256:
        raise ValueError("Expected the original Eco.Framework_20260918.zip; archive SHA256 differs")
    home = ROOT / ".eco"
    sdk = home / "Eco.Framework"
    if sdk.exists():
        if digest_tree(sdk) != SDK_SHA256:
            raise ValueError("Existing .eco/Eco.Framework differs from the tested SDK")
        print("Eco SDK already prepared:", sdk)
        return
    home.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="import-", dir=home) as folder:
        staging = Path(folder)
        with zipfile.ZipFile(archive) as stream:
            for info in stream.infolist():
                path = PurePosixPath(info.filename)
                if path.is_absolute() or ".." in path.parts or "\\" in info.filename:
                    raise ValueError("Unsafe archive path")
                if stat.S_ISLNK(info.external_attr >> 16):
                    raise ValueError("SDK symlinks are unsupported")
                if (
                    not info.is_dir()
                    and path.parts[0] == "Eco.Framework"
                    and ("SharedFiles" in path.parts or "/BuildFiles/Mac/arm64/" in str(path))
                    and not path.name.startswith("._")
                ):
                    target = staging.joinpath(*path.parts)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(stream.read(info))
        incoming = staging / "Eco.Framework"
        files = {
            p.relative_to(incoming).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in incoming.rglob("*") if p.is_file()
        }
        save_json(incoming / "arpo-sdk.json", {
            "archive": "Eco.Framework_20260918.zip",
            "archive_sha256": ARCHIVE_SHA256,
            "architecture": "arm64",
            "files": files,
        })
        if digest_tree(incoming) != SDK_SHA256:
            raise ValueError("Extracted SDK differs from the tested SDK")
        incoming.rename(sdk)
    print("Eco SDK prepared:", sdk)


def run_com_cases(binary, output, stream):
    records = []
    checks = 0
    for case in COM_CASES:
        env = {**os.environ, "ARPO_COM_CASE": case}
        result = subprocess.run([binary], capture_output=True, text=True, timeout=30, env=env)
        log = result.stdout + result.stderr
        stream.write(log)
        (output / (case + ".log")).write_text(log, encoding="utf-8")
        summary = next((line for line in result.stdout.splitlines() if line.startswith("COM_SUMMARY,")), "")
        passed = result.returncode == 0 and ",failures=0," in summary
        if passed:
            checks += int(summary.split(",checks=", 1)[1].split(",", 1)[0])
        records.append({"case": case, "passed": passed, "returncode": result.returncode,
                        "summary": summary})
    save_json(output / "checks.json", {"cases": records, "checks": checks,
                                      "passed": all(row["passed"] for row in records)})
    if not all(row["passed"] for row in records):
        raise RuntimeError("COM contract checks failed; see checks.json")
    return subprocess.CompletedProcess([binary], 0,
        "COM contract: " + str(len(records)) + " scenarios, " + str(checks) + " checks passed\n", "")


def execute(command, variant, framework):
    project = ROOT / "solutions" / VARIANTS[variant]
    output = project / "BuildFiles" / "Standalone" / command
    output.mkdir(parents=True, exist_ok=True)
    log = output / "output.log"
    client = None if command == "demo" else ROOT / (
        "checks/com-contract.c" if command == "com-test" else "checks/reference-public.c")
    try:
        with log.open("w", encoding="utf-8") as stream:
            with contextlib.redirect_stdout(stream), contextlib.redirect_stderr(stream):
                record = build(
                    project, framework, client=client, sanitize=command in {"test", "com-test"},
                    output=str(output.relative_to(project)), sdk_sha256=SDK_SHA256,
                )
                env = dict(os.environ)
                env.pop("ARPO_BITONIC_BENCH", None)
                if command == "benchmark":
                    env["ARPO_BITONIC_BENCH"] = "1"
                if command == "com-test":
                    result = run_com_cases(record["client"], output, stream)
                else:
                    result = subprocess.run(
                        [record["client"]], capture_output=True, text=True,
                        timeout=600 if command == "benchmark" else 180, env=env,
                    )
                stream.write(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError("Client failed, exit code " + str(result.returncode))
        if command == "test" and "SUMMARY,cases=1664,failures=0" not in result.stdout:
            raise RuntimeError("Expected 1664 passing independent checks")
        if command == "benchmark":
            rows = [line.split(",")[1:] for line in result.stdout.splitlines() if line.startswith("BENCH,")]
            if len(rows) != 336 or any(row[-1] != "1" for row in rows) or "BENCH_FAIL" in result.stdout:
                raise RuntimeError("Benchmark results are incomplete or differ from qsort")
            with (output / "timings.csv").open("w", newline="", encoding="utf-8") as stream:
                writer = csv.writer(stream)
                writer.writerow(["n", "pattern", "method", "repeat", "wall_s", "exact"])
                writer.writerows(rows)
        print(variant + ": " + command + " passed; log: " + str(log))
        if command == "demo":
            print(result.stdout, end="")
        elif command == "test":
            print("SUMMARY,cases=1664,failures=0 (UBSan)")
        elif command == "com-test":
            print(result.stdout, end="")
    except Exception:
        print("Diagnostic log:", log, file=sys.stderr)
        raise


def main():
    parser = argparse.ArgumentParser(description="Standalone Eco Lab 64: setup, demo, tests, benchmark")
    parser.add_argument("command", choices=["setup", "demo", "test", "com-test", "benchmark"])
    parser.add_argument("--variant", choices=["short", "detailed", "all"], default="all")
    parser.add_argument("--framework", type=Path, default=ROOT / ".eco/Eco.Framework")
    parser.add_argument("--archive", type=Path,
                        default=Path.home() / "Downloads/Design operating systems/Eco.Framework_20260918.zip")
    args = parser.parse_args()
    try:
        if sys.platform != "darwin" or platform.machine() != "arm64":
            raise ValueError("This verified build profile requires macOS on Apple Silicon")
        if args.command == "setup":
            setup(args.archive.expanduser().resolve())
        else:
            sdk = args.framework.expanduser().resolve()
            if not sdk.is_dir():
                raise ValueError("Run python3 scripts/lab.py setup first, or specify --framework")
            for variant in VARIANTS if args.variant == "all" else [args.variant]:
                execute(args.command, variant, sdk)
                if args.command == "test":
                    execute("com-test", variant, sdk)
        return 0
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as exc:
        print("Lab error:", exc, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
