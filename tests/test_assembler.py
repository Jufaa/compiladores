"""End-to-end assembly checks. Run after make: python3 tests/test_assembler.py."""

import pathlib
import subprocess
import sys
import tempfile


CASES = {
    "arithmetic": ("int main() { return (1000 + 20) / 10 + 7 % 4 - 5 * 2; }", 95),
    "negative": ("int main() { return -17 / 5 + 17 % 5 + 10; }", 9),
    "shadowing": (
        "int x; int main() { int y; x = 3; y = 4; "
        "{ int x; x = 8; y = y + x; } return x + y; }", 15
    ),
    "loop": (
        "int main() { int x, sum; x = 4; sum = 0; "
        "while (x > 0) { sum = sum + x; x = x - 1; } return sum; }", 10
    ),
    "boolean": (
        "boolean same(int x) { return x == 4; } "
        "int main() { if (same(4) && !(3 > 8)) { return 7; } else { return 9; } }", 7
    ),
    "recursion": (
        "int fact(int n) { if (n <= 1) { return 1; } "
        "else { return n * fact(n - 1); } } int main() { return fact(5); }", 120
    ),
    "stack_arguments": (
        "int sum(int a, int b, int c, int d, int e, int f, int g, int h) "
        "{ return a+b+c+d+e+f+g+h; } "
        "int main() { return sum(1,2,3,4,5,6,7,8); }", 36
    ),
    "nested_calls": (
        "int add(int a, int b) { return a+b; } "
        "int main() { return add(add(1,2),add(3,4)); }", 10
    ),
    "void_call": ("void work() { return; } int main() { work(); return 6; }", 6),
    "large_literal": ("int main() { return 1000000 - 999991; }", 9),
    "comparisons": (
        "int main() { if ((2 < 3) && (3 >= 3) && (2 != 4) && (2 <= 2) || false) "
        "{ return 11; } else { return 12; } }", 11
    ),
}


def main():
    compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "./compilador").resolve()
    with tempfile.TemporaryDirectory(prefix="assembler-tests-") as directory:
        scratch = pathlib.Path(directory)
        for name, (source, expected) in CASES.items():
            source_path = scratch / f"{name}.txt"
            assembly = scratch / f"{name}.s"
            executable = scratch / name
            source_path.write_text(source)
            result = subprocess.run(
                [str(compiler), str(source_path), "--asm", str(assembly)],
                capture_output=True, text=True,
            )
            assert result.returncode == 0, (name, result.stdout, result.stderr)
            subprocess.run(["gcc", str(assembly), "-o", str(executable)], check=True)
            result = subprocess.run([str(executable)], capture_output=True, timeout=5)
            assert result.returncode == expected, (name, result.returncode, expected)
            assert result.stdout == b"" and result.stderr == b"", name
            print(f"PASS {name}")

        # Removing backend semantic checks must not bypass the frontend.
        source_path = scratch / "invalid.txt"
        assembly = scratch / "invalid.s"
        source_path.write_text("int main() { return true; }")
        result = subprocess.run(
            [str(compiler), str(source_path), "--asm", str(assembly)],
            capture_output=True, text=True,
        )
        assert result.returncode != 0 and not assembly.exists(), result
        assert "retorno incompatible" in result.stdout, result.stdout
        print("PASS frontend_still_validates")

        for name, source in {
            "float_literal": "int main() { float x; x = 1.5; return 0; }",
            "float_signature": "float value() { return 1.5; } int main() { return 0; }",
        }.items():
            source_path.write_text(source)
            result = subprocess.run(
                [str(compiler), str(source_path), "--asm", str(assembly)],
                capture_output=True, text=True,
            )
            assert result.returncode != 0 and not assembly.exists(), (name, result)
            assert "el backend no soporta flotantes" in result.stderr, result.stderr
            print(f"PASS {name}")

        source_path.write_text("int main() { return 0; }")
        result = subprocess.run(
            [str(compiler), str(source_path), "--asm", str(scratch / "missing" / "out.s")],
            capture_output=True, text=True,
        )
        assert result.returncode != 0, result
        assert "no se pudo abrir" in result.stderr, result.stderr
        print("PASS output_error")


if __name__ == "__main__":
    main()
