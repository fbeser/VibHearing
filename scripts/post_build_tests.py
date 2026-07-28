import os
import shutil
import subprocess
import sys

import click

Import("env")
from SCons.Script import Default


def print_test_output(output):
    use_color = os.environ.get("GITHUB_ACTIONS", "").lower() != "true"
    for raw_line in output.splitlines():
        line = click.unstyle(raw_line)
        if "[FAILED]" in line or "test cases:" in line and "failed" in line:
            click.secho(line, fg="red", bold=True, color=use_color)
        elif (
            "[PASSED]" in line
            or "test cases:" in line and "succeeded" in line
            or "SUMMARY" in line
        ):
            click.secho(line, fg="green", bold=True, color=use_color)
        elif line.startswith("Processing "):
            click.secho(line, fg="cyan", bold=True, color=use_color)
        else:
            click.echo(line, color=use_color)


def native_compiler_is_available():
    return shutil.which("gcc") is not None and shutil.which("g++") is not None


def handle_missing_native_compiler(env):
    use_color = os.environ.get("GITHUB_ACTIONS", "").lower() != "true"
    if not use_color:
        click.secho(
            "Native GCC/G++ toolchain is missing in CI; tests cannot run.",
            fg="red",
            bold=True,
            color=False,
        )
        env.Exit(1)
    click.secho(
        "WARNING: GCC/G++ was not found in PATH; native tests were skipped.",
        fg="yellow",
        bold=True,
        color=True,
    )
    click.secho(
        "Firmware build/upload will continue without local signal-test validation.",
        fg="yellow",
        color=True,
    )


def run_native_tests(env, trigger):
    if not native_compiler_is_available():
        handle_missing_native_compiler(env)
        return

    project_dir = env.subst("$PROJECT_DIR")
    print(f"Running native signal-processing tests {trigger}...")
    result = subprocess.run(
        [sys.executable, "-m", "platformio", "test", "-e", "native"],
        cwd=project_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    print_test_output(result.stdout)
    if result.returncode != 0:
        print("Native tests failed; the requested PlatformIO action was cancelled.")
        env.Exit(result.returncode)
    print("Native tests passed.")


def run_native_tests_after_build(source, target, env):
    run_native_tests(env, "after firmware build")


def run_native_tests_before_upload(source, target, env):
    run_native_tests(env, "before firmware upload")


post_build_test_target = env.Alias(
    "native_tests_after_build",
    "$BUILD_DIR/${PROGNAME}.bin",
    run_native_tests_after_build,
)
env.AlwaysBuild(post_build_test_target)
Default(post_build_test_target)
env.AddPreAction("upload", run_native_tests_before_upload)
